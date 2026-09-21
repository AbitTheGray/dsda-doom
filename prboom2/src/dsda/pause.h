// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Pause Mode

#pragma once

#include "doomtype.h"

#define PAUSE_COMMAND   1
#define PAUSE_PLAYBACK  2
#define PAUSE_BUILDMODE 4

dboolean dsda_Paused(void);
dboolean dsda_PausedViaMenu(void);
dboolean dsda_PausedOutsideDemo(void);
dboolean dsda_CameraPaused(void);
dboolean dsda_PauseMode(int mode);
void dsda_RemovePauseMode(int mode);
void dsda_ApplyPauseMode(int mode);
void dsda_TogglePauseMode(int mode);
void dsda_ResetPauseMode(void);
int dsda_MaskPause(void);
void dsda_UnmaskPause(int mask);
