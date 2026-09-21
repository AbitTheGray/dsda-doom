// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Event information structures.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


#include "doomtype.hpp"


//
// Event handling.
//

// Input event types.
typedef enum
{
	ev_keydown,
	ev_keyup,
	ev_mouse,
	ev_mousemotion,
	ev_joystick,
	ev_move_analog,
	ev_look_analog,
	ev_trigger,
	ev_text,
} evtype_t;

typedef union
{
	int i;
	float f;
} event_data_t;

typedef struct
{
	evtype_t type;
	event_data_t data1;
	event_data_t data2;
	char* text;
} event_t;


typedef enum
{
	ga_nothing,
	ga_loadlevel,
	ga_newgame,
	ga_loadgame,
	ga_playdemo,
	ga_completed,
	ga_victory,
	ga_worlddone,

	// hexen
	ga_leavemap
} gameaction_t;



//
// Button/action code definitions.
//
typedef enum
{
	BT_ATTACK = 1, // Press "Fire".
	BT_USE    = 2, // Use button, to open doors, activate switches.

	// Flag, weapon change pending.
	// If true, the next 4 bits hold weapon num.
	BT_CHANGE = 4,

	// The 4bit weapon mask and shift, convenience.
	BT_WEAPONMASK_OLD = (8 + 16 + 32),      // e6y
	BT_WEAPONMASK     = (8 + 16 + 32 + 64), // extended to pick up SSG // phares
	BT_WEAPONSHIFT    = 3,

	// Special events
	BT_SPECIAL     = 128,
	BT_SPECIALMASK = 3,
	BT_PAUSE       = 1,  // Pause the game.
	BT_JOIN        = 64, // Demo joined.
} buttoncode_t;


//
// GLOBAL VARIABLES
//

extern gameaction_t gameaction;

#ifdef __cplusplus
}
#endif
