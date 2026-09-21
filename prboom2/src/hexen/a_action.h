// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __HEXEN_A_ACTION__
#define __HEXEN_A_ACTION__

#include "p_mobj.h"

extern int localQuakeHappening[MAX_MAXPLAYERS];

dboolean A_LocalQuake(byte * args, mobj_t * victim);
void P_SpawnDirt(mobj_t * actor, fixed_t radius);
void A_BridgeRemove(mobj_t * actor);

#endif
