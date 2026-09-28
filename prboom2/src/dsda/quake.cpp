// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Quake

#include "doomstat.hpp"
#include "m_random.hpp"
#include "p_maputl.hpp"
#include "p_inter.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"

#include "dsda/global.hpp"

// TODO: move here
extern int localQuakeHappening[MAX_MAXPLAYERS];

// The unit of quake distance is 64 (2^6)
#define DISTBITS 6

void dsda_ResetQuakes()
{
	memset(localQuakeHappening, 0, g_maxplayers * sizeof(*localQuakeHappening));
}

void dsda_UpdateQuakeIntensity(int player_num, int intensity)
{
	localQuakeHappening[player_num] = intensity;
}

void dsda_UpdateQuake(quake_t* quake)
{
	int i;

	for(i = 0; i < g_maxplayers; ++i)
	{
		mobj_t* mo;
		fixed_t dist;

		if(!playeringame[i] || (players[i].cheats & CheatFlag::NoClip) != CheatFlag{})
			continue;

		mo = players[i].mo;
		dist = P_AproxDistance(quake->location->x - mo->x,
			quake->location->y - mo->y) >> (FRACBITS + DISTBITS);

		// TODO: This won't get changed until a quake ends if we leave the radius
		if(dist < quake->tremor_radius)
			dsda_UpdateQuakeIntensity(i, quake->intensity);

		if(dist < quake->damage_radius && mo->z <= mo->floorz)
		{
			angle_t an;

			if(P_Random(RandomClass::Hexen) < 50)
				P_DamageMobj(mo, nullptr, nullptr, HITDICE(1));

			an = P_Random(RandomClass::Hexen) << 24;
			P_ThrustMobj(mo, an, quake->intensity << (FRACBITS - 1));
		}
	}

	--quake->duration;
	if(quake->duration <= 0)
	{
		dsda_ResetQuakes();
		P_RemoveThinker(&quake->thinker);
	}
}

void dsda_SpawnQuake(mobj_t* location, int intensity, int duration,
	int damage_radius, int tremor_radius)
{
	quake_t* quake;

	quake = static_cast<quake_t*>(Z_MallocLevel(sizeof(*quake)));
	memset(quake, 0, sizeof(*quake));
	P_AddThinker(&quake->thinker);
	quake->thinker.function = reinterpret_cast<think_t>(dsda_UpdateQuake);
	P_SetTarget(&quake->location, location);
	quake->intensity = intensity;
	quake->duration = duration;
	quake->damage_radius = damage_radius;
	quake->tremor_radius = tremor_radius;
}
