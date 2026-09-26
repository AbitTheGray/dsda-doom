// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mobj Info

#include <utility>

#include <stdlib.h>
#include <string.h>

#include "sounds.hpp"
#include "p_map.hpp"

#include "dsda/map_format.hpp"

#include "mobjinfo.hpp"

mobjinfo_t* mobjinfo;
int num_mobj_types;
int mobj_types_zero;
byte* edited_mobjinfo_bits;

static void dsda_ResetMobjInfo(int from, int to)
{
	int i;

	for(i = from; i < to; ++i)
	{
		mobjinfo[i].droppeditem = MobjType::Null;
		mobjinfo[i].infighting_group = std::to_underlying(InfightingGroup::Default);
		mobjinfo[i].projectile_group = std::to_underlying(ProjectileGroup::Default);
		mobjinfo[i].splash_group = std::to_underlying(SplashGroup::Default);
		mobjinfo[i].altspeed = NO_ALTSPEED;
		mobjinfo[i].meleerange = MELEERANGE;
		mobjinfo[i].visibility = VF_DOOM;
	}
}

static void dsda_EnsureCapacity(int limit)
{
	while(limit >= num_mobj_types)
	{
		int old_num_mobj_types = num_mobj_types;

		num_mobj_types *= 2;

		mobjinfo = static_cast<mobjinfo_t*>(Z_Realloc(mobjinfo, num_mobj_types * sizeof(*mobjinfo)));
		memset(mobjinfo + old_num_mobj_types, 0,
			(num_mobj_types - old_num_mobj_types) * sizeof(*mobjinfo));

		edited_mobjinfo_bits = (byte*)Z_Realloc(edited_mobjinfo_bits, num_mobj_types * sizeof(*edited_mobjinfo_bits));
		memset(edited_mobjinfo_bits + old_num_mobj_types, 0,
			(num_mobj_types - old_num_mobj_types) * sizeof(*edited_mobjinfo_bits));

		dsda_ResetMobjInfo(old_num_mobj_types, num_mobj_types);
	}
}

static deh_index_hash_t deh_mobj_index_hash;

int dsda_FindDehMobjIndex(int index)
{
	return dsda_FindDehIndex(index, &deh_mobj_index_hash);
}

int dsda_GetDehMobjIndex(int index)
{
	return dsda_GetDehIndex(index, &deh_mobj_index_hash);
}

// Dehacked has the index off by 1
int dsda_TranslateDehMobjIndex(int index)
{
	return dsda_GetDehMobjIndex(index - 1) + 1;
}

dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index)
{
	dsda_deh_mobjinfo_t deh_mobjinfo;

	dsda_EnsureCapacity(index);

	deh_mobjinfo.info = &mobjinfo[index];
	deh_mobjinfo.edited_bits = &edited_mobjinfo_bits[index];

	return deh_mobjinfo;
}

void dsda_InitializeMobjInfo(int zero, int max, int count)
{
	extern dboolean hexen;

	num_mobj_types = count;
	mobj_types_zero = zero;

	mobjinfo = static_cast<mobjinfo_t*>(Z_Calloc(num_mobj_types, sizeof(*mobjinfo)));

	if(hexen) return;

	deh_mobj_index_hash.start_index = num_mobj_types;
	deh_mobj_index_hash.end_index = num_mobj_types;
	edited_mobjinfo_bits = static_cast<decltype(edited_mobjinfo_bits)>(Z_Calloc(num_mobj_types, sizeof(*edited_mobjinfo_bits)));
}

// Changing the renderer causes a reset that accesses this list,
//   so we can't free it.
void dsda_FreeDehMobjInfo()
{
	// free(edited_mobjinfo_bits);
}

MobjType ZMT_MAPSPOT = ZMT_UNDEFINED;
MobjType ZMT_MAPSPOT_GRAVITY = ZMT_UNDEFINED;
MobjType ZMT_TELEPORTDEST2 = ZMT_UNDEFINED;
MobjType ZMT_TELEPORTDEST3 = ZMT_UNDEFINED;
MobjType ZMT_AMBIENTSOUND = ZMT_UNDEFINED;

