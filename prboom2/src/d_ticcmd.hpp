// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  System specific interface stuff.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

typedef struct
{
	byte actions;
	byte save_slot;
	byte load_slot;
	signed short look;
} excmd_t;

/* The data sampled per tick (single player)
 * and transmitted to other peers (multiplayer).
 * Mainly movements/button commands per game tick,
 * plus a checksum for internal state consistency.
 * CPhipps - explicitely signed the elements, since they have to be signed to work right
 */
typedef struct
{
	signed char forwardmove; /* *2048 for move       */
	signed char sidemove;    /* *2048 for move       */
	signed short angleturn;  /* <<16 for angle delta */
	byte buttons;

	// heretic
	byte lookfly; // look/fly up/down/centering
	byte arti;    // artitype_t to use

	// dsda extension
	excmd_t ex;
} ticcmd_t;

#ifdef __cplusplus
}
#endif
