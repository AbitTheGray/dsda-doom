// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Settings

#include <errno.h>
#include <stdlib.h>

#include "doomstat.hpp"
#include "m_menu.hpp"
#include "e6y.hpp"
#include "r_things.hpp"
#include "w_wad.hpp"
#include "g_game.hpp"
#include "gl_struct.hpp"
#include "lprintf.hpp"
#include "i_capture.hpp"
#include "i_main.hpp"
#include "i_video.hpp"

#include "dsda/args.hpp"
#include "dsda/build.hpp"
#include "dsda/configuration.hpp"
#include "dsda/exhud.hpp"
#include "dsda/features.hpp"
#include "dsda/key_frame.hpp"
#include "dsda/map_format.hpp"
#include "dsda/skip.hpp"

#include "settings.hpp"

int dsda_skip_next_wipe;

extern "C" void gld_ResetAutomapTransparency();
extern "C" void dsda_InitQuickstartCache();
extern "C" void dsda_InitParallelSFXFilter();
extern "C" void G_UpdateMouseSensitivity();
void dsda_InitSettings()
{

	dsda_UpdateStrictMode();
	G_UpdateMouseSensitivity();
	dsda_InitQuickstartCache();
	dsda_InitParallelSFXFilter();
	gld_ResetAutomapTransparency();
}

static int dsda_ComplvlStrToNum(const char* data, int length)
{
	if(length == 7 && !strncasecmp("vanilla", data, 7))
	{
		if(gamemode == commercial)
		{
			if(gamemission == pack_plut || gamemission == pack_tnt)
				return 4;
			else
				return 2;
		}
		else
			return 3;
	}
	else if(length == 4 && !strncasecmp("boom", data, 4))
		return 9;
	else if(length == 3 && !strncasecmp("mbf", data, 3))
		return 11;
	else if(length == 5 && !strncasecmp("mbf21", data, 5))
		return 21;
	return -1;
}

static int dsda_WadCompatibilityLevel()
{
	static int complvl = -1;
	static int last_numwadfiles = -1;

	// This might be called before all wads are loaded
	if(numwadfiles != last_numwadfiles)
	{
		last_numwadfiles = numwadfiles;
		int num = W_CheckNumForName("COMPLVL");

		if(num != LUMP_NOT_FOUND)
		{
			int length = W_LumpLength(num);
			const char* data = (const char*)W_LumpByNum(num);

			complvl = dsda_ComplvlStrToNum(data, length);
			lprintf(LO_INFO, "Detected COMPLVL lump: %i\n", complvl);
		}
	}

	return complvl;
}

int dsda_CompatibilityLevel()
{
	int level;
	dsda_arg_t* complevel_arg;

	if(raven) return doom_12_compatibility;

	if(map_format.zdoom) return mbf21_compatibility;

	complevel_arg = dsda_Arg(dsda_arg_complevel);

	if(complevel_arg->count)
	{
		const char* arg_val = complevel_arg->value.v_string;
		char* str_end;
		errno = 0;
		level = strtol(arg_val, &str_end, 0);
		if(errno == 0 && *str_end == '\0')
		{
			if(level >= -1 && level < MAX_COMPATIBILITY_LEVEL)
			{
				return level;
			}
		}
		else
		{
			level = dsda_ComplvlStrToNum(arg_val, strlen(arg_val));
			if(level != -1)
			{
				return level;
			}
		}
		I_Error("-complevel value of \"%s\" did not match any known complevel.", arg_val);
	}

	if(!demoplayback)
	{
		level = dsda_WadCompatibilityLevel();

		if(level >= 0)
			return level;
	}

	return UNSPECIFIED_COMPLEVEL;
}

void dsda_SetTas(dboolean t)
{
	dsda_UpdateIntConfig(dsda_config_strict_mode, !t, true);
}

int dsda_ViewBob()
{
	return dsda_IntConfig(dsda_config_viewbob);
}

int dsda_WeaponBob()
{
	return dsda_IntConfig(dsda_config_weaponbob);
}

dboolean dsda_FixViewBobFloorJolt()
{
	return dsda_IntConfig(dsda_config_fix_viewbob_floor_jolt);
}

dboolean dsda_ShowMessages()
{
	return dsda_IntConfig(dsda_config_show_messages);
}

dboolean dsda_AutoRun()
{
	return dsda_IntConfig(dsda_config_autorun);
}

dboolean dsda_MouseLook()
{
	return dsda_IntConfig(dsda_config_freelook);
}

dboolean dsda_VertMouse()
{
	return dsda_IntConfig(dsda_config_vertmouse);
}

dboolean dsda_StrictMode()
{
	return dsda_IntConfig(dsda_config_strict_mode) && demorecording;
}

dboolean dsda_MuteSfx()
{
	return dsda_IntConfig(dsda_config_mute_sfx) ||
		(!I_WindowFocused() && dsda_IntConfig(dsda_config_mute_unfocused_window) && !capturing_video);
}

dboolean dsda_MuteMusic()
{
	return dsda_IntConfig(dsda_config_mute_music) ||
		(!I_WindowFocused() && dsda_IntConfig(dsda_config_mute_unfocused_window) && !capturing_video);
}

