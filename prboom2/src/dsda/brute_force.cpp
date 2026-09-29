// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Brute Force

#include <utility>

#include <math.h>

#include "cpp/EnumArray.hpp"

#include "d_player.hpp"
#include "d_ticcmd.hpp"
#include "doomstat.hpp"
#include "lprintf.hpp"
#include "m_random.hpp"
#include "r_state.hpp"

#include "dsda/build.hpp"
#include "dsda/demo.hpp"
#include "dsda/features.hpp"
#include "dsda/key_frame.hpp"
#include "dsda/skip.hpp"
#include "dsda/time.hpp"
#include "dsda/utility.hpp"

#include "brute_force.hpp"

#define MAX_BF_DEPTH 35
#define MAX_BF_CONDITIONS 16

typedef struct
{
	int min;
	int max;
	int i;
} bf_range_t;

typedef struct
{
	dsda_key_frame_t key_frame;
	bf_range_t forwardmove;
	bf_range_t sidemove;
	bf_range_t angleturn;
	byte buttons;
} bf_t;

typedef struct
{
	BruteForceAttribute attribute;
	BruteForceOperator operator_;
	fixed_t value;
	fixed_t secondary_value;
} bf_condition_t;

typedef struct
{
	BruteForceAttribute attribute;
	BruteForceLimit limit;
	fixed_t value;
	dboolean enabled;
	dboolean evaluated;
	fixed_t best_value;
	int best_depth;
	bf_t best_bf[MAX_BF_DEPTH];
} bf_target_t;

static bf_t brute_force[MAX_BF_DEPTH];
static int bf_depth;
static int bf_logictic;
static int bf_condition_count;
static bf_condition_t bf_condition[MAX_BF_CONDITIONS];
static long long bf_volume;
static long long bf_volume_max;
static dboolean bf_mode;
static dboolean bf_nomonsters;
static dsda_key_frame_t nomo_key_frame;
static bf_target_t bf_target;
static ticcmd_t bf_result[MAX_BF_DEPTH];

constinit EnumArray<const char*, BruteForceAttribute, BruteForceAttribute::AttributeMax> dsda_bf_attribute_names = {
	{At(BruteForceAttribute::X), "x"},
	{At(BruteForceAttribute::Y), "y"},
	{At(BruteForceAttribute::Z), "z"},
	{At(BruteForceAttribute::Momx), "vx"},
	{At(BruteForceAttribute::Momy), "vy"},
	{At(BruteForceAttribute::Speed), "spd"},
	{At(BruteForceAttribute::Damage), "dmg"},
	{At(BruteForceAttribute::Rng), "rng"},
	{At(BruteForceAttribute::Arm), "arm"},
	{At(BruteForceAttribute::Hp), "hp"},
	{At(BruteForceAttribute::Ammo0), "am0"},
	{At(BruteForceAttribute::Ammo1), "am1"},
	{At(BruteForceAttribute::Ammo2), "am2"},
	{At(BruteForceAttribute::Ammo3), "am3"},
	{At(BruteForceAttribute::Ammo4), "am4"},
	{At(BruteForceAttribute::Ammo5), "am5"},
	{At(BruteForceAttribute::Bmapwidth), "bmw"},
};

const char* dsda_bf_misc_names[std::to_underlying(BruteForceAttribute::MiscMax)] = {
	"line skip",
	"line activation",
	"have item",
};

constinit EnumArray<const char*, BruteForceOperator, BruteForceOperator::Max> dsda_bf_operator_names = {
	{At(BruteForceOperator::LessThan), "<"},
	{At(BruteForceOperator::LessThanOrEqualTo), "<="},
	{At(BruteForceOperator::GreaterThan), ">"},
	{At(BruteForceOperator::GreaterThanOrEqualTo), ">="},
	{At(BruteForceOperator::EqualTo), "=="},
	{At(BruteForceOperator::NotEqualTo), "!="}
};

const char* dsda_bf_limit_names[std::to_underlying(BruteForceLimit::Count)] = {
	"acap",
	"max",
	"min",
};

