// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Core thinker processing prototypes.
 */

#pragma once

#include <utility>

#include "d_think.hpp"
#include "p_mobj.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

/* Called by C_Ticker, can call G_PlayerExited.
 * Carries out all thinking of monsters and players. */

void P_Ticker();

void P_InitThinkers();
void P_AddThinker(thinker_t* thinker);
void P_RemoveThinker(thinker_t* thinker);
void P_RemoveThinkerDelayed(thinker_t* thinker); // killough 4/25/98

void P_UpdateThinker(thinker_t* thinker); // killough 8/29/98

void P_SetTarget(mobj_t** mo, mobj_t* target); // killough 11/98

/* killough 8/29/98: threads of thinkers, for more efficient searches
 * cph 2002/01/13: for consistency with the main thinker list, keep objects
 * pending deletion on a class list too
 */
enum struct ThinkerClass : int32_t
{
	Delete,
	Misc,
	Friends,
	Enemies,
	Count,
	All = Count, /* For P_NextThinker, indicates "any class" */
};

extern thinker_t thinkerclasscap[];
#define thinkercap thinkerclasscap[std::to_underlying(ThinkerClass::All)]

/* cph 2002/01/13 - iterator for thinker lists */
thinker_t* P_NextThinker(thinker_t*, ThinkerClass);

void P_CleanThinkers();

#ifdef __cplusplus
}
#endif
