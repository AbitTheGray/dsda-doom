// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Console

#include <algorithm>
#include <utility>

#include "d_deh.hpp"
#include "doomstat.hpp"
#include "g_game.hpp"
#include "hu_lib.hpp"
#include "hu_stuff.hpp"
#include "i_main.hpp"
#include "i_system.hpp"
#include "lprintf.hpp"
#include "m_cheat.hpp"
#include "m_file.hpp"
#include "m_menu.hpp"
#include "m_misc.hpp"
#include "p_inter.hpp"
#include "p_map.hpp"
#include "p_maputl.hpp"
#include "p_mobj.hpp"
#include "p_setup.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "p_user.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "smooth.hpp"
#include "v_video.hpp"

#include "dsda.hpp"
#include "dsda/build.hpp"
#include "dsda/brute_force.hpp"
#include "dsda/configuration.hpp"
#include "dsda/demo.hpp"
#include "dsda/exhud.hpp"
#include "dsda/features.hpp"
#include "dsda/font.hpp"
#include "dsda/global.hpp"
#include "dsda/map_format.hpp"
#include "dsda/messenger.hpp"
#include "dsda/mobjinfo.hpp"
#include "dsda/playback.hpp"
#include "dsda/settings.hpp"
#include "dsda/stretch.hpp"
#include "dsda/tracker.hpp"
#include "dsda/utility.hpp"

#include "console.hpp"

#define target_player players[consoleplayer]

#define CONSOLE_TEXT_FLAGS (PatchTranslation::AlignTop | PatchTranslation::ExText)
#define CONSOLE_ENTRY_SIZE 64

// Where a console command may run besides normal play: in demo playback or recording, and in strict mode.
enum struct ConsoleCommandFlag : uint8_t
{
	Never = 0,
	Demo = Bit<uint8_t>(0u),
	Strict = Bit<uint8_t>(1u),
	Always = Demo | Strict,
};
ENUM_FLAGS_FUNC(ConsoleCommandFlag)

typedef struct console_entry_s
{
	char text[CONSOLE_ENTRY_SIZE];
	struct console_entry_s* prev;
	struct console_entry_s* next;
} console_entry_t;

static console_entry_t* console_history_head;
static console_entry_t* console_entry;
static int console_entry_index;
static char console_message[CONSOLE_ENTRY_SIZE + 2] = {' ', ' '};
static char* console_message_entry = console_message + 2;
static hu_textline_t hu_console_prompt;
static hu_textline_t hu_console_message;

static char** dsda_console_script_lines[CONSOLE_SCRIPT_COUNT];

static int console_height;

int dsda_ConsoleHeight()
{
	return console_height;
}

static void dsda_DrawConsole()
{
	console_height = V_FillHeightVPT(0, 0, 16, 0, static_cast<PatchTranslation>(CONSOLE_TEXT_FLAGS));
	HUlib_drawTextLine(&hu_console_prompt, false);
	HUlib_drawTextLine(&hu_console_message, false);
}

menu_t dsda_ConsoleDef = {
	0,
	nullptr,
	nullptr,
	dsda_DrawConsole,
	0, 0,
	0, MENUF_TEXTINPUT
};

static dboolean dsda_ExecuteConsole(const char* command_line, dboolean noise);

static void dsda_UpdateConsoleDisplay()
{
	const char* s;
	int i;

	s = console_entry->text;
	HUlib_clearTextLine(&hu_console_prompt);
	HUlib_addCharToTextLine(&hu_console_prompt, '$');
	HUlib_addCharToTextLine(&hu_console_prompt, ' ');
	for(i = 0; *s && i < console_entry_index; ++i)
		HUlib_addCharToTextLine(&hu_console_prompt, *(s++));
	HUlib_addCharToTextLine(&hu_console_prompt, '_');
	while(*s) HUlib_addCharToTextLine(&hu_console_prompt, *(s++));

	s = console_message;
	HUlib_clearTextLine(&hu_console_message);
	while(*s) HUlib_addCharToTextLine(&hu_console_message, *(s++));
}

static void dsda_ResetConsoleEntry()
{
	console_entry_index = strlen(console_entry->text);
	dsda_UpdateConsoleDisplay();
}

dboolean dsda_OpenConsole()
{
	static dboolean firsttime = true;

	if(gamestate != GameState::Level)
		return false;

	if(firsttime)
	{
		firsttime = false;

		HUlib_initTextLine(
			&hu_console_prompt,
			0,
			8,
			&exhud_font,
			ColorRange::Gray,
			static_cast<PatchTranslation>(CONSOLE_TEXT_FLAGS
		));

		HUlib_initTextLine(
			&hu_console_message,
			0,
			0,
			&exhud_font,
			ColorRange::Gray,
			static_cast<PatchTranslation>(CONSOLE_TEXT_FLAGS
		));

		console_history_head = static_cast<console_entry_t *>(Z_Calloc(sizeof(console_entry_t), 1));
		console_entry = console_history_head;
	}

	dsda_TrackFeature(FeatureFlag::Console);

	M_StartControlPanel();
	M_SetupNextMenu(&dsda_ConsoleDef);
	dsda_ResetConsoleEntry();

	return true;
}

extern "C" void G_ExitLevel(int position);
static dboolean console_LevelExit(const char* command, const char* args)
{

	int position = 0;

	if(hexen)
		return false;

	sscanf(args, "%d", &position);

	G_ExitLevel(position);

	return true;
}

extern "C" void G_SecretExitLevel(int position);
static dboolean console_LevelSecretExit(const char* command, const char* args)
{

	int position = 0;

	if(hexen)
		return false;

	sscanf(args, "%d", &position);

	G_SecretExitLevel(position);

	return true;
}

static dboolean console_ActivateLine(mobj_t* mobj, int id, dboolean bossaction)
{
	if(!mobj || id < 0 || id >= numlines)
		return false;

	P_MapStart();
	P_UseSpecialLine(mobj, &lines[id], 0, bossaction);
	map_format.cross_special_line(&lines[id], 0, mobj, bossaction);
	map_format.shoot_special_line(mobj, &lines[id]);
	P_MapEnd();

	return true;
}

static dboolean console_PlayerActivateLine(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id) != 1)
		return false;

	return console_ActivateLine(target_player.mo, id, false);
}

static dboolean console_PlayerSetHealth(const char* command, const char* args)
{
	int health;

	if(sscanf(args, "%i", &health))
	{
		target_player.mo->health = health;
		target_player.health = health;

		return true;
	}

	return false;
}

static dboolean console_PlayerKill(const char* command, const char* args)
{
	P_DamageMobj(target_player.mo, nullptr, nullptr, 10000);

	return true;
}

static dboolean console_PlayerSetArmor(const char* command, const char* args)
{
	int arg_count;
	int armorpoints, armortype;

	arg_count = sscanf(args, "%i %i", &armorpoints, &armortype);

	if(arg_count != 2 || (armortype != 1 && armortype != 2))
		armortype = target_player.armortype;

	if(arg_count)
	{
		target_player.armorpoints[std::to_underlying(ArmorType::Armor)] = armorpoints;

		if(armortype == 0) armortype = 1;
		target_player.armortype = armortype;

		return true;
	}

	return false;
}

extern "C" dboolean P_GiveWeapon(player_t* player, WeaponType weapon, dboolean dropped);
static dboolean console_PlayerGiveWeapon(const char* command, const char* args)
{
	void TryPickupWeapon(player_t* player, PClass weaponClass,
		WeaponType weaponType, mobj_t* weapon,
		const char* message);

	int weapon;

	if(sscanf(args, "%i", &weapon))
	{
		if(hexen)
		{
			mobj_t mo;

			if(weapon < 0 || weapon >= std::to_underlying(WeaponType::HexenCount))
				return false;

			memset(&mo, 0, sizeof(mo));

			mo.intflags |= MobjIntFlag::Fake;

			TryPickupWeapon(&target_player, target_player.pclass, static_cast<WeaponType>(weapon), &mo, "WEAPON");
		}
		else
		{
			if(weapon < 0 || weapon >= std::to_underlying(WeaponType::Count))
				return false;

			P_GiveWeapon(&target_player, static_cast<WeaponType>(weapon), false);
		}

		return true;
	}

	return false;
}

