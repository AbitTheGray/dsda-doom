// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Status bar code.
 *      Does the face/direction indicator animatin.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 */

#pragma once

#include "doomtype.h"
#include "d_event.h"
#include "r_defs.h"

// Size of statusbar.
// Now sensitive for scaling.

// proff 08/18/98: Changed for high-res
#define ST_HEIGHT 32
#define ST_WIDTH  320
#define ST_Y      (200 - ST_HEIGHT)

// e6y: wide-res
extern int ST_SCALED_HEIGHT;
extern int ST_SCALED_WIDTH;
extern int ST_SCALED_Y;
extern int ST_SCALED_OFFSETX;

void ST_SetScaledWidth(void);
void ST_LoadTextColors(void);

//
// STATUS BAR
//

// Called by main loop.
dboolean ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(dboolean refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// After changing videomode;
void ST_SetResolution(void);

void ST_Refresh(void);

int ST_HealthColor(int health);

// States for status bar code.
typedef enum
{
	AutomapState,
	FirstPersonState
} st_stateenum_t;

extern int st_palette; // cph 2006/04/06 - make palette visible

// e6y: makes sense for wide resolutions
extern patchnum_t grnrock;
extern patchnum_t brdr_b;