constinit EnumArray<const char*, BruteForceItem, BruteForceItem::Max> dsda_bf_item_names = {
	{At(BruteForceItem::RedKeyCard), "rkc"},
	{At(BruteForceItem::YellowKeyCard), "ykc"},
	{At(BruteForceItem::BlueKeyCard), "bkc"},
	{At(BruteForceItem::RedSkullKey), "rsk"},
	{At(BruteForceItem::YellowSkullKey), "ysk"},
	{At(BruteForceItem::BlueSkullKey), "bsk"},

	{At(BruteForceItem::Fist), "f"},
	{At(BruteForceItem::Pistol), "p"},
	{At(BruteForceItem::Shotgun), "sg"},
	{At(BruteForceItem::Chaingun), "cg"},
	{At(BruteForceItem::RocketLauncher), "rl"},
	{At(BruteForceItem::PlasmaGun), "pg"},
	{At(BruteForceItem::Bfg), "bfg"},
	{At(BruteForceItem::Chainsaw), "cs"},
	{At(BruteForceItem::SuperShotgun), "ssg"},
};

static constinit EnumArray<dboolean, BruteForceAttribute, BruteForceAttribute::AttributeMax> fixed_point_attribute = {
	{At(BruteForceAttribute::X), true},
	{At(BruteForceAttribute::Y), true},
	{At(BruteForceAttribute::Z), true},
	{At(BruteForceAttribute::Momx), true},
	{At(BruteForceAttribute::Momy), true},
	{At(BruteForceAttribute::Speed), true},
	{At(BruteForceAttribute::Damage), true},
	{At(BruteForceAttribute::Rng), false},
	{At(BruteForceAttribute::Arm), false},
	{At(BruteForceAttribute::Hp), false},
	{At(BruteForceAttribute::Ammo0), false},
	{At(BruteForceAttribute::Ammo1), false},
	{At(BruteForceAttribute::Ammo2), false},
	{At(BruteForceAttribute::Ammo3), false},
	{At(BruteForceAttribute::Ammo4), false},
	{At(BruteForceAttribute::Ammo5), false},
	{At(BruteForceAttribute::Bmapwidth), false},
};

static dboolean dsda_AdvanceBFRange(bf_range_t* range)
{
	++range->i;

	if(range->i > range->max)
	{
		range->i = range->min;
		return false;
	}

	return true;
}

static dboolean dsda_AdvanceBruteForceFrame(int frame)
{
	if(!dsda_AdvanceBFRange(&brute_force[frame].angleturn))
		if(!dsda_AdvanceBFRange(&brute_force[frame].sidemove))
			if(!dsda_AdvanceBFRange(&brute_force[frame].forwardmove))
				return false;

	return true;
}

static int dsda_AdvanceBruteForce()
{
	int i;

	for(i = bf_depth - 1; i >= 0; --i)
		if(dsda_AdvanceBruteForceFrame(i))
			break;

	return i;
}

static void dsda_CopyBFCommandDepth(ticcmd_t* cmd, bf_t* bf)
{
	memset(cmd, 0, sizeof(*cmd));

	cmd->angleturn = bf->angleturn.i << 8;
	cmd->forwardmove = bf->forwardmove.i;
	cmd->sidemove = bf->sidemove.i;
	cmd->buttons = static_cast<ButtonCode>(bf->buttons);
}

static void dsda_CopyBFResult(bf_t* bf, int depth)
{
	int i;

	for(i = 0; i < depth; ++i)
		dsda_CopyBFCommandDepth(&bf_result[i], &bf[i]);

	if(i != MAX_BF_DEPTH)
		memset(&bf_result[i], 0, sizeof(ticcmd_t) * (MAX_BF_DEPTH - i));
}

static void dsda_RestoreBFKeyFrame(int frame)
{
	dsda_RestoreKeyFrame(&brute_force[frame].key_frame, true);
}

static void dsda_StoreBFKeyFrame(int frame)
{
	dsda_StoreKeyFrame(&brute_force[frame].key_frame, true, false);
}

