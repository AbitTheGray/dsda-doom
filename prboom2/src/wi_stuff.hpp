// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Intermission screens.
 */

#pragma once

#include "doomdef.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

//#include "v_video.hpp"

// States for the intermission

enum struct WiState : int32_t
{
	NoState = -1,
	StatCount,
	ShowNextLoc
};

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
