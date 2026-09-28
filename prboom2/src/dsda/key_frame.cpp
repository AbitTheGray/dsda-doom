// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Key Frame

#include <utility>

#include <time.h>

#include "doomstat.hpp"
#include "s_advsound.hpp"
#include "s_sound.hpp"
#include "st_stuff.hpp"
#include "p_saveg.hpp"
#include "p_map.hpp"
#include "r_draw.hpp"
#include "r_fps.hpp"
#include "r_main.hpp"
#include "g_game.hpp"
#include "m_file.hpp"
#include "i_system.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"

#include "heretic/sb_bar.hpp"

#include "dsda.hpp"
#include "dsda/args.hpp"
#include "dsda/build.hpp"
#include "dsda/configuration.hpp"
#include "dsda/demo.hpp"
#include "dsda/features.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"
#include "dsda/options.hpp"
#include "dsda/pause.hpp"
#include "dsda/playback.hpp"
#include "dsda/save.hpp"
#include "dsda/settings.hpp"
#include "dsda/time.hpp"

#include "key_frame.hpp"

static dboolean auto_kf_timed_out;
static int auto_kf_timeout_count;

#define TIMEOUT_LIMIT 1

static dsda_key_frame_t first_kf;
static dsda_key_frame_t temp_kf;
dsda_key_frame_t quick_kf;
auto_kf_t* auto_key_frames;
int auto_kf_size = 0;
static auto_kf_t* last_auto_kf;
dsda_key_frame_t* playback_key_frames;
int playback_kf_size = 0;
static int restore_key_frame_index = -1;

static int dsda_auto_key_frame_interval;
static int dsda_auto_key_frame_depth;
static int dsda_auto_key_frame_timeout;

static int autoKeyFrameTimeout()
{
	return dsda_StartInBuildMode() ? 0 : dsda_auto_key_frame_timeout;
}

static int autoKeyFrameDepth()
{
	if(dsda_StartInBuildMode() && dsda_auto_key_frame_depth < 60)
		return 60;

	return dsda_auto_key_frame_depth;
}

static int autoKeyFrameInterval()
{
	if(dsda_StartInBuildMode())
		return 1;

	return dsda_auto_key_frame_interval;
}

static dboolean autoKFExists(auto_kf_t* auto_kf)
{
	return auto_kf && auto_kf->auto_index && auto_kf->kf.buffer;
}

void dsda_ForgetAutoKeyFrames()
{
	if(last_auto_kf)
		last_auto_kf->auto_index = 0;
}

static void dsda_ResetParentKF(dsda_key_frame_t* kf)
{
	kf->parent.auto_kf = nullptr;
	kf->parent.buffer = nullptr;
}

static void dsda_AttachAutoKF(dsda_key_frame_t* kf)
{
	if(autoKFExists(last_auto_kf))
	{
		kf->parent.auto_kf = last_auto_kf;
		kf->parent.buffer = last_auto_kf->kf.buffer;
	}
	else
		dsda_ResetParentKF(kf);
}

static void dsda_ResolveParentKF(dsda_key_frame_t* kf)
{
	if(autoKFExists(kf->parent.auto_kf) && kf->parent.auto_kf->kf.buffer == kf->parent.buffer)
		last_auto_kf = kf->parent.auto_kf;
	else
	{
		dsda_ResetParentKF(kf);
		dsda_ForgetAutoKeyFrames();
	}
}

static void dsda_RewindKF(auto_kf_t** current)
{
	auto_kf_t* auto_kf;

	auto_kf = *current;

	if(auto_kf &&
		auto_kf->auto_index && auto_kf->prev->auto_index &&
		auto_kf->auto_index == auto_kf->prev->auto_index + 1)
		*current = auto_kf->prev;
	else
		*current = nullptr;
}

static dsda_key_frame_t* dsda_ClosestKeyFrame(int target_tic_count)
{
	dsda_key_frame_t* closest = nullptr;

	if(auto_key_frames)
		for(int i = 0; i < auto_kf_size; i++)
			if(auto_key_frames[i].kf.game_tic_count <= target_tic_count)
				if(!closest || auto_key_frames[i].kf.game_tic_count > closest->game_tic_count)
					closest = &auto_key_frames[i].kf;

	if(playback_key_frames)
		for(int i = 0; i < playback_kf_size; i++)
			if(playback_key_frames[i].game_tic_count <= target_tic_count)
				if(!closest || playback_key_frames[i].game_tic_count > closest->game_tic_count)
					closest = &playback_key_frames[i];

	if(!demorecording && temp_kf.buffer)
		if(temp_kf.game_tic_count <= target_tic_count)
			if(!closest || temp_kf.game_tic_count > closest->game_tic_count)
				closest = &temp_kf;

	if(!demorecording && quick_kf.buffer)
		if(quick_kf.game_tic_count <= target_tic_count)
			if(!closest || quick_kf.game_tic_count > closest->game_tic_count)
				closest = &quick_kf;

	if(first_kf.buffer)
		if(first_kf.game_tic_count <= target_tic_count)
			if(!closest || first_kf.game_tic_count > closest->game_tic_count)
				closest = &first_kf;

	return closest;
}

