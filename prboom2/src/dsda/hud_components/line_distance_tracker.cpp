// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Line Distance Tracker HUD Component

#include "base.hpp"

#include "line_distance_tracker.hpp"

void dsda_LineDistanceTrackerHC(char* str, size_t max_size, int id)
{
	line_t* line;
	mobj_t* mo;
	double distance;
	double radius;

	line = &lines[id];
	mo = players[displayplayer].mo;
	radius = (double)mo->radius / FRACUNIT;
	distance = dsda_DistancePointToLine(line->v1->x, line->v1->y, line->v2->x, line->v2->y,
		mo->x, mo->y);

	snprintf(
		str,
		max_size,
		"%sld %d: %.03f",
		distance < radius ? dsda_TextColor(TextColorIndex::ExhudLineClose) : dsda_TextColor(TextColorIndex::ExhudLineFar),
		id,
		distance
	);
}