static dboolean console_PlayerGiveAmmo(const char* command, const char* args)
{
	int ammo;
	int amount;
	int arg_count;

	arg_count = sscanf(args, "%i %i", &ammo, &amount);

	if(arg_count == 2)
	{
		if(ammo < 0 || ammo >= g_numammo || amount <= 0)
			return false;

		if(hexen)
		{
			target_player.ammo[ammo] += amount;
			if(target_player.ammo[ammo] > MAX_MANA)
				target_player.ammo[ammo] = MAX_MANA;
		}
		else
		{
			target_player.ammo[ammo] += amount;
			if(target_player.ammo[ammo] > target_player.maxammo[ammo])
				target_player.ammo[ammo] = target_player.maxammo[ammo];
		}

		return true;
	}
	else if(arg_count == 1)
	{
		if(ammo < 0 || ammo >= g_numammo)
			return false;

		if(hexen)
			target_player.ammo[ammo] = MAX_MANA;
		else
			target_player.ammo[ammo] = target_player.maxammo[ammo];

		return true;
	}

	return false;
}

static dboolean console_PlayerSetAmmo(const char* command, const char* args)
{
	int ammo;
	int amount;

	if(sscanf(args, "%i %i", &ammo, &amount) == 2)
	{
		if(ammo < 0 || ammo >= g_numammo || amount < 0)
			return false;

		if(hexen)
		{
			target_player.ammo[ammo] = amount;
			if(target_player.ammo[ammo] > MAX_MANA)
				target_player.ammo[ammo] = MAX_MANA;
		}
		else
		{
			target_player.ammo[ammo] = amount;
			if(target_player.ammo[ammo] > target_player.maxammo[ammo])
				target_player.ammo[ammo] = target_player.maxammo[ammo];
		}

		return true;
	}

	return false;
}

static dboolean console_PlayerGiveKey(const char* command, const char* args)
{
	int key;

	if(sscanf(args, "%i", &key))
	{
		if(key < 0 || key >= std::to_underlying(Card::Count))
			return false;

		target_player.cards[key] = true;
		target_player.ravenkeys |= 1 << key;

		return true;
	}

	return false;
}

static dboolean console_PlayerRemoveKey(const char* command, const char* args)
{
	int key;

	if(sscanf(args, "%i", &key))
	{
		if(key < 0 || key >= std::to_underlying(Card::Count))
			return false;

		target_player.cards[key] = false;
		target_player.ravenkeys &= ~(1 << key);

		return true;
	}

	return false;
}

extern "C" void SB_Start();
extern "C" dboolean P_GivePower(player_t* player, PowerType power);
static dboolean console_PlayerGivePower(const char* command, const char* args)
{

	int power;
	int duration = -1;

	if(sscanf(args, "%i %i", &power, &duration))
	{
		if(power < 0 || power >= std::to_underlying(PowerType::Count) ||
			power == std::to_underlying(PowerType::Shield) || power == std::to_underlying(PowerType::Health2) || power == std::to_underlying(PowerType::Minotaur))
			return false;

		target_player.powers[power] = 0;
		P_GivePower(&target_player, static_cast<PowerType>(power));
		if(power != std::to_underlying(PowerType::Strength))
			target_player.powers[power] = duration;

		if(raven) SB_Start();

		return true;
	}

	return false;
}

extern "C" void SB_Start();
static dboolean console_PlayerRemovePower(const char* command, const char* args)
{

	int power;

	if(sscanf(args, "%i", &power))
	{
		if(power < 0 || power >= std::to_underlying(PowerType::Count) ||
			power == std::to_underlying(PowerType::Shield) || power == std::to_underlying(PowerType::Health2) || power == std::to_underlying(PowerType::Minotaur))
			return false;

		target_player.powers[power] = 0;

		if(power == std::to_underlying(PowerType::Invulnerability))
		{
			target_player.mo->flags2 -= (MobjFlag2::Invulnerable | MobjFlag2::Reflective);
			if(target_player.pclass == PClass::Cleric)
			{
				target_player.mo->flags2 -= (MobjFlag2::DontDraw | MobjFlag2::NonShootable);
				target_player.mo->flags -= (MobjFlag::Shadow | MobjFlag::AltShadow);
			}
		}
		else if(power == std::to_underlying(PowerType::Invisibility))
			target_player.mo->flags -= MobjFlag::Shadow;
		else if(power == std::to_underlying(PowerType::Flight))
		{
			P_PlayerEndFlight(&target_player);
		}
		else if(power == std::to_underlying(PowerType::WeaponLevel2) && heretic)
		{
			if((target_player.readyweapon == WeaponType::PhoenixRod)
				&& (target_player.psprites[std::to_underlying(PspNum::Weapon)].state
					!= &states[std::to_underlying(StateId::HereticPhoenixready)])
				&& (target_player.psprites[std::to_underlying(PspNum::Weapon)].state
					!= &states[std::to_underlying(StateId::HereticPhoenixup)]))
			{
				P_SetPsprite(&target_player, PspNum::Weapon, StateId::HereticPhoenixready);
				target_player.ammo[std::to_underlying(AmmoType::PhoenixRod)] -= USE_PHRD_AMMO_2;
				target_player.refire = 0;
			}
			else if((target_player.readyweapon == WeaponType::Gauntlets)
				|| (target_player.readyweapon == WeaponType::Staff))
			{
				target_player.pendingweapon = target_player.readyweapon;
			}
		}

		if(raven) SB_Start();

		return true;
	}

	return false;
}

static dboolean console_PlayerSetCoordinate(const char* args, int* dest)
{
	int x, x_frac = 0;

	if(sscanf(args, "%i.%i", &x, &x_frac))
	{
		*dest = FRACUNIT * x;

		if(args[0] == '-')
			*dest -= x_frac;
		else
			*dest += x_frac;

		return true;
	}

	return false;
}

static dboolean console_PlayerSetX(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->x);
}

static dboolean console_PlayerSetY(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->y);
}

static dboolean console_PlayerSetZ(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->z);
}

static dboolean console_PlayerSetVX(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->momx);
}

static dboolean console_PlayerSetVY(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->momy);
}

static dboolean console_PlayerSetVZ(const char* command, const char* args)
{
	return console_PlayerSetCoordinate(args, &target_player.mo->momz);
}

static void console_PlayerRoundCoordinate(int* x)
{
	int bits = *x & 0xffff;

	if(!bits)
		return;

	if(bits >= 0x8000)
		*x = (*x & ~0xffff) + FRACUNIT;
	else
		*x = *x & ~0xffff;
}

static dboolean console_PlayerRoundX(const char* command, const char* args)
{
	console_PlayerRoundCoordinate(&target_player.mo->x);

	return true;
}

static dboolean console_PlayerRoundY(const char* command, const char* args)
{
	console_PlayerRoundCoordinate(&target_player.mo->y);

	return true;
}

static dboolean console_PlayerRoundXY(const char* command, const char* args)
{
	console_PlayerRoundCoordinate(&target_player.mo->x);
	console_PlayerRoundCoordinate(&target_player.mo->y);

	return true;
}

static dboolean console_PlayerSetAngle(const char* command, const char* args)
{
	int a, a_frac = 0;

	if(sscanf(args, "%d.%d", &a, &a_frac))
	{
		target_player.mo->angle = ((angle_t)a) << 24;

		if(args[0] == '-')
			target_player.mo->angle -= (a_frac << 16);
		else
			target_player.mo->angle += (a_frac << 16);

		R_SmoothPlaying_Reset(&target_player);

		return true;
	}

	return false;
}

static dboolean console_PlayerRoundAngle(const char* command, const char* args)
{
	int remainder;

	remainder = (target_player.mo->angle >> 16) & 0xff;

	target_player.mo->angle &= 0xff000000;

	if(remainder >= 0x80)
		target_player.mo->angle += 0x01000000;

	R_SmoothPlaying_Reset(&target_player);

	return true;
}