void dsda_CopyKeyFrame(dsda_key_frame_t* dest, dsda_key_frame_t* source)
{
	*dest = *source;
	dest->buffer = static_cast<decltype(dest->buffer)>(Z_Malloc(dest->buffer_length));
	memcpy(dest->buffer, source->buffer, dest->buffer_length);
}

void dsda_InitAutoKeyFrames()
{
	int i;

	dsda_auto_key_frame_interval = dsda_IntConfig(ConfigId::AutoKeyFrameInterval);
	dsda_auto_key_frame_depth = dsda_IntConfig(ConfigId::AutoKeyFrameDepth);
	dsda_auto_key_frame_timeout = dsda_IntConfig(ConfigId::AutoKeyFrameTimeout);

	auto_kf_size = autoKeyFrameDepth();

	if(!auto_kf_size)
	{
		last_auto_kf = nullptr;
		return;
	}

	++auto_kf_size; // chain includes a terminator

	if(auto_key_frames != nullptr)
		Z_Free(auto_key_frames);

	auto_key_frames = static_cast<decltype(auto_key_frames)>(Z_Calloc(auto_kf_size, sizeof(auto_kf_t)));

	auto_key_frames[0].prev = &auto_key_frames[auto_kf_size - 1];
	auto_key_frames[auto_kf_size - 1].next = &auto_key_frames[0];

	for(i = 0; i < auto_kf_size - 1; ++i)
		auto_key_frames[i].next = &auto_key_frames[i + 1];

	for(i = 1; i < auto_kf_size; ++i)
		auto_key_frames[i].prev = &auto_key_frames[i - 1];

	last_auto_kf = &auto_key_frames[auto_kf_size - 1];
}

void dsda_InitPlaybackKeyFrames()
{
	// Max of 60 keyframes are saved, and they need to have 1 minute in between each
	playback_kf_size = demo_tics_count * demo_playerscount / TICRATE / 60;
	if(playback_kf_size > 60)
		playback_kf_size = 60;

	if(playback_key_frames != nullptr)
		Z_Free(playback_key_frames);

	playback_key_frames = static_cast<decltype(playback_key_frames)>(Z_Calloc(playback_kf_size, sizeof(dsda_key_frame_t)));
}

void dsda_ExportKeyFrame(byte* buffer, int length)
{
	char name[40];
	int timestamp;

	timestamp = totalleveltimes + leveltime;

	snprintf(name, sizeof(name), "backup-%010d.kf", timestamp);

	if(M_FileExists(name))
		snprintf(name, sizeof(name), "backup-%010d-%lld.kf", timestamp, (long long)time(nullptr));

	if(!M_WriteFile(name, buffer, length))
		Log::Fatal("dsda_ExportKeyFrame: Failed to write key frame.");
}

// Stripped down version of G_DoSaveGame
void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete, byte export_)
{
	key_frame->game_tic_count = true_logictic;

	P_InitSaveBuffer();

	P_SAVE_BYTE(complete);
	P_SAVE_X(key_frame->game_tic_count);

	// Store state of demo playback buffer
	dsda_StorePlaybackPosition();

	// Store state of demo recording buffer
	dsda_StoreDemoData(complete);

	dsda_ArchiveAll();

	if(key_frame->buffer != nullptr) Z_Free(key_frame->buffer);

	key_frame->buffer = savebuffer;
	key_frame->buffer_length = save_p - savebuffer;

	P_ForgetSaveBuffer();

	dsda_AttachAutoKF(key_frame);

	if(complete)
	{
		if(demorecording && export_)
			dsda_ExportKeyFrame(key_frame->buffer, key_frame->buffer_length);

		Message::Add("Stored key frame");
	}
}

// Stripped down version of G_DoLoadGame
extern "C" void G_AfterLoad();
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame, dboolean skip_wipe)
{

	byte complete;

	if(key_frame->buffer == nullptr)
	{
		Message::Add("No key frame found");
		return;
	}

	dsda_TrackFeature(FeatureFlag::Keyframe);

	if(skip_wipe || dsda_BuildMode())
		dsda_SkipNextWipe();

	save_p = key_frame->buffer;

	P_LOAD_BYTE(complete);
	P_LOAD_X(key_frame->game_tic_count);

	// Restore state of demo playback buffer
	dsda_RestorePlaybackPosition();

	// Restore state of demo recording buffer
	dsda_RestoreDemoData(complete);

	dsda_UnArchiveAll();

	dsda_RestoreCommandHistory();

	restore_key_frame_index = (totalleveltimes + leveltime) / (TICRATE * autoKeyFrameInterval());

	G_AfterLoad();

	dsda_QueueJoin();

	dsda_ResolveParentKF(key_frame);

	Message::Add("Restored key frame");
}

