// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  Heads-up displays
 */

// killough 5/3/98: remove unnecessary headers

#include <utility>

#include "doomstat.hpp"
#include "hu_stuff.hpp"
#include "hu_lib.hpp"
#include "st_stuff.hpp" /* jff 2/16/98 need loc of status bar */
#include "s_sound.hpp"
#include "dstrings.hpp"
#include "sounds.hpp"
#include "d_deh.hpp"   /* Ty 03/27/98 - externalization of mapnamesx arrays */
#include "g_game.hpp"
#include "r_main.hpp"
#include "p_inter.hpp"
#include "p_tick.hpp"
#include "p_map.hpp"
#include "sc_man.hpp"
#include "m_menu.hpp"
#include "m_misc.hpp"
#include "r_main.hpp"
#include "lprintf.hpp"
#include "p_setup.hpp"
#include "w_wad.hpp"
#include "e6y.hpp" //e6y
#include "g_overflow.hpp"

#include "dsda.hpp"
#include "dsda/exhud.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"
#include "dsda/pause.hpp"
#include "dsda/settings.hpp"
#include "dsda/stretch.hpp"

static player_t* plr;

typedef struct custom_message_s
{
	int ticks;
	SfxId sfx;
	const char* msg;
} custom_message_t;

static custom_message_t custom_message[MAX_MAXPLAYERS];
static custom_message_t* custom_message_p;

//jff 2/16/98 status color change levels
int hud_ammo_red;      // ammo percent less than which status is red
int hud_ammo_yellow;   // ammo percent less is yellow more green
int hud_health_red;    // health amount less than which status is red
int hud_health_yellow; // health amount less than which status is yellow
int hud_health_green;  // health amount above is blue, below is green

extern "C" void HU_InitThresholds()
{
	hud_health_red = dsda_IntConfig(ConfigId::HudHealthRed);
	hud_health_yellow = dsda_IntConfig(ConfigId::HudHealthYellow);
	hud_health_green = dsda_IntConfig(ConfigId::HudHealthGreen);
	hud_ammo_red = dsda_IntConfig(ConfigId::HudAmmoRed);
	hud_ammo_yellow = dsda_IntConfig(ConfigId::HudAmmoYellow);
}

dsda_string_t hud_title;

static void HU_FetchTitle()
{
	if(hud_title.string)
		dsda_FreeString(&hud_title);

	dsda_HUTitle(&hud_title);
}

static void HU_InitMessages()
{
	custom_message_p = &custom_message[displayplayer];
	custom_message_p->ticks = 0;
}

static void HU_InitPlayer()
{
	// killough 3/7/98
	plr = &players[displayplayer];
}

typedef struct crosshair_s
{
	int lump;
	int w, h;
	PatchTranslation flags;
	int target_x, target_y, target_z, target_sprite;
	float target_screen_x, target_screen_y;
} crosshair_t;

static crosshair_t crosshair;

static const char* crosshair_nam[HU_CROSSHAIRS] =
	{nullptr, "CROSS1", "CROSS2", "CROSS3", "CROSS4", "CROSS5", "CROSS6", "CROSS7"};

static int hudadd_crosshair;
static int hudadd_crosshair_scale;
static int hudadd_crosshair_health;
static int hudadd_crosshair_target;
static int hudadd_crosshair_lock_target;

extern "C" void HU_InitCrosshair()
{
	hudadd_crosshair_scale = dsda_IntConfig(ConfigId::HudaddCrosshairScale);
	hudadd_crosshair_health = dsda_IntConfig(ConfigId::HudaddCrosshairHealth);
	hudadd_crosshair_target = dsda_IntConfig(ConfigId::HudaddCrosshairTarget);
	hudadd_crosshair_lock_target = dsda_IntConfig(ConfigId::HudaddCrosshairLockTarget);
	hudadd_crosshair = dsda_IntConfig(ConfigId::HudaddCrosshair);

	if(!hudadd_crosshair || !crosshair_nam[hudadd_crosshair])
		return;

	crosshair.lump = W_CheckNumForNameInternal(crosshair_nam[hudadd_crosshair]);
	if(crosshair.lump == LUMP_NOT_FOUND)
		return;

	crosshair.w = R_NumPatchWidth(crosshair.lump);
	crosshair.h = R_NumPatchHeight(crosshair.lump);

	crosshair.flags = PatchTranslation::Trans;
	if(hudadd_crosshair_scale)
		crosshair.flags |= PatchTranslation::Stretch;
}