static dboolean console_DemoExport(const char* command, const char* args)
{
	char name[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", name) == 1)
	{
		dsda_ExportDemo(name);
		return true;
	}

	return false;
}

static dboolean console_DemoStart(const char* command, const char* args)
{
	char name[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", name) == 1)
		return dsda_StartDemoSegment(name);

	return false;
}

static dboolean console_DemoStop(const char* command, const char* args)
{
	if(!demorecording)
		return false;

	G_CheckDemoStatus();
	dsda_UpdateStrictMode();

	return true;
}

static dboolean console_DemoJoin(const char* command, const char* args)
{
	if(!demoplayback)
		return false;

	dsda_JoinDemo(nullptr);

	return true;
}

static dboolean console_GameQuit(const char* command, const char* args)
{
	I_SafeExit(0);

	return true;
}

static dboolean console_GameDescribe(const char* command, const char* args)
{
	extern dsda_string_t hud_title;

	dsda_string_t str;

	dsda_StringPrintF(&str, "%s\n"
		"Skill %d\n"
		"%s%s%s",
		hud_title.string, gameskill + 1,
		nomonsters ? "-nomo " : "",
		respawnparm ? "-respawn " : "",
		fastparm ? "-fast" : "");

	dsda_AddAlert(str.string);

	dsda_FreeString(&str);

	return true;
}

static dboolean console_TrackerAddLine(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_TrackLine(id);

	return false;
}

static dboolean console_TrackerRemoveLine(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_UntrackLine(id);

	return false;
}

static dboolean console_TrackerAddLineDistance(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_TrackLineDistance(id);

	return false;
}

static dboolean console_TrackerRemoveLineDistance(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_UntrackLineDistance(id);

	return false;
}

static dboolean console_TrackerAddSector(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_TrackSector(id);

	return false;
}

static dboolean console_TrackerRemoveSector(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_UntrackSector(id);

	return false;
}

static dboolean console_TrackerAddMobj(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_TrackMobj(id);

	return false;
}

static dboolean console_TrackerRemoveMobj(const char* command, const char* args)
{
	int id;

	if(sscanf(args, "%i", &id))
		return dsda_UntrackMobj(id);

	return false;
}

static dboolean console_TrackerAddPlayer(const char* command, const char* args)
{
	return dsda_TrackPlayer(0);
}

static dboolean console_TrackerRemovePlayer(const char* command, const char* args)
{
	return dsda_UntrackPlayer(0);
}

static dboolean console_TrackerReset(const char* command, const char* args)
{
	dsda_WipeTrackers();

	return true;
}

static dboolean console_JumpToTic(const char* command, const char* args)
{
	int tic;

	if(sscanf(args, "%i", &tic))
	{
		if(tic < 0)
			return false;

		dsda_JumpToLogicTic(tic);

		return true;
	}

	return false;
}

static dboolean console_JumpByTic(const char* command, const char* args)
{
	int tic;

	if(sscanf(args, "%i", &tic))
	{
		tic = true_logictic + tic;

		dsda_JumpToLogicTic(tic);

		return true;
	}

	return false;
}

static dboolean console_BuildMF(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildMF(x);
}

static dboolean console_BuildMB(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildMB(x);
}

static dboolean console_BuildSR(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildSR(x);
}

static dboolean console_BuildSL(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildSL(x);
}

static dboolean console_BuildTR(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildTR(x);
}

static dboolean console_BuildTL(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildTL(x);
}

static dboolean console_BuildFU(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildFU(x);
}

static dboolean console_BuildFD(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildFD(x);
}

static dboolean console_BuildFC(const char* command, const char* args)
{
	return dsda_BuildFC();
}

static dboolean console_BuildLU(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildLU(x);
}

static dboolean console_BuildLD(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildLD(x);
}

static dboolean console_BuildLC(const char* command, const char* args)
{
	return dsda_BuildLC();
}

static dboolean console_BuildUA(const char* command, const char* args)
{
	int x;

	return sscanf(args, "%i", &x) && dsda_BuildUA(x);
}

static dboolean console_BruteForceKeep(const char* command, const char* args)
{
	int frame;

	if(!sscanf(args, "%d", &frame))
		return false;

	return dsda_KeepBruteForceFrame(frame);
}

static dboolean console_BruteForceNoMonsters(const char* command, const char* args)
{
	dsda_BruteForceWithoutMonsters();

	return true;
}

static dboolean console_BruteForceMonsters(const char* command, const char* args)
{
	dsda_BruteForceWithMonsters();

	return true;
}

static dboolean console_BruteForceFrame(const char* command, const char* args)
{
	int frame;
	int forwardmove_min, forwardmove_max;
	int sidemove_min, sidemove_max;
	int angleturn_min, angleturn_max;
	byte buttons = 0;
	int weapon = 0;
	char button_str[4] = {0};
	int i, arg_count;

	arg_count = sscanf(
		args, "%i %i:%i %i:%i %i:%i %3s %d", &frame,
		&forwardmove_min, &forwardmove_max,
		&sidemove_min, &sidemove_max,
		&angleturn_min, &angleturn_max,
		button_str, &weapon
	);

	if(arg_count < 7)
		return false;

	if(arg_count > 7)
		for(i = 0; i < 3; ++i)
			switch(button_str[i])
			{
				case 'a':
					buttons |= std::to_underlying(ButtonCode::Attack);
					break;
				case 'u':
					buttons |= std::to_underlying(ButtonCode::Use);
					break;
				case 'c':
					if(weapon > 0 && weapon < 16)
					{
						buttons |= std::to_underlying(ButtonCode::Change);
						buttons |= (weapon << std::to_underlying(ButtonCode::WeaponShift));
					}
					else
						return false;
					break;
				case '\0':
					break;
				default:
					return false;
			}

	return dsda_AddBruteForceFrame(frame,
		forwardmove_min, forwardmove_max,
		sidemove_min, sidemove_max,
		angleturn_min, angleturn_max,
		buttons);
}

static dboolean console_BruteForceStart(const char* command, const char* args)
{
	int depth;
	int forwardmove_min, forwardmove_max;
	int sidemove_min, sidemove_max;
	int angleturn_min, angleturn_max;
	char condition_args[CONSOLE_ENTRY_SIZE];
	int arg_count;

	dsda_ResetBruteForceConditions();

	arg_count = sscanf(
		args, "%i %i:%i %i:%i %i:%i %[^;]", &depth,
		&forwardmove_min, &forwardmove_max,
		&sidemove_min, &sidemove_max,
		&angleturn_min, &angleturn_max,
		condition_args
	);

	if(arg_count == 8)
	{
		int i;

		for(i = 0; i < depth; ++i)
			dsda_AddBruteForceFrame(i,
				forwardmove_min, forwardmove_max,
				sidemove_min, sidemove_max,
				angleturn_min, angleturn_max,
				0);
	}
	else
	{
		arg_count = sscanf(args, "%i %[^;]", &depth, condition_args);

		if(arg_count != 2)
			return false;
	}

	{
		int i;
		char** conditions;

		conditions = dsda_SplitString(condition_args, ",");

		if(!conditions)
			return false;

		for(i = 0; conditions[i]; ++i)
		{
			fixed_t value;
			char attr_s[4] = {0};
			char oper_s[5] = {0};

			if(sscanf(conditions[i], " skip %i", &value) == 1)
			{
				if(value >= numlines || value < 0)
					return false;

				dsda_AddMiscBruteForceCondition(BruteForceAttribute::LineSkip, value);
			}
			else if(sscanf(conditions[i], " act %i", &value) == 1)
			{
				if(value >= numlines || value < 0)
					return false;

				dsda_AddMiscBruteForceCondition(BruteForceAttribute::LineActivation, value);
			}
			else if(sscanf(conditions[i], " have %3[a-zA-Z]", attr_s) == 1)
			{
				const auto item = std::ranges::find_if(dsda_bf_item_names.Keys(), [&](const BruteForceItem key)
				{
					return !strcmp(attr_s, dsda_bf_item_names[key]);
				});

				if(item == dsda_bf_item_names.Keys().end())
					return false;

				dsda_AddMiscBruteForceCondition(BruteForceAttribute::HaveItem, std::to_underlying(*item));
			}
			else if(sscanf(conditions[i], " lack %3[a-zA-Z]", attr_s) == 1)
			{
				const auto item = std::ranges::find_if(dsda_bf_item_names.Keys(), [&](const BruteForceItem key)
				{
					return !strcmp(attr_s, dsda_bf_item_names[key]);
				});

				if(item == dsda_bf_item_names.Keys().end())
					return false;

				dsda_AddMiscBruteForceCondition(BruteForceAttribute::LackItem, std::to_underlying(*item));
			}
			else if(sscanf(conditions[i], " %3[a-zA-Z] %4[a-zA-Z><!=] %i", attr_s, oper_s, &value) == 3)
			{
				int oper_i;

				if(oper_s[0] == '=' && !oper_s[1])
					oper_s[1] = '=';

				const auto attribute = std::ranges::find_if(dsda_bf_attribute_names.Keys(), [&](const BruteForceAttribute key)
				{
					return !strcmp(attr_s, dsda_bf_attribute_names[key]);
				});

				if(attribute == dsda_bf_attribute_names.Keys().end())
					return false;

				for(oper_i = std::to_underlying(BruteForceLimit::TrioZero); oper_i < std::to_underlying(BruteForceLimit::TrioMax); ++oper_i)
					if(!strcmp(oper_s, dsda_bf_limit_names[oper_i]))
						break;

				if(oper_i != std::to_underlying(BruteForceLimit::TrioMax))
				{
					dsda_SetBruteForceTarget(*attribute, static_cast<BruteForceLimit>(oper_i), value, true);
					continue;
				}

				const auto operator_ = std::ranges::find_if(dsda_bf_operator_names.Keys(), [&](const BruteForceOperator key)
				{
					return !strcmp(oper_s, dsda_bf_operator_names[key]);
				});

				if(operator_ == dsda_bf_operator_names.Keys().end())
					return false;

				dsda_AddBruteForceCondition(*attribute, *operator_, value);
			}
			else if(sscanf(conditions[i], " %3s %4s", attr_s, oper_s) == 2)
			{
				int oper_i;

				const auto attribute = std::ranges::find_if(dsda_bf_attribute_names.Keys(), [&](const BruteForceAttribute key)
				{
					return !strcmp(attr_s, dsda_bf_attribute_names[key]);
				});

				if(attribute == dsda_bf_attribute_names.Keys().end())
					return false;

				for(oper_i = std::to_underlying(BruteForceLimit::DuoZero); oper_i < std::to_underlying(BruteForceLimit::DuoMax); ++oper_i)
					if(!strcmp(oper_s, dsda_bf_limit_names[oper_i]))
						break;

				if(oper_i == std::to_underlying(BruteForceLimit::DuoMax))
					return false;

				dsda_SetBruteForceTarget(*attribute, static_cast<BruteForceLimit>(oper_i), 0, false);
			}
			else
			{
				return false;
			}
		}

		Z_Free(conditions);
	}

	return dsda_StartBruteForce(depth);
}

static dboolean console_BuildTurbo(const char* command, const char* args)
{
	dsda_ToggleBuildTurbo();

	return true;
}

extern "C" void M_ClearMenus();
static dboolean console_Exit(const char* command, const char* args)
{

	M_ClearMenus();

	return true;
}

static dboolean console_BasicCheat(const char* command, const char* args)
{
	return M_CheatEntered(command, args);
}

static dboolean console_IDDT(const char* command, const char* args)
{
	M_CheatIDDT();

	return true;
}

static dboolean console_CheatFullClip(const char* command, const char* args)
{
	target_player.cheats = target_player.cheats ^ CheatFlag::InfiniteAmmo;
	return true;
}

static dboolean console_Freeze(const char* command, const char* args)
{
	dsda_ToggleFrozenMode();
	return true;
}

static dboolean console_NoSleep(const char* command, const char* args)
{
	int i;

	for(i = 0; i < numsectors; ++i)
		sectors[i].soundtarget = target_player.mo;

	return true;
}

static dboolean console_ScriptRunLine(const char* line)
{
	if(strlen(line) && line[0] != '#' && line[0] != '!' && line[0] != '/')
	{
		if(strlen(line) >= CONSOLE_ENTRY_SIZE)
		{
			Log::Error("Script line too long: \"{}\" (limit {})\n", line, CONSOLE_ENTRY_SIZE);
			return false;
		}

		if(!dsda_ExecuteConsole(line, false))
		{
			Log::Error("Script line failed: \"{}\"\n", line);
			return false;
		}
	}

	return true;
}

static dboolean console_ScriptRun(const char* command, const char* args)
{
	char name[CONSOLE_ENTRY_SIZE];
	dboolean ret = true;

	if(sscanf(args, "%s", name))
	{
		char* filename;
		char* buffer;

		filename = I_FindFile(name, "");

		if(filename)
		{
			if(M_ReadFileToString(filename, &buffer) != -1)
			{
				char* line;

				for(line = strtok(buffer, "\n;"); line; line = strtok(nullptr, "\n;"))
					if(!console_ScriptRunLine(line))
					{
						ret = false;
						break;
					}

				Z_Free(buffer);
			}
			else
			{
				Log::Error("Unable to read script file ({})\n", filename);
				ret = false;
			}

			Z_Free(filename);
		}
		else
		{
			Log::Error("Cannot find script file ({})\n", std::string_view(name));
			ret = false;
		}

		return ret;
	}

	return false;
}

static dboolean console_Check(const char* command, const char* args)
{
	char name[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", name))
	{
		char* summary;

		summary = dsda_ConfigSummary(name);

		if(summary)
		{
			Log::Info("{}\n", summary);
			Z_Free(summary);
			return true;
		}
	}

	return false;
}

static dboolean console_ChangeConfig(const char* command, const char* args, dboolean persist)
{
	char name[CONSOLE_ENTRY_SIZE];
	char value_string[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s %s", name, value_string))
	{
		ConfigId id;

		id = dsda_ConfigIDByName(name);
		if(id != ConfigId::None)
		{
			ConfigType config_type;

			config_type = dsda_ConfigType(id);
			if(config_type == ConfigType::Int)
			{
				int value_int;

				if(sscanf(value_string, "%d", &value_int))
				{
					dsda_UpdateIntConfig(id, value_int, persist);
					return true;
				}
			}
			else
			{
				dsda_UpdateStringConfig(id, value_string, persist);
				return true;
			}
		}
	}

	return false;
}

static dboolean console_Assign(const char* command, const char* args)
{
	return console_ChangeConfig(command, args, false);
}

static dboolean console_Update(const char* command, const char* args)
{
	return console_ChangeConfig(command, args, true);
}

static dboolean console_ToggleConfig(const char* command, const char* args, dboolean persist)
{
	char name[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", name))
	{
		ConfigId id;

		id = dsda_ConfigIDByName(name);
		if(id != ConfigId::None)
		{
			ConfigType config_type;

			config_type = dsda_ConfigType(id);
			if(config_type == ConfigType::Int)
			{
				dsda_ToggleConfig(id, persist);
				return true;
			}
		}
	}

	return false;
}

static dboolean console_ToggleAssign(const char* command, const char* args)
{
	return console_ToggleConfig(command, args, false);
}

static dboolean console_ToggleUpdate(const char* command, const char* args)
{
	return console_ToggleConfig(command, args, true);
}

extern "C" void M_ForgetCurrentConfig();
static dboolean console_ConfigForget(const char* command, const char* args)
{

	M_ForgetCurrentConfig();

	return true;
}

extern "C" void M_RememberCurrentConfig();
static dboolean console_ConfigRemember(const char* command, const char* args)
{

	M_RememberCurrentConfig();

	return true;
}

extern "C" void M_ForgetWadStats();
static dboolean console_WadStatsForget(const char* command, const char* args)
{

	M_ForgetWadStats();

	return true;
}

extern "C" void M_RememberWadStats();
static dboolean console_WadStatsRemember(const char* command, const char* args)
{

	M_RememberWadStats();

	return true;
}

static dboolean console_FreeTextUpdate(const char* command, const char* args)
{
	dsda_UpdateStringConfig(ConfigId::FreeText, args, true);

	return true;
}

static dboolean console_FreeTextClear(const char* command, const char* args)
{
	dsda_UpdateStringConfig(ConfigId::FreeText, "", true);

	return true;
}

static dboolean console_SetMobjState(mobj_t* mobj, StateId state)
{
	if(state == StateId::Null)
		return false;

	P_MapStart();
	P_SetMobjState(mobj, static_cast<StateId>(state));
	P_MapEnd();

	return true;
}

static dboolean console_MoveMobj(mobj_t* mobj, fixed_t x, fixed_t y)
{
	if(!mobj)
		return false;

	P_MapStart();
	P_UnqualifiedMove(mobj, x, y);
	P_MapEnd();

	return true;
}

static dboolean console_SetTarget(mobj_t* mobj, mobj_t* target)
{
	if(!mobj || !target)
		return false;

	P_SetTarget(&mobj->target, target);
	mobj->threshold = BASETHRESHOLD;

	return true;
}

static void console_SetMobjFlags(mobj_t* mobj, MobjFlag flags, MobjFlag2 flags2)
{
	P_MapStart();
	P_UnsetThingPosition(mobj);
	mobj->flags = flags;
	mobj->flags2 = flags2;
	P_SetThingPosition(mobj);
	P_MapEnd();
}

static dboolean console_TargetSpawn(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->spawnstate);
}

static dboolean console_TargetSee(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->seestate);
}

static dboolean console_TargetPain(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->painstate);
}

static dboolean console_TargetMelee(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->meleestate);
}

static dboolean console_TargetMissile(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->missilestate);
}

