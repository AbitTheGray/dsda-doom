// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Skip Mode

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

dboolean dsda_SkipMode();
void dsda_EnterSkipMode();
void dsda_ExitSkipMode();
void dsda_ToggleSkipMode();
void dsda_SkipToNextMap();
void dsda_SkipToEndOfMap();
void dsda_SkipToLogicTic(int tic);
void dsda_EvaluateSkipModeGTicker();
void dsda_EvaluateSkipModeInitNew();
void dsda_EvaluateSkipModeBuildTiccmd();
void dsda_EvaluateSkipModeDoCompleted();
void dsda_EvaluateSkipModeDoTeleportNewMap();
void dsda_EvaluateSkipModeDoWorldDone();
void dsda_EvaluateSkipModeCheckDemoStatus();
void dsda_HandleSkip();

#ifdef __cplusplus
}
#endif
