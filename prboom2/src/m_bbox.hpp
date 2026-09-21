// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *    Simple bounding box datatype and functions.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <limits.h>
#include "m_fixed.hpp"

/* Bounding box coordinate storage. */
enum
{
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
}; /* bbox coordinates */

/* Bounding box functions. */

void M_ClearBox(fixed_t* box);

void M_AddToBox(fixed_t* box, fixed_t x, fixed_t y);

#ifdef __cplusplus
}
#endif
