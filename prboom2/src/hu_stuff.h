// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  Head up display
 */

#pragma once

#include "d_event.h"
#include "d_think.h"
#include "p_mobj.h"
#include "r_defs.h"

#define HU_MSGTIMEOUT   (4*TICRATE)

#define HU_CROSSHAIRS	8

void HU_Start(void);

dboolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);

mobj_t *HU_Target(void);

int SetCustomMessage(int plr, const char *msg, int ticks, int sfx);

extern int hud_health_red;    // health amount less than which status is red
extern int hud_health_yellow; // health amount less than which status is yellow
extern int hud_health_green;  // health amount above is blue, below is green
extern int hud_ammo_red;      // ammo percent less than which status is red
extern int hud_ammo_yellow;   // ammo percent less is yellow more green