static dboolean console_TargetDeath(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->deathstate);
}

static dboolean console_TargetXDeath(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->xdeathstate);
}

static dboolean console_TargetRaise(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return target && console_SetMobjState(target, target->info->raisestate);
}

static dboolean console_TargetSetState(const char* command, const char* args)
{
	int state;
	mobj_t* target;

	if(sscanf(args, "%d", &state) != 1 || state < 0 || state >= num_states)
		return false;

	target = HU_Target();

	return target && console_SetMobjState(target, static_cast<StateId>(state));
}

static dboolean console_TargetSetHealth(const char* command, const char* args)
{
	int health;
	mobj_t* target;

	if(sscanf(args, "%i", &health) != 1)
		return false;

	target = HU_Target();

	if(!target)
		return false;

	target->health = health;
	if(target->player)
		target->player->health = health;

	return true;
}

static dboolean console_TargetMove(const char* command, const char* args)
{
	fixed_t x, y;
	mobj_t* target;

	if(sscanf(args, "%d %d", &x, &y) != 2)
		return false;

	x <<= FRACBITS;
	y <<= FRACBITS;

	target = HU_Target();

	return console_MoveMobj(target, x, y);
}

static dboolean console_TargetGetTarget(const char* command, const char* args)
{
	mobj_t* target = HU_Target();
	dsda_string_t str;
	if(target)
		dsda_StringPrintF(&str, "Mobj Index : %d", target->index);
	else
		dsda_StringPrintF(&str, "No target selected");

	dsda_AddAlert(str.string);
	dsda_FreeString(&str);
	return true;
}

