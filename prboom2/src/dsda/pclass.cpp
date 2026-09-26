// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Player Class

#include <utility>

#include "info.hpp"

#include "pclass.hpp"

constinit EnumArray<dsda_pclass_t, PClass> pclass = {
	{At(PClass::Null), {
		.armor_increment = {0},
		.auto_armor_save = 0,
		.armor_max = 0,

		.forwardmove = {0x19, 0x32},
		.sidemove = {0x18, 0x28},
		.stroller_threshold = 0x19,
		.turbo_threshold = 0x32,
	}},

	{At(PClass::Fighter), {
		.armor_increment = {25 * FRACUNIT, 20 * FRACUNIT, 15 * FRACUNIT, 5 * FRACUNIT},
		.auto_armor_save = 15 * FRACUNIT,
		.armor_max = 100 * FRACUNIT,

		.forwardmove = {0x1D, 0x3C},
		.sidemove = {0x1B, 0x3B},
		.stroller_threshold = 0x1D,
		.turbo_threshold = 0x3C,

		.normal_state = StateId::HexenFplay,
		.run_state = StateId::HexenFplayRun1,
		.fire_weapon_state = StateId::HexenFplayAtk1,
		.attack_state = StateId::HexenFplayAtk1,
		.attack_end_state = StateId::HexenFplayAtk2,
	}},

	{At(PClass::Cleric), {
		.armor_increment = {10 * FRACUNIT, 25 * FRACUNIT, 5 * FRACUNIT, 20 * FRACUNIT},
		.auto_armor_save = 10 * FRACUNIT,
		.armor_max = 90 * FRACUNIT,

		.forwardmove = {0x19, 0x32},
		.sidemove = {0x18, 0x28},
		.stroller_threshold = 0x19,
		.turbo_threshold = 0x32,

		.normal_state = StateId::HexenCplay,
		.run_state = StateId::HexenCplayRun1,
		.fire_weapon_state = StateId::HexenCplayAtk1,
		.attack_state = StateId::HexenCplayAtk1,
		.attack_end_state = StateId::HexenCplayAtk3,
	}},

	{At(PClass::Mage), {
		.armor_increment = {5 * FRACUNIT, 15 * FRACUNIT, 10 * FRACUNIT, 25 * FRACUNIT},
		.auto_armor_save = 5 * FRACUNIT,
		.armor_max = 80 * FRACUNIT,

		.forwardmove = {0x16, 0x2E},
		.sidemove = {0x15, 0x25},
		.stroller_threshold = 0x16,
		.turbo_threshold = 0x2D,

		.normal_state = StateId::HexenMplay,
		.run_state = StateId::HexenMplayRun1,
		.fire_weapon_state = StateId::HexenMplayAtk1,
		.attack_state = StateId::HexenMplayAtk1,
		.attack_end_state = StateId::HexenMplayAtk2,
	}},

	{At(PClass::Pig), {
		.armor_increment = {0},
		.auto_armor_save = 0,
		.armor_max = 5 * FRACUNIT,

		.forwardmove = {0x18, 0x31},
		.sidemove = {0x17, 0x27},
		.stroller_threshold = 0x18,
		.turbo_threshold = 0x31,

		.normal_state = StateId::HexenPigplay,
		.run_state = StateId::HexenPigplayRun1,
		.fire_weapon_state = StateId::HexenPigplayAtk1,
		.attack_state = StateId::HexenPigplayAtk1,
		.attack_end_state = StateId::HexenPigplayAtk1,
	}},
};

extern "C" void dsda_ResetNullPClass()
{
	if(heretic)
	{
		pclass[PClass::Null].normal_state = StateId::HereticPlay;
		pclass[PClass::Null].run_state = StateId::HereticPlayRun1;
		pclass[PClass::Null].fire_weapon_state = StateId::HereticPlayAtk2;
		pclass[PClass::Null].attack_state = StateId::HereticPlayAtk1;
		pclass[PClass::Null].attack_end_state = StateId::HereticPlayAtk2;
	}
	else
	{
		pclass[PClass::Null].normal_state = StateId::Play;
		pclass[PClass::Null].run_state = StateId::PlayRun1;
		pclass[PClass::Null].fire_weapon_state = StateId::PlayAtk1;
		pclass[PClass::Null].attack_state = StateId::PlayAtk1;
		pclass[PClass::Null].attack_end_state = StateId::PlayAtk2;
	}
}
