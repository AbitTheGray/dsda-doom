// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  Head up display
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_event.hpp"
#include "d_think.hpp"
#include "p_mobj.hpp"
#include "r_defs.hpp"

#define HU_MSGTIMEOUT   (4*TICRATE)

#define HU_CROSSHAIRS	8

void HU_Start();

dboolean HU_Responder(event_t* ev);

void HU_Ticker();
void HU_Drawer();

mobj_t* HU_Target();

int SetCustomMessage(int plr, const char* msg, int ticks, int sfx);

extern int hud_health_red;    // health amount less than which status is red
extern int hud_health_yellow; // health amount less than which status is yellow
extern int hud_health_green;  // health amount above is blue, below is green
extern int hud_ammo_red;      // ammo percent less than which status is red
extern int hud_ammo_yellow;   // ammo percent less is yellow more green

#ifdef __cplusplus
}
#endif
