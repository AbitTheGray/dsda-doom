// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Playback

#pragma once

#include <optional>
#include <string_view>

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define PLAYBACK_NORMAL      0
#define PLAYBACK_JOIN_ON_END 1

void dsda_RestartPlayback();
dboolean dsda_JumpToLogicTic(int tic);
dboolean dsda_JumpToLogicTicFrom(int tic, int from_tic);
void dsda_ExecutePlaybackOptions();
const char* dsda_ParsePlaybackOptions();
void dsda_ClearPlaybackStream();
void dsda_InitDemoPlayback();
void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour);
void dsda_StorePlaybackPosition();
void dsda_RestorePlaybackPosition();
void dsda_JoinDemo(ticcmd_t* cmd);
void dsda_TryPlaybackOneTick(ticcmd_t* cmd);

#ifdef __cplusplus
}
#endif

/// The demo to play back, as named on the command line (`-playdemo`, `-fastdemo`, ...); none when no demo is played back.
/// Set once while the command line is parsed at startup, so the view stays valid for the rest of the run.
std::optional<std::string_view> dsda_PlaybackName();
