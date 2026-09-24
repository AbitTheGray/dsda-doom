// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Sector Tracker HUD Component

#include "base.hpp"

#include "sector_tracker.hpp"

void dsda_SectorTrackerHC(char* str, size_t max_size, int id)
{
	dboolean active;
	int special;

	active = P_PlaneActive(&sectors[id]);
	special = sectors[id].special;

	snprintf(
		str,
		max_size,
		"%ss %d: %d %d %d",
		active
		? dsda_TextColor(TextColorIndex::ExhudSectorActive)
		: special
		? dsda_TextColor(TextColorIndex::ExhudSectorSpecial)
		: dsda_TextColor(TextColorIndex::ExhudSectorNormal),
		id, special, active,
		sectors[id].floorheight >> FRACBITS
	);
}
