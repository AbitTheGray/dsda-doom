// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Sector Tracker HUD Component

#include "base.h"

#include "sector_tracker.h"

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
		? dsda_TextColor(dsda_tc_exhud_sector_active)
		: special
		? dsda_TextColor(dsda_tc_exhud_sector_special)
		: dsda_TextColor(dsda_tc_exhud_sector_normal),
		id, special, active,
		sectors[id].floorheight >> FRACBITS
	);
}
