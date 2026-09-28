// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Teleportation.
 */

#include <utility>

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "p_spec.hpp"
#include "p_maputl.hpp"
#include "p_map.hpp"
#include "r_main.hpp"
#include "p_tick.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "p_user.hpp"
#include "smooth.hpp"
#include "m_random.hpp"

#include "dsda/id_list.hpp"
#include "dsda/thing_id.hpp"

// More will be added
static dboolean P_IsTeleportDestination(mobj_t* mo)
{
	return mo->type == MobjType::Teleportman ||
		mo->type == ZMT_TELEPORTDEST2 || mo->type == ZMT_TELEPORTDEST3;
}

static dboolean P_UseTeleportDestinationHeight(mobj_t* mo)
{
	return mo->type == ZMT_TELEPORTDEST2 || mo->type == ZMT_TELEPORTDEST3;
}

static dboolean P_IsMapSpot(mobj_t* mo)
{
	return mo->type == ZMT_MAPSPOT || mo->type == ZMT_MAPSPOT_GRAVITY;
}


static struct
{
	mobj_t* telept;
	dboolean checked;
}* sectors_telept;

static void P_InitTeleptFromSector()
{
	if(sectors_telept == nullptr)
	{
		sectors_telept = static_cast<decltype(sectors_telept)>(Z_CallocLevel(numsectors, sizeof(*sectors_telept)));
	}
}

void P_ResetTeleptList()
{
	sectors_telept = nullptr;
}

void P_ResetTeleptFromSector(int i)
{
	if(sectors_telept == nullptr)
	{
		P_InitTeleptFromSector();
	}

	sectors_telept[i].checked = false;
}

static mobj_t* P_TeleptFromSector(int i)
{
	if(sectors_telept == nullptr)
	{
		P_InitTeleptFromSector();
	}

	if(sectors_telept[i].checked)
	{
		return sectors_telept[i].telept;
	}

	sectors_telept[i].telept = nullptr;

	for(thinker_t* thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
	{
		mobj_t* m;
		if(thinker->function == reinterpret_cast<think_t>(P_MobjThinker)
			&& (m = (mobj_t*)thinker)->type == MobjType::Teleportman
			&& m->subsector->sector->iSectorID == i)
		{
			sectors_telept[i].telept = m;
			break;
		}
	}

	sectors_telept[i].checked = true;
	return sectors_telept[i].telept;
}

static mobj_t* P_TeleportDestination(short thing_id, int tag)
{
	const int* id_p;

	// ZDoom-y
	if(thing_id)
	{
		int count = 0;
		mobj_t* target;
		thing_id_search_t search;

		dsda_ResetThingIDSearch(&search);
		while((target = dsda_FindMobjFromThingID(thing_id, &search)))
		{
			if(P_IsTeleportDestination(target))
			{
				if(!tag || target->subsector->sector->tag == tag)
				{
					++count;
				}
			}
		}

		if(!count)
		{
			if(!tag)
			{
				// Fall back on map spots
				dsda_ResetThingIDSearch(&search);
				while((target = dsda_FindMobjFromThingID(thing_id, &search)))
				{
					if(P_IsMapSpot(target))
					{
						break;
					}
				}

				// Fall back on any nonblocking thing
				if(!target)
				{
					dsda_ResetThingIDSearch(&search);
					while((target = dsda_FindMobjFromThingID(thing_id, &search)))
					{
						if((target->flags & MobjFlag::Solid) == MobjFlag{})
						{
							break;
						}
					}
				}

				return target;
			}
		}
		else
		{
			if(count > 1)
			{
				count = 1 + (P_Random(RandomClass::Hexen) % count);
			}

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingID(thing_id, &search)))
			{
				if(P_IsTeleportDestination(target))
				{
					if(!tag || target->subsector->sector->tag == tag)
					{
						if(!--count)
						{
							return target;
						}
					}
				}
			}
		}

		return nullptr;
	}

	// Legacy
	FIND_SECTORS(id_p, tag)
	{
		mobj_t* m = nullptr;
		if((m = P_TeleptFromSector(*id_p)) != nullptr)
		{
			return m;
		}
	}
	return nullptr;
}
//
// TELEPORTATION
//
// killough 5/3/98: reformatted, cleaned up

