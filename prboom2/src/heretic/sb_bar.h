// SPDX-License-Identifier: GPL-2.0-or-later

// SB_bar.h

#ifndef __SB_BAR__
#define __SB_BAR__

#include "d_event.h"

#define STARTREDPALS     1
#define STARTBONUSPALS   9
#define STARTPOISONPALS 13
#define STARTICEPAL     21
#define STARTHOLYPAL    22
#define STARTSCOURGEPAL 25
#define NUMREDPALS       8
#define NUMBONUSPALS     4
#define NUMPOISONPALS    8

void SB_Start(void);
void SB_Init(void);
void SB_Ticker(void);
void SB_Drawer(dboolean statusbaron, dboolean refresh);
void SB_PaletteFlash(dboolean forceChange);

// hexen

void SB_SetClassData(void);

#endif
