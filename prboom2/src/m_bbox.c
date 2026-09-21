// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Main loop menu stuff.
 *      Random number LUT.
 *      Default Config File.
 *      PCX Screenshots.
 */

#include "m_bbox.h"

void M_ClearBox (fixed_t *box)
{
  box[BOXTOP] = box[BOXRIGHT] = INT_MIN;
  box[BOXBOTTOM] = box[BOXLEFT] = INT_MAX;
}

void M_AddToBox(fixed_t* box,fixed_t x,fixed_t y)
{
  if (x<box[BOXLEFT])
    box[BOXLEFT] = x;
  else if (x>box[BOXRIGHT])
    box[BOXRIGHT] = x;
  if (y<box[BOXBOTTOM])
    box[BOXBOTTOM] = y;
  else if (y>box[BOXTOP])
    box[BOXTOP] = y;
}
