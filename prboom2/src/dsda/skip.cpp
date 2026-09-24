// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Skip Mode

#include <utility>

#include "d_main.hpp"
#include "doomstat.hpp"
#include "e6y.hpp"
#include "i_main.hpp"
#include "i_sound.hpp"
#include "s_sound.hpp"
#include "smooth.hpp"
#include "v_video.hpp"
#include "gl_struct.hpp"

#include "dsda/args.hpp"
#include "dsda/build.hpp"
#include "dsda/demo.hpp"
#include "dsda/features.hpp"
#include "dsda/pause.hpp"
#include "dsda/playback.hpp"

#include "skip.hpp"

static dboolean skip_mode;

static int demo_skiptics;
static dboolean skip_until_next_map;
static dboolean skip_until_end_of_map;
static dboolean skip_until_logictic;
static dboolean demo_warp_reached;
static int skip_until_map;
static int skip_until_episode;

dboolean dsda_SkipMode()
{
	return skip_mode;
}

static void dsda_CacheSkipSetting(dboolean* old, dboolean* current)
{
	*old = *current;
	*current = true;
}

static dboolean old_fastdemo, old_nodrawers, old_nosfxparm, old_nomusicparm;

static void dsda_ApplySkipSettings()
{
	if(skip_mode)
		return;

	dsda_CacheSkipSetting(&old_fastdemo, &fastdemo);
	dsda_CacheSkipSetting(&old_nodrawers, &nodrawers);
	dsda_CacheSkipSetting(&old_nosfxparm, &nosfxparm);
	dsda_CacheSkipSetting(&old_nomusicparm, &nomusicparm);
}

static void dsda_ResetSkipSettings()
{
	fastdemo = old_fastdemo;
	nodrawers = old_nodrawers;
	nosfxparm = old_nosfxparm;
	nomusicparm = old_nomusicparm;
}

extern "C" void M_ClearMenus();
void dsda_EnterSkipMode()
{

	dsda_TrackFeature(FeatureFlag::Skip);
	dsda_ApplySkipSettings();

	skip_mode = true;

	M_ClearMenus();
	dsda_ResetPauseMode();
	S_StopMusic();
	I_Init2();

	if(dsda_BuildMode())
		dsda_ApplyPauseMode(PAUSE_BUILDMODE);
}

void dsda_ExitSkipMode()
{
	skip_mode = false;

	dsda_ResetSkipSettings();

	skip_until_next_map = false;
	skip_until_end_of_map = false;
	skip_until_logictic = 0;
	skip_until_map = -1;
	skip_until_episode = -1;
	demo_warp_reached = false;
	demo_skiptics = 0;

	I_Init2();
	I_InitSound();
	S_Init();
	S_RestartMusic();

	if(V_IsOpenGLMode())
		gld_PreprocessLevel();
}

void dsda_ToggleSkipMode()
{
	dsda_SkipMode() ? dsda_ExitSkipMode() : dsda_EnterSkipMode();
}

void dsda_SkipToNextMap()
{
	skip_until_next_map = true;
	dsda_EnterSkipMode();
}

void dsda_SkipToEndOfMap()
{
	skip_until_end_of_map = true;
	dsda_EnterSkipMode();
}

void dsda_SkipToLogicTic(int tic)
{
	skip_until_logictic = tic;
	dsda_EnterSkipMode();
}

void dsda_EvaluateSkipModeGTicker()
{
	if(dsda_SkipMode() && skip_until_logictic && skip_until_logictic <= true_logictic)
		dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeInitNew()
{
	if(dsda_SkipMode() && skip_until_map == gamemap && skip_until_episode == gameepisode)
		demo_warp_reached = true;
}

void dsda_EvaluateSkipModeBuildTiccmd()
{
	dboolean at_target_tic;

	if(!dsda_SkipMode() || gametic <= 0) return;

	at_target_tic = demo_skiptics > 0 ? gametic > demo_skiptics : dsda_DemoTic() - demo_skiptics >= demo_tics_count;
	if((!skip_until_logictic && skip_until_map == -1 && demo_skiptics && at_target_tic) ||
		(demo_warp_reached && at_target_tic))
		dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeDoCompleted()
{
	if(dsda_SkipMode() && (skip_until_end_of_map || demo_warp_reached))
		dsda_ExitSkipMode();
}

void dsda_EvaluateSkipModeDoTeleportNewMap()
{
	if(dsda_SkipMode())
	{
		static int firstmap = 1;

		demo_warp_reached = skip_until_next_map ||
		(
			gamemode == GameMode::Commercial ? (skip_until_map == gamemap) : (skip_until_episode == gameepisode && skip_until_map == gamemap)
		);

		if(demo_warp_reached && demo_skiptics == 0 && !firstmap)
			dsda_ExitSkipMode();

		firstmap = 0;
	}
}

void dsda_EvaluateSkipModeDoWorldDone()
{
	if(dsda_SkipMode())
	{
		static int firstmap = 1;

		demo_warp_reached = skip_until_next_map ||
		(
			gamemode == GameMode::Commercial ? (skip_until_map == gamemap) : (skip_until_episode == gameepisode && skip_until_map == gamemap)
		);

		if(demo_warp_reached && demo_skiptics == 0 && !firstmap)
			dsda_ExitSkipMode();

		firstmap = 0;
	}
}

void dsda_EvaluateSkipModeCheckDemoStatus()
{
	if(dsda_SkipMode() && (skip_until_end_of_map || skip_until_next_map))
		dsda_ExitSkipMode();
}

void dsda_HandleSkip()
{
	extern int warpmap;
	extern int warpepisode;

	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::Skipsec);
	if(arg->found)
	{
		float min, sec;

		if(sscanf(arg->value.v_string, "%f:%f", &min, &sec) == 2)
			demo_skiptics = (int)((60 * min + sec) * TICRATE);
		else if(sscanf(arg->value.v_string, "%f", &sec) == 1)
			demo_skiptics = (int)(sec * TICRATE);
	}

	arg = dsda_Arg(ArgId::Skiptic);
	if(arg->found)
		demo_skiptics = arg->value.v_int;

	if(dsda_PlaybackName() && (warpmap != -1 || demo_skiptics))
	{
		skip_until_map = warpmap;
		skip_until_episode = warpepisode;

		dsda_EnterSkipMode();
	}
}