static void dsda_PrintBFProgress()
{
	int percent;
	unsigned long long elapsed_time;

	percent = 100 * bf_volume / bf_volume_max;
	elapsed_time = dsda_ElapsedTimeMS(DsdaTimer::BruteForce);

	Log::Info("  {} / {} sequences tested ({}%) in {:.2f} seconds!\n",
		bf_volume, bf_volume_max, percent, static_cast<float>(elapsed_time) / 1000);
}

enum struct BruteForceResult : uint8_t
{
	Failure,
	Success,
};

static const char* bf_result_text[2] = {"FAILURE", "SUCCESS"};
static dboolean brute_force_ended;

dboolean dsda_BruteForceEnded()
{
	return brute_force_ended;
}

static void dsda_EndBF(const BruteForceResult result)
{
	brute_force_ended = true;

	Log::Info("Brute force complete ({})!\n", bf_result_text[std::to_underlying(result)]);
	dsda_PrintBFProgress();

	if(bf_nomonsters)
		dsda_RestoreKeyFrame(&nomo_key_frame, true);
	else
		dsda_RestoreBFKeyFrame(0);

	bf_mode = false;

	if(result == BruteForceResult::Success)
		dsda_QueueBuildCommands(bf_result, bf_depth);
	else
		dsda_ExitSkipMode();
}

static fixed_t dsda_BFAttribute(BruteForceAttribute attribute)
{
	extern int bmapwidth;

	player_t* player;

	player = &players[displayplayer];

	switch(attribute)
	{
		case BruteForceAttribute::X:
			return player->mo->x;
		case BruteForceAttribute::Y:
			return player->mo->y;
		case BruteForceAttribute::Z:
			return player->mo->z;
		case BruteForceAttribute::Momx:
			return player->mo->momx;
		case BruteForceAttribute::Momy:
			return player->mo->momy;
		case BruteForceAttribute::Speed:
			return P_PlayerSpeed(player);
		case BruteForceAttribute::Damage:
		{
			extern int player_damage_last_tic;

			return player_damage_last_tic;
		}
		case BruteForceAttribute::Rng:
			return rng.rndindex;
		case BruteForceAttribute::Arm:
			return player->armorpoints[std::to_underlying(ArmorType::Armor)];
		case BruteForceAttribute::Hp:
			return player->health;
		case BruteForceAttribute::Ammo0:
			return player->ammo[0];
		case BruteForceAttribute::Ammo1:
			return player->ammo[1];
		case BruteForceAttribute::Ammo2:
			return player->ammo[2];
		case BruteForceAttribute::Ammo3:
			return player->ammo[3];
		case BruteForceAttribute::Ammo4:
			return player->ammo[4];
		case BruteForceAttribute::Ammo5:
			return player->ammo[5];
		case BruteForceAttribute::Bmapwidth:
			return bmapwidth;
		default:
			return 0;
	}
}

static dboolean dsda_BFHaveItem(BruteForceItem item)
{
	player_t* player;

	player = &players[displayplayer];

	switch(item)
	{
		case BruteForceItem::RedKeyCard:
			return player->cards[std::to_underlying(Card::RedCard)];
		case BruteForceItem::YellowKeyCard:
			return player->cards[std::to_underlying(Card::YellowCard)];
		case BruteForceItem::BlueKeyCard:
			return player->cards[std::to_underlying(Card::BlueCard)];
		case BruteForceItem::RedSkullKey:
			return player->cards[std::to_underlying(Card::RedSkull)];
		case BruteForceItem::YellowSkullKey:
			return player->cards[std::to_underlying(Card::YellowSkull)];
		case BruteForceItem::BlueSkullKey:
			return player->cards[std::to_underlying(Card::BlueSkull)];
		case BruteForceItem::Fist:
			return player->weaponowned[std::to_underlying(WeaponType::Fist)];
		case BruteForceItem::Pistol:
			return player->weaponowned[std::to_underlying(WeaponType::Pistol)];
		case BruteForceItem::Shotgun:
			return player->weaponowned[std::to_underlying(WeaponType::Shotgun)];
		case BruteForceItem::Chaingun:
			return player->weaponowned[std::to_underlying(WeaponType::Chaingun)];
		case BruteForceItem::RocketLauncher:
			return player->weaponowned[std::to_underlying(WeaponType::Missile)];
		case BruteForceItem::PlasmaGun:
			return player->weaponowned[std::to_underlying(WeaponType::Plasma)];
		case BruteForceItem::Bfg:
			return player->weaponowned[std::to_underlying(WeaponType::Bfg)];
		case BruteForceItem::Chainsaw:
			return player->weaponowned[std::to_underlying(WeaponType::Chainsaw)];
		case BruteForceItem::SuperShotgun:
			return player->weaponowned[std::to_underlying(WeaponType::Supershotgun)];
		default:
			return false;
	}
}

