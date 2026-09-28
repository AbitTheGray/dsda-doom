// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Skill Info

#pragma once

#include "doomdef.hpp"
#include "m_fixed.hpp"

#include "cpp/Util.hpp"

// Properties of a skill level (bit 6 is unused, as upstream).
enum struct SkillFlag : uint16_t
{
	SpawnMulti = Bit<uint16_t>(0u),
	FastMonsters = Bit<uint16_t>(1u),
	InstantReaction = Bit<uint16_t>(2u),
	DisableCheats = Bit<uint16_t>(3u),
	NoPain = Bit<uint16_t>(4u),
	DefaultSkill = Bit<uint16_t>(5u),
	PlayerRespawn = Bit<uint16_t>(7u),
	EasyBossBrain = Bit<uint16_t>(8u),
	MustConfirm = Bit<uint16_t>(9u),
	AutoUseHealth = Bit<uint16_t>(10u),
};
ENUM_FLAGS_FUNC(SkillFlag)

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct
{
	fixed_t ammo_factor;
	fixed_t damage_factor;
	fixed_t armor_factor;
	fixed_t health_factor;
	fixed_t monster_health_factor;
	fixed_t friend_health_factor;
	int respawn_time;
	int spawn_filter;
	char key;
	const char* must_confirm;
	const char* name;
	const char* pic_name;
	int text_color;
	SkillFlag flags;
} skill_info_t;

extern skill_info_t skill_info;
extern skill_info_t* skill_infos;

extern int num_skills;

void dsda_InitSkills();
void dsda_RefreshGameSkill();
void dsda_UpdateGameSkill(int skill);

void dsda_AlterGameFlags();
void dsda_InitGameModifiers();
void dsda_RefreshPistolStart();
void dsda_RefreshAlwaysPistolStart();

#ifdef __cplusplus
}
#endif
