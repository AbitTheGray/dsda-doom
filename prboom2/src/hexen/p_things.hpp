// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "info.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

dboolean EV_ThingProjectile(byte* args, dboolean gravity);
dboolean EV_ThingSpawn(byte* args, dboolean fog);
dboolean EV_ThingActivate(int tid);
dboolean EV_ThingDeactivate(int tid);
dboolean EV_ThingRemove(int tid);
dboolean EV_ThingDestroy(int tid);

extern MobjType TranslateThingType[];

#ifdef __cplusplus
}
#endif
