// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mobj Tracker HUD Component

#include "base.hpp"

#include "mobj_tracker.hpp"

void dsda_MobjTrackerHC(char* str, size_t max_size, int id, mobj_t* mobj)
{
	int health;

	health = mobj->health;

	if(mobj->thinker.function == reinterpret_cast<think_t>(P_RemoveThinkerDelayed))
		health = 0;

	snprintf(
		str,
		max_size,
		"%sm %d: %d",
		health > 0 ? dsda_TextColor(TextColorIndex::ExhudMobjAlive) : dsda_TextColor(TextColorIndex::ExhudMobjDead),
		id, health
	);
}