extern "C" dboolean HU_CrosshairEnabled()
{
	return hudadd_crosshair > 0;
}

void SetCrosshairTarget()
{
	crosshair.target_screen_x = 0.0f;
	crosshair.target_screen_y = 0.0f;

	if(hudadd_crosshair_lock_target && crosshair.target_sprite >= 0)
	{
		float x, y, z;
		float winx, winy, winz;

		x = -(float)crosshair.target_x / MAP_SCALE;
		z = (float)crosshair.target_y / MAP_SCALE;
		y = (float)crosshair.target_z / MAP_SCALE;

		if(R_Project(x, y, z, &winx, &winy, &winz))
		{
			int top, bottom, h;
			stretch_param_t* params = dsda_StretchParams(crosshair.flags);

			if(V_IsSoftwareMode())
			{
				winy += (float)(viewheight / 2 - centery);
			}

			top = SCREENHEIGHT;
			h = crosshair.h;
			if(hudadd_crosshair_scale)
			{
				h = h * params->video->height / 200;
			}
			bottom = top - viewheight + h;
			winy = BETWEEN(bottom, top, winy);

			if(!hudadd_crosshair_scale)
			{
				crosshair.target_screen_x = winx - (crosshair.w / 2);
				crosshair.target_screen_y = SCREENHEIGHT - winy - (crosshair.h / 2);
			}
			else
			{
				crosshair.target_screen_x = (winx - params->deltax1) * 320.0f / params->video->width - (crosshair.w / 2);
				crosshair.target_screen_y = 200 - (winy - params->deltay1) * 200.0f / params->video->height - (crosshair.h / 2);
			}
		}
	}
}

mobj_t* HU_Target()
{
	angle_t an = plr->mo->angle;

	// intercepts overflow guard
	overflows_enabled = false;
	P_AimLineAttack(plr->mo, an, 16 * 64 * FRACUNIT, 0);
	if(plr->readyweapon == WeaponType::Missile || plr->readyweapon == WeaponType::Plasma || plr->readyweapon == WeaponType::Bfg)
	{
		if(!linetarget)
			P_AimLineAttack(plr->mo, an += 1 << 26, 16 * 64 * FRACUNIT, 0);
		if(!linetarget)
			P_AimLineAttack(plr->mo, an -= 2 << 26, 16 * 64 * FRACUNIT, 0);
	}
	overflows_enabled = true;

	return linetarget;
}

void HU_DrawCrosshair()
{
	ColorRange cm;

	if(!hudadd_crosshair)
		return;

	crosshair.target_sprite = -1;

	if(
		!crosshair_nam[hudadd_crosshair] ||
		crosshair.lump == -1 ||
		automap_full ||
		menuactive != MenuActive::Inactive ||
		dsda_Paused()
	)
	{
		return;
	}

	if(hudadd_crosshair_health)
		cm = ST_HealthColor(plr->health);
	else
		cm = static_cast<ColorRange>(dsda_IntConfig(ConfigId::HudaddCrosshairColor));

	if(hudadd_crosshair_target || hudadd_crosshair_lock_target)
	{
		mobj_t* target;

		target = HU_Target();

		if(target && !(target->flags & MF_SHADOW))
		{
			crosshair.target_x = target->x;
			crosshair.target_y = target->y;
			crosshair.target_z = target->z;
			crosshair.target_z += target->height / 2 + target->height / 8;
			crosshair.target_sprite = std::to_underlying(target->sprite);

			if(hudadd_crosshair_target)
				cm = static_cast<ColorRange>(dsda_IntConfig(ConfigId::HudaddCrosshairTargetColor));
		}
	}

	SetCrosshairTarget();

	if(crosshair.target_screen_x != 0)
	{
		float x = crosshair.target_screen_x;
		float y = crosshair.target_screen_y;
		V_DrawNumPatchPrecise(x, y, 0, crosshair.lump, cm, static_cast<PatchTranslation>(crosshair.flags));
	}
	else
	{
		int x, y, st_height;

		if(!hudadd_crosshair_scale)
		{
			st_height = (R_PartialView() ? ST_SCALED_HEIGHT : 0);
			x = (SCREENWIDTH - crosshair.w) / 2;
			y = (SCREENHEIGHT - st_height - crosshair.h) / 2;
		}
		else
		{
			st_height = (R_PartialView() ? ST_HEIGHT : 0);
			x = (320 - crosshair.w) / 2;
			y = (200 - st_height - crosshair.h) / 2;
		}

		V_DrawNumPatch(x, y, 0, crosshair.lump, cm, static_cast<PatchTranslation>(crosshair.flags));
	}
}

