// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Intermission screens.
 */

#pragma once

//#include "v_video.h"

#include "doomdef.h"

// States for the intermission

typedef enum
{
	NoState = -1,
	StatCount,
	ShowNextLoc
} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker(void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer(void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t* wbstartstruct);

// Release intermission screen memory
void WI_End(void);
