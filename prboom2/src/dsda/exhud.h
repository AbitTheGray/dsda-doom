// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended HUD

#ifndef __DSDA_EXHUD__
#define __DSDA_EXHUD__

void dsda_InitExHud(void);
void dsda_UpdateExHud(void);
void dsda_DrawExHud(void);
void dsda_DrawExIntermission(void);
void dsda_ToggleRenderStats(void);
void dsda_RefreshExHudFPS(void);
void dsda_RefreshExHudMinimap(void);
void dsda_RefreshExHudLevelSplits(void);
void dsda_RefreshExHudCoordinateDisplay(void);
void dsda_RefreshExHudCommandDisplay(void);
void dsda_RefreshMapCoordinates(void);
void dsda_RefreshMapTotals(void);
void dsda_RefreshMapTime(void);
void dsda_RefreshMapTitle(void);

#endif
