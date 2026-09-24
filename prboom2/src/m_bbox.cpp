// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Main loop menu stuff.
 *      Random number LUT.
 *      Default Config File.
 *      PCX Screenshots.
 */

#include <utility>

#include "m_bbox.hpp"

void M_ClearBox(fixed_t* box)
{
	box[std::to_underlying(BoxEdge::Top)] = box[std::to_underlying(BoxEdge::Right)] = INT_MIN;
	box[std::to_underlying(BoxEdge::Bottom)] = box[std::to_underlying(BoxEdge::Left)] = INT_MAX;
}

void M_AddToBox(fixed_t* box, fixed_t x, fixed_t y)
{
	if(x < box[std::to_underlying(BoxEdge::Left)])
		box[std::to_underlying(BoxEdge::Left)] = x;
	else if(x > box[std::to_underlying(BoxEdge::Right)])
		box[std::to_underlying(BoxEdge::Right)] = x;
	if(y < box[std::to_underlying(BoxEdge::Bottom)])
		box[std::to_underlying(BoxEdge::Bottom)] = y;
	else if(y > box[std::to_underlying(BoxEdge::Top)])
		box[std::to_underlying(BoxEdge::Top)] = y;
}
