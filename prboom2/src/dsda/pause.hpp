// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Pause Mode

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define PAUSE_COMMAND   1
#define PAUSE_PLAYBACK  2
#define PAUSE_BUILDMODE 4

dboolean dsda_Paused();
dboolean dsda_PausedViaMenu();
dboolean dsda_PausedOutsideDemo();
dboolean dsda_CameraPaused();
dboolean dsda_PauseMode(int mode);
void dsda_RemovePauseMode(int mode);
void dsda_ApplyPauseMode(int mode);
void dsda_TogglePauseMode(int mode);
void dsda_ResetPauseMode();
int dsda_MaskPause();
void dsda_UnmaskPause(int mask);

#ifdef __cplusplus
}
#endif