static dboolean dsda_BFMiscConditionReached(int i)
{
	switch(bf_condition[i].attribute)
	{
		case BruteForceAttribute::LineSkip:
			return lines[bf_condition[i].value].player_activations == bf_condition[i].secondary_value;
		case BruteForceAttribute::LineActivation:
			return lines[bf_condition[i].value].player_activations > bf_condition[i].secondary_value;
		case BruteForceAttribute::HaveItem:
			return dsda_BFHaveItem(static_cast<BruteForceItem>(bf_condition[i].value));
		case BruteForceAttribute::LackItem:
			return !dsda_BFHaveItem(static_cast<BruteForceItem>(bf_condition[i].value));
		default:
			return false;
	}
}

static dboolean dsda_BFConditionReached(int i)
{
	fixed_t value;

	if(bf_condition[i].operator_ == BruteForceOperator::Misc)
		return dsda_BFMiscConditionReached(i);

	value = dsda_BFAttribute(bf_condition[i].attribute);

	switch(bf_condition[i].operator_)
	{
		case BruteForceOperator::LessThan:
			return value < bf_condition[i].value;
		case BruteForceOperator::LessThanOrEqualTo:
			return value <= bf_condition[i].value;
		case BruteForceOperator::GreaterThan:
			return value > bf_condition[i].value;
		case BruteForceOperator::GreaterThanOrEqualTo:
			return value >= bf_condition[i].value;
		case BruteForceOperator::EqualTo:
			return value == bf_condition[i].value;
		case BruteForceOperator::NotEqualTo:
			return value != bf_condition[i].value;
		default:
			return false;
	}
}

static void dsda_BFUpdateBestResult(fixed_t value)
{
	int i;
	char str[FIXED_STRING_LENGTH];
	char cmd_str[COMMAND_MOVEMENT_STRING_LENGTH];

	bf_target.evaluated = true;
	bf_target.best_value = value;
	bf_target.best_depth = true_logictic - bf_logictic;

	for(i = 0; i < bf_target.best_depth; ++i)
		bf_target.best_bf[i] = brute_force[i];

	dsda_CopyBFResult(bf_target.best_bf, bf_target.best_depth);

	if(fixed_point_attribute[bf_target.attribute])
		dsda_FixedToString(str, value);
	else
		snprintf(str, FIXED_STRING_LENGTH, "%i", value);

	Log::Info("New best: {} = {}\n", dsda_bf_attribute_names[bf_target.attribute], std::string_view(str));

	for(i = 0; i < bf_target.best_depth; ++i)
	{
		dsda_PrintCommandMovement(cmd_str, &bf_result[i]);
		Log::Info("    {}\n", std::string_view(cmd_str));
	}

	Log::Info("\n");
}

static dboolean dsda_BFNewBestResult(fixed_t value)
{
	if(!bf_target.evaluated)
		return true;

	switch(bf_target.limit)
	{
		case BruteForceLimit::Acap:
			return abs(value - bf_target.value) < abs(bf_target.best_value - bf_target.value);
		case BruteForceLimit::Max:
			return value > bf_target.best_value;
		case BruteForceLimit::Min:
			return value < bf_target.best_value;
		default:
			return false;
	}
}

static void dsda_BFEvaluateTarget()
{
	fixed_t value;

	value = dsda_BFAttribute(bf_target.attribute);

	if(dsda_BFNewBestResult(value))
		dsda_BFUpdateBestResult(value);
}

