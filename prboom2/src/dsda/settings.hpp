// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Settings

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

#define UNSPECIFIED_COMPLEVEL -2

void dsda_InitSettings();
int dsda_CompatibilityLevel();
void dsda_SetTas(dboolean t);
int dsda_ViewBob();
int dsda_WeaponBob();
dboolean dsda_FixViewBobFloorJolt();
dboolean dsda_ShowMessages();
dboolean dsda_AutoRun();
dboolean dsda_MouseLook();
dboolean dsda_VertMouse();
dboolean dsda_StrictMode();
dboolean dsda_MuteSfx();
dboolean dsda_MuteMusic();
dboolean dsda_ProcessCheatCodes();
dboolean dsda_CycleGhostColors();
dboolean dsda_AlwaysSR50();
dboolean dsda_HideHorns();
dboolean dsda_HideWeapon();
dboolean dsda_SwitchWhenAmmoRunsOut();
dboolean dsda_SkipQuitPrompt();
dboolean dsda_TrackSplits();
dboolean dsda_ShowSplitData();
dboolean dsda_CommandDisplay();
dboolean dsda_CoordinateDisplay();
dboolean dsda_ShowFPS();
dboolean dsda_ShowMinimap();
dboolean dsda_ShowLevelSplits();
dboolean dsda_ShowDemoAttempts();
dboolean dsda_ShowHealthBars();
dboolean dsda_MapCoordinates();
dboolean dsda_MapTotals();
dboolean dsda_MapTime();
dboolean dsda_MapTitle();
dboolean dsda_PainPalette();
dboolean dsda_BonusPalette();
dboolean dsda_PowerPalette();
dboolean dsda_RenderWipeScreen();
dboolean dsda_WipeAtFullSpeed();
int dsda_ShowAliveMonsters();
int dsda_CycleShowAliveMonsters();
int dsda_RevealAutomap();
void dsda_ResetRevealMap();
int dsda_GameSpeed();
void dsda_UpdateGameSpeed(int value);
int dsda_AutoKeyFrameInterval();
int dsda_AutoKeyFrameDepth();
void dsda_SkipNextWipe();
dboolean dsda_PendingSkipWipe();
dboolean dsda_SkipWipe();

dboolean dsda_AllowGameController();
dboolean dsda_AllowMouse();
void dsda_WatchGameControllerEvent();
void dsda_WatchMouseEvent();
void dsda_LiftInputRestrictions();

#ifdef __cplusplus
}
#endif
