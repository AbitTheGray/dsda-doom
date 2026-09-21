// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended HUD

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitExHud();
void dsda_UpdateExHud();
void dsda_DrawExHud();
void dsda_DrawExIntermission();
void dsda_ToggleRenderStats();
void dsda_RefreshExHudFPS();
void dsda_RefreshExHudMinimap();
void dsda_RefreshExHudLevelSplits();
void dsda_RefreshExHudCoordinateDisplay();
void dsda_RefreshExHudCommandDisplay();
void dsda_RefreshMapCoordinates();
void dsda_RefreshMapTotals();
void dsda_RefreshMapTime();
void dsda_RefreshMapTitle();

#ifdef __cplusplus
}
#endif
