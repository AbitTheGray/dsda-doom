// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Line Tracker HUD Component

#include "base.h"

#include "line_tracker.h"

void dsda_LineTrackerHC(char* str, size_t max_size, int id)
{
	snprintf(
		str,
		max_size,
		"%sl %d: %d %d",
		lines[id].special ? dsda_TextColor(dsda_tc_exhud_line_special) : dsda_TextColor(dsda_tc_exhud_line_normal),
		id,
		lines[id].special,
		lines[id].player_activations
	);
}
