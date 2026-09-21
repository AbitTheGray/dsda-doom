// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Intermission screens.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//#include "v_video.hpp"

#include "doomdef.hpp"

// States for the intermission

typedef enum
{
	NoState = -1,
	StatCount,
	ShowNextLoc
} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker();

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer();

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t* wbstartstruct);

// Release intermission screen memory
void WI_End();

#ifdef __cplusplus
}
#endif