static dboolean console_TargetSetTarget(const char* command, const char* args)
{
	mobj_t* target;
	int new_target_index;
	mobj_t* new_target;

	if(sscanf(args, "%d", &new_target_index) != 1)
		return false;

	target = HU_Target();
	new_target = dsda_FindMobj(new_target_index);

	return console_SetTarget(target, new_target);
}

static dboolean console_TargetTargetPlayer(const char* command, const char* args)
{
	mobj_t* target;

	target = HU_Target();

	return console_SetTarget(target, target_player.mo);
}

static dboolean console_TargetActivateLine(const char* command, const char* args)
{
	int id;
	mobj_t* target;

	if(sscanf(args, "%i", &id) != 1)
		return false;

	target = HU_Target();

	return console_ActivateLine(target, id, false);
}

static dboolean console_TargetAddFlags(const char* command, const char* args)
{
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", flag_str) != 1)
		return false;

	target = HU_Target();

	if(!target)
		return false;

	flags = target->flags | deh_stringToMobjFlags(flag_str);
	flags2 = target->flags2 | deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_TargetRemoveFlags(const char* command, const char* args)
{
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", flag_str) != 1)
		return false;

	target = HU_Target();

	if(!target)
		return false;

	flags = target->flags - deh_stringToMobjFlags(flag_str);
	flags2 = target->flags2 - deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_TargetSetFlags(const char* command, const char* args)
{
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%s", flag_str) != 1)
		return false;

	target = HU_Target();

	if(!target)
		return false;

	flags = deh_stringToMobjFlags(flag_str);
	flags2 = deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_MobjSpawn(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->spawnstate);
}

static dboolean console_MobjSee(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->seestate);
}

static dboolean console_MobjPain(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->painstate);
}

static dboolean console_MobjMelee(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->meleestate);
}

static dboolean console_MobjMissile(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->missilestate);
}

static dboolean console_MobjDeath(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->deathstate);
}

static dboolean console_MobjXDeath(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->xdeathstate);
}

static dboolean console_MobjRaise(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1 || index < 0)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, target->info->raisestate);
}

static dboolean console_MobjSetState(const char* command, const char* args)
{
	int state;
	int index;
	mobj_t* target;

	if(sscanf(args, "%d %d", &index, &state) != 2 || index < 0 || state < 0 || state >= num_states)
		return false;

	target = dsda_FindMobj(index);

	return target && console_SetMobjState(target, static_cast<StateId>(state));
}

static dboolean console_MobjSetHealth(const char* command, const char* args)
{
	int health;
	int index;
	mobj_t* target;

	if(sscanf(args, "%d %i", &index, &health) != 2)
		return false;

	target = dsda_FindMobj(index);

	if(!target)
		return false;

	target->health = health;
	if(target->player)
		target->player->health = health;

	return true;
}

static dboolean console_MobjMove(const char* command, const char* args)
{
	fixed_t x, y;
	int index;
	mobj_t* target;

	if(sscanf(args, "%d %d %d", &index, &x, &y) != 3)
		return false;

	x <<= FRACBITS;
	y <<= FRACBITS;

	target = dsda_FindMobj(index);

	return console_MoveMobj(target, x, y);
}

static dboolean console_MobjSetTarget(const char* command, const char* args)
{
	int index;
	mobj_t* target;
	int new_target_index;
	mobj_t* new_target;

	if(sscanf(args, "%d %d", &index, &new_target_index) != 2)
		return false;

	target = dsda_FindMobj(index);
	new_target = dsda_FindMobj(new_target_index);

	return console_SetTarget(target, new_target);
}

static dboolean console_MobjTargetPlayer(const char* command, const char* args)
{
	int index;
	mobj_t* target;

	if(sscanf(args, "%d", &index) != 1)
		return false;

	target = dsda_FindMobj(index);

	return console_SetTarget(target, target_player.mo);
}

static dboolean console_MobjActivateLine(const char* command, const char* args)
{
	int id;
	int index;
	mobj_t* target;

	if(sscanf(args, "%d %i", &index, &id) != 2)
		return false;

	target = dsda_FindMobj(index);

	return console_ActivateLine(target, id, false);
}

static dboolean console_MobjAddFlags(const char* command, const char* args)
{
	int index;
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &index, flag_str) != 2)
		return false;

	target = dsda_FindMobj(index);

	if(!target)
		return false;

	flags = target->flags | deh_stringToMobjFlags(flag_str);
	flags2 = target->flags2 | deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_MobjRemoveFlags(const char* command, const char* args)
{
	int index;
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &index, flag_str) != 2)
		return false;

	target = dsda_FindMobj(index);

	if(!target)
		return false;

	flags = target->flags - deh_stringToMobjFlags(flag_str);
	flags2 = target->flags2 - deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_MobjSetFlags(const char* command, const char* args)
{
	int index;
	mobj_t* target;
	MobjFlag flags;
	MobjFlag2 flags2;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &index, flag_str) != 2)
		return false;

	target = dsda_FindMobj(index);

	if(!target)
		return false;

	flags = deh_stringToMobjFlags(flag_str);
	flags2 = deh_stringToMBF21MobjFlags(flag_str);

	console_SetMobjFlags(target, flags, flags2);

	return true;
}

static dboolean console_BossActivateLine(const char* command, const char* args)
{
	int id;
	int index;
	mobj_t* target;

	if(sscanf(args, "%d %i", &index, &id) != 2)
		return false;

	target = dsda_FindMobj(index);

	return console_ActivateLine(target, id, true);
}

static dboolean console_Spawn(const char* command, const char* args)
{
	fixed_t x, y, z;
	int type;

	if(sscanf(args, "%d %d %d %d", &x, &y, &z, &type) != 4 || type < 0)
		return false;

	x <<= FRACBITS;
	y <<= FRACBITS;
	z <<= FRACBITS;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	return P_SpawnMobj(x, y, z, static_cast<MobjType>(type)) != nullptr;
}

static dboolean console_SpawnRelative(const char* command, const char* args)
{
	fixed_t x, y, z;
	int type;

	if(sscanf(args, "%d %d %d %d", &x, &y, &z, &type) != 4 || type < 0)
		return false;

	x <<= FRACBITS;
	y <<= FRACBITS;
	z <<= FRACBITS;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	return P_SpawnMobj(target_player.mo->x + x,
		target_player.mo->y + y,
		target_player.mo->z + z, static_cast<MobjType>(type)) != nullptr;
}

static dboolean console_StateSetTics(const char* command, const char* args)
{
	int id;
	int value;

	if(sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].tics = value;

	return true;
}

static dboolean console_StateSetMisc1(const char* command, const char* args)
{
	int id;
	int value;

	if(sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].misc1 = value;

	return true;
}

static dboolean console_StateSetMisc2(const char* command, const char* args)
{
	int id;
	int value;

	if(sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].misc2 = value;

	return true;
}

static dboolean console_StateSetArgs1(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[0] = value;

	return true;
}

static dboolean console_StateSetArgs2(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[1] = value;

	return true;
}

static dboolean console_StateSetArgs3(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[2] = value;

	return true;
}

static dboolean console_StateSetArgs4(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[3] = value;

	return true;
}

static dboolean console_StateSetArgs5(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[4] = value;

	return true;
}

static dboolean console_StateSetArgs6(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[5] = value;

	return true;
}

static dboolean console_StateSetArgs7(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[6] = value;

	return true;
}

static dboolean console_StateSetArgs8(const char* command, const char* args)
{
	int id;
	statearg_t value;

	if(sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
		return false;

	states[id].args[7] = value;

	return true;
}

static dboolean console_MobjInfoSetHealth(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].spawnhealth = value;

	return true;
}

static dboolean console_MobjInfoSetRadius(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	value <<= FRACBITS;

	mobjinfo[type].radius = value;

	return true;
}

