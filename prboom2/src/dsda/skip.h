// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Skip Mode

#pragma once

#include "doomtype.h"

dboolean dsda_SkipMode(void);
void dsda_EnterSkipMode(void);
void dsda_ExitSkipMode(void);
void dsda_ToggleSkipMode(void);
void dsda_SkipToNextMap(void);
void dsda_SkipToEndOfMap(void);
void dsda_SkipToLogicTic(int tic);
void dsda_EvaluateSkipModeGTicker(void);
void dsda_EvaluateSkipModeInitNew(void);
void dsda_EvaluateSkipModeBuildTiccmd(void);
void dsda_EvaluateSkipModeDoCompleted(void);
void dsda_EvaluateSkipModeDoTeleportNewMap(void);
void dsda_EvaluateSkipModeDoWorldDone(void);
void dsda_EvaluateSkipModeCheckDemoStatus(void);
void dsda_HandleSkip(void);
