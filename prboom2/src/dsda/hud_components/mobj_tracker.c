// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mobj Tracker HUD Component

#include "base.h"

#include "mobj_tracker.h"

void dsda_MobjTrackerHC(char* str, size_t max_size, int id, mobj_t* mobj)
{
	int health;

	health = mobj->health;

	if(mobj->thinker.function == P_RemoveThinkerDelayed)
		health = 0;

	snprintf(
		str,
		max_size,
		"%sm %d: %d",
		health > 0 ? dsda_TextColor(dsda_tc_exhud_mobj_alive) : dsda_TextColor(dsda_tc_exhud_mobj_dead),
		id, health
	);
}