static dboolean dsda_BFConditionsReached()
{
	int i, reached;

	reached = 0;
	for(i = 0; i < bf_condition_count; ++i)
		reached += dsda_BFConditionReached(i);

	if(reached == bf_condition_count)
		if(bf_target.enabled)
		{
			dsda_BFEvaluateTarget();

			return false;
		}

	return reached == bf_condition_count;
}

dboolean dsda_BruteForce()
{
	return bf_mode;
}

void dsda_ResetBruteForceConditions()
{
	bf_condition_count = 0;
	memset(&bf_target, 0, sizeof(bf_target));
}

void dsda_AddMiscBruteForceCondition(BruteForceAttribute attribute, fixed_t value)
{
	if(bf_condition_count == MAX_BF_CONDITIONS)
		return;

	bf_condition[bf_condition_count].attribute = attribute;
	bf_condition[bf_condition_count].operator_ = BruteForceOperator::Misc;
	bf_condition[bf_condition_count].value = value;

	switch(attribute)
	{
		case BruteForceAttribute::LineSkip:
		case BruteForceAttribute::LineActivation:
			bf_condition[bf_condition_count].secondary_value = lines[value].player_activations;
			break;
		default:
			break;
	}

	++bf_condition_count;

	Log::Info("Added brute force condition: {} {}\n",
		dsda_bf_misc_names[std::to_underlying(attribute)],
		value);
}

void dsda_AddBruteForceCondition(
	BruteForceAttribute attribute,
	BruteForceOperator operator_,
	fixed_t value
)
{
	if(bf_condition_count == MAX_BF_CONDITIONS)
		return;

	bf_condition[bf_condition_count].attribute = attribute;
	bf_condition[bf_condition_count].operator_ = operator_;
	bf_condition[bf_condition_count].value = value;

	if(fixed_point_attribute[attribute])
		bf_condition[bf_condition_count].value <<= FRACBITS;

	++bf_condition_count;

	Log::Info("Added brute force condition: {} {} {}\n",
		dsda_bf_attribute_names[attribute],
		dsda_bf_operator_names[operator_],
		value);
}

void dsda_SetBruteForceTarget(BruteForceAttribute attribute,
	BruteForceLimit limit, fixed_t value, dboolean has_value)
{
	bf_target.attribute = attribute;
	bf_target.limit = limit;
	bf_target.value = value;
	bf_target.best_value = dsda_BFAttribute(attribute);
	bf_target.enabled = true;

	if(has_value)
	{
		Log::Info("Set brute force target: {} {} {}\n",
			dsda_bf_attribute_names[attribute],
			dsda_bf_limit_names[std::to_underlying(limit)],
			value);

		if(fixed_point_attribute[attribute])
			bf_target.value <<= FRACBITS;
	}
	else
		Log::Info("Set brute force target: {} {}\n",
			dsda_bf_attribute_names[attribute],
			dsda_bf_limit_names[std::to_underlying(limit)]);
}

static void dsda_SortIntPair(int* a, int* b)
{
	if(*a > *b)
	{
		int temp;

		temp = *a;
		*a = *b;
		*b = temp;
	}
}

int dsda_KeepBruteForceFrame(int i)
{
	ticcmd_t cmd;

	if(!dsda_CopyPendingCmd(&cmd, i))
		return false;

	brute_force[i].forwardmove.min = cmd.forwardmove;
	brute_force[i].forwardmove.max = cmd.forwardmove;

	brute_force[i].sidemove.min = cmd.sidemove;
	brute_force[i].sidemove.max = cmd.sidemove;

	brute_force[i].angleturn.min = cmd.angleturn >> 8;
	brute_force[i].angleturn.max = cmd.angleturn >> 8;

	brute_force[i].buttons = std::to_underlying(cmd.buttons);

	return true;
}