void HU_AnnounceMap()
{
	if(dsda_IntConfig(ConfigId::AnnounceMap))
	{
		static int last_gamemap;
		static int last_gameepisode;

		if(gamemap != last_gamemap || gameepisode != last_gameepisode)
		{
			const char* author;

			last_gamemap = gamemap;
			last_gameepisode = gameepisode;

			author = dsda_MapAuthor();
			if(author && author[0])
			{
				dsda_string_t message;

				dsda_StringPrintF(&message, "%s by %s", hud_title.string, author);
				dsda_AddAlert(message.string);
				dsda_FreeString(&message);
			}
			else
			{
				dsda_AddAlert(hud_title.string);
			}
		}
	}
}

//
// HU_Start()
//
// Create and initialize the heads-up display
//
// This routine must be called after any change to the heads up configuration
// in order for the changes to take effect in the actual displays
//
// Passed nothing, returns nothing
//
void HU_Start()
{
	HU_InitThresholds();
	HU_InitPlayer();
	HU_InitMessages();
	HU_FetchTitle();
	HU_InitCrosshair();

	dsda_InitExHud();

	HU_AnnounceMap();
}

//
// HU_Drawer()
//
// Draw all the pieces of the heads-up display
//
// Passed nothing, returns nothing
//
void HU_Drawer()
{
	V_BeginUIDraw();

	HU_DrawCrosshair();

	dsda_DrawExHud();

	V_EndUIDraw();
}

char* secret_message;

static void HU_UpdateSecretMessage(const char* message)
{
	if(secret_message)
		Z_Free(secret_message);

	secret_message = Z_Strdup(message);
}

extern "C" char* HU_SecretMessage()
{
	return custom_message_p->ticks > 0 ? secret_message : nullptr;
}

//
// HU_Ticker()
//
// Update the hud displays once per frame
//
// Passed nothing, returns nothing
//
void HU_Ticker()
{
	int i;

	dsda_UpdateMessenger();

	// centered messages
	for(i = 0; i < g_maxplayers; i++)
	{
		if(custom_message[i].ticks > 0)
			custom_message[i].ticks--;
	}

	if(custom_message_p->msg)
	{
		HU_UpdateSecretMessage(custom_message_p->msg);

		custom_message_p->msg = nullptr;

		if(std::to_underlying(custom_message_p->sfx) > 0 && std::to_underlying(custom_message_p->sfx) < num_sfx)
		{
			S_StartVoidSound(custom_message_p->sfx);
		}
	}

	dsda_UpdateExHud();
}

//
// HU_Responder()
//
// Responds to input events that affect the heads up displays
//
// Passed the event to respond to, returns true if the event was handled
//
dboolean HU_Responder(event_t* ev)
{
	if(dsda_InputActivated(InputId::RepeatMessage)) // phares
	{
		dsda_ReplayMessage();

		return true;
	}

	return false;
}

int SetCustomMessage(int plr, const char* msg, int ticks, SfxId sfx)
{
	custom_message_t item;

	if(plr < 0 || plr >= g_maxplayers || !msg || ticks < 0 || std::to_underlying(sfx) < 0 || std::to_underlying(sfx) >= num_sfx)
	{
		return false;
	}

	item.msg = msg;
	item.ticks = ticks;
	item.sfx = sfx;

	custom_message[plr] = item;

	return true;
}
