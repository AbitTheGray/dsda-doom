// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Playback

#include "doomstat.hpp"
#include "g_game.hpp"
#include "i_system.hpp"
#include "lprintf.hpp"
#include "p_saveg.hpp"
#include "w_wad.hpp"

#include "dsda/args.hpp"
#include "dsda/demo.hpp"
#include "dsda/exdemo.hpp"
#include "dsda/input.hpp"
#include "dsda/key_frame.hpp"
#include "dsda/skip.hpp"

#include "playback.hpp"

static const byte* playback_origin_p;
static const byte* playback_p;
static int playback_length;
static int playback_behaviour;

static dsda_arg_t* playdemo_arg;
static dsda_arg_t* playlump_arg;
static dsda_arg_t* fastdemo_arg;
static dsda_arg_t* timedemo_arg;
static dsda_arg_t* recordfromto_arg;
static char* playback_name;
static char* playback_filename;

extern int demo_tics;

dboolean demoplayback;
dboolean userdemo;

void dsda_RestartPlayback()
{
	G_StartDemoPlayback(playback_origin_p, playback_length, playback_behaviour);
}

dboolean dsda_JumpToLogicTic(int tic)
{
	if(tic < 0)
		return false;

	if(!dsda_RestoreClosestKeyFrame(tic))
		return false;

	if(tic != true_logictic)
		dsda_SkipToLogicTic(tic);

	return true;
}

dboolean dsda_JumpToLogicTicFrom(int tic, int from_tic)
{
	if(tic < 0 || tic > true_logictic)
		return false;

	if(!dsda_RestoreClosestKeyFrame(from_tic))
		return false;

	if(tic != true_logictic)
		dsda_SkipToLogicTic(tic);

	return true;
}

std::optional<std::string_view> dsda_PlaybackName()
{
	if(!playback_name)
		return std::nullopt;

	return playback_name;
}

void dsda_ExecutePlaybackOptions()
{
	if(playlump_arg)
	{
		if(W_CheckNumForName(playback_name) == LUMP_NOT_FOUND)
			I_Error("Unable to find required internal demo lump \"%s\"", playback_name);
	}

	if(playdemo_arg)
	{
		G_DeferedPlayDemo(playback_name);
		userdemo = true;
	}
	else if(fastdemo_arg)
	{
		G_DeferedPlayDemo(playback_name);
		fastdemo = true;
		timingdemo = true;
		userdemo = true;
	}
	else if(timedemo_arg)
	{
		G_DeferedPlayDemo(playback_name);
		singletics = true;
		timingdemo = true;
		userdemo = true;
	}
	else if(recordfromto_arg)
	{
		userdemo = true;
		G_ContinueDemo(playback_name);
	}
}

static void dsda_UpdatePlaybackName(const char* name, dboolean require_file)
{
	if(playback_name)
		Z_Free(playback_name);

	if(playback_filename)
		Z_Free(playback_filename);

	playback_name = Z_Strdup(name);

	if(require_file)
		playback_filename = I_FindFile(playback_name, ".lmp");
	else
		playback_filename = nullptr;
}

const char* dsda_ParsePlaybackOptions()
{
	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::Playdemo);
	if(arg->found)
	{
		playdemo_arg = arg;
		dsda_UpdatePlaybackName(arg->value.v_string, true);
		// fall back to lump if file not found
		if(!playback_filename)
			playlump_arg = arg;
		return playback_filename;
	}

	arg = dsda_Arg(ArgId::Playlump);
	if(arg->found)
	{
		playlump_arg = arg;
		dsda_UpdatePlaybackName(arg->value.v_string, false);
		playdemo_arg = arg;
		return playback_filename;
	}

	arg = dsda_Arg(ArgId::Fastdemo);
	if(arg->found)
	{
		fastdemo_arg = arg;
		fastdemo = true;
		dsda_UpdatePlaybackName(arg->value.v_string, true);
		// fall back to lump if file not found
		if(!playback_filename)
			playlump_arg = arg;
		return playback_filename;
	}

	arg = dsda_Arg(ArgId::Timedemo);
	if(arg->found)
	{
		timedemo_arg = arg;
		dsda_UpdatePlaybackName(arg->value.v_string, true);
		// fall back to lump if file not found
		if(!playback_filename)
			playlump_arg = arg;
		return playback_filename;
	}

	arg = dsda_Arg(ArgId::Recordfromto);
	if(arg->found)
	{
		recordfromto_arg = arg;
		dsda_SetDemoBaseName(arg->value.v_string_array[1]);
		dsda_UpdatePlaybackName(arg->value.v_string_array[0], true);
		// require a file
		if(!playback_filename)
			playback_filename = I_RequireFile(arg->value.v_string_array[0], ".lmp");
		return playback_filename;
	}

	return nullptr;
}

void dsda_InitDemoPlayback()
{
	demoplayback = true;
}

void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour)
{
	playback_origin_p = demo_p;
	playback_p = demo_p;
	playback_length = length;
	playback_behaviour = behaviour;
	demo_tics = 0;
}

void dsda_StorePlaybackPosition()
{
	P_SAVE_X(demo_tics);
	P_SAVE_X(playback_p);
}

void dsda_RestorePlaybackPosition()
{
	P_LOAD_X(demo_tics);
	P_LOAD_X(playback_p);
}

void dsda_ClearPlaybackStream()
{
	playback_origin_p = nullptr;
	playback_p = nullptr;
	playback_length = 0;
	playback_behaviour = 0;
	demo_tics = 0;

	demoplayback = false;
	userdemo = false;
}

static dboolean dsda_EndOfPlaybackStream()
{
	return *playback_p == DEMOMARKER ||
		playback_p + dsda_BytesPerTic() > playback_origin_p + playback_length;
}

void dsda_JoinDemo(ticcmd_t* cmd)
{
	if(!demoplayback)
		return;

	if(dsda_SkipMode())
		dsda_ExitSkipMode();

	if(demorecording)
		dsda_WriteQueueToDemo(playback_p, playback_length - (playback_p - playback_origin_p));

	dsda_ClearPlaybackStream();

	if(cmd)
		dsda_JoinDemoCmd(cmd);
	else
		dsda_QueueJoin();

	dsda_MergeExDemoFeatures();
}

void dsda_TryPlaybackOneTick(ticcmd_t* cmd)
{
	dboolean ended = false;

	if(!playback_p)
		return;

	if(dsda_EndOfPlaybackStream())
		ended = true;
	else
	{
		G_ReadOneTick(cmd, &playback_p);

		++demo_tics;
	}

	if(ended)
	{
		if(playback_behaviour & PLAYBACK_JOIN_ON_END)
			dsda_JoinDemo(cmd);
		else
			G_CheckDemoStatus();
	}
	else if(dsda_InputActive(InputId::JoinDemo) || dsda_InputJoyBActive(InputId::Use))
		dsda_JoinDemo(cmd);
}