int dsda_AddBruteForceFrame(int i,
	int forwardmove_min, int forwardmove_max,
	int sidemove_min, int sidemove_max,
	int angleturn_min, int angleturn_max,
	byte buttons)
{
	if(i < 0 || i >= MAX_BF_DEPTH)
		return false;

	dsda_SortIntPair(&forwardmove_min, &forwardmove_max);
	dsda_SortIntPair(&sidemove_min, &sidemove_max);
	dsda_SortIntPair(&angleturn_min, &angleturn_max);

	brute_force[i].forwardmove.min = forwardmove_min;
	brute_force[i].forwardmove.max = forwardmove_max;

	brute_force[i].sidemove.min = sidemove_min;
	brute_force[i].sidemove.max = sidemove_max;

	brute_force[i].angleturn.min = angleturn_min;
	brute_force[i].angleturn.max = angleturn_max;

	brute_force[i].buttons = buttons;

	return true;
}

void dsda_BruteForceWithoutMonsters()
{
	bf_nomonsters = true;
}

void dsda_BruteForceWithMonsters()
{
	bf_nomonsters = false;
}

dboolean dsda_StartBruteForce(int depth)
{
	int i;

	if(!dsda_BuildMode())
	{
		Log::Warn("You cannot start brute force outside of build mode!\n");
		return false;
	}

	if(depth <= 0 || depth > MAX_BF_DEPTH)
		return false;

	dsda_TrackFeature(FeatureFlag::Bruteforce);

	Log::Info("Brute force starting:\n");

	bf_depth = depth;
	bf_logictic = true_logictic;
	bf_volume = 0;
	bf_volume_max = 1;

	for(i = 0; i < bf_depth; ++i)
	{
		Log::Info("  {}: F {}:{} S {}:{} T {}:{} B {}\n", i,
			brute_force[i].forwardmove.min, brute_force[i].forwardmove.max,
			brute_force[i].sidemove.min, brute_force[i].sidemove.max,
			brute_force[i].angleturn.min, brute_force[i].angleturn.max,
			brute_force[i].buttons);

		bf_volume_max *= (brute_force[i].forwardmove.max - brute_force[i].forwardmove.min + 1) *
			(brute_force[i].sidemove.max - brute_force[i].sidemove.min + 1) *
			(brute_force[i].angleturn.max - brute_force[i].angleturn.min + 1);

		brute_force[i].forwardmove.i = brute_force[i].forwardmove.min;
		brute_force[i].sidemove.i = brute_force[i].sidemove.min;
		brute_force[i].angleturn.i = brute_force[i].angleturn.min;
	}

	Log::Info("Testing {} sequences with depth {}\n\n", bf_volume_max, bf_depth);

	bf_mode = true;

	if(bf_nomonsters)
	{
		Log::Info("Warning: ignoring monsters! The result may desync with monsters!\n");
		dsda_StoreKeyFrame(&nomo_key_frame, true, false);
		P_RemoveMonsters();
	}

	dsda_EnterSkipMode();

	dsda_StartTimer(DsdaTimer::BruteForce);

	return true;
}

void dsda_UpdateBruteForce()
{
	int frame;

	frame = true_logictic - bf_logictic;

	if(frame == bf_depth)
	{
		if(bf_volume % 10000 == 0)
			dsda_PrintBFProgress();

		frame = dsda_AdvanceBruteForce();

		if(frame >= 0)
			dsda_RestoreBFKeyFrame(frame);
	}
	else
		dsda_StoreBFKeyFrame(frame);
}

void dsda_EvaluateBruteForce()
{
	if(true_logictic - bf_logictic != bf_depth)
		return;

	++bf_volume;

	if(dsda_BFConditionsReached())
	{
		dsda_CopyBFResult(brute_force, bf_depth);
		dsda_EndBF(BruteForceResult::Success);
	}
	else if(bf_volume >= bf_volume_max)
	{
		if(bf_target.enabled && bf_target.evaluated)
			dsda_EndBF(BruteForceResult::Success);
		else
			dsda_EndBF(BruteForceResult::Failure);
	}
}

void dsda_CopyBruteForceCommand(ticcmd_t* cmd)
{
	int depth;

	depth = true_logictic - bf_logictic;

	if(depth >= bf_depth)
	{
		memset(cmd, 0, sizeof(*cmd));

		return;
	}

	dsda_CopyBFCommandDepth(cmd, &brute_force[depth]);
}
