// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Destructible

#include "lprintf.hpp"
#include "p_map.hpp"
#include "p_maputl.hpp"
#include "p_spec.hpp"
#include "r_main.hpp"

#include "dsda/utility.hpp"
#include "dsda/destructible.hpp"

extern bomb_t bomb;

typedef struct
{
	int health;
	int size;
	int* line_ids;
} health_group_t;

typedef struct
{
	int id;
	health_group_t group;
} health_group_entry_t;

#define HEALTH_GROUP_HASH_MAX 128

static health_group_entry_t* health_group_hash[HEALTH_GROUP_HASH_MAX];

static health_group_t* dsda_HealthGroup(int id)
{
	int i;
	int hash_id;
	health_group_entry_t* list;

	hash_id = (id % HEALTH_GROUP_HASH_MAX);
	list = health_group_hash[hash_id];

	if(!list)
	{
		list = static_cast<health_group_entry_t *>(Z_CallocLevel(2, sizeof(*list)));
		health_group_hash[hash_id] = list;
		list[0].id = id;
		return &list[0].group;
	}

	for(i = 0; list[i].id; ++i)
		if(list[i].id == id)
			return &list[i].group;

	list = static_cast<health_group_entry_t *>(Z_ReallocLevel(list, sizeof(*list) * (i + 2)));
	health_group_hash[hash_id] = list;
	memset(&list[i + 1], 0, sizeof(*list));
	list[i].id = id;
	return &list[i].group;
}

void dsda_AddLineToHealthGroup(line_t* line)
{
	health_group_t* group;

	group = dsda_HealthGroup(line->healthgroup);
	if(group->health && group->health != line->health)
		Log::Warn("Line {} health does not match group {}!\n", line->iLineID, line->healthgroup);

	group->health = line->health;
	group->line_ids =
		static_cast<int *>(Z_ReallocLevel(group->line_ids, sizeof(*group->line_ids) * (group->size + 1)));
	group->line_ids[group->size++] = line->iLineID;
}

void dsda_ResetHealthGroups()
{
	memset(health_group_hash, 0, HEALTH_GROUP_HASH_MAX * sizeof(*health_group_hash));
}

static void dsda_DamageHealthGroup(int id, mobj_t* source, int damage)
{
	int i;
	health_group_t* group;

	group = dsda_HealthGroup(id);
	group->health -= damage;
	if(group->health < 0)
		group->health = 0;

	for(i = 0; i < group->size; ++i)
	{
		line_t* line;

		line = &lines[group->line_ids[i]];
		line->health = group->health;
		P_ActivateLine(line, source, 0, SPAC_DAMAGE | (line->health ? 0 : SPAC_DEATH));
	}
}

void dsda_DamageLinedef(line_t* line, mobj_t* source, int damage)
{
	if(damage <= 0)
		return;

	if(line->healthgroup)
	{
		dsda_DamageHealthGroup(line->healthgroup, source, damage);
	}
	else
	{
		line->health -= damage;
		if(line->health < 0)
			line->health = 0;

		P_ActivateLine(line, source, 0, SPAC_DAMAGE | (line->health ? 0 : SPAC_DEATH));
	}
}

static dboolean dsda_RadiusAttackLine(line_t* line)
{
	fixed_t dist;
	mobj_t target;
	sector_t* frontsector;
	sector_t* backsector;
	int bombside;
	dboolean sighted;
	const fixed_t fudge = (FRACUNIT >> 4);

	if(!line->health)
		return true;

	bombside = P_PointOnLineSide(bomb.spot->x, bomb.spot->y, line);
	if(line->sidenum[bombside] == NO_INDEX)
		return true;

	dist = dsda_FixedDistancePointToLine(line->v1->x, line->v1->y,
		line->v2->x, line->v2->y,
		bomb.spot->x, bomb.spot->y,
		&target.x, &target.y);
	dist = (dist >> FRACBITS);
	if(dist >= bomb.distance)
		return true;

	// The target is currently "on the line"
	// Move it towards the bomb slightly to make sure it's on the right side
	if(bomb.spot->x - target.x > fudge)
		target.x += fudge;
	if(target.x - bomb.spot->x > fudge)
		target.x -= fudge;
	if(bomb.spot->y - target.y > fudge)
		target.y += fudge;
	if(target.y - bomb.spot->y > fudge)
		target.y -= fudge;

	// P_CheckSight needs subsector
	target.subsector = R_PointInSubsector(target.x, target.y);

	frontsector = sides[line->sidenum[bombside]].sector;
	if(line->sidenum[!bombside] != NO_INDEX)
		backsector = sides[line->sidenum[!bombside]].sector;
	else
		backsector = nullptr;

	sighted = false;

	if(!backsector || (line->flags & LineFlag::BlockEverything) != LineFlag{})
	{
		if(frontsector->ceilingheight > frontsector->floorheight)
		{
			target.z = frontsector->floorheight;
			target.height = frontsector->ceilingheight - frontsector->floorheight;

			sighted = P_CheckSight(&target, bomb.spot);
		}
	}
	else
	{
		fixed_t front_top, back_top, front_bottom, back_bottom;

		front_top = frontsector->ceilingheight;
		back_top = backsector->ceilingheight;
		front_bottom = frontsector->floorheight;
		back_bottom = backsector->floorheight;

		if(front_top > back_top)
		{
			target.z = back_top;
			target.height = front_top - back_top;

			sighted = P_CheckSight(&target, bomb.spot);
		}

		if(!sighted && front_bottom < back_bottom)
		{
			target.z = front_bottom;
			target.height = back_bottom - front_bottom;

			sighted = P_CheckSight(&target, bomb.spot);
		}
	}

	if(sighted)
	{
		int damage;

		damage = P_SplashDamage(dist);

		dsda_DamageLinedef(line, bomb.source, damage);
	}

	return true;
}

void dsda_RadiusAttackDestructibles(int xl, int xh, int yl, int yh)
{
	int x, y;

	// avoid collision with nested P_BlockLinesIterator
	validcount2++;

	for(y = yl; y <= yh; ++y)
		for(x = xl; x <= xh; ++x)
			P_BlockLinesIterator2(x, y, dsda_RadiusAttackLine);
}
