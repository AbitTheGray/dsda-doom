// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Line Tracker HUD Component

#include "base.hpp"

#include "line_tracker.hpp"

void dsda_LineTrackerHC(char* str, size_t max_size, int id)
{
	snprintf(
		str,
		max_size,
		"%sl %d: %d %d",
		lines[id].special ? dsda_TextColor(TextColorIndex::ExhudLineSpecial) : dsda_TextColor(TextColorIndex::ExhudLineNormal),
		id,
		lines[id].special,
		lines[id].player_activations
	);
}