void dsda_StoreTempKeyFrame()
{
	dsda_StoreKeyFrame(&temp_kf, true, true);
}

void dsda_StoreQuickKeyFrame()
{
	dsda_StoreKeyFrame(&quick_kf, true, true);
}

void dsda_RestoreQuickKeyFrame()
{
	dsda_RestoreKeyFrame(&quick_kf, false);
}

dboolean dsda_RestoreClosestKeyFrame(int tic)
{
	dsda_key_frame_t* key_frame;

	key_frame = dsda_ClosestKeyFrame(tic);

	if(!key_frame)
		return false;

	dsda_RestoreKeyFrame(key_frame, true);

	return true;
}

void dsda_RestoreKeyFrameFile(const char* name)
{
	char* filename;
	dsda_key_frame_t key_frame = {nullptr};

	filename = I_RequireFile(name, ".kf");
	M_ReadFile(filename, &key_frame.buffer);
	Z_Free(filename);

	dsda_RestoreKeyFrame(&key_frame, false);
	Z_Free(key_frame.buffer);
}

void dsda_ContinueKeyFrame()
{
	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::FromKeyFrame);
	if(arg->found)
	{
		dsda_RestoreKeyFrameFile(arg->value.v_string);
	}
}

void dsda_RewindAutoKeyFrame()
{
	auto_kf_t* load_kf;

	load_kf = last_auto_kf;
	dsda_RewindKF(&load_kf);

	if(load_kf)
		dsda_RestoreKeyFrame(&load_kf->kf, true);
	else
		Message::Add("No key frame found"); // rewind past the depth limit
}

void dsda_ResetAutoKeyFrameTimeout()
{
	auto_kf_timed_out = false;
	auto_kf_timeout_count = 0;
}

void dsda_UpdateAutoKeyFrames()
{
	int key_frame_index;
	int current_time;
	int interval_tics;
	dsda_key_frame_t* current_key_frame;

	if(
		auto_kf_timed_out ||
		auto_kf_size == 0 ||
		gamestate != GameState::Level ||
		gameaction != GameAction::Nothing
	)
		return;

	current_time = totalleveltimes + leveltime;
	interval_tics = TICRATE * autoKeyFrameInterval();

	// Automatically save a key frame each interval
	if(current_time % interval_tics == 0)
	{
		key_frame_index = current_time / interval_tics;

		// Don't duplicate on rewind
		if(key_frame_index == restore_key_frame_index)
		{
			restore_key_frame_index = -1;
			return;
		}

		last_auto_kf = last_auto_kf->next;
		last_auto_kf->next->auto_index = 0;
		last_auto_kf->auto_index = last_auto_kf->prev->auto_index + 1;

		current_key_frame = &last_auto_kf->kf;

		{
			unsigned long long elapsed_time;

			dsda_StartTimer(DsdaTimer::KeyFrame);
			dsda_StoreKeyFrame(current_key_frame, false, false);
			elapsed_time = dsda_ElapsedTimeMS(DsdaTimer::KeyFrame);

			if(autoKeyFrameTimeout())
			{
				if(elapsed_time > autoKeyFrameTimeout())
				{
					++auto_kf_timeout_count;

					if(auto_kf_timeout_count > TIMEOUT_LIMIT)
					{
						auto_kf_timed_out = true;
						Message::Add("Slow key framing: rewind disabled");
					}
				}
				else
					auto_kf_timeout_count = 0;
			}
		}

		if(!first_kf.buffer)
			dsda_CopyKeyFrame(&first_kf, current_key_frame);
	}
}

void dsda_UpdatePlaybackKeyFrames()
{
	int current_time;
	int interval_tics;

	if(gameaction != GameAction::Nothing || playback_kf_size == 0) return;

	current_time = totalleveltimes + leveltime;
	interval_tics = (demo_tics_count * demo_playerscount) / playback_kf_size;

	// Automatically save a key frame each interval
	if(current_time % interval_tics == 0 &&
		current_time / interval_tics < playback_kf_size)
	{
		dsda_key_frame_t* current_key_frame = &playback_key_frames[current_time / interval_tics];

		if(!current_key_frame->buffer)
			dsda_StoreKeyFrame(current_key_frame, false, false);
	}
}
