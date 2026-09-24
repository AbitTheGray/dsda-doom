// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Brute Force

#pragma once

#include <utility>

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct BruteForceAttribute : int32_t
{
	X,
	Y,
	Z,
	Momx,
	Momy,
	Speed,
	Damage,
	Rng,
	Arm,
	Hp,
	Ammo0,
	Ammo1,
	Ammo2,
	Ammo3,
	Ammo4,
	Ammo5,
	Bmapwidth,
	AttributeMax,

	LineSkip = 0,
	LineActivation,
	HaveItem,
	LackItem,
	MiscMax,
};

enum struct BruteForceOperator : int32_t
{
	LessThan,
	LessThanOrEqualTo,
	GreaterThan,
	GreaterThanOrEqualTo,
	EqualTo,
	NotEqualTo,
	Max,

	Misc,
};

enum struct BruteForceItem : int32_t
{
	RedKeyCard,
	YellowKeyCard,
	BlueKeyCard,
	RedSkullKey,
	YellowSkullKey,
	BlueSkullKey,
	Fist,
	Pistol,
	Shotgun,
	Chaingun,
	RocketLauncher,
	PlasmaGun,
	Bfg,
	Chainsaw,
	SuperShotgun,
	Max,
};

enum struct BruteForceLimit : int32_t
{
	TrioZero,
	Acap = TrioZero,
	TrioMax,

	DuoZero = TrioMax,
	Max            = DuoZero,
	Min,
	DuoMax,

	Count = DuoMax
};

extern const char* dsda_bf_attribute_names[std::to_underlying(BruteForceAttribute::AttributeMax)];
extern const char* dsda_bf_operator_names[std::to_underlying(BruteForceOperator::Max)];
extern const char* dsda_bf_item_names[std::to_underlying(BruteForceItem::Max)];
extern const char* dsda_bf_limit_names[std::to_underlying(BruteForceLimit::Count)];

dboolean dsda_BruteForce();
dboolean dsda_BruteForceEnded();
void dsda_ResetBruteForceConditions();
void dsda_SetBruteForceTarget(BruteForceAttribute attribute,
	BruteForceLimit limit, fixed_t value, dboolean has_value);
void dsda_AddMiscBruteForceCondition(BruteForceAttribute attribute, fixed_t value);
void dsda_AddBruteForceCondition(BruteForceAttribute attribute,
	BruteForceOperator operator_, fixed_t value);
dboolean dsda_StartBruteForce(int depth);
int dsda_KeepBruteForceFrame(int i);
int dsda_AddBruteForceFrame(int i,
	int forwardmove_min, int forwardmove_max,
	int sidemove_min, int sidemove_max,
	int angleturn_min, int angleturn_max,
	byte buttons);
void dsda_BruteForceWithoutMonsters();
void dsda_BruteForceWithMonsters();
void dsda_UpdateBruteForce();
void dsda_EvaluateBruteForce();
void dsda_CopyBruteForceCommand(ticcmd_t* cmd);

#ifdef __cplusplus
}
#endif
