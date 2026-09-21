// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Playback

#include "doomtype.h"

#define PLAYBACK_NORMAL      0
#define PLAYBACK_JOIN_ON_END 1

void dsda_RestartPlayback(void);
dboolean dsda_JumpToLogicTic(int tic);
dboolean dsda_JumpToLogicTicFrom(int tic, int from_tic);
void dsda_ExecutePlaybackOptions(void);
const char* dsda_ParsePlaybackOptions(void);
const char* dsda_PlaybackName(void);
void dsda_ClearPlaybackStream(void);
void dsda_InitDemoPlayback(void);
void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour);
void dsda_StorePlaybackPosition(void);
void dsda_RestorePlaybackPosition(void);
void dsda_JoinDemo(ticcmd_t* cmd);
void dsda_TryPlaybackOneTick(ticcmd_t* cmd);
