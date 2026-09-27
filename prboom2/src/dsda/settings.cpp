// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Settings

#include <utility>

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
		if(gamemode == GameMode::Commercial)
		{
			if(gamemission == GameMission::PackPlut || gamemission == GameMission::PackTnt)
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
			Log::Info("Detected COMPLVL lump: {}\n", complvl);
		}
	}

	return complvl;
}

CompLevel dsda_CompatibilityLevel()
{
	int level;
	dsda_arg_t* complevel_arg;

	if(raven) return CompLevel::Doom12;

	if(map_format.zdoom) return CompLevel::Mbf21;

	complevel_arg = dsda_Arg(ArgId::Complevel);

	if(complevel_arg->count)
	{
		const char* arg_val = complevel_arg->value.v_string;
		char* str_end;
		errno = 0;
		level = strtol(arg_val, &str_end, 0);
		if(errno == 0 && *str_end == '\0')
		{
			if(level >= -1 && level < std::to_underlying(CompLevel::Max))
			{
				return static_cast<CompLevel>(level);
			}
		}
		else
		{
			level = dsda_ComplvlStrToNum(arg_val, strlen(arg_val));
			if(level != -1)
			{
				return static_cast<CompLevel>(level);
			}
		}
		Log::Fatal("-complevel value of \"{}\" did not match any known complevel.", arg_val);
	}

	if(!demoplayback)
	{
		level = dsda_WadCompatibilityLevel();

		if(level >= 0)
			return static_cast<CompLevel>(level);
	}

	return static_cast<CompLevel>(UNSPECIFIED_COMPLEVEL);
}

void dsda_SetTas(dboolean t)
{
	dsda_UpdateIntConfig(ConfigId::StrictMode, !t, true);
}

int dsda_ViewBob()
{
	return dsda_IntConfig(ConfigId::Viewbob);
}

int dsda_WeaponBob()
{
	return dsda_IntConfig(ConfigId::Weaponbob);
}

dboolean dsda_FixViewBobFloorJolt()
{
	return dsda_IntConfig(ConfigId::FixViewbobFloorJolt);
}

dboolean dsda_ShowMessages()
{
	return dsda_IntConfig(ConfigId::ShowMessages);
}

dboolean dsda_AutoRun()
{
	return dsda_IntConfig(ConfigId::Autorun);
}

dboolean dsda_MouseLook()
{
	return dsda_IntConfig(ConfigId::Freelook);
}

dboolean dsda_VertMouse()
{
	return dsda_IntConfig(ConfigId::Vertmouse);
}

dboolean dsda_StrictMode()
{
	return dsda_IntConfig(ConfigId::StrictMode) && demorecording;
}

dboolean dsda_MuteSfx()
{
	return dsda_IntConfig(ConfigId::MuteSfx) ||
		(!I_WindowFocused() && dsda_IntConfig(ConfigId::MuteUnfocusedWindow) && !capturing_video);
}

dboolean dsda_MuteMusic()
{
	return dsda_IntConfig(ConfigId::MuteMusic) ||
		(!I_WindowFocused() && dsda_IntConfig(ConfigId::MuteUnfocusedWindow) && !capturing_video);
}

dboolean dsda_ProcessCheatCodes()
{
	return dsda_IntConfig(ConfigId::CheatCodes);
}

dboolean dsda_CycleGhostColors()
{
	return dsda_IntConfig(ConfigId::CycleGhostColors);
}

dboolean dsda_AlwaysSR50()
{
	return dsda_IntConfig(ConfigId::MovementStrafe50);
}

dboolean dsda_HideHorns()
{
	return dsda_IntConfig(ConfigId::HideHorns);
}

dboolean dsda_HideWeapon()
{
	return dsda_IntConfig(ConfigId::HideWeapon);
}

dboolean dsda_SwitchWhenAmmoRunsOut()
{
	return dsda_IntConfig(ConfigId::SwitchWhenAmmoRunsOut);
}

dboolean dsda_SkipQuitPrompt()
{
	return dsda_IntConfig(ConfigId::SkipQuitPrompt) || dsda_SkipMode();
}

dboolean dsda_TrackSplits()
{
	return demorecording || (demoplayback && dsda_Flag(ArgId::TrackPlayback));
}

dboolean dsda_ShowSplitData()
{
	return dsda_IntConfig(ConfigId::ShowSplitData);
}

dboolean dsda_CommandDisplay()
{
	return dsda_IntConfig(ConfigId::CommandDisplay) || dsda_BuildMode();
}

dboolean dsda_CoordinateDisplay()
{
	return dsda_IntConfig(ConfigId::CoordinateDisplay);
}

dboolean dsda_ShowFPS()
{
	return dsda_IntConfig(ConfigId::ShowFps);
}

dboolean dsda_ShowMinimap()
{
	return dsda_IntConfig(ConfigId::ShowMinimap);
}

dboolean dsda_ShowLevelSplits()
{
	return dsda_IntConfig(ConfigId::ShowLevelSplits);
}

dboolean dsda_ShowDemoAttempts()
{
	return dsda_IntConfig(ConfigId::ShowDemoAttempts) && demorecording;
}

dboolean dsda_MapCoordinates()
{
	return dsda_IntConfig(ConfigId::MapCoordinates);
}

dboolean dsda_MapTotals()
{
	return dsda_IntConfig(ConfigId::MapTotals);
}

dboolean dsda_MapTime()
{
	return dsda_IntConfig(ConfigId::MapTime);
}

dboolean dsda_MapTitle()
{
	return dsda_IntConfig(ConfigId::MapTitle);
}

dboolean dsda_PainPalette()
{
	return dsda_IntConfig(ConfigId::PaletteOndamage);
}

dboolean dsda_BonusPalette()
{
	return dsda_IntConfig(ConfigId::PaletteOnbonus);
}

dboolean dsda_PowerPalette()
{
	return dsda_IntConfig(ConfigId::PaletteOnpowers);
}

dboolean dsda_ShowHealthBars()
{
	return dsda_IntConfig(ConfigId::GlHealthBar);
}

dboolean dsda_WipeAtFullSpeed()
{
	return dsda_IntConfig(ConfigId::WipeAtFullSpeed);
}

int dsda_ShowAliveMonsters()
{
	return dsda_IntConfig(ConfigId::ShowAliveMonsters);
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
	return dsda_IntConfig(ConfigId::GameSpeed);
}

void dsda_UpdateGameSpeed(int value)
{
	dsda_UpdateIntConfig(ConfigId::GameSpeed, value, true);
}

void dsda_SkipNextWipe()
{
	dsda_skip_next_wipe = 1;
}

// In raven, strict mode does not affect this setting
dboolean dsda_RenderWipeScreen()
{
	return raven ? dsda_TransientIntConfig(ConfigId::RenderWipescreen) : dsda_IntConfig(ConfigId::RenderWipescreen);
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
		dsda_TrackFeature(FeatureFlag::MouseAndController);
}

void dsda_WatchMouseEvent()
{
	mouse_used = true;

	if(game_controller_used)
		dsda_TrackFeature(FeatureFlag::MouseAndController);
}

void dsda_LiftInputRestrictions()
{
	game_controller_used = false;
	mouse_used = false;
}
