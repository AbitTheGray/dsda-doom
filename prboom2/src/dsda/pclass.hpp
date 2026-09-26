// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Player Class

#pragma once

#include <stdint.h>
#include <utility>

enum struct StateId : int32_t;

#include "m_fixed.hpp"
#include "doomdef.hpp"

#include "cpp/EnumArray.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct dsda_pclass_s
{
	int armor_increment[std::to_underlying(ArmorType::Count)];
	int auto_armor_save;
	int armor_max;

	fixed_t forwardmove[2];
	fixed_t sidemove[2];
	fixed_t stroller_threshold;
	fixed_t turbo_threshold;

	StateId normal_state;
	StateId run_state;
	StateId fire_weapon_state;
	StateId attack_state;
	StateId attack_end_state;
} dsda_pclass_t;

extern EnumArray<dsda_pclass_t, PClass> pclass;

#ifdef __cplusplus
}
#endif