dboolean dsda_ProcessCheatCodes()
{
	return dsda_IntConfig(dsda_config_cheat_codes);
}

dboolean dsda_CycleGhostColors()
{
	return dsda_IntConfig(dsda_config_cycle_ghost_colors);
}

dboolean dsda_AlwaysSR50()
{
	return dsda_IntConfig(dsda_config_movement_strafe50);
}

dboolean dsda_HideHorns()
{
	return dsda_IntConfig(dsda_config_hide_horns);
}

dboolean dsda_HideWeapon()
{
	return dsda_IntConfig(dsda_config_hide_weapon);
}

dboolean dsda_SwitchWhenAmmoRunsOut()
{
	return dsda_IntConfig(dsda_config_switch_when_ammo_runs_out);
}

dboolean dsda_SkipQuitPrompt()
{
	return dsda_IntConfig(dsda_config_skip_quit_prompt) || dsda_SkipMode();
}

dboolean dsda_TrackSplits()
{
	return demorecording || (demoplayback && dsda_Flag(dsda_arg_track_playback));
}

dboolean dsda_ShowSplitData()
{
	return dsda_IntConfig(dsda_config_show_split_data);
}

dboolean dsda_CommandDisplay()
{
	return dsda_IntConfig(dsda_config_command_display) || dsda_BuildMode();
}

dboolean dsda_CoordinateDisplay()
{
	return dsda_IntConfig(dsda_config_coordinate_display);
}

dboolean dsda_ShowFPS()
{
	return dsda_IntConfig(dsda_config_show_fps);
}

dboolean dsda_ShowMinimap()
{
	return dsda_IntConfig(dsda_config_show_minimap);
}

dboolean dsda_ShowLevelSplits()
{
	return dsda_IntConfig(dsda_config_show_level_splits);
}

dboolean dsda_ShowDemoAttempts()
{
	return dsda_IntConfig(dsda_config_show_demo_attempts) && demorecording;
}

dboolean dsda_MapCoordinates()
{
	return dsda_IntConfig(dsda_config_map_coordinates);
}

dboolean dsda_MapTotals()
{
	return dsda_IntConfig(dsda_config_map_totals);
}

dboolean dsda_MapTime()
{
	return dsda_IntConfig(dsda_config_map_time);
}

dboolean dsda_MapTitle()
{
	return dsda_IntConfig(dsda_config_map_title);
}

dboolean dsda_PainPalette()
{
	return dsda_IntConfig(dsda_config_palette_ondamage);
}

dboolean dsda_BonusPalette()
{
	return dsda_IntConfig(dsda_config_palette_onbonus);
}

dboolean dsda_PowerPalette()
{
	return dsda_IntConfig(dsda_config_palette_onpowers);
}

dboolean dsda_ShowHealthBars()
{
	return dsda_IntConfig(dsda_config_gl_health_bar);
}

dboolean dsda_WipeAtFullSpeed()
{
	return dsda_IntConfig(dsda_config_wipe_at_full_speed);
}

int dsda_ShowAliveMonsters()
{
	return dsda_IntConfig(dsda_config_show_alive_monsters);
}

int dsda_reveal_map;

int dsda_RevealAutomap()
{
	if(dsda_StrictMode()) return 0;

	return dsda_reveal_map;
}

void dsda_ResetRevealMap()
{
	dsda_reveal_map = 0;
}

int dsda_GameSpeed()
{
	return dsda_IntConfig(dsda_config_game_speed);
}

void dsda_UpdateGameSpeed(int value)
{
	dsda_UpdateIntConfig(dsda_config_game_speed, value, true);
}

void dsda_SkipNextWipe()
{
	dsda_skip_next_wipe = 1;
}

// In raven, strict mode does not affect this setting
dboolean dsda_RenderWipeScreen()
{
	return raven ? dsda_TransientIntConfig(dsda_config_render_wipescreen) : dsda_IntConfig(dsda_config_render_wipescreen);
}

dboolean dsda_PendingSkipWipe()
{
	return dsda_skip_next_wipe || !dsda_RenderWipeScreen();
}

dboolean dsda_SkipWipe()
{
	if(dsda_skip_next_wipe)
	{
		dsda_skip_next_wipe = 0;
		return true;
	}

	// Hexen doesnt have screen wipe
	if(hexen)
		return true;

	// Heretic doesnt have screen wipe, but allow it during demos (QOL for quickstarting)
	if(heretic && !demorecording)
		return true;

	return !dsda_RenderWipeScreen();
}

static dboolean game_controller_used;
static dboolean mouse_used;

dboolean dsda_AllowGameController()
{
	return !dsda_StrictMode() || !mouse_used;
}

dboolean dsda_AllowMouse()
{
	return !dsda_StrictMode() || !game_controller_used;
}

void dsda_WatchGameControllerEvent()
{
	game_controller_used = true;

	if(mouse_used)
		dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_WatchMouseEvent()
{
	mouse_used = true;

	if(game_controller_used)
		dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_LiftInputRestrictions()
{
	game_controller_used = false;
	mouse_used = false;
}
