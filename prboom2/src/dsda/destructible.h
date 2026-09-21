// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Destructible

#pragma once

#include "p_mobj.h"

void dsda_AddLineToHealthGroup(line_t* line);
void dsda_ResetHealthGroups(void);
void dsda_DamageLinedef(line_t* line, mobj_t* source, int damage);
void dsda_RadiusAttackDestructibles(int xl, int xh, int yl, int yh);
