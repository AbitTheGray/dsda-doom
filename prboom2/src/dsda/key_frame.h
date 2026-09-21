// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Key Frame

#pragma once

#include "doomtype.h"

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

void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete, byte export);
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame, dboolean skip_wipe);
dboolean dsda_RestoreClosestKeyFrame(int tic);
int dsda_KeyFrameRestored(void);
void dsda_ContinueKeyFrame(void);

void dsda_InitAutoKeyFrames(void);
void dsda_UpdateAutoKeyFrames(void);
void dsda_ForgetAutoKeyFrames(void);
void dsda_RewindAutoKeyFrame(void);
void dsda_ResetAutoKeyFrameTimeout(void);

void dsda_InitPlaybackKeyFrames(void);
void dsda_UpdatePlaybackKeyFrames(void);

void dsda_StoreTempKeyFrame(void);
void dsda_StoreQuickKeyFrame(void);
void dsda_RestoreQuickKeyFrame(void);
