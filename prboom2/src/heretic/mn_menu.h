// SPDX-License-Identifier: GPL-2.0-or-later

// MN_menu.h

#pragma once

void MN_Init(void);
void MN_Ticker(void);
void MN_Drawer(void);
void MN_DrawMainMenu(void);
void MN_DrawOptions(void);
void MN_DrawSetup(void);
void MN_DrawMouse(void);
void MN_DrawSound(void);
void MN_DrawLoad(void);
void MN_DrawSave(void);
void MN_DrawPause(void);
void MN_DrawMessage(const char* messageString);
void MN_DrawSlider(int x, int y, int width, int range, int slot, int color);
void MN_DrawTitle(int y, const char* text, int cm);
void MN_DrTextA(const char* text, int x, int y);
int MN_TextAHeight(const char* text);
int MN_TextAWidth(const char* text);
void MN_DrTextB(const char* text, int x, int y);
int MN_TextBWidth(const char* text);

// hexen

void MN_DrawEpisode(void);
void MN_UpdateClass(int choice);
void MN_DrTextAYellow(const char* text, int x, int y);
void MN_DrawSkillMenu(void);