static int P_TeleportToDestination(mobj_t* destination, line_t* line, mobj_t* thing, int flags)
{
	fixed_t oldx = thing->x;
	fixed_t oldy = thing->y;
	fixed_t oldz = thing->z;
	fixed_t momx = thing->momx;
	fixed_t momy = thing->momy;
	fixed_t z = thing->z - thing->floorz;
	player_t* player = thing->player;
	angle_t angle = 0;
	fixed_t c = 0, s = 0;

	// Get the angle between the exit thing and source linedef.
	// Rotate 90 degrees, so that walking perpendicularly across
	// teleporter linedef causes thing to exit in the direction
	// indicated by the exit thing.
	if(flags & (TELF_ROTATEBOOM | TELF_ROTATEBOOMINVERSE) && line)
	{
		angle = R_PointToAngle2(0, 0, line->dx, line->dy) - destination->angle + ANG90;

		if(flags & TELF_ROTATEBOOMINVERSE)
			angle = angle + ANG180;

		s = finesine[angle >> ANGLETOFINESHIFT];
		c = finecosine[angle >> ANGLETOFINESHIFT];
	}

	// killough 5/12/98: exclude voodoo dolls:
	if(player && player->mo != thing)
		player = nullptr;

	if(!P_TeleportMove(thing, destination->x, destination->y, false)) /* killough 8/9/98 */
		return 0;

	if(flags & (TELF_ROTATEBOOM | TELF_ROTATEBOOMINVERSE))
	{
		if(line)
		{
			// Rotate thing according to difference in angles
			thing->angle += angle;

			// Rotate thing's momentum to come out of exit just like it entered
			thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
			thing->momy = FixedMul(momy, c) + FixedMul(momx, s);
		}
	}
	else if(!(flags & TELF_KEEPORIENTATION))
	{
		thing->angle = destination->angle;
	}

	if(P_UseTeleportDestinationHeight(destination))
		thing->z = destination->z;
	else if(flags & TELF_KEEPHEIGHT)
		thing->z = thing->floorz + z;
	else if(compatibility_level != CompLevel::Finaldoom)
		thing->z = thing->floorz;
	thing->PrevZ = thing->z;

	if(flags & TELF_SOURCEFOG)
	{
		// spawn teleport fog and emit sound at source
		S_StartMobjSound(P_SpawnMobj(oldx, oldy, oldz, MobjType::Tfog), SfxId::Telept);
	}

	if(flags & TELF_DESTFOG)
	{
		// spawn teleport fog and emit sound at destination
		S_StartMobjSound(
			P_SpawnMobj(
				destination->x + 20 * finecosine[destination->angle >> ANGLETOFINESHIFT],
				destination->y + 20 * finesine[destination->angle >> ANGLETOFINESHIFT],
				thing->z, MobjType::Tfog
			),
			SfxId::Telept
		);
	}

	/* don't move for a bit
	* cph - DEMOSYNC - BOOM had (player) here? */
	if(
		thing->player &&
		((flags & TELF_DESTFOG) || !(flags & TELF_KEEPORIENTATION)) &&
		!(flags & TELF_KEEPVELOCITY)
	)
		thing->reactiontime = 18;

	if(!(flags & TELF_KEEPORIENTATION) && !(flags & TELF_KEEPVELOCITY))
	{
		thing->momx = thing->momy = thing->momz = 0;

		/* killough 10/98: kill all bobbing momentum too */
		if(player)
			player->momx = player->momy = 0;
	}

	if(player)
	{
		// This code was different between silent and non-silent functions.
		if(flags & TELF_KEEPORIENTATION)
		{
			// Adjust player's view, in case there has been a height change
			if(player)
			{
				// Save the current deltaviewheight, used in stepping
				fixed_t deltaviewheight = player->deltaviewheight;

				// Clear deltaviewheight, since we don't want any changes
				player->deltaviewheight = 0;

				// Set player's view according to the newly set parameters
				P_CalcHeight(player);

				// Reset the delta to have the same dynamics as before
				player->deltaviewheight = deltaviewheight;
			}
		}
		else
		{
			player->viewz = thing->z + player->viewheight;
		}

		// e6y
		R_ResetAfterTeleport(player);
	}

	return 1;
}