static mobjinfo_t zmt_mapspot_info = {
	.doomednum = 9001,
	.spawnstate = StateId::Null,
	.spawnhealth = 1000,
	.seestate = StateId::Null,
	.seesound = SfxId::None,
	.reactiontime = 8,
	.attacksound = SfxId::None,
	.painstate = StateId::Null,
	.painchance = 0,
	.painsound = SfxId::None,
	.meleestate = StateId::Null,
	.missilestate = StateId::Null,
	.deathstate = StateId::Null,
	.xdeathstate = StateId::Null,
	.deathsound = SfxId::None,
	.speed = 0,
	.radius = 20 * FRACUNIT,
	.height = 16 * FRACUNIT,
	.mass = 100,
	.damage = 0,
	.activesound = SfxId::None,
	.flags = MobjFlag::NoBlockmap | MobjFlag::NoSector | MobjFlag::NoGravity,
	.raisestate = StateId::Null,
	.droppeditem = MobjType::Null,
	.crashstate = StateId::Null,
	.flags2 = MobjFlag2{},
	.infighting_group = std::to_underlying(InfightingGroup::Default),
	.projectile_group = std::to_underlying(ProjectileGroup::Default),
	.splash_group = std::to_underlying(SplashGroup::Default),
	.ripsound = SfxId::None,
	.altspeed = NO_ALTSPEED,
	.meleerange = MELEERANGE,
	.bloodcolor = 0,
	.visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_mapspot_gravity_info = {
	.doomednum = 9013,
	.spawnstate = StateId::Null,
	.spawnhealth = 1000,
	.seestate = StateId::Null,
	.seesound = SfxId::None,
	.reactiontime = 8,
	.attacksound = SfxId::None,
	.painstate = StateId::Null,
	.painchance = 0,
	.painsound = SfxId::None,
	.meleestate = StateId::Null,
	.missilestate = StateId::Null,
	.deathstate = StateId::Null,
	.xdeathstate = StateId::Null,
	.deathsound = SfxId::None,
	.speed = 0,
	.radius = 20 * FRACUNIT,
	.height = 16 * FRACUNIT,
	.mass = 100,
	.damage = 0,
	.activesound = SfxId::None,
	.flags = MobjFlag{},
	.raisestate = StateId::Null,
	.droppeditem = MobjType::Null,
	.crashstate = StateId::Null,
	.flags2 = MobjFlag2::DontDraw,
	.infighting_group = std::to_underlying(InfightingGroup::Default),
	.projectile_group = std::to_underlying(ProjectileGroup::Default),
	.splash_group = std::to_underlying(SplashGroup::Default),
	.ripsound = SfxId::None,
	.altspeed = NO_ALTSPEED,
	.meleerange = MELEERANGE,
	.bloodcolor = 0,
	.visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest2_info = {
	.doomednum = 9044,
	.spawnstate = StateId::Null,
	.spawnhealth = 1000,
	.seestate = StateId::Null,
	.seesound = SfxId::None,
	.reactiontime = 8,
	.attacksound = SfxId::None,
	.painstate = StateId::Null,
	.painchance = 0,
	.painsound = SfxId::None,
	.meleestate = StateId::Null,
	.missilestate = StateId::Null,
	.deathstate = StateId::Null,
	.xdeathstate = StateId::Null,
	.deathsound = SfxId::None,
	.speed = 0,
	.radius = 20 * FRACUNIT,
	.height = 16 * FRACUNIT,
	.mass = 100,
	.damage = 0,
	.activesound = SfxId::None,
	.flags = MobjFlag::NoBlockmap | MobjFlag::NoSector | MobjFlag::NoGravity,
	.raisestate = StateId::Null,
	.droppeditem = MobjType::Null,
	.crashstate = StateId::Null,
	.flags2 = MobjFlag2{},
	.infighting_group = std::to_underlying(InfightingGroup::Default),
	.projectile_group = std::to_underlying(ProjectileGroup::Default),
	.splash_group = std::to_underlying(SplashGroup::Default),
	.ripsound = SfxId::None,
	.altspeed = NO_ALTSPEED,
	.meleerange = MELEERANGE,
	.bloodcolor = 0,
	.visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest3_info = {
	.doomednum = 9043,
	.spawnstate = StateId::Null,
	.spawnhealth = 1000,
	.seestate = StateId::Null,
	.seesound = SfxId::None,
	.reactiontime = 8,
	.attacksound = SfxId::None,
	.painstate = StateId::Null,
	.painchance = 0,
	.painsound = SfxId::None,
	.meleestate = StateId::Null,
	.missilestate = StateId::Null,
	.deathstate = StateId::Null,
	.xdeathstate = StateId::Null,
	.deathsound = SfxId::None,
	.speed = 0,
	.radius = 20 * FRACUNIT,
	.height = 16 * FRACUNIT,
	.mass = 100,
	.damage = 0,
	.activesound = SfxId::None,
	.flags = MobjFlag::NoBlockmap | MobjFlag::NoSector,
	.raisestate = StateId::Null,
	.droppeditem = MobjType::Null,
	.crashstate = StateId::Null,
	.flags2 = MobjFlag2{},
	.infighting_group = std::to_underlying(InfightingGroup::Default),
	.projectile_group = std::to_underlying(ProjectileGroup::Default),
	.splash_group = std::to_underlying(SplashGroup::Default),
	.ripsound = SfxId::None,
	.altspeed = NO_ALTSPEED,
	.meleerange = MELEERANGE,
	.bloodcolor = 0,
	.visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_ambient_sound = {
	.doomednum = 14064,
	.spawnstate = StateId::Null,
	.spawnhealth = 1000,
	.seestate = StateId::Null,
	.seesound = SfxId::None,
	.reactiontime = 8,
	.attacksound = SfxId::None,
	.painstate = StateId::Null,
	.painchance = 0,
	.painsound = SfxId::None,
	.meleestate = StateId::Null,
	.missilestate = StateId::Null,
	.deathstate = StateId::Null,
	.xdeathstate = StateId::Null,
	.deathsound = SfxId::None,
	.speed = 0,
	.radius = 20 * FRACUNIT,
	.height = 16 * FRACUNIT,
	.mass = 100,
	.damage = 0,
	.activesound = SfxId::None,
	.flags = MobjFlag::NoBlockmap | MobjFlag::NoSector,
	.raisestate = StateId::Null,
	.droppeditem = MobjType::Null,
	.crashstate = StateId::Null,
	.flags2 = MobjFlag2{},
	.infighting_group = std::to_underlying(InfightingGroup::Default),
	.projectile_group = std::to_underlying(ProjectileGroup::Default),
	.splash_group = std::to_underlying(SplashGroup::Default),
	.ripsound = SfxId::None,
	.altspeed = NO_ALTSPEED,
	.meleerange = MELEERANGE,
	.bloodcolor = 0,
	.visibility = VF_DOOM,
};

typedef struct
{
	MobjType* index_p;
	mobjinfo_t* mobjinfo_p;
} append_mobjinfo_t;

static append_mobjinfo_t append_mobjinfo[] = {
	{&ZMT_MAPSPOT, &zmt_mapspot_info},
	{&ZMT_MAPSPOT_GRAVITY, &zmt_mapspot_gravity_info},
	{&ZMT_TELEPORTDEST2, &zmt_teleportdest2_info},
	{&ZMT_TELEPORTDEST3, &zmt_teleportdest3_info},
	{&ZMT_AMBIENTSOUND, &zmt_ambient_sound},
};

static int append_mobjinfo_count = sizeof(append_mobjinfo) / sizeof(append_mobjinfo[0]);

void dsda_AppendZDoomMobjInfo()
{
	int i;
	int index;
	dsda_deh_mobjinfo_t mobjinfo;

	index = deh_mobj_index_hash.end_index;
	for(i = 0; i < append_mobjinfo_count; ++i)
	{
		mobjinfo = dsda_GetDehMobjInfo(index);
		*(append_mobjinfo[i].index_p) = static_cast<MobjType>(index);
		*(mobjinfo.info) = *(append_mobjinfo[i].mobjinfo_p);
		++index;
	}
}