static dboolean console_MobjInfoSetHeight(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	value <<= FRACBITS;

	mobjinfo[type].height = value;

	return true;
}

static dboolean console_MobjInfoSetMass(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].mass = value;

	return true;
}

static dboolean console_MobjInfoSetDamage(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].damage = value;

	return true;
}

static dboolean console_MobjInfoSetSpeed(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].speed = value;

	return true;
}

static dboolean console_MobjInfoSetFastSpeed(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].altspeed = value;

	return true;
}

static dboolean console_MobjInfoSetMeleeRange(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	value <<= FRACBITS;

	mobjinfo[type].meleerange = value;

	return true;
}

static dboolean console_MobjInfoSetReactionTime(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].reactiontime = value;

	return true;
}

static dboolean console_MobjInfoSetPainChance(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].painchance = value;

	return true;
}

static dboolean console_MobjInfoSetInfightingGroup(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	if(value < 0)
		value = std::to_underlying(InfightingGroup::Default);
	else
		value += std::to_underlying(InfightingGroup::End);

	mobjinfo[type].infighting_group = value;

	return true;
}

static dboolean console_MobjInfoSetProjectileGroup(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	if(value < 0)
		value = std::to_underlying(ProjectileGroup::Groupless);
	else
		value += std::to_underlying(ProjectileGroup::End);

	mobjinfo[type].projectile_group = value;

	return true;
}

static dboolean console_MobjInfoSetSplashGroup(const char* command, const char* args)
{
	int type;
	int value;

	if(sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	if(value < 0)
		value = std::to_underlying(SplashGroup::Default);
	else
		value += std::to_underlying(SplashGroup::End);

	mobjinfo[type].splash_group = value;

	return true;
}

static dboolean console_MobjInfoAddFlags(const char* command, const char* args)
{
	int type;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].flags |= deh_stringToMobjFlags(flag_str);
	mobjinfo[type].flags2 |= deh_stringToMBF21MobjFlags(flag_str);

	return true;
}

static dboolean console_MobjInfoRemoveFlags(const char* command, const char* args)
{
	int type;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].flags -= deh_stringToMobjFlags(flag_str);
	mobjinfo[type].flags2 -= deh_stringToMBF21MobjFlags(flag_str);

	return true;
}