int EV_TeleportGroup(short group_tid, mobj_t* thing, short source_tid, short dest_tid,
	dboolean move_source, dboolean fog)
{
	int result = 0;
	mobj_t* source;
	mobj_t* dest;
	mobj_t* target;
	thing_id_search_t search;

	dsda_ResetThingIDSearch(&search);
	source = dsda_FindMobjFromThingID(source_tid, &search);

	dest = P_TeleportDestination(dest_tid, 0);

	if(source && dest)
	{
		int flags;
		angle_t an;
		fixed_t dcos, dsin;

		flags = fog ? (TELF_DESTFOG | TELF_SOURCEFOG) : TELF_KEEPORIENTATION;

		an = dest->angle - source->angle;
		dcos = finecosine[an >> ANGLETOFINESHIFT];
		dsin = finesine[an >> ANGLETOFINESHIFT];

		dsda_ResetThingIDSearch(&search);
		while((target = dsda_FindMobjFromThingIDOrMobj(group_tid, thing, &search)))
		{
			fixed_t dx, dy;
			mobj_t target_dest;

			memset(&target_dest, 0, sizeof(target_dest));

			dx = target->x - source->x;
			dy = target->y - source->y;
			target_dest.x = dest->x + FixedMul(dx, dcos) - FixedMul(dy, dsin);
			target_dest.y = dest->y + FixedMul(dx, dsin) + FixedMul(dy, dcos);
			target_dest.angle = target->angle;
			target_dest.type = dest->type;

			result |= P_TeleportToDestination(&target_dest, nullptr, target, flags);
		}

		if(result && move_source)
		{
			P_TeleportToDestination(dest, nullptr, source, TELF_KEEPORIENTATION);
			source->angle = dest->angle;
		}
	}

	return result;
}

int EV_TeleportInSector(int tag, short source_tid, short dest_tid,
	dboolean fog, short group_tid)
{
	int result = 0;
	mobj_t* source;
	mobj_t* dest;
	mobj_t* target;
	thing_id_search_t search;

	dsda_ResetThingIDSearch(&search);
	source = dsda_FindMobjFromThingID(source_tid, &search);

	dest = P_TeleportDestination(dest_tid, 0);

	if(source && dest)
	{
		const int* id_p;
		int flags;
		angle_t an;
		fixed_t dcos, dsin;

		flags = fog ? (TELF_DESTFOG | TELF_SOURCEFOG) : TELF_KEEPORIENTATION;

		an = dest->angle - source->angle;
		dcos = finecosine[an >> ANGLETOFINESHIFT];
		dsin = finesine[an >> ANGLETOFINESHIFT];

		FIND_SECTORS(id_p, tag)
		{
			mobj_in_sector_t mis;

			P_InitSectorSearch(&mis, &sectors[*id_p]);
			while((target = P_FindMobjInSector(&mis)))
			{
				fixed_t dx, dy;
				mobj_t target_dest;

				if(group_tid && target->tid != group_tid)
					continue;

				memset(&target_dest, 0, sizeof(target_dest));

				dx = target->x - source->x;
				dy = target->y - source->y;
				target_dest.x = dest->x + FixedMul(dx, dcos) - FixedMul(dy, dsin);
				target_dest.y = dest->y + FixedMul(dx, dsin) + FixedMul(dy, dcos);
				target_dest.angle = target->angle;
				target_dest.type = dest->type;

				result |= P_TeleportToDestination(&target_dest, nullptr, target, flags);
			}
		}
	}

	return result;
}

