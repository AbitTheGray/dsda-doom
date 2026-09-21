// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Player Class

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "m_fixed.hpp"
#include "doomdef.hpp"

typedef struct dsda_pclass_s
{
	int armor_increment[NUMARMOR];
	int auto_armor_save;
	int armor_max;

	fixed_t forwardmove[2];
	fixed_t sidemove[2];
	fixed_t stroller_threshold;
	fixed_t turbo_threshold;

	int normal_state;
	int run_state;
	int fire_weapon_state;
	int attack_state;
	int attack_end_state;
} dsda_pclass_t;

extern dsda_pclass_t pclass[NUMCLASSES];

#ifdef __cplusplus
}
#endif
