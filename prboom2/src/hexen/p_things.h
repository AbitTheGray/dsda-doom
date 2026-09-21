// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __HEXEN_P_THINGS__
#define __HEXEN_P_THINGS__

#include "info.h"

dboolean EV_ThingProjectile(byte * args, dboolean gravity);
dboolean EV_ThingSpawn(byte * args, dboolean fog);
dboolean EV_ThingActivate(int tid);
dboolean EV_ThingDeactivate(int tid);
dboolean EV_ThingRemove(int tid);
dboolean EV_ThingDestroy(int tid);

extern mobjtype_t TranslateThingType[];

#endif