extern "C" int EV_CompatibleTeleport(short thing_id, int tag, line_t* line, int side, mobj_t* thing, int flags)
{
	mobj_t* m;

	// don't teleport missiles
	// Don't teleport if hit back of line,
	//  so you can get out of teleporter.
	if(side || (thing->flags & MobjFlag::Missile) != MobjFlag{})
		return 0;

	if((m = P_TeleportDestination(thing_id, tag)) != nullptr)
	{
		return P_TeleportToDestination(m, line, thing, flags);
	}

	return 0;
}

//
// Silent linedef-based TELEPORTATION, by Lee Killough
// Primarily for rooms-over-rooms etc.
// This is the complete player-preserving kind of teleporter.
// It has advantages over the teleporter with thing exits.
//

// maximum fixed_t units to move object to avoid hiccups
#define FUDGEFACTOR 10

int EV_SilentLineTeleport(line_t* line, int side, mobj_t* thing,
	int tag, dboolean reverse)
{
	const int* i;
	line_t* l;

	if(side || (thing->flags & MobjFlag::Missile) != MobjFlag{})
		return 0;

	for(i = dsda_FindLinesFromID(tag); *i >= 0; i++)
		if((l = lines + *i) != line && l->backsector)
		{
			// Get the thing's position along the source linedef
			fixed_t pos = D_abs(line->dx) > D_abs(line->dy) ? FixedDiv(thing->x - line->v1->x, line->dx) : FixedDiv(thing->y - line->v1->y, line->dy);

			// Get the angle between the two linedefs, for rotating
			// orientation and momentum. Rotate 180 degrees, and flip
			// the position across the exit linedef, if reversed.
			angle_t angle = (reverse ? pos = FRACUNIT - pos, 0 : ANG180) +
				R_PointToAngle2(0, 0, l->dx, l->dy) -
				R_PointToAngle2(0, 0, line->dx, line->dy);

			// Interpolate position across the exit linedef
			fixed_t x = l->v2->x - FixedMul(pos, l->dx);
			fixed_t y = l->v2->y - FixedMul(pos, l->dy);

			// Sine, cosine of angle adjustment
			fixed_t s = finesine[angle >> ANGLETOFINESHIFT];
			fixed_t c = finecosine[angle >> ANGLETOFINESHIFT];

			// Maximum distance thing can be moved away from interpolated
			// exit, to ensure that it is on the correct side of exit linedef
			int fudge = FUDGEFACTOR;

			// Whether this is a player, and if so, a pointer to its player_t.
			// Voodoo dolls are excluded by making sure thing->player->mo==thing.
			player_t* player = thing->player && thing->player->mo == thing ? thing->player : nullptr;

			// Whether walking towards first side of exit linedef steps down
			int stepdown =
				l->frontsector->floorheight < l->backsector->floorheight;

			// Height of thing above ground
			fixed_t z = thing->z - thing->floorz;

			// Side to exit the linedef on positionally.
			//
			// Notes:
			//
			// This flag concerns exit position, not momentum. Due to
			// roundoff error, the thing can land on either the left or
			// the right side of the exit linedef, and steps must be
			// taken to make sure it does not end up on the wrong side.
			//
			// Exit momentum is always towards side 1 in a reversed
			// teleporter, and always towards side 0 otherwise.
			//
			// Exiting positionally on side 1 is always safe, as far
			// as avoiding oscillations and stuck-in-wall problems,
			// but may not be optimum for non-reversed teleporters.
			//
			// Exiting on side 0 can cause oscillations if momentum
			// is towards side 1, as it is with reversed teleporters.
			//
			// Exiting on side 1 slightly improves player viewing
			// when going down a step on a non-reversed teleporter.

			int side = reverse || (player && stepdown);

			// Make sure we are on correct side of exit linedef.
			while(P_PointOnLineSide(x, y, l) != side && --fudge >= 0)
				if(D_abs(l->dx) > D_abs(l->dy))
					y -= (l->dx < 0) != side ? -1 : 1;
				else
					x += (l->dy < 0) != side ? -1 : 1;

			// Attempt to teleport, aborting if blocked
			if(!P_TeleportMove(thing, x, y, false)) /* killough 8/9/98 */
				return 0;

			// e6y
			if(player && player->mo == thing)
				R_ResetAfterTeleport(player);

			// Adjust z position to be same height above ground as before.
			// Ground level at the exit is measured as the higher of the
			// two floor heights at the exit linedef.
			thing->z = z + sides[l->sidenum[stepdown]].sector->floorheight;
			thing->PrevZ = thing->z;

			// Rotate thing's orientation according to difference in linedef angles
			thing->angle += angle;

			// Momentum of thing crossing teleporter linedef
			x = thing->momx;
			y = thing->momy;

			// Rotate thing's momentum to come out of exit just like it entered
			thing->momx = FixedMul(x, c) - FixedMul(y, s);
			thing->momy = FixedMul(y, c) + FixedMul(x, s);

			// Adjust a player's view, in case there has been a height change
			if(player)
			{
				// Save the current deltaviewheight, used in stepping
				fixed_t deltaviewheight = player->deltaviewheight;

				// Clear deltaviewheight, since we don't want any changes now
				player->deltaviewheight = 0;

				// Set player's view according to the newly set parameters
				P_CalcHeight(player);

				// Reset the delta to have the same dynamics as before
				player->deltaviewheight = deltaviewheight;
			}

			// e6y
			if(player && player->mo == thing)
				R_ResetAfterTeleport(player);

			return 1;
		}
	return 0;
}

