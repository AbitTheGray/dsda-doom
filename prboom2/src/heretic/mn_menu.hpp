// SPDX-License-Identifier: GPL-2.0-or-later

// MN_menu.h

#pragma once

// declared in v_video.hpp; the fixed underlying type makes this enough
enum struct ColorRange : int32_t;

#ifdef __cplusplus
extern "C"
{
#endif

void MN_Init();
void MN_Ticker();
void MN_Drawer();
void MN_DrawMainMenu();
void MN_DrawOptions();
void MN_DrawSetup();
void MN_DrawMouse();
void MN_DrawSound();
void MN_DrawLoad();
void MN_DrawSave();
void MN_DrawPause();
void MN_DrawMessage(const char* messageString);
void MN_DrawSlider(int x, int y, int width, int range, int slot, ColorRange color);
void MN_DrawTitle(int y, const char* text, ColorRange cm);
void MN_DrTextA(const char* text, int x, int y);
int MN_TextAHeight(const char* text);
int MN_TextAWidth(const char* text);
void MN_DrTextB(const char* text, int x, int y);
int MN_TextBWidth(const char* text);

// hexen

void MN_DrawEpisode();
void MN_UpdateClass(int choice);
void MN_DrTextAYellow(const char* text, int x, int y);
void MN_DrawSkillMenu();

#ifdef __cplusplus
}
#endif
