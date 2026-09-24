// SPDX-License-Identifier: GPL-2.0-or-later

// SB_bar.h

#pragma once

#include "d_event.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define STARTREDPALS     1
#define STARTBONUSPALS   9
#define STARTPOISONPALS 13
#define STARTICEPAL     21
#define STARTHOLYPAL    22
#define STARTSCOURGEPAL 25
#define NUMREDPALS       8
#define NUMBONUSPALS     4
#define NUMPOISONPALS    8

void SB_Start();
void SB_Init();
void SB_Ticker();
void SB_Drawer(dboolean statusbaron, dboolean refresh);
void SB_PaletteFlash(dboolean forceChange);

// hexen

void SB_SetClassData();

#ifdef __cplusplus
}
#endif