// heretic

#include "heretic/def.hpp"

dboolean P_Teleport(mobj_t* thing, fixed_t x, fixed_t y, angle_t angle, dboolean useFog)
{
	fixed_t oldx;
	fixed_t oldy;
	fixed_t oldz;
	fixed_t aboveFloor;
	fixed_t fogDelta;
	player_t* player;
	unsigned an;
	mobj_t* fog;

	oldx = thing->x;
	oldy = thing->y;
	oldz = thing->z;
	aboveFloor = thing->z - thing->floorz;
	if(!P_TeleportMove(thing, x, y, false))
	{
		return (false);
	}
	if(thing->player)
	{
		player = thing->player;
		if(player->powers[std::to_underlying(PowerType::Flight)] && aboveFloor)
		{
			thing->z = thing->floorz + aboveFloor;
			if(thing->z + thing->height > thing->ceilingz)
			{
				thing->z = thing->ceilingz - thing->height;
			}
			player->viewz = thing->z + player->viewheight;
		}
		else
		{
			thing->z = thing->floorz;
			player->viewz = thing->z + player->viewheight;
			if(useFog)
			{
				player->lookdir = 0;
			}
		}
	}
	else if((thing->flags & MobjFlag::Missile) != MobjFlag{})
	{
		thing->z = thing->floorz + aboveFloor;
		if(thing->z + thing->height > thing->ceilingz)
		{
			thing->z = thing->ceilingz - thing->height;
		}
	}
	else
	{
		thing->z = thing->floorz;
	}
	// Spawn teleport fog at source and destination
	if(useFog)
	{
		fogDelta = (thing->flags & MobjFlag::Missile) != MobjFlag{} ? 0 : TELEFOGHEIGHT;
		fog = P_SpawnMobj(oldx, oldy, oldz + fogDelta, static_cast<MobjType>(g_mt_tfog));
		S_StartMobjSound(fog, g_sfx_telept);
		an = angle >> ANGLETOFINESHIFT;
		fog = P_SpawnMobj(x + 20 * finecosine[an],
			y + 20 * finesine[an], thing->z + fogDelta, static_cast<MobjType>(g_mt_tfog));
		S_StartMobjSound(fog, g_sfx_telept);
		if(thing->player &&
			!thing->player->powers[std::to_underlying(PowerType::WeaponLevel2)] &&
			!thing->player->powers[std::to_underlying(PowerType::Speed)])
		{
			// Freeze player for about .5 sec
			thing->reactiontime = 18;
		}
		thing->angle = angle;
	}

	if(hexen)
	{
		if((thing->flags2 & MobjFlag2::FootClip) != MobjFlag2{})
		{
			if(thing->z == thing->subsector->sector->floorheight
				&& P_GetThingFloorType(thing) > FloorType::Solid)
			{
				thing->floorclip = 10 * FRACUNIT;
			}
			else
			{
				thing->floorclip = 0;
			}
		}
	}
	else
	{
		if((thing->flags2 & MobjFlag2::FootClip) != MobjFlag2{}
			&& P_GetThingFloorType(thing) != FloorType::Solid)
		{
			thing->flags2 |= MobjFlag2::FeetAreClipped;
		}
		else if((thing->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
		{
			thing->flags2 -= MobjFlag2::FeetAreClipped;
		}
	}

	if((thing->flags & MobjFlag::Missile) != MobjFlag{})
	{
		angle >>= ANGLETOFINESHIFT;
		thing->momx = FixedMul(thing->info->speed, finecosine[angle]);
		thing->momy = FixedMul(thing->info->speed, finesine[angle]);
	}
	else if(useFog)
	{
		thing->momx = thing->momy = thing->momz = 0;
	}

	if(thing->player)
	{
		R_ResetAfterTeleport(thing->player);
	}

	return (true);
}

extern "C" int EV_HereticTeleport(short thing_id, int tag, line_t* line, int side, mobj_t* thing, int flags)
{
	int i;
	mobj_t* m;
	thinker_t* thinker;
	sector_t* sector;

	if((thing->flags2 & MobjFlag2::NoTeleport) != MobjFlag2{})
	{
		return (false);
	}
	if(side == 1)
	{
		// Don't teleport when crossing back side
		return (false);
	}
	for(i = 0; i < numsectors; i++)
	{
		if(sectors[i].tag == tag)
		{
			for(thinker = thinkercap.next; thinker != &thinkercap;
				thinker = thinker->next)
			{
				if(thinker->function != reinterpret_cast<think_t>(P_MobjThinker))
				{
					// Not a mobj
					continue;
				}
				m = (mobj_t*)thinker;
				if(m->type != MobjType::HereticTeleportman)
				{
					// Not a teleportman
					continue;
				}
				sector = m->subsector->sector;
				if(sector - sectors != i)
				{
					// Wrong sector
					continue;
				}
				return (P_Teleport(thing, m->x, m->y, m->angle, true));
			}
		}
	}
	return (false);
}

// hexen

#include "m_random.hpp"
#include "lprintf.hpp"

dboolean EV_HexenTeleport(int tid, mobj_t* thing, dboolean fog)
{
	int i;
	int count;
	mobj_t* mo;
	int searcher;

	if(!thing)
	{
		// Teleport function called with an invalid mobj
		return false;
	}
	if((thing->flags2 & MobjFlag2::NoTeleport) != MobjFlag2{})
	{
		return false;
	}
	count = 0;
	searcher = -1;
	while(P_FindMobjFromTID(tid, &searcher) != nullptr)
	{
		count++;
	}
	if(count == 0)
	{
		return false;
	}
	count = 1 + (P_Random(RandomClass::Hexen) % count);
	searcher = -1;
	mo = nullptr;

	for(i = 0; i < count; i++)
	{
		mo = P_FindMobjFromTID(tid, &searcher);
	}
	if(mo == nullptr)
	{
		Log::Fatal("Can't find teleport mapspot\n");
	}
	return P_Teleport(thing, mo->x, mo->y, mo->angle, fog);
}
