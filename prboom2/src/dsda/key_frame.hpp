// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Key Frame

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

struct auto_kf_s;

typedef struct
{
	byte* buffer;
	struct auto_kf_s* auto_kf;
} parent_kf_t;

typedef struct
{
	byte* buffer;
	int buffer_length;
	int game_tic_count;
	parent_kf_t parent;
} dsda_key_frame_t;

typedef struct auto_kf_s
{
	int auto_index;
	dsda_key_frame_t kf;
	struct auto_kf_s* prev;
	struct auto_kf_s* next;
} auto_kf_t;

void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete, byte export_);
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame, dboolean skip_wipe);
dboolean dsda_RestoreClosestKeyFrame(int tic);
int dsda_KeyFrameRestored();
void dsda_ContinueKeyFrame();

void dsda_InitAutoKeyFrames();
void dsda_UpdateAutoKeyFrames();
void dsda_ForgetAutoKeyFrames();
void dsda_RewindAutoKeyFrame();
void dsda_ResetAutoKeyFrameTimeout();

void dsda_InitPlaybackKeyFrames();
void dsda_UpdatePlaybackKeyFrames();

void dsda_StoreTempKeyFrame();
void dsda_StoreQuickKeyFrame();
void dsda_RestoreQuickKeyFrame();

#ifdef __cplusplus
}
#endif