static dboolean console_MobjInfoSetFlags(const char* command, const char* args)
{
	int type;
	char flag_str[CONSOLE_ENTRY_SIZE];

	if(sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
		return false;

	type = dsda_FindDehMobjIndex(type - 1);

	if(type == DEH_INDEX_NOT_FOUND)
		return false;

	mobjinfo[type].flags = deh_stringToMobjFlags(flag_str);
	mobjinfo[type].flags2 = deh_stringToMBF21MobjFlags(flag_str);

	return true;
}

static dboolean console_MusicRestart(const char* command, const char* args)
{
	S_StopMusic();
	S_RestartMusic();

	return true;
}

static dboolean console_AllGhosts(const char* command, const char* args)
{
	if(bmapwidth)
		bmapwidth = 0;
	else
		P_RestoreOriginalBlockMap();

	return true;
}

typedef dboolean (*console_command_t)(const char*, const char*);

typedef struct
{
	const char* command_name;
	console_command_t command;
	ConsoleCommandFlag flags;
} console_command_entry_t;

static console_command_entry_t console_commands[] = {
	// commands
	{"player.set_health", console_PlayerSetHealth, ConsoleCommandFlag::Never},
	{"player.set_armor", console_PlayerSetArmor, ConsoleCommandFlag::Never},
	{"player.give_weapon", console_PlayerGiveWeapon, ConsoleCommandFlag::Never},
	{"player.give_ammo", console_PlayerGiveAmmo, ConsoleCommandFlag::Never},
	{"player.set_ammo", console_PlayerSetAmmo, ConsoleCommandFlag::Never},
	{"player.give_key", console_PlayerGiveKey, ConsoleCommandFlag::Never},
	{"player.remove_key", console_PlayerRemoveKey, ConsoleCommandFlag::Never},
	{"player.give_power", console_PlayerGivePower, ConsoleCommandFlag::Never},
	{"player.remove_power", console_PlayerRemovePower, ConsoleCommandFlag::Never},
	{"player.set_x", console_PlayerSetX, ConsoleCommandFlag::Never},
	{"player.set_y", console_PlayerSetY, ConsoleCommandFlag::Never},
	{"player.set_z", console_PlayerSetZ, ConsoleCommandFlag::Never},
	{"player.round_x", console_PlayerRoundX, ConsoleCommandFlag::Never},
	{"player.round_y", console_PlayerRoundY, ConsoleCommandFlag::Never},
	{"player.round_xy", console_PlayerRoundXY, ConsoleCommandFlag::Never},
	{"player.set_angle", console_PlayerSetAngle, ConsoleCommandFlag::Never},
	{"player.round_angle", console_PlayerRoundAngle, ConsoleCommandFlag::Never},
	{"player.set_vx", console_PlayerSetVX, ConsoleCommandFlag::Never},
	{"player.set_vy", console_PlayerSetVY, ConsoleCommandFlag::Never},
	{"player.set_vz", console_PlayerSetVZ, ConsoleCommandFlag::Never},
	{"player.kill", console_PlayerKill, ConsoleCommandFlag::Never},

	{"music.restart", console_MusicRestart, ConsoleCommandFlag::Always},

	{"level.exit", console_LevelExit, ConsoleCommandFlag::Never},
	{"level.secret_exit", console_LevelSecretExit, ConsoleCommandFlag::Never},

	{"script.run", console_ScriptRun, ConsoleCommandFlag::Always},
	{"check", console_Check, ConsoleCommandFlag::Always},
	{"assign", console_Assign, ConsoleCommandFlag::Always},
	{"update", console_Update, ConsoleCommandFlag::Always},
	{"toggle_assign", console_ToggleAssign, ConsoleCommandFlag::Always},
	{"toggle_update", console_ToggleUpdate, ConsoleCommandFlag::Always},
	{"config.forget", console_ConfigForget, ConsoleCommandFlag::Always},
	{"config.remember", console_ConfigRemember, ConsoleCommandFlag::Always},
	{"wad_stats.forget", console_WadStatsForget, ConsoleCommandFlag::Always},
	{"wad_stats.remember", console_WadStatsRemember, ConsoleCommandFlag::Always},
	{"free_text.update", console_FreeTextUpdate, ConsoleCommandFlag::Always},
	{"free_text.clear", console_FreeTextClear, ConsoleCommandFlag::Always},

	// tracking
	{"tracker.add_line", console_TrackerAddLine, ConsoleCommandFlag::Demo},
	{"t.al", console_TrackerAddLine, ConsoleCommandFlag::Demo},
	{"tracker.remove_line", console_TrackerRemoveLine, ConsoleCommandFlag::Demo},
	{"t.rl", console_TrackerRemoveLine, ConsoleCommandFlag::Demo},
	{"tracker.add_line_distance", console_TrackerAddLineDistance, ConsoleCommandFlag::Demo},
	{"t.ald", console_TrackerAddLineDistance, ConsoleCommandFlag::Demo},
	{"tracker.remove_line_distance", console_TrackerRemoveLineDistance, ConsoleCommandFlag::Demo},
	{"t.rld", console_TrackerRemoveLineDistance, ConsoleCommandFlag::Demo},
	{"tracker.add_sector", console_TrackerAddSector, ConsoleCommandFlag::Demo},
	{"t.as", console_TrackerAddSector, ConsoleCommandFlag::Demo},
	{"tracker.remove_sector", console_TrackerRemoveSector, ConsoleCommandFlag::Demo},
	{"t.rs", console_TrackerRemoveSector, ConsoleCommandFlag::Demo},
	{"tracker.add_mobj", console_TrackerAddMobj, ConsoleCommandFlag::Demo},
	{"t.am", console_TrackerAddMobj, ConsoleCommandFlag::Demo},
	{"tracker.remove_mobj", console_TrackerRemoveMobj, ConsoleCommandFlag::Demo},
	{"t.rm", console_TrackerRemoveMobj, ConsoleCommandFlag::Demo},
	{"tracker.add_player", console_TrackerAddPlayer, ConsoleCommandFlag::Demo},
	{"t.ap", console_TrackerAddPlayer, ConsoleCommandFlag::Demo},
	{"tracker.remove_player", console_TrackerRemovePlayer, ConsoleCommandFlag::Demo},
	{"t.rp", console_TrackerRemovePlayer, ConsoleCommandFlag::Demo},
	{"tracker.reset", console_TrackerReset, ConsoleCommandFlag::Demo},
	{"t.r", console_TrackerReset, ConsoleCommandFlag::Demo},

	// thing manipulation
	{"target.spawn", console_TargetSpawn, ConsoleCommandFlag::Never},
	{"target.see", console_TargetSee, ConsoleCommandFlag::Never},
	{"target.pain", console_TargetPain, ConsoleCommandFlag::Never},
	{"target.melee", console_TargetMelee, ConsoleCommandFlag::Never},
	{"target.missile", console_TargetMissile, ConsoleCommandFlag::Never},
	{"target.death", console_TargetDeath, ConsoleCommandFlag::Never},
	{"target.xdeath", console_TargetXDeath, ConsoleCommandFlag::Never},
	{"target.raise", console_TargetRaise, ConsoleCommandFlag::Never},
	{"target.set_state", console_TargetSetState, ConsoleCommandFlag::Never},
	{"target.set_health", console_TargetSetHealth, ConsoleCommandFlag::Never},
	{"target.move", console_TargetMove, ConsoleCommandFlag::Never},
	{"target.get_target", console_TargetGetTarget, ConsoleCommandFlag::Never},
	{"target.set_target", console_TargetSetTarget, ConsoleCommandFlag::Never},
	{"target.target_player", console_TargetTargetPlayer, ConsoleCommandFlag::Never},
	{"target.add_flags", console_TargetAddFlags, ConsoleCommandFlag::Never},
	{"target.remove_flags", console_TargetRemoveFlags, ConsoleCommandFlag::Never},
	{"target.set_flags", console_TargetSetFlags, ConsoleCommandFlag::Never},

	{"mobj.spawn", console_MobjSpawn, ConsoleCommandFlag::Never},
	{"mobj.see", console_MobjSee, ConsoleCommandFlag::Never},
	{"mobj.pain", console_MobjPain, ConsoleCommandFlag::Never},
	{"mobj.melee", console_MobjMelee, ConsoleCommandFlag::Never},
	{"mobj.missile", console_MobjMissile, ConsoleCommandFlag::Never},
	{"mobj.death", console_MobjDeath, ConsoleCommandFlag::Never},
	{"mobj.xdeath", console_MobjXDeath, ConsoleCommandFlag::Never},
	{"mobj.raise", console_MobjRaise, ConsoleCommandFlag::Never},
	{"mobj.set_state", console_MobjSetState, ConsoleCommandFlag::Never},
	{"mobj.set_health", console_MobjSetHealth, ConsoleCommandFlag::Never},
	{"mobj.move", console_MobjMove, ConsoleCommandFlag::Never},
	{"mobj.set_target", console_MobjSetTarget, ConsoleCommandFlag::Never},
	{"mobj.target_player", console_MobjTargetPlayer, ConsoleCommandFlag::Never},
	{"mobj.add_flags", console_MobjAddFlags, ConsoleCommandFlag::Never},
	{"mobj.remove_flags", console_MobjRemoveFlags, ConsoleCommandFlag::Never},
	{"mobj.set_flags", console_MobjSetFlags, ConsoleCommandFlag::Never},

	{"spawn", console_Spawn, ConsoleCommandFlag::Never},
	{"spawn_rel", console_SpawnRelative, ConsoleCommandFlag::Never},

	// lines
	{"player.activate_line", console_PlayerActivateLine, ConsoleCommandFlag::Never},
	{"target.activate_line", console_TargetActivateLine, ConsoleCommandFlag::Never},
	{"mobj.activate_line", console_MobjActivateLine, ConsoleCommandFlag::Never},
	{"boss.activate_line", console_BossActivateLine, ConsoleCommandFlag::Never},

	// states
	{"state.set_tics", console_StateSetTics, ConsoleCommandFlag::Never},
	{"state.set_misc1", console_StateSetMisc1, ConsoleCommandFlag::Never},
	{"state.set_misc2", console_StateSetMisc2, ConsoleCommandFlag::Never},
	{"state.set_args1", console_StateSetArgs1, ConsoleCommandFlag::Never},
	{"state.set_args2", console_StateSetArgs2, ConsoleCommandFlag::Never},
	{"state.set_args3", console_StateSetArgs3, ConsoleCommandFlag::Never},
	{"state.set_args4", console_StateSetArgs4, ConsoleCommandFlag::Never},
	{"state.set_args5", console_StateSetArgs5, ConsoleCommandFlag::Never},
	{"state.set_args6", console_StateSetArgs6, ConsoleCommandFlag::Never},
	{"state.set_args7", console_StateSetArgs7, ConsoleCommandFlag::Never},
	{"state.set_args8", console_StateSetArgs8, ConsoleCommandFlag::Never},

	// mobjinfo
	{"mobjinfo.set_health", console_MobjInfoSetHealth, ConsoleCommandFlag::Never},
	{"mobjinfo.set_radius", console_MobjInfoSetRadius, ConsoleCommandFlag::Never},
	{"mobjinfo.set_height", console_MobjInfoSetHeight, ConsoleCommandFlag::Never},
	{"mobjinfo.set_mass", console_MobjInfoSetMass, ConsoleCommandFlag::Never},
	{"mobjinfo.set_damage", console_MobjInfoSetDamage, ConsoleCommandFlag::Never},
	{"mobjinfo.set_speed", console_MobjInfoSetSpeed, ConsoleCommandFlag::Never},
	{"mobjinfo.set_fast_speed", console_MobjInfoSetFastSpeed, ConsoleCommandFlag::Never},
	{"mobjinfo.set_melee_range", console_MobjInfoSetMeleeRange, ConsoleCommandFlag::Never},
	{"mobjinfo.set_reaction_time", console_MobjInfoSetReactionTime, ConsoleCommandFlag::Never},
	{"mobjinfo.set_pain_chance", console_MobjInfoSetPainChance, ConsoleCommandFlag::Never},
	{"mobjinfo.set_infighting_group", console_MobjInfoSetInfightingGroup, ConsoleCommandFlag::Never},
	{"mobjinfo.set_projectile_group", console_MobjInfoSetProjectileGroup, ConsoleCommandFlag::Never},
	{"mobjinfo.set_splash_group", console_MobjInfoSetSplashGroup, ConsoleCommandFlag::Never},
	{"mobjinfo.add_flags", console_MobjInfoAddFlags, ConsoleCommandFlag::Never},
	{"mobjinfo.remove_flags", console_MobjInfoRemoveFlags, ConsoleCommandFlag::Never},
	{"mobjinfo.set_flags", console_MobjInfoSetFlags, ConsoleCommandFlag::Never},

	// traversing time
	{"jump.to_tic", console_JumpToTic, ConsoleCommandFlag::Demo},
	{"jump.by_tic", console_JumpByTic, ConsoleCommandFlag::Demo},

	// build mode
	{"brute_force.start", console_BruteForceStart, ConsoleCommandFlag::Demo},
	{"bf.start", console_BruteForceStart, ConsoleCommandFlag::Demo},
	{"brute_force.frame", console_BruteForceFrame, ConsoleCommandFlag::Demo},
	{"bf.frame", console_BruteForceFrame, ConsoleCommandFlag::Demo},
	{"brute_force.keep", console_BruteForceKeep, ConsoleCommandFlag::Demo},
	{"bf.keep", console_BruteForceKeep, ConsoleCommandFlag::Demo},
	{"brute_force.nomonsters", console_BruteForceNoMonsters, ConsoleCommandFlag::Demo},
	{"bf.nomo", console_BruteForceNoMonsters, ConsoleCommandFlag::Demo},
	{"brute_force.monsters", console_BruteForceMonsters, ConsoleCommandFlag::Demo},
	{"bf.mo", console_BruteForceMonsters, ConsoleCommandFlag::Demo},
	{"build.turbo", console_BuildTurbo, ConsoleCommandFlag::Demo},
	{"b.turbo", console_BuildTurbo, ConsoleCommandFlag::Demo},
	{"mf", console_BuildMF, ConsoleCommandFlag::Demo},
	{"mb", console_BuildMB, ConsoleCommandFlag::Demo},
	{"sr", console_BuildSR, ConsoleCommandFlag::Demo},
	{"sl", console_BuildSL, ConsoleCommandFlag::Demo},
	{"tr", console_BuildTR, ConsoleCommandFlag::Demo},
	{"tl", console_BuildTL, ConsoleCommandFlag::Demo},
	{"fu", console_BuildFU, ConsoleCommandFlag::Demo},
	{"fd", console_BuildFD, ConsoleCommandFlag::Demo},
	{"fc", console_BuildFC, ConsoleCommandFlag::Demo},
	{"lu", console_BuildLU, ConsoleCommandFlag::Demo},
	{"ld", console_BuildLD, ConsoleCommandFlag::Demo},
	{"lc", console_BuildLC, ConsoleCommandFlag::Demo},
	{"ua", console_BuildUA, ConsoleCommandFlag::Demo},

	// demos
	{"demo.export", console_DemoExport, ConsoleCommandFlag::Always},
	{"demo.start", console_DemoStart, ConsoleCommandFlag::Never},
	{"demo.stop", console_DemoStop, ConsoleCommandFlag::Always},
	{"demo.join", console_DemoJoin, ConsoleCommandFlag::Always},

	{"game.quit", console_GameQuit, ConsoleCommandFlag::Always},
	{"game.describe", console_GameDescribe, ConsoleCommandFlag::Always},

	// cheats
	{"idchoppers", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"iddqd", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idkfa", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idfa", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idspispopd", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idclip", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idmypos", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idrate", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"iddt", console_IDDT, ConsoleCommandFlag::Demo},
	{"iddst", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"iddkt", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"iddit", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idclev", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idmus", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholdv", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholds", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholdi", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholdr", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholda", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"idbeholdl", console_BasicCheat, ConsoleCommandFlag::Demo},

	{"skill", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntcomp", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntem", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tnthom", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntka", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntsmart", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntpitch", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntfast", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntice", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"tntpush", console_BasicCheat, ConsoleCommandFlag::Demo},

	{"notarget", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"fly", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"fullclip", console_CheatFullClip, ConsoleCommandFlag::Never},
	{"freeze", console_Freeze, ConsoleCommandFlag::Never},
	{"nosleep", console_NoSleep, ConsoleCommandFlag::Never},
	{"allghosts", console_AllGhosts, ConsoleCommandFlag::Never},

	{"quicken", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"ponce", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"kitty", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"massacre", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"rambo", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"skel", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"shazam", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"ravmap", console_IDDT, ConsoleCommandFlag::Demo},
	{"cockadoodledoo", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"gimme", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"engage", console_BasicCheat, ConsoleCommandFlag::Demo},

	{"satan", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"clubmed", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"butcher", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"nra", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"indiana", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"locksmith", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"sherlock", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"casper", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"init", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"mapsco", console_IDDT, ConsoleCommandFlag::Demo},
	{"deliverance", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"shadowcaster", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"visit", console_BasicCheat, ConsoleCommandFlag::Demo},
	{"puke", console_BasicCheat, ConsoleCommandFlag::Demo},

	// exit
	{"exit", console_Exit, ConsoleCommandFlag::Always},
	{"quit", console_Exit, ConsoleCommandFlag::Always},
	{nullptr}
};

static void dsda_AddConsoleMessage(const char* message)
{
	strncpy(console_message_entry, message, CONSOLE_ENTRY_SIZE - 1);
}

static dboolean dsda_AuthorizeCommand(console_command_entry_t* entry)
{
	if((entry->flags & ConsoleCommandFlag::Demo) == ConsoleCommandFlag{} && (demorecording || demoplayback))
	{
		dsda_AddConsoleMessage("command not allowed in demo mode");
		return false;
	}

	if((entry->flags & ConsoleCommandFlag::Strict) == ConsoleCommandFlag{} && dsda_StrictMode())
	{
		dsda_AddConsoleMessage("command not allowed in strict mode");
		return false;
	}

	if(gamestate != GameState::Level)
	{
		dsda_AddConsoleMessage("command only allowed during levels");
		return false;
	}

	return true;
}

static dboolean dsda_ExecuteConsole(const char* command_line, dboolean noise)
{
	char command[CONSOLE_ENTRY_SIZE];
	char args[CONSOLE_ENTRY_SIZE];
	int scan_count;
	dboolean ret = true;

	scan_count = sscanf(command_line, "%s %[^;]", command, args);

	if(scan_count)
	{
		console_command_entry_t* entry;

		if(scan_count == 1) args[0] = '\0';

		for(entry = console_commands; entry->command; entry++)
		{
			if(!stricmp(command, entry->command_name))
			{
				if(dsda_AuthorizeCommand(entry))
				{
					if(entry->command(command, args))
					{
						dsda_AddConsoleMessage("command executed");

						if(noise)
							S_StartVoidSound(g_sfx_console);
					}
					else
					{
						dsda_AddConsoleMessage("command invalid");
						ret = false;

						if(noise)
							S_StartOptionalSound(g_sfx_mnuerr, g_sfx_oof, false);
					}
				}
				else
				{
					ret = false;

					if(noise)
						S_StartOptionalSound(g_sfx_mnuerr, g_sfx_oof, false);
				}

				break;
			}
		}

		if(!entry->command)
		{
			dsda_AddConsoleMessage("command unknown");
			ret = false;

			if(noise)
				S_StartOptionalSound(g_sfx_mnuerr, g_sfx_oof, false);
		}
	}

	return ret;
}

void dsda_UpdateConsoleText(char* text)
{
	int i;
	int length;

	length = strlen(text);

	for(i = 0; i < length; ++i)
	{
		int shift_i;

		if(text[i] < 32 || text[i] > 126)
			continue;

		if(console_entry_index > CONSOLE_ENTRY_SIZE - 2)
			console_entry_index = CONSOLE_ENTRY_SIZE - 2;

		for(shift_i = strlen(console_entry->text) - 1; shift_i > console_entry_index; --shift_i)
			console_entry->text[shift_i] = console_entry->text[shift_i - 1];

		console_entry->text[console_entry_index] = tolower(text[i]);
		++console_entry_index;
	}

	dsda_UpdateConsoleDisplay();
}

void dsda_UpdateConsoleHistory()
{
	console_entry_t* last_command;

	if(console_entry != console_history_head)
		strcpy(console_history_head->text, console_entry->text);

	last_command = console_history_head->prev;
	if(!last_command || strcmp(last_command->text, console_entry->text))
	{
		console_entry_t* new_head;

		new_head = static_cast<console_entry_t *>(Z_Calloc(sizeof(*new_head), 1));
		new_head->prev = console_history_head;
		console_history_head->next = new_head;
		console_history_head = new_head;
	}
	else
		memset(console_history_head->text, 0, CONSOLE_ENTRY_SIZE);

	console_entry = console_history_head;
}

void dsda_InterpretConsoleCommands(const char* str, dboolean noise, dboolean raise_errors)
{
	int line;
	char* entry;
	char** lines;

	entry = Z_Strdup(str);
	lines = dsda_SplitString(entry, ";");
	for(line = 0; lines[line]; ++line)
		if(!dsda_ExecuteConsole(lines[line], noise) && raise_errors)
			Log::Fatal("Console command failed: {}", lines[line]);

	Z_Free(lines);
	Z_Free(entry);
}

void dsda_UpdateConsole(const MenuAction action)
{
	if(action == MenuAction::Backspace && console_entry_index > 0)
	{
		int shift_i;

		for(shift_i = console_entry_index; console_entry->text[shift_i]; ++shift_i)
			console_entry->text[shift_i - 1] = console_entry->text[shift_i];
		console_entry->text[shift_i - 1] = '\0';

		--console_entry_index;
		dsda_UpdateConsoleDisplay();
	}
	else if(action == MenuAction::Enter)
	{
		dsda_InterpretConsoleCommands(console_entry->text, true, false);

		dsda_UpdateConsoleHistory();
		dsda_ResetConsoleEntry();
	}
	else if(action == MenuAction::Up)
	{
		if(console_entry->prev)
			console_entry = console_entry->prev;
		console_entry_index = strlen(console_entry->text);
		dsda_UpdateConsoleDisplay();
	}
	else if(action == MenuAction::Down)
	{
		if(console_entry->next)
			console_entry = console_entry->next;
		console_entry_index = strlen(console_entry->text);
		dsda_UpdateConsoleDisplay();
	}
	else if(action == MenuAction::Right && console_entry->text[console_entry_index])
	{
		++console_entry_index;
		dsda_UpdateConsoleDisplay();
	}
	else if(action == MenuAction::Left && console_entry_index > 0)
	{
		--console_entry_index;
		dsda_UpdateConsoleDisplay();
	}
}

void dsda_ExecuteConsoleScript(int i)
{
	int line;

	if(gamestate != GameState::Level || i < 0 || i >= CONSOLE_SCRIPT_COUNT)
		return;

	if(!dsda_console_script_lines[i])
	{
		char* dup;

		dup = Z_Strdup(dsda_StringConfig(static_cast<ConfigId>(std::to_underlying(ConfigId::Script0) + i)));
		dsda_console_script_lines[i] = dsda_SplitString(dup, ";");
	}

	for(line = 0; dsda_console_script_lines[i][line]; ++line)
		if(!console_ScriptRunLine(dsda_console_script_lines[i][line]))
		{
			Message::Add("Script {} failed", i);

			return;
		}

	Message::Add("Script {} executed", i);
}
