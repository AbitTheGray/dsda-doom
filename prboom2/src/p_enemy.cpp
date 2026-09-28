// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 */

#include <utility>

#include "doomstat.hpp"
#include "m_random.hpp"
#include "r_main.hpp"
#include "p_maputl.hpp"
#include "p_map.hpp"
#include "p_setup.hpp"
#include "p_spec.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "p_inter.hpp"
#include "g_game.hpp"
#include "p_enemy.hpp"
#include "p_tick.hpp"
#include "i_sound.hpp"
#include "m_bbox.hpp"
#include "hu_stuff.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"//e6y

#include "dsda.hpp"
#include "dsda/configuration.hpp"
#include "dsda/id_list.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/skill_info.hpp"

static mobj_t* current_actor;

enum struct DirType : int32_t
{
	East,
	NorthEast,
	North,
	NorthWest,
	West,
	SouthWest,
	South,
	SouthEast,
	NoDir,
	Count
};

// mobj_t::movedir is stored as a short, because the savegame format says so
static int16_t MoveDir(const DirType dir)
{
	return static_cast<int16_t>(std::to_underlying(dir));
}

static void P_NewChaseDir(mobj_t* actor);
extern "C" void P_ZBumpCheck(mobj_t*); // phares

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// killough 5/5/98: reformatted, cleaned up

static void P_RecursiveSound(sector_t* sec, int soundblocks, mobj_t* soundtarget)
{
	int i;

	// wake up all monsters in this sector
	if(sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
		return; // already flooded

	sec->validcount = validcount;
	sec->soundtraversed = soundblocks + 1;
	P_SetTarget(&sec->soundtarget, soundtarget);

	for(i = 0; i < sec->linecount; i++)
	{
		sector_t* other;
		line_t* check = sec->lines[i];

		if(!(check->flags & ML_TWOSIDED))
			continue;

		P_LineOpening(check, nullptr);

		if(line_opening.range <= 0)
			continue; // closed door

		other = sides[check->sidenum[sides[check->sidenum[0]].sector == sec]].sector;

		if(!(check->flags & ML_SOUNDBLOCK))
			P_RecursiveSound(other, soundblocks, soundtarget);
		else if(!soundblocks)
			P_RecursiveSound(other, 1, soundtarget);
	}
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t* target, mobj_t* emitter)
{
	if(target != nullptr && target->player && ((target->player->cheats & CheatFlag::NoTarget) != CheatFlag{}))
		return;

	validcount++;
	P_RecursiveSound(emitter->subsector->sector, 0, target);
}

//
// P_CheckRange
//

static dboolean P_CheckRange(mobj_t* actor, fixed_t range)
{
	mobj_t* pl = actor->target;

	return // killough 7/18/98: friendly monsters don't attack other friends
		pl &&
		(actor->flags & pl->flags & MobjFlag::Friend) == MobjFlag{} &&
		P_AproxDistance(pl->x - actor->x, pl->y - actor->y) < range &&
		P_CheckSight(actor, actor->target) &&
		( // finite height!
			// TODO: possible "passover" mapinfo flag
			!(raven) ||
			(
				pl->z <= actor->z + actor->height &&
				actor->z <= pl->z + pl->height
			)
		);
}

//
// P_CheckMeleeRange
//

static dboolean P_CheckMeleeRange(mobj_t* actor)
{
	int range;

	if((actor->subsector->sector->flags & SectorFlag::NoAttack) != SectorFlag{})
		return false;

	range = actor->info->meleerange;

	if(compatibility_level != CompLevel::Doom12)
		range += actor->target->info->radius - 20 * FRACUNIT;

	return P_CheckRange(actor, range);
}

//
// P_HitFriend()
//
// killough 12/98
// This function tries to prevent shooting at friends

static dboolean P_HitFriend(mobj_t* actor)
{
	return (actor->flags & MobjFlag::Friend) != MobjFlag{} && actor->target &&
		(P_AimLineAttack(actor,
				R_PointToAngle2(actor->x, actor->y,
					actor->target->x, actor->target->y),
				P_AproxDistance(actor->x - actor->target->x,
					actor->y - actor->target->y), MobjFlag{}),
			linetarget) && linetarget != actor->target &&
		((linetarget->flags ^ actor->flags) & MobjFlag::Friend) == MobjFlag{};
}

//
// P_CheckMissileRange
//
static dboolean P_CheckMissileRange(mobj_t* actor)
{
	fixed_t dist;

	if((actor->subsector->sector->flags & SectorFlag::NoAttack) != SectorFlag{})
		return false;

	if(!P_CheckSight(actor, actor->target))
		return false;

	if((actor->flags & MobjFlag::JustHit) != MobjFlag{})
	{
		// the target just hit the enemy, so fight back!
		actor->flags -= MobjFlag::JustHit;

		/* killough 7/18/98: no friendly fire at corpses
		* killough 11/98: prevent too much infighting among friends
		* cph - yikes, talk about fitting everything on one line... */

		return
			(actor->flags & MobjFlag::Friend) == MobjFlag{} ||
			(actor->target->health > 0 &&
				((actor->target->flags & MobjFlag::Friend) == MobjFlag{} ||
					(actor->target->player ? monster_infighting || P_Random(RandomClass::Defect) > 128 : (actor->target->flags & MobjFlag::JustHit) == MobjFlag{} && P_Random(RandomClass::Defect) > 128)));
	}

	/* killough 7/18/98: friendly monsters don't attack other friendly
	* monsters or players (except when attacked, and then only once)
	*/
	if((actor->flags & actor->target->flags & MobjFlag::Friend) != MobjFlag{})
		return false;

	if(actor->reactiontime)
		return false; // do not attack yet

	// OPTIMIZE: get this from a global checksight
	dist = P_AproxDistance(actor->x - actor->target->x,
		actor->y - actor->target->y) - 64 * FRACUNIT;

	if(actor->info->meleestate == StateId::Null)
		dist -= 128 * FRACUNIT; // no melee attack, so fire more

	dist >>= FRACBITS;

	if((actor->flags2 & MobjFlag2::ShortMRange) != MobjFlag2{})
		if(dist > 14 * 64)
			return false; // too far away

	if((actor->flags2 & MobjFlag2::LongMelee) != MobjFlag2{})
	{
		if(dist < 196)
			return false; // close for fist attack
	}

	if((actor->flags2 & MobjFlag2::RangeHalf) != MobjFlag2{})
		dist >>= 1;

	if(dist > 200)
		dist = 200;

	if((actor->flags2 & MobjFlag2::HigherMProb) != MobjFlag2{} && dist > 160)
		dist = 160;

	if(P_Random(RandomClass::Missrange) < dist)
		return false;

	if(P_HitFriend(actor))
		return false;

	return true;
}

/*
 * P_IsOnLift
 *
 * killough 9/9/98:
 *
 * Returns true if the object is on a lift. Used for AI,
 * since it may indicate the need for crowded conditions,
 * or that a monster should stay on the lift for a while
 * while it goes up or down.
 */

static dboolean P_IsOnLift(const mobj_t* actor)
{
	const sector_t* sec = actor->subsector->sector;
	const int* l;

	// Short-circuit: it's on a lift which is active.
	if(sec->floordata && ((thinker_t*)sec->floordata)->function == reinterpret_cast<think_t>(T_PlatRaise))
		return true;

	// Check to see if it's in a sector which can be activated as a lift.
	if(sec->tag)
		for(l = dsda_FindLinesFromID(sec->tag); *l >= 0; l++)
			switch(lines[*l].special)
			{
				case 10:
				case 14:
				case 15:
				case 20:
				case 21:
				case 22:
				case 47:
				case 53:
				case 62:
				case 66:
				case 67:
				case 68:
				case 87:
				case 88:
				case 95:
				case 120:
				case 121:
				case 122:
				case 123:
				case 143:
				case 162:
				case 163:
				case 181:
				case 182:
				case 144:
				case 148:
				case 149:
				case 211:
				case 227:
				case 228:
				case 231:
				case 232:
				case 235:
				case 236:
					return true;
			}

	return false;
}

/*
 * P_IsUnderDamage
 *
 * killough 9/9/98:
 *
 * Returns nonzero if the object is under damage based on
 * their current position. Returns 1 if the damage is moderate,
 * -1 if it is serious. Used for AI.
 */

static int P_IsUnderDamage(mobj_t* actor)
{
	const struct msecnode_s* seclist;
	const ceiling_t* cl; // Crushing ceiling
	int dir = 0;
	for(seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
		if((cl = static_cast<const ceiling_t*>(seclist->m_sector->ceilingdata)) &&
			cl->thinker.function == reinterpret_cast<think_t>(T_MoveCeiling))
			dir |= cl->direction;
	return dir;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static fixed_t xspeed[8] = {FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000};
static fixed_t yspeed[8] = {0, 47000,FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000};

// 1/11/98 killough: Limit removed on special lines crossed
extern line_t** spechit; // New code -- killough
extern int numspechit;

static dboolean P_Move(mobj_t* actor, dboolean dropoff) /* killough 9/12/98 */
{
	fixed_t tryx, tryy, deltax, deltay, origx, origy;
	dboolean try_ok;
	int movefactor = ORIG_FRICTION_FACTOR; // killough 10/98
	int friction = ORIG_FRICTION;
	int speed;

	if((actor->flags2 & MobjFlag2::Blasted) != MobjFlag2{})
		return true;
	if(actor->movedir == MoveDir(DirType::NoDir))
		return false;

#ifdef RANGECHECK
	if((unsigned)actor->movedir >= 8)
		Log::Fatal("P_Move: Weird actor->movedir!");
#endif

	// killough 10/98: make monsters get affected by ice and sludge too:

	if(monster_friction)
		movefactor = P_GetMoveFactor(actor, &friction);

	speed = actor->info->speed;

	if(friction < ORIG_FRICTION && // sludge
		!(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR - movefactor) / 2)
			* speed) / ORIG_FRICTION_FACTOR))
		speed = 1; // always give the monster a little bit of speed

	tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
	tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);

	try_ok = P_TryMove(actor, tryx, tryy, dropoff);

	// killough 10/98:
	// Let normal momentum carry them, instead of steptoeing them across ice.

	if(try_ok && friction > ORIG_FRICTION)
	{
		actor->x = origx;
		actor->y = origy;
		movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
		actor->momx += FixedMul(deltax, movefactor);
		actor->momy += FixedMul(deltay, movefactor);
	}

	// [RH] If a walking monster is no longer on the floor, move it down
	// to the floor if it is within MaxStepHeight, presuming that it is
	// actually walking down a step.
	if(
		try_ok &&
		map_format.zdoom &&
		actor->z > actor->floorz &&
		actor->z <= actor->floorz + (24 << FRACBITS) &&
		(actor->flags & MobjFlag::NoGravity) == MobjFlag{} &&
		(actor->flags2 & MobjFlag2::OnMobj) == MobjFlag2{})
	{
		fixed_t saved_z = actor->z;

		actor->z = actor->floorz;

		// Make sure that there isn't some other actor between us and
		// the floor we could get stuck in. The old code did not do this.
		if(!P_CheckPosition(actor, actor->x, actor->y))
		{
			actor->z = saved_z;
		}
	}

	if(!try_ok)
	{
		// open any specials
		int good;

		if((actor->flags & MobjFlag::Float) != MobjFlag{} && floatok)
		{
			if(actor->z < tmfloorz) // must adjust height
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;

			actor->flags |= MobjFlag::InFloat;

			return true;
		}

		if(!numspechit)
			return false;

		actor->movedir = MoveDir(DirType::NoDir);

		/* if the special is not a door that can be opened, return false
		*
		* killough 8/9/98: this is what caused monsters to get stuck in
		* doortracks, because it thought that the monster freed itself
		* by opening a door, even if it was moving towards the doortrack,
		* and not the door itself.
		*
		* killough 9/9/98: If a line blocking the monster is activated,
		* return true 90% of the time. If a line blocking the monster is
		* not activated, but some other line is, return false 90% of the
		* time. A bit of randomness is needed to ensure it's free from
		* lockups, but for most cases, it returns the correct result.
		*
		* Do NOT simply return false 1/4th of the time (causes monsters to
		* back out when they shouldn't, and creates secondary stickiness).
		*/

		for(good = false; numspechit--;)
			if(P_UseSpecialLine(actor, spechit[numspechit], 0, false))
				good |= spechit[numspechit] == blockline ? 1 : 2;

		// There are checks elsewhere for numspechit == 0, so we don't want to
		// leave numspechit == -1.
		numspechit = 0;

		if(raven) return good > 0;

		/* cph - compatibility maze here
		* Boom v2.01 and orig. Doom return "good"
		* Boom v2.02 and LxDoom return good && (P_Random(pr_trywalk)&3)
		* MBF plays even more games
		*/
		if(!good || comp[std::to_underlying(CompOption::DoorStuck)]) return good;
		if(!mbf_features)
			return (P_Random(RandomClass::Trywalk) & 3); /* jff 8/13/98 */
		else                                   /* finally, MBF code */
			return ((P_Random(RandomClass::Opendoor) >= 230) ^ (good & 1));
	}
	else
		actor->flags -= MobjFlag::InFloat;

	/* killough 11/98: fall more slowly, under gravity, if felldown==true */
	if(!map_format.zdoom && (actor->flags & MobjFlag::Float) == MobjFlag{} && (!felldown || !mbf_features))
	{
		if(raven && actor->z > actor->floorz)
		{
			P_HitFloor(actor);
		}
		actor->z = actor->floorz;
	}

	return true;
}

/*
 * P_SmartMove
 *
 * killough 9/12/98: Same as P_Move, except smarter
 */

static dboolean P_SmartMove(mobj_t* actor)
{
	mobj_t* target = actor->target;
	int on_lift, dropoff = false, under_damage;
	int tmp_monster_avoid_hazards = (prboom_comp[std::to_underlying(PrboomComp::MonsterAvoidHazards)].state ? true : (demo_compatibility ? false : monster_avoid_hazards)); //e6y

	/* killough 9/12/98: Stay on a lift if target is on one */
	on_lift = !comp[std::to_underlying(CompOption::StayLift)]
		&& target && target->health > 0
		&& target->subsector->sector->tag == actor->subsector->sector->tag &&
		P_IsOnLift(actor);

	under_damage = tmp_monster_avoid_hazards && P_IsUnderDamage(actor); //e6y

	// killough 10/98: allow dogs to drop off of taller ledges sometimes.
	// dropoff==1 means always allow it, dropoff==2 means only up to 128 high,
	// and only if the target is immediately on the other side of the line.

	// haleyjd: allow all friends of HelperType to also jump down

	if((actor->type == MobjType::Dogs || (actor->type == static_cast<MobjType>(HelperThing - 1) && (actor->flags & MobjFlag::Friend) != MobjFlag{}))
		&& target && dog_jumping &&
		((target->flags ^ actor->flags) & MobjFlag::Friend) == MobjFlag{} &&
		P_AproxDistance(actor->x - target->x,
			actor->y - target->y) < FRACUNIT * 144 &&
		P_Random(RandomClass::Dropoff) < 235)
		dropoff = 2;

	if(!P_Move(actor, dropoff))
		return false;

	// killough 9/9/98: avoid crushing ceilings or other damaging areas
	if(
		(on_lift && P_Random(RandomClass::Stayonlift) < 230 && // Stay on lift
			!P_IsOnLift(actor))
		||
		(tmp_monster_avoid_hazards && !under_damage && //e6y  // Get away from damage
			(under_damage = P_IsUnderDamage(actor)) &&
			(under_damage < 0 || P_Random(RandomClass::Avoidcrush) < 200))
	)
		actor->movedir = MoveDir(DirType::NoDir); // avoid the area (most of the time anyway)

	return true;
}

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//

// HERETIC_NOTE: Quite sure P_SmartMove == P_Move for heretic
static dboolean P_TryWalk(mobj_t* actor)
{
	if(!P_SmartMove(actor))
		return false;
	actor->movecount = P_Random(RandomClass::Trywalk) & 15;
	return true;
}

//
// P_DoNewChaseDir
//
// killough 9/8/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//

static void P_DoNewChaseDir(mobj_t* actor, fixed_t deltax, fixed_t deltay)
{
	DirType xdir, ydir, tdir;
	DirType olddir = static_cast<DirType>(actor->movedir);
	DirType turnaround = olddir;

	if(turnaround != DirType::NoDir) // find reverse direction
		turnaround = static_cast<DirType>(std::to_underlying(turnaround) ^ 4);

	xdir =
		deltax > 10 * FRACUNIT ? DirType::East : deltax < -10 * FRACUNIT ? DirType::West : DirType::NoDir;

	ydir =
		deltay < -10 * FRACUNIT ? DirType::South : deltay > 10 * FRACUNIT ? DirType::North : DirType::NoDir;

	// try direct route
	if(xdir != DirType::NoDir && ydir != DirType::NoDir && std::to_underlying(turnaround) !=
		(actor->movedir = MoveDir(deltay < 0 ? deltax > 0 ? DirType::SouthEast : DirType::SouthWest : deltax > 0 ? DirType::NorthEast : DirType::NorthWest)) && P_TryWalk(actor))
		return;

	// try other directions
	if(P_Random(RandomClass::Newchase) > 200 || D_abs(deltay) > D_abs(deltax))
		tdir = xdir, xdir = ydir, ydir = tdir;

	if((xdir == turnaround ? xdir = DirType::NoDir : xdir) != DirType::NoDir &&
		(actor->movedir = MoveDir(xdir), P_TryWalk(actor)))
		return; // either moved forward or attacked

	if((ydir == turnaround ? ydir = DirType::NoDir : ydir) != DirType::NoDir &&
		(actor->movedir = MoveDir(ydir), P_TryWalk(actor)))
		return;

	// there is no direct path to the player, so pick another direction.
	if(olddir != DirType::NoDir && (actor->movedir = MoveDir(olddir), P_TryWalk(actor)))
		return;

	// randomly determine direction of search
	if(P_Random(RandomClass::Newchasedir) & 1)
	{
		for(int32_t dir = std::to_underlying(DirType::East); dir <= std::to_underlying(DirType::SouthEast); dir++)
		{
			tdir = static_cast<DirType>(dir);
			if(tdir != turnaround && (actor->movedir = MoveDir(tdir), P_TryWalk(actor)))
				return;
		}
	}
	else
		for(int32_t dir = std::to_underlying(DirType::SouthEast); dir >= std::to_underlying(DirType::East); dir--)
		{
			tdir = static_cast<DirType>(dir);
			if(tdir != turnaround && (actor->movedir = MoveDir(tdir), P_TryWalk(actor)))
				return;
		}

	if((actor->movedir = MoveDir(turnaround)) != std::to_underlying(DirType::NoDir) && !P_TryWalk(actor))
		actor->movedir = MoveDir(DirType::NoDir);
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.

static fixed_t dropoff_deltax, dropoff_deltay, floorz;

static dboolean PIT_AvoidDropoff(line_t* line)
{
	if(line->backsector && // Ignore one-sided linedefs
		tmbbox[std::to_underlying(BoxEdge::Right)] > line->bbox[std::to_underlying(BoxEdge::Left)] &&
		tmbbox[std::to_underlying(BoxEdge::Left)] < line->bbox[std::to_underlying(BoxEdge::Right)] &&
		tmbbox[std::to_underlying(BoxEdge::Top)] > line->bbox[std::to_underlying(BoxEdge::Bottom)] && // Linedef must be contacted
		tmbbox[std::to_underlying(BoxEdge::Bottom)] < line->bbox[std::to_underlying(BoxEdge::Top)] &&
		P_BoxOnLineSide(tmbbox, line) == -1)
	{
		fixed_t front = line->frontsector->floorheight;
		fixed_t back = line->backsector->floorheight;
		angle_t angle;

		// The monster must contact one of the two floors,
		// and the other must be a tall dropoff (more than 24).

		if(back == floorz && front < floorz - FRACUNIT * 24)
			angle = R_PointToAngle2(0, 0, line->dx, line->dy); // front side dropoff
		else if(front == floorz && back < floorz - FRACUNIT * 24)
			angle = R_PointToAngle2(line->dx, line->dy, 0, 0); // back side dropoff
		else
			return true;

		// Move away from dropoff at a standard speed.
		// Multiple contacted linedefs are cumulative (e.g. hanging over corner)
		dropoff_deltax -= finesine[angle >> ANGLETOFINESHIFT] * 32;
		dropoff_deltay += finecosine[angle >> ANGLETOFINESHIFT] * 32;
	}
	return true;
}

//
// Driver for above
//

static fixed_t P_AvoidDropoff(mobj_t* actor)
{
	int yh = P_GetSafeBlockY((tmbbox[std::to_underlying(BoxEdge::Top)] = actor->y + actor->radius) - bmaporgy);
	int yl = P_GetSafeBlockY((tmbbox[std::to_underlying(BoxEdge::Bottom)] = actor->y - actor->radius) - bmaporgy);
	int xh = P_GetSafeBlockX((tmbbox[std::to_underlying(BoxEdge::Right)] = actor->x + actor->radius) - bmaporgx);
	int xl = P_GetSafeBlockX((tmbbox[std::to_underlying(BoxEdge::Left)] = actor->x - actor->radius) - bmaporgx);
	int bx, by;

	floorz = actor->z; // remember floor height

	dropoff_deltax = dropoff_deltay = 0;

	// check lines

	validcount++;
	for(bx = xl; bx <= xh; bx++)
		for(by = yl; by <= yh; by++)
			P_BlockLinesIterator(bx, by, PIT_AvoidDropoff); // all contacted lines

	return dropoff_deltax | dropoff_deltay; // Non-zero if movement prescribed
}

//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//

static void P_NewChaseDir(mobj_t* actor)
{
	mobj_t* target = actor->target;
	fixed_t deltax = target->x - actor->x;
	fixed_t deltay = target->y - actor->y;

	// killough 8/8/98: sometimes move away from target, keeping distance
	//
	// 1) Stay a certain distance away from a friend, to avoid being in their way
	// 2) Take advantage over an enemy without missiles, by keeping distance

	actor->strafecount = 0;

	if(mbf_features)
	{
		if(
			actor->floorz - actor->dropoffz > FRACUNIT * 24 &&
			actor->z <= actor->floorz &&
			(actor->flags & (MobjFlag::DropOff | MobjFlag::Float)) == MobjFlag{} &&
			!comp[std::to_underlying(CompOption::DropOff)] &&
			P_AvoidDropoff(actor)
		) /* Move away from dropoff */
		{
			P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

			// If moving away from dropoff, set movecount to 1 so that
			// small steps are taken to get monster away from dropoff.

			actor->movecount = 1;
			return;
		}
		else
		{
			fixed_t dist = P_AproxDistance(deltax, deltay);

			// Move away from friends when too close, except
			// in certain situations (e.g. a crowded lift)

			if((actor->flags & target->flags & MobjFlag::Friend) != MobjFlag{} &&
				distfriend << FRACBITS > dist &&
				!P_IsOnLift(target) && !P_IsUnderDamage(actor))
			{
				deltax = -deltax, deltay = -deltay;
			}
			else if(target->health > 0 && ((actor->flags ^ target->flags) & MobjFlag::Friend) != MobjFlag{})
			{
				// Live enemy target
				if(
					monster_backing &&
					actor->info->missilestate != StateId::Null &&
					actor->type != MobjType::Skull &&
					(
						(target->info->missilestate == StateId::Null && dist < target->info->meleerange * 2) ||
						(
							target->player && dist < target->player->mo->info->meleerange * 3 &&
							weaponinfo[std::to_underlying(target->player->readyweapon)].flags & WPF_FLEEMELEE
						)
					)
				)
				{
					// Back away from melee attacker
					actor->strafecount = P_Random(RandomClass::Enemystrafe) & 15;
					deltax = -deltax, deltay = -deltay;
				}
			}
		}
	}

	P_DoNewChaseDir(actor, deltax, deltay);

	// If strafing, set movecount to strafecount so that old Doom
	// logic still works the same, except in the strafing part

	if(actor->strafecount)
		actor->movecount = actor->strafecount;
}

//
// P_IsVisible
//
// killough 9/9/98: whether a target is visible to a monster
//

static dboolean P_IsVisible(mobj_t* actor, mobj_t* mo, dboolean allaround)
{
	if(!allaround)
	{
		angle_t an = R_PointToAngle2(actor->x, actor->y,
			mo->x, mo->y) - actor->angle;
		if(an > ANG90 && an < ANG270 &&
			P_AproxDistance(mo->x - actor->x, mo->y - actor->y) > WAKEUPRANGE)
			return false;
	}
	return P_CheckSight(actor, mo);
}

//
// PIT_FindTarget
//
// killough 9/5/98
//
// Finds monster targets for other monsters
//

static int current_allaround;

static dboolean PIT_FindTarget(mobj_t* mo)
{
	mobj_t* actor = current_actor;

	if(!(((mo->flags ^ actor->flags) & MobjFlag::Friend) != MobjFlag{} && // Invalid target
		mo->health > 0 && ((mo->flags & MobjFlag::CountKill) != MobjFlag{} || mo->type == MobjType::Skull)))
		return true;

	// If the monster is already engaged in a one-on-one attack
	// with a healthy friend, don't attack around 60% the time
	{
		const mobj_t* targ = mo->target;
		if(targ && targ->target == mo &&
			P_Random(RandomClass::Skiptarget) > 100 &&
			((targ->flags ^ mo->flags) & MobjFlag::Friend) != MobjFlag{} &&
			targ->health * 2 >= P_MobjSpawnHealth(targ))
			return true;
	}

	if(!P_IsVisible(actor, mo, current_allaround))
		return true;

	P_SetTarget(&actor->lastenemy, actor->target); // Remember previous target
	P_SetTarget(&actor->target, mo);               // Found target

	// Move the selected monster to the end of its associated
	// list, so that it gets searched last next time.

	{
		thinker_t* cap = &thinkerclasscap[std::to_underlying((mo->flags & MobjFlag::Friend) != MobjFlag{} ? ThinkerClass::Friends : ThinkerClass::Enemies)];
		(mo->thinker.cprev->cnext = mo->thinker.cnext)->cprev = mo->thinker.cprev;
		(mo->thinker.cprev = cap->cprev)->cnext = &mo->thinker;
		(mo->thinker.cnext = cap)->cprev = &mo->thinker;
	}

	return false;
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//

static dboolean P_LookForPlayers(mobj_t* actor, dboolean allaround)
{
	player_t* player;
	int stop, stopc, c;
	dboolean unseen[MAX_MAXPLAYERS] = {0};

	if(raven) return Raven_P_LookForPlayers(actor, allaround);

	if((actor->flags & MobjFlag::Friend) != MobjFlag{})
	{
		// killough 9/9/98: friendly monsters go about players differently
		int anyone;

		// Go back to a player, no matter whether it's visible or not
		for(anyone = 0; anyone <= 1; anyone++)
			for(c = 0; c < g_maxplayers; c++)
				if(playeringame[c] && players[c].playerstate == PlayerState::Live &&
					(anyone || P_IsVisible(actor, players[c].mo, allaround)))
				{
					P_SetTarget(&actor->target, players[c].mo);

					// killough 12/98:
					// get out of refiring loop, to avoid hitting player accidentally

					if(actor->info->missilestate != StateId::Null)
					{
						P_SetMobjState(actor, actor->info->seestate);
						actor->flags -= MobjFlag::JustHit;
					}

					return true;
				}

		return false;
	}

	// Change mask of 3 to (g_maxplayers-1) -- killough 2/15/98:
	stop = (actor->lastlook - 1) & (g_maxplayers - 1);

	c = 0;

	stopc = !mbf_features &&
		!demo_compatibility && monsters_remember
		? g_maxplayers
		: 2; // killough 9/9/98

	for(;; actor->lastlook = (actor->lastlook + 1) & (g_maxplayers - 1))
	{
		if(!playeringame[actor->lastlook])
			continue;

		// killough 2/15/98, 9/9/98:
		if(c++ == stopc || actor->lastlook == stop) // done looking
		{
			// e6y
			// Fixed Boom incompatibilities. The following code was missed.
			// There are no more desyncs on Donce's demos on horror.wad

			// Use last known enemy if no players sighted -- killough 2/15/98:
			if(!mbf_features && !demo_compatibility && monsters_remember)
			{
				if(actor->lastenemy && actor->lastenemy->health > 0)
				{
					actor->target = actor->lastenemy;
					actor->lastenemy = nullptr;
					return true;
				}
			}

			return false;
		}

		player = &players[actor->lastlook];

		if((player->cheats & CheatFlag::NoTarget) != CheatFlag{})
			continue; // no target

		if(player->health <= 0)
			continue; // dead

		if(unseen[actor->lastlook] || !P_IsVisible(actor, player->mo, allaround))
		{
			unseen[actor->lastlook] = true;
			continue;
		}

		P_SetTarget(&actor->target, player->mo);

		/* killough 9/9/98: give monsters a threshold towards getting players
		* (we don't want it to be too easy for a player with dogs :)
		*/
		if(!comp[std::to_underlying(CompOption::Pursuit)])
			actor->threshold = 60;

		return true;
	}
}

//
// Friendly monsters, by Lee Killough 7/18/98
//
// Friendly monsters go after other monsters first, but
// also return to owner if they cannot find any targets.
// A marine's best friend :)  killough 7/18/98, 9/98
//

static dboolean P_LookForMonsters(mobj_t* actor, dboolean allaround)
{
	thinker_t *cap, *th;

	if(demo_compatibility)
		return false;

	if(actor->lastenemy && actor->lastenemy->health > 0 && monsters_remember &&
		(actor->lastenemy->flags & actor->flags & MobjFlag::Friend) == MobjFlag{}) // not friends
	{
		P_SetTarget(&actor->target, actor->lastenemy);
		P_SetTarget(&actor->lastenemy, nullptr);
		return true;
	}

	/* Old demos do not support monster-seeking bots */
	if(!mbf_features)
		return false;

	// Search the threaded list corresponding to this object's potential targets
	cap = &thinkerclasscap[std::to_underlying((actor->flags & MobjFlag::Friend) != MobjFlag{} ? ThinkerClass::Enemies : ThinkerClass::Friends)];

	// Search for new enemy

	if(cap->cnext != cap) // Empty list? bail out early
	{
		int x = P_GetSafeBlockX(actor->x - bmaporgx);
		int y = P_GetSafeBlockY(actor->y - bmaporgy);
		int d;

		current_actor = actor;
		current_allaround = allaround;

		// There is a bug in cl11+ that causes the player to get added
		//   to the monster friend list when damaged to below 50% health.
		// This causes all monsters to believe friend monsters exist.
		// The search algorithm is expensive and massively so on maps with many monsters.
		// We still need to match rng calls for demo sync, but PIT_FindTarget is a no op.
		if(((mobj_t*)cap->cnext)->player && cap->cnext == cap->cprev)
		{
			P_Random(RandomClass::Friends);
			return false;
		}

		// Search first in the immediate vicinity.

		if(!P_BlockThingsIterator(x, y, PIT_FindTarget))
			return true;

		for(d = 1; d < 5; d++)
		{
			int i = 1 - d;
			do
				if(!P_BlockThingsIterator(x + i, y - d, PIT_FindTarget) ||
					!P_BlockThingsIterator(x + i, y + d, PIT_FindTarget))
					return true;
			while(++i < d);

			do
				if(!P_BlockThingsIterator(x - d, y + i, PIT_FindTarget) ||
					!P_BlockThingsIterator(x + d, y + i, PIT_FindTarget))
					return true;
			while(--i + d >= 0);
		}

		{
			// Random number of monsters, to prevent patterns from forming
			int n = (P_Random(RandomClass::Friends) & 31) + 15;

			for(th = cap->cnext; th != cap; th = th->cnext)
				if(--n < 0)
				{
					// Only a subset of the monsters were searched. Move all of
					// the ones which were searched so far, to the end of the list.

					(cap->cnext->cprev = cap->cprev)->cnext = cap->cnext;
					(cap->cprev = th->cprev)->cnext = cap;
					(th->cprev = cap)->cnext = th;
					break;
				}
				else if(!PIT_FindTarget((mobj_t*)th)) // If target sighted
					return true;
		}
	}

	return false; // No monster found
}

//
// P_LookForTargets
//
// killough 9/5/98: look for targets to go after, depending on kind of monster
//

static dboolean P_LookForTargets(mobj_t* actor, int allaround)
{
	return (actor->flags & MobjFlag::Friend) != MobjFlag{} ? P_LookForMonsters(actor, allaround) || P_LookForPlayers(actor, allaround) : P_LookForPlayers(actor, allaround) || P_LookForMonsters(actor, allaround);
}

//
// P_HelpFriend
//
// killough 9/8/98: Help friends in danger of dying
//

static dboolean P_HelpFriend(mobj_t* actor)
{
	thinker_t *cap, *th;

	// If less than 33% health, self-preservation rules
	if(actor->health * 3 < P_MobjSpawnHealth(actor))
		return false;

	current_actor = actor;
	current_allaround = true;

	// Possibly help a friend under 50% health
	cap = &thinkerclasscap[std::to_underlying((actor->flags & MobjFlag::Friend) != MobjFlag{} ? ThinkerClass::Friends : ThinkerClass::Enemies)];

	for(th = cap->cnext; th != cap; th = th->cnext)
		if(((mobj_t*)th)->health * 2 >= P_MobjSpawnHealth((mobj_t*)th))
		{
			if(P_Random(RandomClass::Helpfriend) < 180)
				break;
		}
		else if((((mobj_t*)th)->flags & MobjFlag::JustHit) != MobjFlag{} &&
			((mobj_t*)th)->target &&
			((mobj_t*)th)->target != actor->target &&
			!PIT_FindTarget(((mobj_t*)th)->target))
		{
			// Ignore any attacking monsters, while searching for friend
			actor->threshold = BASETHRESHOLD;
			return true;
		}

	return false;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
extern "C" void A_KeenDie(mobj_t* mo)
{
	thinker_t* th;
	line_t junk;

	A_Fall(mo);

	// scan the remaining thinkers to see if all Keens are dead

	for(th = thinkercap.next; th != &thinkercap; th = th->next)
		if(th->function == reinterpret_cast<think_t>(P_MobjThinker))
		{
			mobj_t* mo2 = (mobj_t*)th;
			if(mo2 != mo && mo2->type == mo->type && mo2->health > 0)
				return; // other Keen not dead
		}

	junk.special_args[0] = 666;
	EV_DoDoor(&junk, static_cast<VerticalDoorType>(VerticalDoorType::OpenDoor));
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//

extern "C" void A_Look(mobj_t* actor)
{
	mobj_t* targ = actor->subsector->sector->soundtarget;
	actor->threshold = 0; // any shot will wake up

	if(targ && targ->player && ((targ->player->cheats & CheatFlag::NoTarget) != CheatFlag{}))
		return;

	/* killough 7/18/98:
	* Friendly monsters go after other monsters first, but
	* also return to player, without attacking them, if they
	* cannot find any targets. A marine's best friend :)
	*/
	actor->pursuecount = 0;

	if(
		!((actor->flags & MobjFlag::Friend) != MobjFlag{} && P_LookForTargets(actor, false)) &&
		!(
			(targ = actor->subsector->sector->soundtarget) &&
			(targ->flags & MobjFlag::Shootable) != MobjFlag{} &&
			(
				P_SetTarget(&actor->target, targ),
				(actor->flags & MobjFlag::Ambush) == MobjFlag{} ||
				P_CheckSight(actor, targ)
			)
		) &&
		(
			(actor->flags & MobjFlag::Friend) != MobjFlag{} || !P_LookForTargets(actor, false)
		)
	)
		return;

	// go into chase state

	if(actor->info->seesound != SfxId::None)
	{
		SfxId sound;
		sound = actor->info->seesound;

		if(!raven)
			switch(sound)
			{
				case SfxId::Posit1:
				case SfxId::Posit2:
				case SfxId::Posit3:
					sound = SfxVariant(SfxId::Posit1, P_Random(RandomClass::See) % 3);
					break;

				case SfxId::Bgsit1:
				case SfxId::Bgsit2:
					sound = SfxVariant(SfxId::Bgsit1, P_Random(RandomClass::See) % 2);
					break;

				default:
					break;
			}

		if((actor->flags2 & (MobjFlag2::Boss | MobjFlag2::FullVolSounds)) != MobjFlag2{})
			S_StartVoidSound(sound); // full volume
		else
		{
			S_StartMobjSound(actor, sound);

			// [FG] make seesounds uninterruptible
			if(full_sounds)
				S_UnlinkSound(actor);
		}
	}
	P_SetMobjState(actor, actor->info->seestate);
}

//
// A_KeepChasing
//
// killough 10/98:
// Allows monsters to continue movement while attacking
//

static void A_KeepChasing(mobj_t* actor)
{
	if(actor->movecount)
	{
		actor->movecount--;
		if(actor->strafecount)
			actor->strafecount--;
		P_SmartMove(actor);
	}
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//

extern "C" void A_Chase(mobj_t* actor)
{
	if(actor->reactiontime)
		actor->reactiontime--;

	if(actor->threshold)
	{
		/* modify target threshold */
		if(compatibility_level == CompLevel::Doom12)
		{
			actor->threshold--;
		}
		else
		{
			if(!actor->target || actor->target->health <= 0)
				actor->threshold = 0;
			else
				actor->threshold--;
		}
	}

	if(raven && (skill_info.flags & SkillFlag::FastMonsters) != SkillFlag{})
	{
		// Monsters move faster in nightmare mode
		actor->tics -= actor->tics / 2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

	/* turn towards movement direction if not there yet
	* killough 9/7/98: keep facing towards target if strafing or backing out
	*/

	if(actor->strafecount)
		A_FaceTarget(actor);
	else if(actor->movedir < 8)
	{
		int delta = (actor->angle &= (7 << 29)) - (actor->movedir << 29);
		if(delta > 0)
			actor->angle -= ANG90 / 2;
		else if(delta < 0)
			actor->angle += ANG90 / 2;
	}

	if(!actor->target || (actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		if(!P_LookForTargets(actor,true))                   // look for a new target
			P_SetMobjState(actor, actor->info->spawnstate); // no new target
		return;
	}

	// do not attack twice in a row
	if((actor->flags & MobjFlag::JustAttacked) != MobjFlag{})
	{
		actor->flags -= MobjFlag::JustAttacked;
		if((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{})
			P_NewChaseDir(actor);
		return;
	}

	// check for melee attack
	if(actor->info->meleestate != StateId::Null && P_CheckMeleeRange(actor))
	{
		if(actor->info->attacksound != SfxId::None)
			S_StartMobjSound(actor, actor->info->attacksound);
		P_SetMobjState(actor, actor->info->meleestate);
		/* killough 8/98: remember an attack
		* cph - DEMOSYNC? */
		if(actor->info->missilestate == StateId::Null && !raven)
			actor->flags |= MobjFlag::JustHit;
		return;
	}

	// check for missile attack
	if(actor->info->missilestate != StateId::Null)
		if(!((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{} && actor->movecount))
			if(P_CheckMissileRange(actor))
			{
				P_SetMobjState(actor, actor->info->missilestate);
				actor->flags |= MobjFlag::JustAttacked;
				return;
			}

	if(!actor->threshold)
	{
		if(!mbf_features)
		{
			/* killough 9/9/98: for backward demo compatibility */
			if(netgame && !P_CheckSight(actor, actor->target) &&
				P_LookForPlayers(actor, true))
				return;
		}
		/* killough 7/18/98, 9/9/98: new monster AI */
		else if(help_friends && P_HelpFriend(actor))
			return; /* killough 9/8/98: Help friends in need */
			/* Look for new targets if current one is bad or is out of view */
		else if(actor->pursuecount)
			actor->pursuecount--;
		else
		{
			/* Our pursuit time has expired. We're going to think about
			* changing targets */
			actor->pursuecount = BASETHRESHOLD;

			// look for new target, unless conditions are met
			if(
				!(
					actor->target &&             // have a target
					actor->target->health > 0 && // and the target is alive
					(
						(comp[std::to_underlying(CompOption::Pursuit)] && !netgame) || // and using old pursuit behaviour
						(
							( // or the target is not friendly
								((actor->target->flags ^ actor->flags) & MobjFlag::Friend) != MobjFlag{} ||
								((actor->flags & MobjFlag::Friend) == MobjFlag{} && monster_infighting)
							) &&
							P_CheckSight(actor, actor->target) // and we can see it
						)
					)
				) &&
				P_LookForTargets(actor, true)
			)
				return;

			/* (Current target was good, or no new target was found.)
			*
			* If monster is a missile-less friend, give up pursuit and
			* return to player, if no attacks have occurred recently.
			*/

			if(actor->info->missilestate == StateId::Null && (actor->flags & MobjFlag::Friend) != MobjFlag{})
			{
				if((actor->flags & MobjFlag::JustHit) != MobjFlag{})          /* if recent action, */
					actor->flags -= MobjFlag::JustHit;       /* keep fighting */
				else if(P_LookForPlayers(actor, true)) /* else return to player */
					return;
			}
		}
	}

	if(actor->strafecount)
		actor->strafecount--;

	// chase towards player
	if(--actor->movecount < 0 || !P_SmartMove(actor))
		P_NewChaseDir(actor);

	// make active sound
	if(actor->info->activesound != SfxId::None && P_Random(RandomClass::See) < 3)
	{
		if(heretic && actor->type == MobjType::HereticWizard && P_Random(RandomClass::Heretic) < 128)
		{
			S_StartMobjSound(actor, actor->info->seesound);
		}
		else if(heretic && actor->type == MobjType::HereticSorcerer2)
		{
			S_StartVoidSound(actor->info->activesound);
		}
		else if(hexen && actor->type == MobjType::HexenBishop && P_Random(RandomClass::Hexen) < 128)
		{
			S_StartMobjSound(actor, actor->info->seesound);
		}
		else if(hexen && actor->type == MobjType::HexenPig)
		{
			S_StartMobjSound(actor, SfxVariant(SfxId::HexenPigActive1, P_Random(RandomClass::Hexen) & 1));
		}
		else if(hexen && (actor->flags2 & MobjFlag2::Boss) != MobjFlag2{})
		{
			S_StartVoidSound(actor->info->activesound);
		}
		else
		{
			S_StartMobjSound(actor, actor->info->activesound);
		}
	}
}

//
// A_FaceTarget
//
extern "C" void A_FaceTarget(mobj_t* actor)
{
	if(!actor->target)
		return;
	actor->flags -= MobjFlag::Ambush;
	actor->angle = R_PointToAngle2(actor->x, actor->y,
		actor->target->x, actor->target->y);
	if((actor->target->flags & MobjFlag::Shadow) != MobjFlag{})
	{
		// killough 5/5/98: remove dependence on order of evaluation:
		int t = P_Random(RandomClass::Facetarget);
		actor->angle += (t - P_Random(RandomClass::Facetarget)) << 21;
	}
}

//
// A_PosAttack
//

extern "C" void A_PosAttack(mobj_t* actor)
{
	int angle, damage, slope, t;

	if(!actor->target)
		return;
	A_FaceTarget(actor);
	angle = actor->angle;
	slope = P_AimLineAttack(actor, angle, MISSILERANGE, MobjFlag{}); /* killough 8/2/98 */
	S_StartMobjSound(actor, SfxId::Pistol);

	// killough 5/5/98: remove dependence on order of evaluation:
	t = P_Random(RandomClass::Posattack);
	angle += (t - P_Random(RandomClass::Posattack)) << 20;
	damage = (P_Random(RandomClass::Posattack) % 5 + 1) * 3;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

extern "C" void A_SPosAttack(mobj_t* actor)
{
	int i, bangle, slope;

	if(!actor->target)
		return;
	S_StartMobjSound(actor, SfxId::Shotgn);
	A_FaceTarget(actor);
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE, MobjFlag{}); /* killough 8/2/98 */
	for(i = 0; i < 3; i++)
	{
		// killough 5/5/98: remove dependence on order of evaluation:
		int t = P_Random(RandomClass::Sposattack);
		int angle = bangle + ((t - P_Random(RandomClass::Sposattack)) << 20);
		int damage = ((P_Random(RandomClass::Sposattack) % 5) + 1) * 3;
		P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
	}
}

extern "C" void A_CPosAttack(mobj_t* actor)
{
	int angle, bangle, damage, slope, t;

	if(!actor->target)
		return;
	S_StartMobjSound(actor, SfxId::Shotgn);
	A_FaceTarget(actor);
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE, MobjFlag{}); /* killough 8/2/98 */

	// killough 5/5/98: remove dependence on order of evaluation:
	t = P_Random(RandomClass::Cposattack);
	angle = bangle + ((t - P_Random(RandomClass::Cposattack)) << 20);
	damage = ((P_Random(RandomClass::Cposattack) % 5) + 1) * 3;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

extern "C" void A_CPosRefire(mobj_t* actor)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor);

	/* killough 12/98: Stop firing if a friend has gotten in the way */
	if(P_HitFriend(actor))
		goto stop;

	/* killough 11/98: prevent refiring on friends continuously */
	if(P_Random(RandomClass::Cposrefire) < 40)
	{
		if(actor->target && (actor->flags & actor->target->flags & MobjFlag::Friend) != MobjFlag{})
			goto stop;
		else
			return;
	}

	if(!actor->target || actor->target->health <= 0
		|| !P_CheckSight(actor, actor->target))
	stop:
		P_SetMobjState(actor, actor->info->seestate);
}

extern "C" void A_SpidRefire(mobj_t* actor)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor);

	/* killough 12/98: Stop firing if a friend has gotten in the way */
	if(P_HitFriend(actor))
		goto stop;

	if(P_Random(RandomClass::Spidrefire) < 10)
		return;

	// killough 11/98: prevent refiring on friends continuously
	if(!actor->target || actor->target->health <= 0
		|| (actor->flags & actor->target->flags & MobjFlag::Friend) != MobjFlag{}
		|| !P_CheckSight(actor, actor->target))
	stop:
		P_SetMobjState(actor, actor->info->seestate);
}

extern "C" void A_BspiAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MobjType::Arachplaz); // launch a missile
}

//
// A_TroopAttack
//

extern "C" void A_TroopAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	if(P_CheckMeleeRange(actor))
	{
		int damage;
		S_StartMobjSound(actor, SfxId::Claw);
		damage = (P_Random(RandomClass::Troopattack) % 8 + 1) * 3;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}
	P_SpawnMissile(actor, actor->target, MobjType::Troopshot); // launch a missile
}

extern "C" void A_SargAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	if(compatibility_level == CompLevel::Doom12)
	{
		int damage = ((P_Random(RandomClass::Sargattack) % 10) + 1) * 4;
		P_LineAttack(actor, actor->angle, MELEERANGE, 0, damage);
	}
	else
	{
		if(P_CheckMeleeRange(actor))
		{
			int damage = ((P_Random(RandomClass::Sargattack) % 10) + 1) * 4;
			P_DamageMobj(actor->target, actor, actor, damage);
		}
	}
}

extern "C" void A_HeadAttack(mobj_t* actor)
{
	int i;
	mobj_t* fire;
	mobj_t* baseFire;
	mobj_t* mo;
	mobj_t* target;
	int randAttack;
	static int atkResolve1[] = {50, 150};
	static int atkResolve2[] = {150, 200};
	int dist;

	// Ice ball     (close 20% : far 60%)
	// Fire column  (close 40% : far 20%)
	// Whirlwind    (close 40% : far 20%)
	// Distance threshold = 8 cells

	target = actor->target;
	if(target == nullptr) return;

	A_FaceTarget(actor);

	if(P_CheckMeleeRange(actor))
	{
		int damage = heretic ? HITDICE(6) : (P_Random(RandomClass::Headattack) % 6 + 1) * 10;
		P_DamageMobj(target, actor, actor, damage);
		return;
	}

	if(!heretic)
	{
		P_SpawnMissile(actor, target, MobjType::Headshot);
		return;
	}

	dist = P_AproxDistance(actor->x - target->x, actor->y - target->y)
		> 8 * 64 * FRACUNIT;
	randAttack = P_Random(RandomClass::Heretic);
	if(randAttack < atkResolve1[dist])
	{
		// Ice ball
		P_SpawnMissile(actor, target, MobjType::HereticHeadfx1);
		S_StartMobjSound(actor, SfxId::HereticHedat2);
	}
	else if(randAttack < atkResolve2[dist])
	{
		// Fire column
		baseFire = P_SpawnMissile(actor, target, MobjType::HereticHeadfx3);
		if(baseFire != nullptr)
		{
			P_SetMobjState(baseFire, StateId::HereticHeadfx34); // Don't grow
			for(i = 0; i < 5; i++)
			{
				fire = P_SpawnMobj(baseFire->x, baseFire->y,
					baseFire->z, MobjType::HereticHeadfx3);
				if(i == 0)
				{
					S_StartMobjSound(actor, SfxId::HereticHedat1);
				}
				P_SetTarget(&fire->target, baseFire->target);
				fire->angle = baseFire->angle;
				fire->momx = baseFire->momx;
				fire->momy = baseFire->momy;
				fire->momz = baseFire->momz;
				fire->damage = 0;
				fire->health = (i + 1) * 2;
				P_CheckMissileSpawn(fire);
			}
		}
	}
	else
	{
		// Whirlwind
		mo = P_SpawnMissile(actor, target, MobjType::HereticWhirlwind);
		if(mo != nullptr)
		{
			mo->z -= 32 * FRACUNIT;
			P_SetTarget(&mo->special1.m, target);
			mo->special2.i = 50;       // Timer for active sound
			mo->health = 20 * TICRATE; // Duration
			S_StartMobjSound(actor, SfxId::HereticHedat3);
		}
	}
}

extern "C" void A_CyberAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MobjType::Rocket);
}

extern "C" void A_BruisAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	if(P_CheckMeleeRange(actor))
	{
		int damage;
		S_StartMobjSound(actor, SfxId::Claw);
		damage = (P_Random(RandomClass::Bruisattack) % 8 + 1) * 10;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}
	P_SpawnMissile(actor, actor->target, MobjType::Bruisershot); // launch a missile
}

//
// A_SkelMissile
//

extern "C" void A_SkelMissile(mobj_t* actor)
{
	mobj_t* mo;

	if(!actor->target)
		return;

	A_FaceTarget(actor);
	actor->z += 16 * FRACUNIT; // so missile spawns higher
	mo = P_SpawnMissile(actor, actor->target, MobjType::Tracer);
	actor->z -= 16 * FRACUNIT; // back to normal

	mo->x += mo->momx;
	mo->y += mo->momy;
	P_SetTarget(&mo->tracer, actor->target);
}

int TRACEANGLE = 0xc000000;

extern "C" void A_Tracer(mobj_t* actor)
{
	angle_t exact;
	fixed_t dist;
	fixed_t slope;
	mobj_t* dest;
	mobj_t* th;

	/* killough 1/18/98: this is why some missiles do not have smoke
	* and some do. Also, internal demos start at random gametics, thus
	* the bug in which revenants cause internal demos to go out of sync.
	*
	* killough 3/6/98: fix revenant internal demo bug by subtracting
	* levelstarttic from gametic.
	*
	* killough 9/29/98: use new "basetic" so that demos stay in sync
	* during pauses and menu activations, while retaining old demo sync.
	*
	* leveltime would have been better to use to start with in Doom, but
	* since old demos were recorded using gametic, we must stick with it,
	* and improvise around it (using leveltime causes desync across levels).
	*/

	if(boom_logictic & 3)
		return;

	// spawn a puff of smoke behind the rocket
	P_SpawnPuff(actor->x, actor->y, actor->z);

	th = P_SpawnMobj(actor->x - actor->momx,
		actor->y - actor->momy,
		actor->z, MobjType::Smoke);

	th->momz = FRACUNIT;
	th->tics -= P_Random(RandomClass::Tracer) & 3;
	if(th->tics < 1)
		th->tics = 1;

	// adjust direction
	dest = actor->tracer;

	if(!dest || dest->health <= 0)
		return;

	// change angle
	exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

	if(exact != actor->angle)
	{
		if(exact - actor->angle > 0x80000000)
		{
			actor->angle -= TRACEANGLE;
			if(exact - actor->angle < 0x80000000)
				actor->angle = exact;
		}
		else
		{
			actor->angle += TRACEANGLE;
			if(exact - actor->angle > 0x80000000)
				actor->angle = exact;
		}
	}

	exact = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
	actor->momy = FixedMul(actor->info->speed, finesine[exact]);

	// change slope
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);

	dist = dist / actor->info->speed;

	if(dist < 1)
		dist = 1;

	slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;

	if(slope < actor->momz)
		actor->momz -= FRACUNIT / 8;
	else
		actor->momz += FRACUNIT / 8;
}

extern "C" void A_SkelWhoosh(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	S_StartMobjSound(actor, SfxId::Skeswg);
}

extern "C" void A_SkelFist(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	if(P_CheckMeleeRange(actor))
	{
		int damage = ((P_Random(RandomClass::Skelfist) % 10) + 1) * 6;
		S_StartMobjSound(actor, SfxId::Skepch);
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//

mobj_t* corpsehit;
mobj_t* vileobj;
fixed_t viletryx;
fixed_t viletryy;
int viletryradius;

static dboolean PIT_VileCheck(mobj_t* thing)
{
	int maxdist;
	dboolean check;

	if((thing->flags & MobjFlag::Corpse) == MobjFlag{})
		return true; // not a monster

	if(thing->tics != -1)
		return true; // not lying still yet

	if(thing->info->raisestate == g_s_null)
		return true; // monster doesn't have a raise state

	maxdist = thing->info->radius + viletryradius;

	if(D_abs(thing->x - viletryx) > maxdist || D_abs(thing->y - viletryy) > maxdist)
		return true; // not actually touching

	// Check to see if the radius and height are zero. If they are      // phares
	// then this is a crushed monster that has been turned into a       //   |
	// gib. One of the options may be to ignore this guy.               //   V

	// Option 1: the original, buggy method, -> ghost (compatibility)
	// Option 2: ressurect the monster, but not as a ghost
	// Option 3: ignore the gib

	//    if (Option3)                                                  //   ^
	//        if ((thing->height == 0) && (thing->radius == 0))         //   |
	//            return true;                                          // phares

	corpsehit = thing;
	corpsehit->momx = corpsehit->momy = 0;
	if(comp[std::to_underlying(CompOption::Vile)]) // phares
	{
		//   |
		corpsehit->height <<= 2; //   V
		check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
		corpsehit->height >>= 2;
	}
	else
	{
		int height, radius;

		height = corpsehit->height; // save temporarily
		radius = corpsehit->radius; // save temporarily
		corpsehit->height = corpsehit->info->height;
		corpsehit->radius = corpsehit->info->radius;
		corpsehit->flags |= MobjFlag::Solid;
		check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
		corpsehit->height = height; // restore
		corpsehit->radius = radius; // restore                      //   ^
		corpsehit->flags -= MobjFlag::Solid;
	} //   |
	// phares
	if(!check)
		return true; // doesn't fit here
	return false;    // got one, so stop checking
}

dboolean P_RaiseThing(mobj_t* corpse, mobj_t* raiser)
{
	MobjFlag oldflags;
	fixed_t oldheight, oldradius;
	mobjinfo_t* info;

	if((corpse->flags & MobjFlag::Corpse) == MobjFlag{})
		return false;

	info = corpse->info;

	if(info->raisestate == g_s_null)
		return false;

	corpse->momx = 0;
	corpse->momy = 0;

	oldheight = corpse->height;
	oldradius = corpse->radius;
	oldflags = corpse->flags;

	corpse->height = info->height;
	corpse->radius = info->radius;
	corpse->flags |= MobjFlag::Solid;

	if(!P_CheckPosition(corpse, corpse->x, corpse->y))
	{
		corpse->height = oldheight;
		corpse->radius = oldradius;
		corpse->flags = oldflags;
		return false;
	}

	S_StartMobjSound(corpse, SfxId::Slop);

	P_SetMobjState(corpse, info->raisestate);

	corpse->flags = info->flags;
	corpse->flags |= MobjFlag::Ressurected;
	corpse->flags -= MobjFlag::JustHit;

	if(raiser)
	{
		corpse->flags = (corpse->flags - MobjFlag::Friend) | (raiser->flags & MobjFlag::Friend);
	}

	dsda_WatchResurrection(corpse, raiser);

	// Allow ghost monsters to be rendered translucent
	if(corpse->height == 0 && corpse->radius == 0
		&& dsda_IntConfig(ConfigId::TranslucentGhosts))
		corpse->flags |= MobjFlag::Translucent;

	if(((corpse->flags ^ MobjFlag::CountKill) & (MobjFlag::Friend | MobjFlag::CountKill)) == MobjFlag{})
		totallive++;

	corpse->health = P_MobjSpawnHealth(corpse);
	corpse->color = 0;
	P_SetTarget(&corpse->target, nullptr);
	P_SetTarget(&corpse->lastenemy, nullptr);

	P_UpdateThinker(&corpse->thinker);

	return true;
}

//
// P_HealCorpse
// Check for ressurecting a body
//

static dboolean P_HealCorpse(mobj_t* actor, int radius, StateId healstate, SfxId healsound)
{
	int xl, xh;
	int yl, yh;
	int bx, by;

	if(actor->movedir != MoveDir(DirType::NoDir))
	{
		// check for corpses to raise
		viletryx =
			actor->x + actor->info->speed * xspeed[actor->movedir];
		viletryy =
			actor->y + actor->info->speed * yspeed[actor->movedir];

		xl = P_GetSafeBlockX(viletryx - bmaporgx - MAXRADIUS * 2);
		xh = P_GetSafeBlockX(viletryx - bmaporgx + MAXRADIUS * 2);
		yl = P_GetSafeBlockY(viletryy - bmaporgy - MAXRADIUS * 2);
		yh = P_GetSafeBlockY(viletryy - bmaporgy + MAXRADIUS * 2);

		vileobj = actor;
		viletryradius = radius;
		for(bx = xl; bx <= xh; bx++)
		{
			for(by = yl; by <= yh; by++)
			{
				// Call PIT_VileCheck to check
				// whether object is a corpse
				// that canbe raised.
				if(!P_BlockThingsIterator(bx, by, PIT_VileCheck))
				{
					mobjinfo_t* info;

					// got one!
					mobj_t* temp = actor->target;
					actor->target = corpsehit;
					A_FaceTarget(actor);
					actor->target = temp;

					P_SetMobjState(actor, static_cast<StateId>(healstate));
					S_StartMobjSound(corpsehit, healsound);
					info = corpsehit->info;

					P_SetMobjState(corpsehit, info->raisestate);

					if(comp[std::to_underlying(CompOption::Vile)])          // phares
						corpsehit->height <<= 2; //   |
					else                         //   V
					{
						corpsehit->height = info->height; // fix Ghost bug
						corpsehit->radius = info->radius; // fix Ghost bug
					}                                     // phares

					/* killough 7/18/98:
					* friendliness is transferred from AV to raised corpse
					*/
					corpsehit->flags =
						(info->flags - MobjFlag::Friend) | (actor->flags & MobjFlag::Friend);
					corpsehit->flags = corpsehit->flags | MobjFlag::Ressurected; //e6y

					dsda_WatchResurrection(corpsehit, actor);

					// Allow ghost monsters to be rendered translucent
					if(corpsehit->height == 0 && corpsehit->radius == 0
						&& dsda_IntConfig(ConfigId::TranslucentGhosts))
						corpsehit->flags |= MobjFlag::Translucent;

					if(((corpsehit->flags ^ MobjFlag::CountKill) & (MobjFlag::Friend | MobjFlag::CountKill)) == MobjFlag{})
						totallive++;

					corpsehit->health = P_MobjSpawnHealth(corpsehit);
					corpsehit->color = 0;
					P_SetTarget(&corpsehit->target, nullptr); // killough 11/98

					if(mbf_features)
					{
						/* kilough 9/9/98 */
						P_SetTarget(&corpsehit->lastenemy, nullptr);
						corpsehit->flags -= MobjFlag::JustHit;
					}

					/* killough 8/29/98: add to appropriate thread */
					P_UpdateThinker(&corpsehit->thinker);

					return true;
				}
			}
		}
	}
	return false;
}

//
// A_VileChase
//

extern "C" void A_VileChase(mobj_t* actor)
{
	if(!P_HealCorpse(actor, mobjinfo[std::to_underlying(MobjType::Vile)].radius, StateId::VileHeal1, static_cast<SfxId>(SfxId::Slop)))
		A_Chase(actor); // Return to normal attack.
}

//
// A_VileStart
//

extern "C" void A_VileStart(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::Vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//

extern "C" void A_StartFire(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::Flamst);
	A_Fire(actor);
}

extern "C" void A_FireCrackle(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::Flame);
	A_Fire(actor);
}

extern "C" void A_Fire(mobj_t* actor)
{
	mobj_t* target;
	unsigned an;
	mobj_t* dest = actor->tracer;

	if(!dest)
		return;

	target = P_SubstNullMobj(actor->target);

	// don't move it if the vile lost sight
	if(!P_CheckSight(target, dest))
		return;

	an = dest->angle >> ANGLETOFINESHIFT;

	P_UnsetThingPosition(actor);
	actor->x = dest->x + FixedMul(24 * FRACUNIT, finecosine[an]);
	actor->y = dest->y + FixedMul(24 * FRACUNIT, finesine[an]);
	actor->z = dest->z;
	P_SetThingPosition(actor);
}

//
// A_VileTarget
// Spawn the hellfire
//

extern "C" void A_VileTarget(mobj_t* actor)
{
	mobj_t* fog;

	if(!actor->target)
		return;

	A_FaceTarget(actor);

	// killough 12/98: fix Vile fog coordinates // CPhipps - compatibility optioned
	fog = P_SpawnMobj(actor->target->x,
		(compatibility_level < CompLevel::Lxdoom1) ? actor->target->x : actor->target->y,
		actor->target->z, MobjType::Fire);

	P_SetTarget(&actor->tracer, fog);
	P_SetTarget(&fog->target, actor);
	P_SetTarget(&fog->tracer, actor->target);
	A_Fire(fog);
}

//
// A_VileAttack
//

extern "C" void A_VileAttack(mobj_t* actor)
{
	mobj_t* fire;
	int an;

	if(!actor->target)
		return;

	A_FaceTarget(actor);

	if(!P_CheckSight(actor, actor->target))
		return;

	S_StartMobjSound(actor, SfxId::Barexp);
	P_DamageMobj(actor->target, actor, actor, 20);
	actor->target->momz = 1000 * FRACUNIT / actor->target->info->mass;

	an = actor->angle >> ANGLETOFINESHIFT;

	fire = actor->tracer;

	if(!fire)
		return;

	// move the fire between the vile and the player
	fire->x = actor->target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
	fire->y = actor->target->y - FixedMul(24 * FRACUNIT, finesine[an]);
	P_RadiusAttack(fire, actor, 70, 70, BF_DAMAGESOURCE | BF_HORIZONTAL);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//

#define FATSPREAD       (ANG90/8)

extern "C" void A_FatRaise(mobj_t* actor)
{
	A_FaceTarget(actor);
	S_StartMobjSound(actor, SfxId::Manatk);
}

extern "C" void A_FatAttack1(mobj_t* actor)
{
	mobj_t* mo;
	mobj_t* target;
	int an;

	if(!actor->target)
		return;

	A_FaceTarget(actor);

	// Change direction  to ...
	actor->angle += FATSPREAD;
	target = P_SubstNullMobj(actor->target);
	P_SpawnMissile(actor, target, MobjType::Fatshot);

	mo = P_SpawnMissile(actor, target, MobjType::Fatshot);
	mo->angle += FATSPREAD;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

extern "C" void A_FatAttack2(mobj_t* actor)
{
	mobj_t* mo;
	mobj_t* target;
	int an;

	if(!actor->target)
		return;

	A_FaceTarget(actor);
	// Now here choose opposite deviation.
	actor->angle -= FATSPREAD;
	target = P_SubstNullMobj(actor->target);
	P_SpawnMissile(actor, target, MobjType::Fatshot);

	mo = P_SpawnMissile(actor, target, MobjType::Fatshot);
	mo->angle -= FATSPREAD * 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

extern "C" void A_FatAttack3(mobj_t* actor)
{
	mobj_t* mo;
	mobj_t* target;
	int an;

	if(!actor->target)
		return;

	A_FaceTarget(actor);

	target = P_SubstNullMobj(actor->target);

	mo = P_SpawnMissile(actor, target, MobjType::Fatshot);
	mo->angle -= FATSPREAD / 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);

	mo = P_SpawnMissile(actor, target, MobjType::Fatshot);
	mo->angle += FATSPREAD / 2;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

extern "C" void A_SkullAttack(mobj_t* actor)
{
	mobj_t* dest;
	angle_t an;
	int dist;

	if(!actor->target)
		return;

	dest = actor->target;
	actor->flags |= MobjFlag::SkullFly;

	S_StartMobjSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
	actor->momy = FixedMul(SKULLSPEED, finesine[an]);
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	dist = dist / SKULLSPEED;

	if(dist < 1)
		dist = 1;
	actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

extern "C" void A_BetaSkullAttack(mobj_t* actor)
{
	int damage;

	if(compatibility_level < CompLevel::Mbf)
		return;

	if(!actor->target || actor->target->type == MobjType::Skull)
		return;

	S_StartMobjSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	damage = (P_Random(RandomClass::Skullfly) % 8 + 1) * actor->info->damage;
	P_DamageMobj(actor->target, actor, actor, damage);
}

extern "C" void A_Stop(mobj_t* actor)
{
	if(compatibility_level < CompLevel::Mbf)
		return;

	actor->momx = actor->momy = actor->momz = 0;
}

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//

static void A_PainShootSkull(mobj_t* actor, angle_t angle)
{
	fixed_t x, y, z;
	mobj_t* newmobj;
	angle_t an;
	int prestep;

	// The original code checked for 20 skulls on the level,            // phares
	// and wouldn't spit another one if there were. If not in           // phares
	// compatibility mode, we remove the limit.                         // phares
	// phares
	if(comp[std::to_underlying(CompOption::Pain)]) /* killough 10/98: compatibility-optioned */
	{
		// count total number of skulls currently on the level
		int count = 0;
		thinker_t* currentthinker = nullptr;
		while((currentthinker = P_NextThinker(currentthinker, ThinkerClass::All)) != nullptr)
			if((currentthinker->function == reinterpret_cast<think_t>(P_MobjThinker))
				&& ((mobj_t*)currentthinker)->type == MobjType::Skull)
				count++;
		if(count > 20) // phares
			return;    // phares
	}

	// okay, there's room for another one

	an = angle >> ANGLETOFINESHIFT;

	prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[std::to_underlying(MobjType::Skull)].radius) / 2;

	x = actor->x + FixedMul(prestep, finecosine[an]);
	y = actor->y + FixedMul(prestep, finesine[an]);
	z = actor->z + 8 * FRACUNIT;

	if(comp[std::to_underlying(CompOption::Skull)])                          /* killough 10/98: compatibility-optioned */
		newmobj = P_SpawnMobj(x, y, z, MobjType::Skull); // phares
	else                                          //   V
	{
		// Check whether the Lost Soul is being fired through a 1-sided
		// wall or an impassible line, or a "monsters can't cross" line.
		// If it is, then we don't allow the spawn. This is a bug fix, but
		// it should be considered an enhancement, since it may disturb
		// existing demos, so don't do it in compatibility mode.

		if(Check_Sides(actor, x, y))
			return;

		newmobj = P_SpawnMobj(x, y, z, MobjType::Skull);

		// Check to see if the new Lost Soul's z value is above the
		// ceiling of its new sector, or below the floor. If so, kill it.

		if((newmobj->z >
				(newmobj->subsector->sector->ceilingheight - newmobj->height)) ||
			(newmobj->z < newmobj->subsector->sector->floorheight))
		{
			// kill it immediately
			P_DamageMobj(newmobj, actor, actor, 10000);
			return; //   ^
		}           //   |
	}               // phares

	/* killough 7/20/98: PEs shoot lost souls with the same friendliness */
	newmobj->flags = (newmobj->flags - MobjFlag::Friend) | (actor->flags & MobjFlag::Friend);

	/* killough 8/29/98: add to appropriate thread */
	P_UpdateThinker(&newmobj->thinker);

	// Check for movements.
	// killough 3/15/98: don't jump over dropoffs:

	if(!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
	{
		// kill it immediately
		P_DamageMobj(newmobj, actor, actor, 10000);
		return;
	}

	P_SetTarget(&newmobj->target, actor->target);
	A_SkullAttack(newmobj);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//

extern "C" void A_PainAttack(mobj_t* actor)
{
	if(!actor->target)
		return;
	A_FaceTarget(actor);
	A_PainShootSkull(actor, actor->angle);
}

extern "C" void A_PainDie(mobj_t* actor)
{
	A_Fall(actor);
	A_PainShootSkull(actor, actor->angle + ANG90);
	A_PainShootSkull(actor, actor->angle + ANG180);
	A_PainShootSkull(actor, actor->angle + ANG270);
}

extern "C" void Heretic_A_Scream(mobj_t* actor);
extern "C" void Hexen_A_Scream(mobj_t* actor);

extern "C" void A_Scream(mobj_t* actor)
{
	SfxId sound;

	if(heretic) return Heretic_A_Scream(actor);
	if(hexen) return Hexen_A_Scream(actor);

	switch(actor->info->deathsound)
	{
		case SfxId::None:
			return;

		case SfxId::Podth1:
		case SfxId::Podth2:
		case SfxId::Podth3:
			sound = SfxVariant(SfxId::Podth1, P_Random(RandomClass::Scream) % 3);
			break;

		case SfxId::Bgdth1:
		case SfxId::Bgdth2:
			sound = SfxVariant(SfxId::Bgdth1, P_Random(RandomClass::Scream) % 2);
			break;

		default:
			sound = actor->info->deathsound;
			break;
	}

	// Check for bosses.
	if((actor->flags2 & (MobjFlag2::Boss | MobjFlag2::FullVolSounds)) != MobjFlag2{})
		S_StartVoidSound(sound); // full volume
	else
		S_StartMobjSound(actor, sound);
}

extern "C" void A_XScream(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::Slop);
}

extern "C" void A_SkullPop(mobj_t* actor)
{
	mobj_t* mo;
	player_t* player;

	if(hexen && !actor->player)
	{
		return;
	}

	actor->flags -= MobjFlag::Solid;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, static_cast<MobjType>(g_skullpop_mt));
	//mo->target = actor;
	mo->momx = P_SubRandom() << 9;
	mo->momy = P_SubRandom() << 9;
	mo->momz = FRACUNIT * 2 + (P_Random(RandomClass::Heretic) << 6);
	// Attach player mobj to bloody skull
	player = actor->player;
	actor->player = nullptr;
	if(hexen)
		actor->special1.i = std::to_underlying(player->pclass);
	mo->player = player;
	mo->health = actor->health;
	mo->angle = actor->angle;
	mo->pitch = 0;
	if(player)
	{
		player->mo = mo;
		player->lookdir = 0;
		player->damagecount = 32;
	}
}

extern "C" void A_Pain(mobj_t* actor)
{
	if(actor->info->painsound != SfxId::None)
		S_StartMobjSound(actor, actor->info->painsound);
}

extern "C" void A_Fall(mobj_t* actor)
{
	// actor is on ground, it can be walked over
	actor->flags -= MobjFlag::Solid;
}

//
// A_Explode
//
extern "C" void A_Explode(mobj_t* thingy)
{
	int damage;
	int distance;
	int flags;

	damage = 128;
	distance = 128;
	flags = BF_DAMAGESOURCE;

	if(raven)
	{
		switch(thingy->type)
		{
			case MobjType::HereticFirebomb: // Time Bombs
			case MobjType::HexenFirebomb:   // Time Bombs
				thingy->z += 32 * FRACUNIT;
				thingy->flags -= MobjFlag::Shadow;
				break;
			case MobjType::HereticMntrfx2: // Minotaur floor fire
				damage = 24;
				distance = damage;
				break;
			case MobjType::HereticSor2fx1: // D'Sparil missile
				damage = 80 + (P_Random(RandomClass::Heretic) & 31);
				distance = damage;
				break;
			case MobjType::HexenMntrfx2: // Minotaur floor fire
				damage = 24;
				break;
			case MobjType::HexenBishop: // Bishop radius death
				damage = 25 + (P_Random(RandomClass::Hexen) & 15);
				break;
			case MobjType::HexenHammerMissile: // Fighter Hammer
				damage = 128;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenFswordMissile: // Fighter Runesword
				damage = 64;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenCircleflame: // Cleric Flame secondary flames
				damage = 20;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenSorcball1: // Sorcerer balls
			case MobjType::HexenSorcball2:
			case MobjType::HexenSorcball3:
				distance = 255;
				damage = 255;
				thingy->special_args[0] = 1; // don't play bounce
				break;
			case MobjType::HexenSorcfx1: // Sorcerer spell 1
				damage = 30;
				break;
			case MobjType::HexenSorcfx4: // Sorcerer spell 4
				damage = 20;
				break;
			case MobjType::HexenTreedestructible:
				damage = 10;
				break;
			case MobjType::HexenDragonFx2:
				damage = 80;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenMstaffFx:
				damage = 64;
				distance = 192;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenMstaffFx2:
				damage = 80;
				distance = 192;
				flags &= ~BF_DAMAGESOURCE;
				break;
			case MobjType::HexenPoisoncloud:
				damage = 4;
				distance = 40;
				break;
			case MobjType::HexenZxmasTree:
			case MobjType::HexenZshrub2:
				damage = 30;
				distance = 64;
				break;
			default:
				break;
		}
	}

	P_RadiusAttack(thingy, thingy->target, damage, distance, flags);
	if(
		heretic ||
		(
			hexen &&
			thingy->z <= thingy->floorz + (distance << FRACBITS) &&
			thingy->type != MobjType::HexenPoisoncloud
		)
	)
		P_HitFloor(thingy);
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//

dboolean P_CheckBossDeath(mobj_t* mo)
{
	int i;
	thinker_t* th;

	// make sure there is a player alive for victory
	for(i = 0; i < g_maxplayers; i++)
		if(playeringame[i] && players[i].health > 0)
			break;

	if(i == g_maxplayers)
		return false; // no one left alive, so do not end game

	// scan the remaining thinkers to see
	// if all bosses are dead
	for(th = thinkercap.next; th != &thinkercap; th = th->next)
		if(th->function == reinterpret_cast<think_t>(P_MobjThinker))
		{
			mobj_t* mo2 = (mobj_t*)th;

			if(mo2 != mo && mo2->type == mo->type && mo2->health > 0)
				return false; // other boss not dead
		}

	return true;
}

extern "C" void A_BossDeath(mobj_t* mo)
{
	line_t junk;

	if(dsda_BossAction(mo))
	{
		return;
	}

	// heretic_note: probably we can adopt the clean heretic style and merge
	if(heretic) return Heretic_A_BossDeath(mo);

	if(gamemode == GameMode::Commercial)
	{
		if(gamemap != 7)
			return;

		if((mo->flags2 & (MobjFlag2::Map07Boss1 | MobjFlag2::Map07Boss2)) == MobjFlag2{})
			return;
	}
	else
	{
		// e6y
		// Additional check of gameepisode is necessary, because
		// there is no right or wrong solution for E4M6 in original EXEs,
		// there's nothing to emulate.
		if(comp[std::to_underlying(CompOption::Value666)] && gameepisode < 4)
		{
			// e6y
			// Only following checks are present in doom2.exe ver. 1.666 and 1.9
			// instead of separate checks for each episode in doomult.exe, plutonia.exe and tnt.exe
			// There is no more desync on doom.wad\episode3.lmp
			// http://www.doomworld.com/idgames/index.php?id=6909
			if(gamemap != 8)
				return;
			if((mo->flags2 & MobjFlag2::E1M8Boss) != MobjFlag2{} && gameepisode != 1)
				return;
		}
		else
		{
			switch(gameepisode)
			{
				case 1:
					if(gamemap != 8)
						return;

					if((mo->flags2 & MobjFlag2::E1M8Boss) == MobjFlag2{})
						return;
					break;

				case 2:
					if(gamemap != 8)
						return;

					if((mo->flags2 & MobjFlag2::E2M8Boss) == MobjFlag2{})
						return;
					break;

				case 3:
					if(gamemap != 8)
						return;

					if((mo->flags2 & MobjFlag2::E3M8Boss) == MobjFlag2{})
						return;

					break;

				case 4:
					switch(gamemap)
					{
						case 6:
							if((mo->flags2 & MobjFlag2::E4M6Boss) == MobjFlag2{})
								return;
							break;

						case 8:
							if((mo->flags2 & MobjFlag2::E4M8Boss) == MobjFlag2{})
								return;
							break;

						default:
							return;
							break;
					}
					break;

				default:
					if(gamemap != 8)
						return;
					break;
			}
		}
	}

	if(!P_CheckBossDeath(mo))
	{
		return;
	}

	// victory!
	if(gamemode == GameMode::Commercial)
	{
		if(gamemap == 7)
		{
			if((mo->flags2 & MobjFlag2::Map07Boss1) != MobjFlag2{})
			{
				junk.special_args[0] = 666;
				EV_DoFloor(&junk, FloorKind::LowerFloorToLowest);
				return;
			}

			if((mo->flags2 & MobjFlag2::Map07Boss2) != MobjFlag2{})
			{
				junk.special_args[0] = 667;
				EV_DoFloor(&junk, FloorKind::RaiseToTexture);
				return;
			}
		}
	}
	else
	{
		switch(gameepisode)
		{
			case 1:
				junk.special_args[0] = 666;
				EV_DoFloor(&junk, FloorKind::LowerFloorToLowest);
				return;
				break;

			case 4:
				switch(gamemap)
				{
					case 6:
						junk.special_args[0] = 666;
						EV_DoDoor(&junk, static_cast<VerticalDoorType>(VerticalDoorType::BlazeOpen));
						return;
						break;

					case 8:
						junk.special_args[0] = 666;
						EV_DoFloor(&junk, FloorKind::LowerFloorToLowest);
						return;
						break;
				}
		}
	}
	G_ExitLevel(0);
}


extern "C" void A_Hoof(mobj_t* mo)
{
	S_StartMobjSound(mo, SfxId::Hoof);
	A_Chase(mo);
}

extern "C" void A_Metal(mobj_t* mo)
{
	S_StartMobjSound(mo, SfxId::Metal);
	A_Chase(mo);
}

extern "C" void A_BabyMetal(mobj_t* mo)
{
	S_StartMobjSound(mo, SfxId::Bspwlk);
	A_Chase(mo);
}

extern "C" void A_OpenShotgun2(player_t* player, pspdef_t* psp)
{
	S_StartMobjSound(player->mo, SfxId::Dbopn);
}

extern "C" void A_LoadShotgun2(player_t* player, pspdef_t* psp)
{
	S_StartMobjSound(player->mo, SfxId::Dbload);
}

extern "C" void A_CloseShotgun2(player_t* player, pspdef_t* psp)
{
	S_StartMobjSound(player->mo, SfxId::Dbcls);
	A_ReFire(player, psp);
}

// killough 2/7/98: Remove limit on icon landings:
mobj_t** braintargets;
int numbraintargets_alloc;
int numbraintargets;

struct brain_s brain; // killough 3/26/98: global state of boss brain

// killough 3/26/98: initialize icon landings at level startup,
// rather than at boss wakeup, to prevent savegame-related crashes

void P_SpawnBrainTargets() // killough 3/26/98: renamed old function
{
	thinker_t* thinker;

	// find all the target spots
	numbraintargets = 0;
	brain.targeton = 0;
	brain.easy = 0; // killough 3/26/98: always init easy to 0

	for(thinker = thinkercap.next;
		thinker != &thinkercap;
		thinker = thinker->next)
		if(thinker->function == reinterpret_cast<think_t>(P_MobjThinker))
		{
			mobj_t* m = (mobj_t*)thinker;

			if(m->type == MobjType::Bosstarget)
			{
				// killough 2/7/98: remove limit on icon landings:
				if(numbraintargets >= numbraintargets_alloc)
					braintargets = static_cast<decltype(braintargets)>(Z_Realloc(braintargets,
						(numbraintargets_alloc = numbraintargets_alloc ? numbraintargets_alloc * 2 : 32) * sizeof *braintargets));
				braintargets[numbraintargets++] = m;
			}
		}
}

extern "C" void A_BrainAwake(mobj_t* mo)
{
	//e6y
	if(demo_compatibility && !prboom_comp[std::to_underlying(PrboomComp::BoomBrainAwake)].state)
	{
		brain.targeton = 0;
		brain.easy = 0;
	}

	S_StartVoidSound(SfxId::Bossit); // killough 3/26/98: only generates sound now
}

extern "C" void A_BrainPain(mobj_t* mo)
{
	S_StartVoidSound(SfxId::Bospn);
}

extern "C" void A_BrainScream(mobj_t* mo)
{
	int x;
	for(x = mo->x - 196 * FRACUNIT; x < mo->x + 320 * FRACUNIT; x += FRACUNIT * 8)
	{
		int y = mo->y - 320 * FRACUNIT;
		int z = 128 + P_Random(RandomClass::Brainscream) * 2 * FRACUNIT;
		mobj_t* th = P_SpawnMobj(x, y, z, MobjType::Rocket);
		th->momz = P_Random(RandomClass::Brainscream) * 512;
		P_SetMobjState(th, StateId::Brainexplode1);
		th->tics -= P_Random(RandomClass::Brainscream) & 7;
		if(th->tics < 1)
			th->tics = 1;
	}
	S_StartVoidSound(SfxId::Bosdth);
}

extern "C" void A_BrainExplode(mobj_t* mo)
{
	// killough 5/5/98: remove dependence on order of evaluation:
	int t = P_Random(RandomClass::Brainexp);
	int x = mo->x + (t - P_Random(RandomClass::Brainexp)) * 2048;
	int y = mo->y;
	int z = 128 + P_Random(RandomClass::Brainexp) * 2 * FRACUNIT;
	mobj_t* th = P_SpawnMobj(x, y, z, MobjType::Rocket);
	th->momz = P_Random(RandomClass::Brainexp) * 512;
	P_SetMobjState(th, StateId::Brainexplode1);
	th->tics -= P_Random(RandomClass::Brainexp) & 7;
	if(th->tics < 1)
		th->tics = 1;
}

extern "C" void A_BrainDie(mobj_t* mo)
{
	G_ExitLevel(0);
}

extern "C" void A_BrainSpit(mobj_t* mo)
{
	mobj_t *targ, *newmobj;

	if(!numbraintargets) // killough 4/1/98: ignore if no targets
		return;

	brain.easy ^= 1; // killough 3/26/98: use brain struct
	if((skill_info.flags & SkillFlag::EasyBossBrain) != SkillFlag{} && !brain.easy)
		return;

	// shoot a cube at current target
	targ = braintargets[brain.targeton++]; // killough 3/26/98:
	brain.targeton %= numbraintargets;     // Use brain struct for targets

	// spawn brain missile
	newmobj = P_SpawnMissile(mo, targ, MobjType::Spawnshot);

	// e6y: do not crash with 'incorrect' DEHs
	if(!newmobj || !newmobj->state || newmobj->momy == 0 || newmobj->state->tics == 0)
		Log::Fatal("A_BrainSpit: can't spawn brain missile (incorrect DEH)");

	P_SetTarget(&newmobj->target, targ);
	newmobj->reactiontime = (short)(((targ->y - mo->y) / newmobj->momy) / newmobj->state->tics);

	// killough 7/18/98: brain friendliness is transferred
	newmobj->flags = (newmobj->flags - MobjFlag::Friend) | (mo->flags & MobjFlag::Friend);

	// killough 8/29/98: add to appropriate thread
	P_UpdateThinker(&newmobj->thinker);

	S_StartVoidSound(SfxId::Bospit);
}

// travelling cube sound
extern "C" void A_SpawnSound(mobj_t* mo)
{
	S_StartMobjSound(mo, SfxId::Boscub);
	A_SpawnFly(mo);
}

extern "C" void A_SpawnFly(mobj_t* mo)
{
	mobj_t* newmobj;
	mobj_t* fog;
	mobj_t* targ;
	int r;
	MobjType type;

	if(--mo->reactiontime)
		return; // still flying

	targ = P_SubstNullMobj(mo->target);

	// First spawn teleport fog.
	fog = P_SpawnMobj(targ->x, targ->y, targ->z, MobjType::Spawnfire);
	S_StartMobjSound(fog, SfxId::Telept);

	// Randomly select monster to spawn.
	r = P_Random(RandomClass::Spawnfly);

	// Probability distribution (kind of :), decreasing likelihood.
	if(r < 50)
		type = MobjType::Troop;
	else if(r < 90)
		type = MobjType::Sergeant;
	else if(r < 120)
		type = MobjType::Shadows;
	else if(r < 130)
		type = MobjType::Pain;
	else if(r < 160)
		type = MobjType::Head;
	else if(r < 162)
		type = MobjType::Vile;
	else if(r < 172)
		type = MobjType::Undead;
	else if(r < 192)
		type = MobjType::Baby;
	else if(r < 222)
		type = MobjType::Fatso;
	else if(r < 246)
		type = MobjType::Knight;
	else
		type = MobjType::Bruiser;

	newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, static_cast<MobjType>(type));

	/* killough 7/18/98: brain friendliness is transferred */
	newmobj->flags = (newmobj->flags - MobjFlag::Friend) | (mo->flags & MobjFlag::Friend);

	//e6y: monsters spawned by Icon of Sin should not be countable for total killed.
	newmobj->flags |= MobjFlag::Ressurected;

	dsda_WatchIconSpawn(newmobj);

	/* killough 8/29/98: add to appropriate thread */
	P_UpdateThinker(&newmobj->thinker);

	if(P_LookForTargets(newmobj,true)) /* killough 9/4/98 */
		P_SetMobjState(newmobj, newmobj->info->seestate);

	// telefrag anything in this spot
	P_TeleportMove(newmobj, newmobj->x, newmobj->y, true); /* killough 8/9/98 */

	// remove self (i.e., cube).
	P_RemoveMobj(mo);
}

extern "C" void A_PlayerScream(mobj_t* mo)
{
	SfxId sound = SfxId::Pldeth; // Default death sound.
	if(gamemode != GameMode::Shareware && mo->health < -50)
		sound = SfxId::Pdiehi; // IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
	S_StartMobjSound(mo, sound);
}

/* cph - MBF-added codepointer functions */

// killough 11/98: kill an object
extern "C" void A_Die(mobj_t* actor)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	P_DamageMobj(actor, nullptr, nullptr, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//

extern "C" void A_Detonate(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	P_RadiusAttack(mo, mo->target, mo->info->damage, mo->info->damage, BF_DAMAGESOURCE);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//

extern "C" void A_Mushroom(mobj_t* actor)
{
	int i, j, n;

	// Mushroom parameters are part of code pointer's state
	dboolean use_misc;
	fixed_t misc1, misc2;

	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	use_misc = mbf21 || (
		compatibility_level == CompLevel::Mbf &&
		!prboom_comp[std::to_underlying(PrboomComp::DoNotUseMisc12FrameParametersInAMushroom)].state
	);
	misc1 = ((use_misc && actor->state->misc1) ? actor->state->misc1 : FRACUNIT * 4);
	misc2 = ((use_misc && actor->state->misc2) ? actor->state->misc2 : FRACUNIT / 2);
	n = actor->info->damage;

	A_Explode(actor); // First make normal explosion

	// Now launch mushroom cloud
	for(i = -n; i <= n; i += 8)
		for(j = -n; j <= n; j += 8)
		{
			mobj_t target = *actor, *mo;
			target.x += i << FRACBITS; // Aim in many directions from source
			target.y += j << FRACBITS;
			target.z += P_AproxDistance(i, j) * misc1;       // Aim up fairly high
			mo = P_SpawnMissile(actor, &target, MobjType::Fatshot); // Launch fireball
			mo->momx = FixedMul(mo->momx, misc2);
			mo->momy = FixedMul(mo->momy, misc2); // Slow down a bit
			mo->momz = FixedMul(mo->momz, misc2);
			mo->flags -= MobjFlag::NoGravity; // Make debris fall under gravity
		}
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//

extern "C" void A_Spawn(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	if(mo->state->misc1)
	{
		mobj_t* newmobj =
			P_SpawnMobj(mo->x, mo->y, (mo->state->misc2 << FRACBITS) + mo->z, static_cast<MobjType>(mo->state->misc1 - 1));

		if(
			mbf_features &&
			comp[std::to_underlying(CompOption::FriendlySpawn)] &&
			!prboom_comp[std::to_underlying(PrboomComp::DoNotInheritFriendlynessFlagOnSpawn)].state
		)
			newmobj->flags = (newmobj->flags - MobjFlag::Friend) | (mo->flags & MobjFlag::Friend);
	}
}

extern "C" void A_Turn(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	mo->angle += (unsigned int)(((uint64_t)mo->state->misc1 << 32) / 360);
}

extern "C" void A_Face(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	mo->angle = (unsigned int)(((uint64_t)mo->state->misc1 << 32) / 360);
}

extern "C" void A_Scratch(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo))
		? mo->state->misc2 ? S_StartMobjSound(mo, static_cast<SfxId>(mo->state->misc2)) : (void)0,
		P_DamageMobj(mo->target, mo, mo, mo->state->misc1)
		: (void)0;
}

extern "C" void A_PlaySound(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	S_StartMobjSound(mo->state->misc2 ? nullptr : mo, static_cast<SfxId>(mo->state->misc1));
}

extern "C" void A_RandomJump(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	if(P_Random(RandomClass::Randomjump) < mo->state->misc2)
		P_SetMobjState(mo, static_cast<StateId>(mo->state->misc1));
}

//
// This allows linedef effects to be activated inside deh frames.
//

extern "C" void A_LineEffect(mobj_t* mo)
{
	if(compatibility_level < CompLevel::Lxdoom1 &&
		!prboom_comp[std::to_underlying(PrboomComp::ApplyMbfCodepointersToAnyComplevel)].state)
		return;

	if((mo->intflags & MobjIntFlag::Linedone) == MobjIntFlag{}) // Unless already used up
	{
		line_t junk = *lines;                        // Fake linedef set to 1st
		if((junk.special = (short)mo->state->misc1)) // Linedef type
		{
			// [FG] made static
			static player_t player;                                 // Remember player status
			player_t* oldplayer = mo->player;                       // Remember player status
			mo->player = &player;                                   // Fake player
			player.health = 100;                                    // Alive player
			junk.special_args[0] = (short)mo->state->misc2;         // Sector tag for linedef
			if(!P_UseSpecialLine(mo, &junk, 0, false))              // Try using it
				map_format.cross_special_line(&junk, 0, mo, false); // Try crossing it
			if(!junk.special)                                       // If type cleared,
				mo->intflags |= MobjIntFlag::Linedone;                       // no more for this thing
			mo->player = oldplayer;                                 // Restore player status
		}
	}
}

//
// [XA] New mbf21 codepointers
//

//
// A_SpawnObject
// Basically just A_Spawn with better behavior and more args.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: X spawn offset (fixed point), relative to calling actor
//   args[3]: Y spawn offset (fixed point), relative to calling actor
//   args[4]: Z spawn offset (fixed point), relative to calling actor
//   args[5]: X velocity (fixed point)
//   args[6]: Y velocity (fixed point)
//   args[7]: Z velocity (fixed point)
//
extern "C" void A_SpawnObject(mobj_t* actor)
{
	int type, angle, ofs_x, ofs_y, ofs_z, vel_x, vel_y, vel_z;
	angle_t an;
	int fan, dx, dy;
	mobj_t* mo;

	if(!mbf21 || !actor->state->args[0])
		return;

	type = actor->state->args[0] - 1;
	angle = actor->state->args[1];
	ofs_x = actor->state->args[2];
	ofs_y = actor->state->args[3];
	ofs_z = actor->state->args[4];
	vel_x = actor->state->args[5];
	vel_y = actor->state->args[6];
	vel_z = actor->state->args[7];

	// calculate position offsets
	an = actor->angle + (unsigned int)(((int64_t)angle << 16) / 360);
	fan = an >> ANGLETOFINESHIFT;
	dx = FixedMul(ofs_x, finecosine[fan]) - FixedMul(ofs_y, finesine[fan]);
	dy = FixedMul(ofs_x, finesine[fan]) + FixedMul(ofs_y, finecosine[fan]);

	// spawn it, yo
	mo = P_SpawnMobj(actor->x + dx, actor->y + dy, actor->z + ofs_z, static_cast<MobjType>(type));
	if(!mo)
		return;

	// angle dangle
	mo->angle = an;

	// set velocity
	mo->momx = FixedMul(vel_x, finecosine[fan]) - FixedMul(vel_y, finesine[fan]);
	mo->momy = FixedMul(vel_x, finesine[fan]) + FixedMul(vel_y, finecosine[fan]);
	mo->momz = vel_z;

	// if spawned object is a missile, set target+tracer
	if((mo->info->flags & (MobjFlag::Missile | MobjFlag::Bounces)) != MobjFlag{})
	{
		// if spawner is also a missile, copy 'em
		if((actor->info->flags & (MobjFlag::Missile | MobjFlag::Bounces)) != MobjFlag{})
		{
			P_SetTarget(&mo->target, actor->target);
			P_SetTarget(&mo->tracer, actor->tracer);
		}
		// otherwise, set 'em as if a monster fired 'em
		else
		{
			P_SetTarget(&mo->target, actor);
			P_SetTarget(&mo->tracer, actor->target);
		}
	}

	// [XA] don't bother with the dont-inherit-friendliness hack
	// that exists in A_Spawn, 'cause WTF is that about anyway?
}

//
// A_MonsterProjectile
// A parameterized monster projectile attack.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling actor's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling actor's angle
//   args[4]: Z spawn offset, relative to actor's default projectile fire height
//
extern "C" void A_MonsterProjectile(mobj_t* actor)
{
	int type, angle, pitch, spawnofs_xy, spawnofs_z;
	mobj_t* mo;
	int an;

	if(!mbf21 || !actor->target || !actor->state->args[0])
		return;

	type = actor->state->args[0] - 1;
	angle = actor->state->args[1];
	pitch = actor->state->args[2];
	spawnofs_xy = actor->state->args[3];
	spawnofs_z = actor->state->args[4];

	A_FaceTarget(actor);
	mo = P_SpawnMissile(actor, actor->target, static_cast<MobjType>(type));
	if(!mo)
		return;

	// adjust angle
	mo->angle += (unsigned int)(((int64_t)angle << 16) / 360);
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[an]);
	mo->momy = FixedMul(mo->info->speed, finesine[an]);

	// adjust pitch (approximated, using Doom's ye olde
	// finetangent table; same method as monster aim)
	mo->momz += FixedMul(mo->info->speed, DegToSlope(pitch));

	// adjust position
	an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
	mo->x += FixedMul(spawnofs_xy, finecosine[an]);
	mo->y += FixedMul(spawnofs_xy, finesine[an]);
	mo->z += spawnofs_z;

	// always set the 'tracer' field, so this pointer
	// can be used to fire seeker missiles at will.
	P_SetTarget(&mo->tracer, actor->target);
}

//
// A_MonsterBulletAttack
// A parameterized monster bullet attack.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 3d5, customize the 3); if not set, defaults to 3
//   args[4]: Attack damage modulus (e.g. for 3d5, customize the 5); if not set, defaults to 5
//
extern "C" void A_MonsterBulletAttack(mobj_t* actor)
{
	int hspread, vspread, numbullets, damagebase, damagemod;
	int aimslope, i, damage, angle, slope;

	if(!mbf21 || !actor->target)
		return;

	hspread = actor->state->args[0];
	vspread = actor->state->args[1];
	numbullets = actor->state->args[2];
	damagebase = actor->state->args[3];
	damagemod = actor->state->args[4];

	A_FaceTarget(actor);
	S_StartMobjSound(actor, actor->info->attacksound);

	aimslope = P_AimLineAttack(actor, actor->angle, MISSILERANGE, MobjFlag{});

	for(i = 0; i < numbullets; i++)
	{
		damage = (P_Random(RandomClass::Mbf21) % damagemod + 1) * damagebase;
		angle = actor->angle + static_cast<angle_t>(P_RandomHitscanAngle(RandomClass::Mbf21, hspread));
		slope = aimslope + P_RandomHitscanSlope(RandomClass::Mbf21, vspread);

		P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
	}
}

//
// A_MonsterMeleeAttack
// A parameterized monster melee attack.
//   args[0]: Base damage of attack (e.g. for 3d8, customize the 3); if not set, defaults to 3
//   args[1]: Attack damage modulus (e.g. for 3d8, customize the 8); if not set, defaults to 8
//   args[2]: Sound to play if attack hits
//   args[3]: Range (fixed point); if not set, defaults to monster's melee range
//
extern "C" void A_MonsterMeleeAttack(mobj_t* actor)
{
	int damagebase, damagemod, range;
	SfxId hitsound;
	int damage;

	if(!mbf21 || !actor->target)
		return;

	damagebase = actor->state->args[0];
	damagemod = actor->state->args[1];
	hitsound = static_cast<SfxId>(actor->state->args[2]);
	range = actor->state->args[3];

	if(range == 0)
		range = actor->info->meleerange;

	range += actor->target->info->radius - 20 * FRACUNIT;

	A_FaceTarget(actor);
	if(!P_CheckRange(actor, range))
		return;

	S_StartMobjSound(actor, hitsound);

	damage = (P_Random(RandomClass::Mbf21) % damagemod + 1) * damagebase;
	P_DamageMobj(actor->target, actor, actor, damage);
}

//
// A_RadiusDamage
// A parameterized version of A_Explode. Friggin' finally. :P
//   args[0]: Damage (int)
//   args[1]: Radius (also int; no real need for fractional precision here)
//
extern "C" void A_RadiusDamage(mobj_t* actor)
{
	if(!mbf21 || !actor->state)
		return;

	P_RadiusAttack(actor, actor->target, actor->state->args[0], actor->state->args[1], BF_DAMAGESOURCE);
}

//
// A_NoiseAlert
// Alerts nearby monsters (via sound) to the calling actor's target's presence.
//
extern "C" void A_NoiseAlert(mobj_t* actor)
{
	if(!mbf21 || !actor->target)
		return;

	P_NoiseAlert(actor->target, actor);
}

//
// A_HealChase
// A parameterized version of A_VileChase.
//   args[0]: State to jump to on the calling actor when resurrecting a corpse
//   args[1]: Sound to play when resurrecting a corpse
//
extern "C" void A_HealChase(mobj_t* actor)
{
	int state, sound;

	if(!mbf21 || !actor)
		return;

	state = actor->state->args[0];
	sound = actor->state->args[1];

	if(!P_HealCorpse(actor, actor->info->radius, static_cast<StateId>(state), static_cast<SfxId>(sound)))
		A_Chase(actor);
}

//
// A_SeekTracer
// A parameterized seeker missile function.
//   args[0]: direct-homing threshold angle (degrees, in fixed point)
//   args[1]: maximum turn angle (degrees, in fixed point)
//
extern "C" void A_SeekTracer(mobj_t* actor)
{
	angle_t threshold, maxturnangle;

	if(!mbf21 || !actor)
		return;

	threshold = FixedToAngle(actor->state->args[0]);
	maxturnangle = FixedToAngle(actor->state->args[1]);

	P_SeekerMissile(actor, &actor->tracer, threshold, maxturnangle, true);
}

//
// A_FindTracer
// Search for a valid tracer (seek target), if the calling actor doesn't already have one.
//   args[0]: field-of-view to search in (degrees, in fixed point); if zero, will search in all directions
//   args[1]: distance to search (map blocks, i.e. 128 units)
//
extern "C" void A_FindTracer(mobj_t* actor)
{
	angle_t fov;
	int dist;

	if(!mbf21 || !actor || actor->tracer)
		return;

	fov = FixedToAngle(actor->state->args[0]);
	dist = (actor->state->args[1]);

	P_SetTarget(&actor->tracer, P_RoughTargetSearch(actor, fov, dist));
}

//
// A_ClearTracer
// Clear current tracer (seek target).
//
extern "C" void A_ClearTracer(mobj_t* actor)
{
	if(!mbf21 || !actor)
		return;

	P_SetTarget(&actor->tracer, nullptr);
}

//
// A_JumpIfHealthBelow
// Jumps to a state if caller's health is below the specified threshold.
//   args[0]: State to jump to
//   args[1]: Health threshold
//
extern "C" void A_JumpIfHealthBelow(mobj_t* actor)
{
	int state, health;

	if(!mbf21 || !actor)
		return;

	state = actor->state->args[0];
	health = actor->state->args[1];

	if(actor->health < health)
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_JumpIfTargetInSight
// Jumps to a state if caller's target is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
extern "C" void A_JumpIfTargetInSight(mobj_t* actor)
{
	int state;
	angle_t fov;

	if(!mbf21 || !actor || !actor->target)
		return;

	state = (actor->state->args[0]);
	fov = FixedToAngle(actor->state->args[1]);

	// Check FOV first since it's faster
	if(fov > 0 && !P_CheckFov(actor, actor->target, fov))
		return;

	if(P_CheckSight(actor, actor->target))
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_JumpIfTargetCloser
// Jumps to a state if caller's target is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold
//
extern "C" void A_JumpIfTargetCloser(mobj_t* actor)
{
	int state, distance;

	if(!mbf21 || !actor || !actor->target)
		return;

	state = actor->state->args[0];
	distance = actor->state->args[1];

	if(distance > P_AproxDistance(actor->x - actor->target->x,
		actor->y - actor->target->y))
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_JumpIfTracerInSight
// Jumps to a state if caller's tracer (seek target) is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
extern "C" void A_JumpIfTracerInSight(mobj_t* actor)
{
	angle_t fov;
	int state;

	if(!mbf21 || !actor || !actor->tracer)
		return;

	state = (actor->state->args[0]);
	fov = FixedToAngle(actor->state->args[1]);

	// Check FOV first since it's faster
	if(fov > 0 && !P_CheckFov(actor, actor->tracer, fov))
		return;

	if(P_CheckSight(actor, actor->tracer))
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_JumpIfTracerCloser
// Jumps to a state if caller's tracer (seek target) is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold (fixed point)
//
extern "C" void A_JumpIfTracerCloser(mobj_t* actor)
{
	int state, distance;

	if(!mbf21 || !actor || !actor->tracer)
		return;

	state = actor->state->args[0];
	distance = actor->state->args[1];

	if(distance > P_AproxDistance(actor->x - actor->tracer->x,
		actor->y - actor->tracer->y))
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_JumpIfFlagsSet
// Jumps to a state if caller has the specified thing flags set.
//   args[0]: State to jump to
//   args[1]: Standard Flag(s) to check
//   args[2]: MBF21 Flag(s) to check
//
extern "C" void A_JumpIfFlagsSet(mobj_t* actor)
{
	int state;
	MobjFlag flags;
	MobjFlag2 flags2;

	if(!mbf21 || !actor)
		return;

	state = actor->state->args[0];
	flags = static_cast<MobjFlag>(actor->state->args[1]);
	flags2 = static_cast<MobjFlag2>(actor->state->args[2]);

	if((actor->flags & flags) == flags &&
		(actor->flags2 & flags2) == flags2)
		P_SetMobjState(actor, static_cast<StateId>(state));
}

//
// A_AddFlags
// Adds the specified thing flags to the caller.
//   args[0]: Standard Flag(s) to add
//   args[1]: MBF21 Flag(s) to add
//
extern "C" void A_AddFlags(mobj_t* actor)
{
	MobjFlag flags;
	MobjFlag2 flags2;
	dboolean update_blockmap;

	if(!mbf21 || !actor)
		return;

	flags = static_cast<MobjFlag>(actor->state->args[0]);
	flags2 = static_cast<MobjFlag2>(actor->state->args[1]);

	// unlink/relink the thing from the blockmap if
	// the NOBLOCKMAP or NOSECTOR flags are added
	update_blockmap = ((flags & MobjFlag::NoBlockmap) != MobjFlag{} && (actor->flags & MobjFlag::NoBlockmap) == MobjFlag{})
		|| ((flags & MobjFlag::NoSector) != MobjFlag{} && (actor->flags & MobjFlag::NoSector) == MobjFlag{});

	if(update_blockmap)
		P_UnsetThingPosition(actor);

	actor->flags |= flags;
	actor->flags2 |= flags2;

	if(update_blockmap)
		P_SetThingPosition(actor);
}

//
// A_RemoveFlags
// Removes the specified thing flags from the caller.
//   args[0]: Flag(s) to remove
//   args[1]: MBF21 Flag(s) to remove
//
extern "C" void A_RemoveFlags(mobj_t* actor)
{
	MobjFlag flags;
	MobjFlag2 flags2;
	dboolean update_blockmap;

	if(!mbf21 || !actor)
		return;

	flags = static_cast<MobjFlag>(actor->state->args[0]);
	flags2 = static_cast<MobjFlag2>(actor->state->args[1]);

	// unlink/relink the thing from the blockmap if
	// the NOBLOCKMAP or NOSECTOR flags are removed
	update_blockmap = ((flags & MobjFlag::NoBlockmap) != MobjFlag{} && (actor->flags & MobjFlag::NoBlockmap) != MobjFlag{})
		|| ((flags & MobjFlag::NoSector) != MobjFlag{} && (actor->flags & MobjFlag::NoSector) != MobjFlag{});

	if(update_blockmap)
		P_UnsetThingPosition(actor);

	actor->flags -= flags;
	actor->flags2 -= flags2;

	if(update_blockmap)
		P_SetThingPosition(actor);
}



// heretic

#include "heretic/def.hpp"

#define MAX_BOSS_SPOTS 8

typedef struct
{
	fixed_t x;
	fixed_t y;
	angle_t angle;
} BossSpot_t;

static int BossSpotCount;
static BossSpot_t BossSpots[MAX_BOSS_SPOTS];

void P_InitMonsters()
{
	BossSpotCount = 0;
}

void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle)
{
	if(BossSpotCount == MAX_BOSS_SPOTS)
	{
		Log::Fatal("Too many boss spots.");
	}
	BossSpots[BossSpotCount].x = x;
	BossSpots[BossSpotCount].y = y;
	BossSpots[BossSpotCount].angle = angle;
	BossSpotCount++;
}

extern "C" void A_DripBlood(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_SubRandom();
	r2 = P_SubRandom();

	mo = P_SpawnMobj(actor->x + (r2 << 11),
		actor->y + (r1 << 11), actor->z,
		MobjType::HereticBlood);
	mo->momx = P_SubRandom() << 10;
	mo->momy = P_SubRandom() << 10;
	mo->flags2 |= MobjFlag2::LoGrav;
}

extern "C" void A_KnightAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		S_StartMobjSound(actor, SfxId::HereticKgtat2);
		return;
	}
	// Throw axe
	S_StartMobjSound(actor, actor->info->attacksound);
	if(actor->type == MobjType::HereticKnightghost || P_Random(RandomClass::Heretic) < 40)
	{
		// Red axe
		P_SpawnMissile(actor, actor->target, MobjType::HereticRedaxe);
		return;
	}
	// Green axe
	P_SpawnMissile(actor, actor->target, MobjType::HereticKnightaxe);
}

extern "C" void A_ImpExplode(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HereticImpchunk1);
	mo->momx = P_SubRandom() << 10;
	mo->momy = P_SubRandom() << 10;
	mo->momz = 9 * FRACUNIT;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HereticImpchunk2);
	mo->momx = P_SubRandom() << 10;
	mo->momy = P_SubRandom() << 10;
	mo->momz = 9 * FRACUNIT;
	if(actor->special1.i == 666)
	{
		// Extreme death crash
		P_SetMobjState(actor, StateId::HereticImpXcrash1);
	}
}

extern "C" void A_BeastPuff(mobj_t* actor)
{
	if(P_Random(RandomClass::Heretic) > 64)
	{
		int r1, r2, r3;
		r1 = P_SubRandom();
		r2 = P_SubRandom();
		r3 = P_SubRandom();
		P_SpawnMobj(actor->x + (r3 << 10),
			actor->y + (r2 << 10),
			actor->z + (r1 << 10), MobjType::HereticPuffy);
	}
}

extern "C" void A_ImpMeAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(RandomClass::Heretic) & 7));
	}
}

extern "C" void A_ImpMsAttack(mobj_t* actor)
{
	mobj_t* dest;
	angle_t an;
	int dist;

	if(!actor->target || P_Random(RandomClass::Heretic) > 64)
	{
		P_SetMobjState(actor, actor->info->seestate);
		return;
	}
	dest = actor->target;
	actor->flags |= MobjFlag::SkullFly;
	S_StartMobjSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(12 * FRACUNIT, finecosine[an]);
	actor->momy = FixedMul(12 * FRACUNIT, finesine[an]);
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	dist = dist / (12 * FRACUNIT);
	if(dist < 1)
	{
		dist = 1;
	}
	actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

extern "C" void A_ImpMsAttack2(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(RandomClass::Heretic) & 7));
		return;
	}
	P_SpawnMissile(actor, actor->target, MobjType::HereticImpball);
}

extern "C" void A_ImpDeath(mobj_t* actor)
{
	actor->flags -= MobjFlag::Solid;
	actor->flags2 |= MobjFlag2::FootClip;
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, StateId::HereticImpCrash1);
	}
}

extern "C" void A_ImpXDeath1(mobj_t* actor)
{
	actor->flags -= MobjFlag::Solid;
	actor->flags |= MobjFlag::NoGravity;
	actor->flags2 |= MobjFlag2::FootClip;
	actor->special1.i = 666; // Flag the crash routine
}

extern "C" void A_ImpXDeath2(mobj_t* actor)
{
	actor->flags -= MobjFlag::NoGravity;
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, StateId::HereticImpCrash1);
	}
}

dboolean P_UpdateChicken(mobj_t* actor, int tics)
{
	mobj_t* fog;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	MobjType moType;
	mobj_t* mo;
	mobj_t oldChicken;

	actor->special1.i -= tics;
	if(actor->special1.i > 0)
	{
		return (false);
	}
	moType = static_cast<MobjType>(actor->special2.i);
	x = actor->x;
	y = actor->y;
	z = actor->z;
	oldChicken = *actor;
	P_SetMobjState(actor, StateId::HereticFreetargmobj);
	mo = P_SpawnMobj(x, y, z, moType);
	dsda_WatchUnMorph(mo);
	if(P_TestMobjLocation(mo) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, MobjType::HereticChicken);
		mo->angle = oldChicken.angle;
		mo->flags = oldChicken.flags;
		mo->health = oldChicken.health;
		P_SetTarget(&mo->target, oldChicken.target);
		mo->special1.i = 5 * TICRATE; // Next try in 5 seconds
		mo->special2.i = std::to_underlying(moType);
		dsda_WatchMorph(mo);
		return (false);
	}
	mo->angle = oldChicken.angle;
	P_SetTarget(&mo->target, oldChicken.target);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HereticTfog);
	S_StartMobjSound(fog, SfxId::HereticTelept);
	return (true);
}

extern "C" void A_ChicAttack(mobj_t* actor)
{
	if(P_UpdateChicken(actor, 18))
	{
		return;
	}
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 1 + (P_Random(RandomClass::Heretic) & 1));
	}
}

extern "C" void A_ChicLook(mobj_t* actor)
{
	if(P_UpdateChicken(actor, 10))
	{
		return;
	}
	A_Look(actor);
}

extern "C" void A_ChicChase(mobj_t* actor)
{
	if(P_UpdateChicken(actor, 3))
	{
		return;
	}
	A_Chase(actor);
}

extern "C" void A_ChicPain(mobj_t* actor)
{
	if(P_UpdateChicken(actor, 10))
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->painsound);
}

extern "C" void A_Feathers(mobj_t* actor)
{
	int i;
	int count;
	mobj_t* mo;

	if(actor->health > 0)
	{
		// Pain
		count = P_Random(RandomClass::Heretic) < 32 ? 2 : 1;
	}
	else
	{
		// Death
		count = 5 + (P_Random(RandomClass::Heretic) & 3);
	}
	for(i = 0; i < count; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z + 20 * FRACUNIT,
			MobjType::HereticFeather);
		P_SetTarget(&mo->target, actor);
		mo->momx = P_SubRandom() << 8;
		mo->momy = P_SubRandom() << 8;
		mo->momz = FRACUNIT + (P_Random(RandomClass::Heretic) << 9);
		P_SetMobjState(mo, StateVariant(StateId::HereticFeather1, (P_Random(RandomClass::Heretic) & 7)));
	}
}

extern "C" void A_MummyAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
		S_StartMobjSound(actor, SfxId::HereticMumat2);
		return;
	}
	S_StartMobjSound(actor, SfxId::HereticMumat1);
}

extern "C" void A_MummyAttack2(mobj_t* actor)
{
	mobj_t* mo;

	if(!actor->target)
	{
		return;
	}

	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MobjType::HereticMummyfx1);

	if(mo != nullptr)
	{
		P_SetTarget(&mo->special1.m, actor->target);
	}
}

extern "C" void A_MummyFX1Seek(mobj_t* actor)
{
	P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 20, false);
}

extern "C" void A_MummySoul(mobj_t* mummy)
{
	mobj_t* mo;

	mo = P_SpawnMobj(mummy->x, mummy->y, mummy->z + 10 * FRACUNIT,
		MobjType::HereticMummysoul);
	mo->momz = FRACUNIT;
}

extern "C" void A_Sor1Pain(mobj_t* actor)
{
	actor->special1.i = 20; // Number of steps to walk fast
	A_Pain(actor);
}

extern "C" void A_Sor1Chase(mobj_t* actor)
{
	if(actor->special1.i)
	{
		actor->special1.i--;
		actor->tics -= 3;
	}
	A_Chase(actor);
}

extern "C" void A_Srcr1Attack(mobj_t* actor)
{
	mobj_t* mo;
	fixed_t momz;
	angle_t angle;

	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(8));
		return;
	}
	if(actor->health > (P_MobjSpawnHealth(actor) / 3) * 2)
	{
		// Spit one fireball
		P_SpawnMissile(actor, actor->target, MobjType::HereticSrcrfx1);
	}
	else
	{
		// Spit three fireballs
		mo = P_SpawnMissile(actor, actor->target, MobjType::HereticSrcrfx1);
		if(mo)
		{
			momz = mo->momz;
			angle = mo->angle;
			P_SpawnMissileAngle(actor, MobjType::HereticSrcrfx1, angle - ANG1_X * 3, momz);
			P_SpawnMissileAngle(actor, MobjType::HereticSrcrfx1, angle + ANG1_X * 3, momz);
		}
		if(actor->health < P_MobjSpawnHealth(actor) / 3)
		{
			// Maybe attack again
			if(actor->special1.i)
			{
				// Just attacked, so don't attack again
				actor->special1.i = 0;
			}
			else
			{
				// Set state to attack again
				actor->special1.i = 1;
				P_SetMobjState(actor, StateId::HereticSrcr1Atk4);
			}
		}
	}
}

extern "C" void A_SorcererRise(mobj_t* actor)
{
	mobj_t* mo;

	actor->flags -= MobjFlag::Solid;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HereticSorcerer2);
	P_SetMobjState(mo, StateId::HereticSor2Rise1);
	mo->angle = actor->angle;
	P_SetTarget(&mo->target, actor->target);
}

void P_DSparilTeleport(mobj_t* actor)
{
	int i;
	fixed_t x;
	fixed_t y;
	fixed_t prevX;
	fixed_t prevY;
	fixed_t prevZ;
	mobj_t* mo;

	if(!BossSpotCount)
	{
		// No spots
		return;
	}
	i = P_Random(RandomClass::Heretic);
	do
	{
		i++;
		x = BossSpots[i % BossSpotCount].x;
		y = BossSpots[i % BossSpotCount].y;
	}
	while(P_AproxDistance(actor->x - x, actor->y - y) < 128 * FRACUNIT);
	prevX = actor->x;
	prevY = actor->y;
	prevZ = actor->z;
	if(P_TeleportMove(actor, x, y, false))
	{
		mo = P_SpawnMobj(prevX, prevY, prevZ, MobjType::HereticSor2telefade);
		S_StartMobjSound(mo, SfxId::HereticTelept);
		P_SetMobjState(actor, StateId::HereticSor2Tele1);
		S_StartMobjSound(actor, SfxId::HereticTelept);
		actor->z = actor->floorz;
		actor->angle = BossSpots[i % BossSpotCount].angle;
		actor->momx = actor->momy = actor->momz = 0;
	}
}


extern "C" void A_Srcr2Decide(mobj_t* actor)
{
	static int chance[] = {
		192, 120, 120, 120, 64, 64, 32, 16, 0
	};

	if(!BossSpotCount)
	{
		// No spots
		return;
	}
	if(P_Random(RandomClass::Heretic) < chance[actor->health / (P_MobjSpawnHealth(actor) / 8)])
	{
		P_DSparilTeleport(actor);
	}
}

extern "C" void A_Srcr2Attack(mobj_t* actor)
{
	int chance;

	if(!actor->target)
	{
		return;
	}
	S_StartVoidSound(actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(20));
		return;
	}
	chance = actor->health < P_MobjSpawnHealth(actor) / 2 ? 96 : 48;
	if(P_Random(RandomClass::Heretic) < chance)
	{
		// Wizard spawners
		P_SpawnMissileAngle(actor, MobjType::HereticSor2fx2,
			actor->angle - ANG45, FRACUNIT / 2);
		P_SpawnMissileAngle(actor, MobjType::HereticSor2fx2,
			actor->angle + ANG45, FRACUNIT / 2);
	}
	else
	{
		// Blue bolt
		P_SpawnMissile(actor, actor->target, MobjType::HereticSor2fx1);
	}
}

extern "C" void A_BlueSpark(mobj_t* actor)
{
	int i;
	mobj_t* mo;

	for(i = 0; i < 2; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HereticSor2fxspark);
		mo->momx = P_SubRandom() << 9;
		mo->momy = P_SubRandom() << 9;
		mo->momz = FRACUNIT + (P_Random(RandomClass::Heretic) << 8);
	}
}

extern "C" void A_GenWizard(mobj_t* actor)
{
	mobj_t* mo;
	mobj_t* fog;

	mo = P_SpawnMobj(actor->x, actor->y,
		actor->z - mobjinfo[std::to_underlying(MobjType::HereticWizard)].height / 2, MobjType::HereticWizard);
	if(P_TestMobjLocation(mo) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		return;
	}
	actor->momx = actor->momy = actor->momz = 0;
	P_SetMobjState(actor, static_cast<StateId>(mobjinfo[std::to_underlying(actor->type)].deathstate));
	actor->flags -= MobjFlag::Missile;
	fog = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HereticTfog);
	S_StartMobjSound(fog, SfxId::HereticTelept);
}

void P_Massacre()
{
	mobj_t* mo;
	thinker_t* think;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mo = (mobj_t*)think;
		if((mo->flags & MobjFlag::CountKill) != MobjFlag{} && (mo->health > 0))
		{
			if(hexen)
			{
				mo->flags2 -= (MobjFlag2::NonShootable | MobjFlag2::Invulnerable);
				mo->flags |= MobjFlag::Shootable;
			}
			P_DamageMobj(mo, nullptr, nullptr, 10000);
		}
	}
}

extern "C" void A_Sor2DthInit(mobj_t* actor)
{
	actor->special1.i = 7; // Animation loop counter
	P_Massacre();          // Kill monsters early
}

extern "C" void A_Sor2DthLoop(mobj_t* actor)
{
	if(--actor->special1.i)
	{
		// Need to loop
		P_SetMobjState(actor, StateId::HereticSor2Die4);
	}
}

extern "C" void A_SorZap(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSorzap);
}

extern "C" void A_SorRise(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSorrise);
}

extern "C" void A_SorDSph(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSordsph);
}

extern "C" void A_SorDExp(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSordexp);
}

extern "C" void A_SorDBon(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSordbon);
}

extern "C" void A_SorSightSnd(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HereticSorsit);
}

extern "C" void A_MinotaurAtk1(mobj_t* actor)
{
	player_t* player;

	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, g_mntr_atk1_sfx);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		if(heretic && (player = actor->target->player) != nullptr)
		{
			// Squish the player
			player->deltaviewheight = -16 * FRACUNIT;
		}
	}
}

extern "C" void A_MinotaurDecide(mobj_t* actor)
{
	angle_t angle;
	mobj_t* target;
	int dist;

	target = actor->target;
	if(!target)
	{
		return;
	}
	if(heretic)
		S_StartMobjSound(actor, SfxId::HereticMinsit);
	dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
	if(target->z + target->height > actor->z
		&& target->z + target->height < actor->z + actor->height
		&& dist < g_mntr_decide_range * 64 * FRACUNIT
		&& dist > 1 * 64 * FRACUNIT && P_Random(RandomClass::Heretic) < g_mntr_charge_rng)
	{
		// Charge attack
		// Don't call the state function right away
		P_SetMobjStateNF(actor, static_cast<StateId>(g_mntr_charge_state));
		actor->flags |= MobjFlag::SkullFly;
		A_FaceTarget(actor);
		angle = actor->angle >> ANGLETOFINESHIFT;
		actor->momx = FixedMul(g_mntr_charge_speed, finecosine[angle]);
		actor->momy = FixedMul(g_mntr_charge_speed, finesine[angle]);
		// Charge duration
		if(hexen)
			actor->special_args[4] = TICRATE / 2;
		else
			actor->special1.i = TICRATE / 2;
	}
	else if(target->z == target->floorz
		&& dist < 9 * 64 * FRACUNIT && P_Random(RandomClass::Heretic) < g_mntr_fire_rng)
	{
		// Floor fire attack
		P_SetMobjState(actor, static_cast<StateId>(g_mntr_fire_state));
		actor->special2.i = 0;
	}
	else
	{
		// Swing attack
		A_FaceTarget(actor);
		// Don't need to call P_SetMobjState because the current state
		// falls through to the swing attack
	}
}

extern "C" void A_MinotaurCharge(mobj_t* actor)
{
	mobj_t* puff;

	if(hexen && !actor->target)
		return;

	if(hexen ? actor->special_args[4] > 0 : actor->special1.i)
	{
		puff = P_SpawnMobj(actor->x, actor->y, actor->z, static_cast<MobjType>(g_mntr_charge_puff));
		puff->momz = 2 * FRACUNIT;
		if(hexen)
			actor->special_args[4]--;
		else
			actor->special1.i--;
	}
	else
	{
		actor->flags -= MobjFlag::SkullFly;
		P_SetMobjState(actor, actor->info->seestate);
	}
}

extern "C" void A_MinotaurAtk2(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;
	fixed_t momz;

	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, g_mntr_atk2_sfx);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(g_mntr_atk2_dice));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, static_cast<MobjType>(g_mntr_atk2_missile));
	if(mo)
	{
		if(heretic)
			S_StartMobjSound(mo, g_mntr_atk2_sfx);
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngle(actor, static_cast<MobjType>(g_mntr_atk2_missile), angle - (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, static_cast<MobjType>(g_mntr_atk2_missile), angle + (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, static_cast<MobjType>(g_mntr_atk2_missile), angle - (ANG45 / 16), momz);
		P_SpawnMissileAngle(actor, static_cast<MobjType>(g_mntr_atk2_missile), angle + (ANG45 / 16), momz);
	}
}

extern "C" void A_MinotaurAtk3(mobj_t* actor)
{
	mobj_t* mo;
	player_t* player;

	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(g_mntr_atk3_dice));
		if((player = actor->target->player) != nullptr)
		{
			// Squish the player
			player->deltaviewheight = -16 * FRACUNIT;
		}
	}
	else
	{
		mo = P_SpawnMissile(actor, actor->target, static_cast<MobjType>(g_mntr_atk3_missile));
		if(mo != nullptr)
		{
			S_StartMobjSound(mo, g_mntr_atk3_sfx);
		}
	}
	if(P_Random(RandomClass::Heretic) < 192 && actor->special2.i == 0)
	{
		P_SetMobjState(actor, static_cast<StateId>(g_mntr_atk3_state));
		actor->special2.i = 1;
	}
}

extern "C" void A_MntrFloorFire(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_SubRandom();
	r2 = P_SubRandom();

	actor->z = actor->floorz;
	mo = P_SpawnMobj(actor->x + (r2 << 10),
		actor->y + (r1 << 10), ONFLOORZ,
		static_cast<MobjType>(g_mntr_fire));
	P_SetTarget(&mo->target, actor->target);
	mo->momx = 1; // Force block checking
	P_CheckMissileSpawn(mo);
}

extern "C" void A_BeastAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		return;
	}
	P_SpawnMissile(actor, actor->target, MobjType::HereticBeastball);
}

extern "C" void A_WhirlwindSeek(mobj_t* actor)
{
	actor->health -= 3;
	if(actor->health < 0)
	{
		actor->momx = actor->momy = actor->momz = 0;
		P_SetMobjState(actor, static_cast<StateId>(mobjinfo[std::to_underlying(actor->type)].deathstate));
		actor->flags -= MobjFlag::Missile;
		return;
	}
	if((actor->special2.i -= 3) < 0)
	{
		actor->special2.i = 58 + (P_Random(RandomClass::Heretic) & 31);
		S_StartMobjSound(actor, SfxId::HereticHedat3);
	}
	if(actor->special1.m
		&& (((mobj_t*)(actor->special1.m))->flags & MobjFlag::Shadow) != MobjFlag{})
	{
		return;
	}
	P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 30, false);
}

extern "C" void A_HeadIceImpact(mobj_t* ice)
{
	unsigned int i;
	angle_t angle;
	mobj_t* shard;

	for(i = 0; i < 8; i++)
	{
		shard = P_SpawnMobj(ice->x, ice->y, ice->z, MobjType::HereticHeadfx2);
		angle = i * ANG45;
		P_SetTarget(&shard->target, ice->target);
		shard->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		shard->momx = FixedMul(shard->info->speed, finecosine[angle]);
		shard->momy = FixedMul(shard->info->speed, finesine[angle]);
		shard->momz = (fixed_t)(-.6 * FRACUNIT);
		P_CheckMissileSpawn(shard);
	}
}

extern "C" void A_HeadFireGrow(mobj_t* fire)
{
	fire->health--;
	fire->z += 9 * FRACUNIT;
	if(fire->health == 0)
	{
		fire->damage = fire->info->damage;
		P_SetMobjState(fire, StateId::HereticHeadfx34);
	}
}

extern "C" void A_SnakeAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		P_SetMobjState(actor, StateId::HereticSnakeWalk1);
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MobjType::HereticSnakeproA);
}

extern "C" void A_SnakeAttack2(mobj_t* actor)
{
	if(!actor->target)
	{
		P_SetMobjState(actor, StateId::HereticSnakeWalk1);
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MobjType::HereticSnakeproB);
}

extern "C" void A_ClinkAttack(mobj_t* actor)
{
	int damage;

	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		damage = ((P_Random(RandomClass::Heretic) % 7) + 3);
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

extern "C" void A_GhostOff(mobj_t* actor)
{
	actor->flags -= MobjFlag::Shadow;
}

extern "C" void A_WizAtk1(mobj_t* actor)
{
	A_FaceTarget(actor);
	actor->flags -= MobjFlag::Shadow;
}

extern "C" void A_WizAtk2(mobj_t* actor)
{
	A_FaceTarget(actor);
	actor->flags |= MobjFlag::Shadow;
}

extern "C" void A_WizAtk3(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;
	fixed_t momz;

	actor->flags -= MobjFlag::Shadow;
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MobjType::HereticWizfx1);
	if(mo)
	{
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngle(actor, MobjType::HereticWizfx1, angle - (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, MobjType::HereticWizfx1, angle + (ANG45 / 8), momz);
	}
}

void P_DropItem(mobj_t* source, MobjType type, int special, int chance)
{
	mobj_t* mo;

	if(P_Random(RandomClass::Heretic) > chance)
	{
		return;
	}
	mo = P_SpawnMobj(source->x, source->y,
		source->z + (source->height >> 1), static_cast<MobjType>(type));
	mo->momx = P_SubRandom() << 8;
	mo->momy = P_SubRandom() << 8;
	mo->momz = FRACUNIT * 5 + (P_Random(RandomClass::Heretic) << 10);
	mo->flags |= MobjFlag::Dropped;
	mo->health = special;
}

extern "C" void A_NoBlocking(mobj_t* actor)
{
	actor->flags -= MobjFlag::Solid;

	if(hexen)
		return;

	// Check for monsters dropping things
	switch(actor->type)
	{
		case MobjType::HereticMummy:
		case MobjType::HereticMummyleader:
		case MobjType::HereticMummyghost:
		case MobjType::HereticMummyleaderghost:
			P_DropItem(actor, MobjType::HereticAmgwndwimpy, 3, 84);
			break;
		case MobjType::HereticKnight:
		case MobjType::HereticKnightghost:
			P_DropItem(actor, MobjType::HereticAmcbowwimpy, 5, 84);
			break;
		case MobjType::HereticWizard:
			P_DropItem(actor, MobjType::HereticAmblsrwimpy, 10, 84);
			P_DropItem(actor, MobjType::HereticArtitomeofpower, 0, 4);
			break;
		case MobjType::HereticHead:
			P_DropItem(actor, MobjType::HereticAmblsrwimpy, 10, 84);
			P_DropItem(actor, MobjType::HereticArtiegg, 0, 51);
			break;
		case MobjType::HereticBeast:
			P_DropItem(actor, MobjType::HereticAmcbowwimpy, 10, 84);
			break;
		case MobjType::HereticClink:
			P_DropItem(actor, MobjType::HereticAmskrdwimpy, 20, 84);
			break;
		case MobjType::HereticSnake:
			P_DropItem(actor, MobjType::HereticAmphrdwimpy, 5, 84);
			break;
		case MobjType::HereticMinotaur:
			P_DropItem(actor, MobjType::HereticArtisuperheal, 0, 51);
			P_DropItem(actor, MobjType::HereticAmphrdwimpy, 10, 84);
			break;
		default:
			break;
	}
}

extern "C" void A_PodPain(mobj_t* actor)
{
	int i;
	int count;
	int chance;
	mobj_t* goo;

	chance = P_Random(RandomClass::Heretic);
	if(chance < 128)
	{
		return;
	}
	count = chance > 240 ? 2 : 1;
	for(i = 0; i < count; i++)
	{
		goo = P_SpawnMobj(actor->x, actor->y,
			actor->z + 48 * FRACUNIT, MobjType::HereticPodgoo);
		P_SetTarget(&goo->target, actor);
		goo->momx = P_SubRandom() << 9;
		goo->momy = P_SubRandom() << 9;
		goo->momz = FRACUNIT / 2 + (P_Random(RandomClass::Heretic) << 9);
	}
}

extern "C" void A_RemovePod(mobj_t* actor)
{
	mobj_t* mo;

	if(actor->special2.m)
	{
		mo = (mobj_t*)actor->special2.m;
		if(mo->special1.i > 0)
		{
			mo->special1.i--;
		}
	}
}

#define MAX_GEN_PODS 16

extern "C" void A_MakePod(mobj_t* actor)
{
	mobj_t* mo;
	fixed_t x;
	fixed_t y;

	if(actor->special1.i == MAX_GEN_PODS)
	{
		// Too many generated pods
		return;
	}
	x = actor->x;
	y = actor->y;
	mo = P_SpawnMobj(x, y, ONFLOORZ, MobjType::HereticPod);
	if(P_CheckPosition(mo, x, y) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		return;
	}
	P_SetMobjState(mo, StateId::HereticPodGrow1);
	P_ThrustMobj(mo, P_Random(RandomClass::Heretic) << 24, (fixed_t)(4.5 * FRACUNIT));
	S_StartMobjSound(mo, SfxId::HereticNewpod);
	actor->special1.i++;                 // Increment generated pod count
	P_SetTarget(&mo->special2.m, actor); // Link the generator to the pod
	return;
}

extern "C" void A_ESound(mobj_t* mo)
{
	SfxId sound = SfxId::None;

	switch(mo->type)
	{
		case MobjType::HereticSoundwaterfall:
			sound = SfxId::HereticWaterfl;
			break;
		case MobjType::HereticSoundwind:
			sound = SfxId::HereticWind;
			break;
		case MobjType::HexenSoundwind:
			sound = SfxId::HexenWind;
			break;
		default:
			break;
	}
	S_StartMobjSound(mo, sound);
}

extern "C" void A_SpawnTeleGlitter(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_Random(RandomClass::Heretic);
	r2 = P_Random(RandomClass::Heretic);
	mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
		actor->y + ((r1 & 31) - 16) * FRACUNIT,
		actor->subsector->sector->floorheight, MobjType::HereticTeleglitter);
	mo->momz = FRACUNIT / 4;
}

extern "C" void A_SpawnTeleGlitter2(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_Random(RandomClass::Heretic);
	r2 = P_Random(RandomClass::Heretic);
	mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
		actor->y + ((r1 & 31) - 16) * FRACUNIT,
		actor->subsector->sector->floorheight, MobjType::HereticTeleglitter2);
	mo->momz = FRACUNIT / 4;
}

extern "C" void A_AccTeleGlitter(mobj_t* actor)
{
	if(++actor->health > TICRATE)
	{
		actor->momz += actor->momz / 2;
	}
}

extern "C" void A_InitKeyGizmo(mobj_t* gizmo)
{
	mobj_t* mo;
	StateId state = static_cast<StateId>(g_s_null);

	switch(gizmo->type)
	{
		case MobjType::HereticKeygizmoblue:
			state = StateId::HereticKgzBluefloat1;
			break;
		case MobjType::HereticKeygizmogreen:
			state = StateId::HereticKgzGreenfloat1;
			break;
		case MobjType::HereticKeygizmoyellow:
			state = StateId::HereticKgzYellowfloat1;
			break;
		default:
			break;
	}
	mo = P_SpawnMobj(gizmo->x, gizmo->y, gizmo->z + 60 * FRACUNIT,
		MobjType::HereticKeygizmofloat);
	P_SetMobjState(mo, static_cast<StateId>(state));
}

extern "C" void A_VolcanoSet(mobj_t* volcano)
{
	volcano->tics = 105 + (P_Random(RandomClass::Heretic) & 127);
}

extern "C" void A_VolcanoBlast(mobj_t* volcano)
{
	int i;
	int count;
	mobj_t* blast;
	angle_t angle;

	count = 1 + (P_Random(RandomClass::Heretic) % 3);
	for(i = 0; i < count; i++)
	{
		blast = P_SpawnMobj(volcano->x, volcano->y, volcano->z + 44 * FRACUNIT, MobjType::HereticVolcanoblast);
		P_SetTarget(&blast->target, volcano);
		angle = P_Random(RandomClass::Heretic) << 24;
		blast->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		blast->momx = FixedMul(1 * FRACUNIT, finecosine[angle]);
		blast->momy = FixedMul(1 * FRACUNIT, finesine[angle]);
		blast->momz = (fixed_t)(2.5 * FRACUNIT) + (P_Random(RandomClass::Heretic) << 10);
		S_StartMobjSound(blast, SfxId::HereticVolsht);
		P_CheckMissileSpawn(blast);
	}
}

extern "C" void A_VolcBallImpact(mobj_t* ball)
{
	unsigned int i;
	mobj_t* tiny;
	angle_t angle;

	if(ball->z <= ball->floorz)
	{
		ball->flags |= MobjFlag::NoGravity;
		ball->flags2 -= MobjFlag2::LoGrav;
		ball->z += 28 * FRACUNIT;
		//ball->momz = 3*FRACUNIT;
	}
	P_RadiusAttack(ball, ball->target, 25, 25, BF_DAMAGESOURCE);
	for(i = 0; i < 4; i++)
	{
		tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MobjType::HereticVolcanotblast);
		P_SetTarget(&tiny->target, ball);
		angle = i * ANG90;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = FixedMul((fixed_t)(FRACUNIT * .7), finecosine[angle]);
		tiny->momy = FixedMul((fixed_t)(FRACUNIT * .7), finesine[angle]);
		tiny->momz = FRACUNIT + (P_Random(RandomClass::Heretic) << 9);
		P_CheckMissileSpawn(tiny);
	}
}

extern "C" void A_CheckSkullFloor(mobj_t* actor)
{
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, static_cast<StateId>(g_s_bloodyskullx1));
		if(hexen)
			S_StartMobjSound(actor, SfxId::HexenDrip);
	}
}

extern "C" void A_CheckSkullDone(mobj_t* actor)
{
	if(actor->special2.i == 666)
	{
		P_SetMobjState(actor, static_cast<StateId>(g_s_bloodyskullx2));
	}
}

extern "C" void A_CheckBurnGone(mobj_t* actor)
{
	if(actor->special2.i == 666)
	{
		P_SetMobjState(actor, static_cast<StateId>(g_s_play_fdth20));
	}
}

extern "C" void A_FreeTargMobj(mobj_t* mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	mo->z = mo->ceilingz + 4 * FRACUNIT;
	mo->flags -= (MobjFlag::Shootable | MobjFlag::Float | MobjFlag::SkullFly | MobjFlag::Solid);
	mo->flags |= MobjFlag::Corpse | MobjFlag::DropOff | MobjFlag::NoGravity;
	mo->flags2 -= (MobjFlag2::PassMobj | MobjFlag2::LoGrav);
	mo->player = nullptr;

	// hexen_note: can we do this in heretic too?
	if(hexen)
	{
		mo->flags -= (MobjFlag::CountKill);
		mo->flags2 |= MobjFlag2::DontDraw;
		mo->health = -1000; // Don't resurrect
	}
}

static int bodyqueslot, bodyquesize;
static mobj_t** bodyque;

extern "C" void A_ResetPlayerCorpseQueue()
{
	bodyqueslot = 0;
	bodyquesize = dsda_IntConfig(ConfigId::MaxPlayerCorpse);
}

extern "C" void A_AddPlayerCorpse(mobj_t* actor)
{
	if(bodyquesize > 0)
	{
		static int queuesize;

		if(queuesize < bodyquesize)
		{
			bodyque = static_cast<mobj_t**>(Z_Realloc(bodyque, bodyquesize * sizeof(*bodyque)));
			memset(bodyque + queuesize, 0, (bodyquesize - queuesize) * sizeof(*bodyque));
			queuesize = bodyquesize;
		}

		if(bodyqueslot >= bodyquesize)
			P_RemoveMobj(bodyque[bodyqueslot % bodyquesize]);

		bodyque[bodyqueslot++ % bodyquesize] = actor;
	}
	else if(!bodyquesize)
		P_RemoveMobj(actor);
}

extern "C" void A_FlameSnd(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::HereticHedat1); // Burn sound
}

extern "C" void A_HideThing(mobj_t* actor)
{
	//P_UnsetThingPosition(actor);
	actor->flags2 |= MobjFlag2::DontDraw;
}

extern "C" void A_UnHideThing(mobj_t* actor)
{
	//P_SetThingPosition(actor);
	actor->flags2 -= MobjFlag2::DontDraw;
}

extern "C" void Heretic_A_Scream(mobj_t* actor)
{
	switch(actor->type)
	{
		case MobjType::HereticChicplayer:
		case MobjType::HereticSorcerer1:
		case MobjType::HereticMinotaur:
			// Make boss death sounds full volume
			S_StartVoidSound(actor->info->deathsound);
			break;
		case MobjType::HereticPlayer:
			// Handle the different player death screams
			if(actor->special1.i < 10)
			{
				// Wimpy death sound
				S_StartMobjSound(actor, SfxId::HereticPlrwdth);
			}
			else if(actor->health > -50)
			{
				// Normal death sound
				S_StartMobjSound(actor, actor->info->deathsound);
			}
			else if(actor->health > -100)
			{
				// Crazy death sound
				S_StartMobjSound(actor, SfxId::HereticPlrcdth);
			}
			else
			{
				// Extreme death sound
				S_StartMobjSound(actor, SfxId::HereticGibdth);
			}
			break;
		default:
			S_StartMobjSound(actor, actor->info->deathsound);
			break;
	}
}

void Heretic_A_BossDeath(mobj_t* actor)
{
	mobj_t* mo;
	thinker_t* think;
	line_t dummyLine;
	static MobjType bossType[6] = {
		MobjType::HereticHead,
		MobjType::HereticMinotaur,
		MobjType::HereticSorcerer2,
		MobjType::HereticHead,
		MobjType::HereticMinotaur,
		static_cast<MobjType>(-1)
	};

	if(gamemap != 8)
	{
		// Not a boss level
		return;
	}
	if(actor->type != bossType[gameepisode - 1])
	{
		// Not considered a boss in this episode
		return;
	}
	// Make sure all bosses are dead
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mo = (mobj_t*)think;
		if((mo != actor) && (mo->type == actor->type) && (mo->health > 0))
		{
			// Found a living boss
			return;
		}
	}
	if(gameepisode > 1)
	{
		// Kill any remaining monsters
		P_Massacre();
	}
	dummyLine.special_args[0] = 666;
	EV_DoFloor(&dummyLine, FloorKind::LowerFloor);
}

#define MONS_LOOK_LIMIT 64

// Not proxied by P_LookForMonsters - this is post-death brawling
dboolean Heretic_P_LookForMonsters(mobj_t* actor)
{
	int count;
	mobj_t* mo;
	thinker_t* think;

	if(!P_CheckSight(players[0].mo, actor))
	{
		// Player can't see monster
		return (false);
	}
	count = 0;
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mo = (mobj_t*)think;
		if((mo->flags & MobjFlag::CountKill) == MobjFlag{} || (mo == actor) || (mo->health <= 0))
		{
			// Not a valid monster
			continue;
		}
		if(P_AproxDistance(actor->x - mo->x, actor->y - mo->y) > g_mons_look_range)
		{
			// Out of range
			continue;
		}
		if(P_Random(RandomClass::Heretic) < 16)
		{
			// Skip
			continue;
		}
		if(count++ > MONS_LOOK_LIMIT)
		{
			// Stop searching
			return (false);
		}
		if(!P_CheckSight(actor, mo))
		{
			// Out of sight
			continue;
		}
		if(actor->type == MobjType::HexenMinotaur)
		{
			// hexen_note: this is supposed to be mo->target != actor->special1.p->mo
			// - hexen never actually set p (player_t *), so it's m (mobj_t *) (union)
			// - mo is the first entry in player_t
			// - thinker_t is the first entry in mobj_t, whose first entry is the previous thinker
			// - this resolves to actor->special1.m->thinker.prev
			if((mo->type == MobjType::HexenMinotaur) &&
				(mo->target != (mobj_t*)actor->special1.m->thinker.prev))
			{
				continue;
			}
		}
		// Found a target monster
		P_SetTarget(&actor->target, mo);
		return (true);
	}
	return (false);
}

dboolean Raven_P_LookForPlayers(mobj_t* actor, dboolean allaround)
{
	int c;
	int stop;
	player_t* player;
	angle_t an;
	fixed_t dist;

	if(!netgame && players[0].health <= 0)
	{
		// Single player game and player is dead, look for monsters
		return (Heretic_P_LookForMonsters(actor));
	}
	c = 0;
	stop = (actor->lastlook - 1) & 3;
	for(;; actor->lastlook = (actor->lastlook + 1) & 3)
	{
		if(!playeringame[actor->lastlook])
			continue;

		if(c++ == 2 || actor->lastlook == stop)
			return false; // done looking

		player = &players[actor->lastlook];

		if((players->cheats & CheatFlag::NoTarget) != CheatFlag{})
			continue; // no target

		if(player->health <= 0)
			continue; // dead
		if(!P_CheckSight(actor, player->mo))
			continue; // out of sight

		if(!allaround)
		{
			an = R_PointToAngle2(actor->x, actor->y,
				player->mo->x, player->mo->y) - actor->angle;
			if(an > ANG90 && an < ANG270)
			{
				dist = P_AproxDistance(player->mo->x - actor->x,
					player->mo->y - actor->y);
				// if real close, react anyway
				if(dist > WAKEUPRANGE)
					continue; // behind back
			}
		}
		if((player->mo->flags & MobjFlag::Shadow) != MobjFlag{})
		{
			// Player is invisible
			if((P_AproxDistance(player->mo->x - actor->x,
					player->mo->y - actor->y) > SNEAKRANGE)
				&& P_AproxDistance(player->mo->momx, player->mo->momy)
				< 5 * FRACUNIT)
			{
				// Player is sneaking - can't detect
				return (false);
			}
			if(P_Random(RandomClass::Heretic) < 225)
			{
				// Player isn't sneaking, but still didn't detect
				return (false);
			}
		}
		if(actor->type == MobjType::HexenMinotaur)
		{
			// hexen_note: minotaur was intended to store the player_t* but doesn't
			if(actor->special1.m == (mobj_t*)player)
			{
				continue; // Don't target master
			}
		}

		P_SetTarget(&actor->target, player->mo);
		return (true);
	}
	return (false);
}

// hexen

#include "hexen/a_action.hpp"
#include "hexen/p_acs.hpp"

extern fixed_t FloatBobOffsets[64];

// Corpse queue for monsters - this should be saved out
#define CORPSEQUEUESIZE	64
mobj_t* corpseQueue[CORPSEQUEUESIZE];
int corpseQueueSlot;

// throw another corpse on the queue
extern "C" void A_QueueCorpse(mobj_t* actor)
{
	mobj_t* corpse;

	if(corpseQueueSlot >= CORPSEQUEUESIZE)
	{
		// Too many corpses - remove an old one
		corpse = corpseQueue[corpseQueueSlot % CORPSEQUEUESIZE];
		if(corpse)
			P_RemoveMobj(corpse);
	}
	corpseQueue[corpseQueueSlot % CORPSEQUEUESIZE] = actor;
	corpseQueueSlot++;
}

// Remove a mobj from the queue (for resurrection)
extern "C" void A_DeQueueCorpse(mobj_t* actor)
{
	int slot;

	for(slot = 0; slot < CORPSEQUEUESIZE; slot++)
	{
		if(corpseQueue[slot] == actor)
		{
			corpseQueue[slot] = nullptr;
			break;
		}
	}
}

void P_InitCreatureCorpseQueue(dboolean corpseScan)
{
	thinker_t* think;
	mobj_t* mo;

	// Initialize queue
	corpseQueueSlot = 0;
	memset(corpseQueue, 0, sizeof(mobj_t*) * CORPSEQUEUESIZE);

	if(!corpseScan)
		return;

	// Search mobj list for corpses and place them in this queue
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
			continue;
		mo = (mobj_t*)think;
		if((mo->flags & MobjFlag::Corpse) == MobjFlag{})
			continue; // Must be a corpse
		if((mo->flags & MobjFlag::IceCorpse) != MobjFlag{})
			continue; // Not ice corpses
		// Only corpses that call A_QueueCorpse from death routine
		switch(mo->type)
		{
			case MobjType::HexenCentaur:
			case MobjType::HexenCentaurleader:
			case MobjType::HexenDemon:
			case MobjType::HexenDemon2:
			case MobjType::HexenWraith:
			case MobjType::HexenWraithb:
			case MobjType::HexenBishop:
			case MobjType::HexenEttin:
			case MobjType::HexenPig:
			case MobjType::HexenCentaurShield:
			case MobjType::HexenCentaurSword:
			case MobjType::HexenDemonchunk1:
			case MobjType::HexenDemonchunk2:
			case MobjType::HexenDemonchunk3:
			case MobjType::HexenDemonchunk4:
			case MobjType::HexenDemonchunk5:
			case MobjType::HexenDemon2chunk1:
			case MobjType::HexenDemon2chunk2:
			case MobjType::HexenDemon2chunk3:
			case MobjType::HexenDemon2chunk4:
			case MobjType::HexenDemon2chunk5:
			case MobjType::HexenFiredemonSplotch1:
			case MobjType::HexenFiredemonSplotch2:
				A_QueueCorpse(mo); // Add corpse to queue
				break;
			default:
				break;
		}
	}
}

dboolean P_CheckMeleeRange2(mobj_t* actor)
{
	mobj_t* mo;
	fixed_t dist;

	if(!actor->target)
	{
		return (false);
	}
	mo = actor->target;
	dist = P_AproxDistance(mo->x - actor->x, mo->y - actor->y);
	if(dist >= MELEERANGE * 2 || dist < MELEERANGE)
	{
		return (false);
	}
	if(!P_CheckSight(actor, mo))
	{
		return (false);
	}
	if(mo->z > actor->z + actor->height)
	{
		// Target is higher than the attacker
		return (false);
	}
	else if(actor->z > mo->z + mo->height)
	{
		// Attacker is higher
		return (false);
	}
	return (true);
}

extern "C" void A_SetInvulnerable(mobj_t* actor)
{
	actor->flags2 |= MobjFlag2::Invulnerable;
}

extern "C" void A_UnSetInvulnerable(mobj_t* actor)
{
	actor->flags2 -= MobjFlag2::Invulnerable;
}

extern "C" void A_SetReflective(mobj_t* actor)
{
	actor->flags2 |= MobjFlag2::Reflective;

	if((actor->type == MobjType::HexenCentaur) || (actor->type == MobjType::HexenCentaurleader))
	{
		A_SetInvulnerable(actor);
	}
}

extern "C" void A_UnSetReflective(mobj_t* actor)
{
	actor->flags2 -= MobjFlag2::Reflective;

	if((actor->type == MobjType::HexenCentaur) || (actor->type == MobjType::HexenCentaurleader))
	{
		A_UnSetInvulnerable(actor);
	}
}

dboolean P_UpdateMorphedMonster(mobj_t* actor, int tics)
{
	mobj_t* fog;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	MobjType moType;
	mobj_t* mo;
	mobj_t oldMonster;

	actor->special1.i -= tics;
	if(actor->special1.i > 0)
	{
		return (false);
	}
	moType = static_cast<MobjType>(actor->special2.i);
	switch(moType)
	{
		case MobjType::HexenWraithb: // These must remain morphed
		case MobjType::HexenSerpent:
		case MobjType::HexenSerpentleader:
		case MobjType::HexenMinotaur:
			return (false);
		default:
			break;
	}
	x = actor->x;
	y = actor->y;
	z = actor->z;
	oldMonster = *actor; // Save pig vars

	map_format.remove_mobj_thing_id(actor);
	P_SetMobjState(actor, StateId::HexenFreetargmobj);
	mo = P_SpawnMobj(x, y, z, moType);
	dsda_WatchUnMorph(mo);
	if(P_TestMobjLocation(mo) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(oldMonster.type));
		mo->angle = oldMonster.angle;
		mo->flags = oldMonster.flags;
		mo->health = oldMonster.health;
		P_SetTarget(&mo->target, oldMonster.target);
		mo->special = oldMonster.special;
		mo->special1.i = 5 * TICRATE; // Next try in 5 seconds
		mo->special2.i = std::to_underlying(moType);
		mo->tid = oldMonster.tid;
		memcpy(mo->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
		map_format.add_mobj_thing_id(mo, oldMonster.tid);
		dsda_WatchMorph(mo);
		return (false);
	}
	mo->angle = oldMonster.angle;
	P_SetTarget(&mo->target, oldMonster.target);
	mo->tid = oldMonster.tid;
	mo->special = oldMonster.special;
	memcpy(mo->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
	map_format.add_mobj_thing_id(mo, oldMonster.tid);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HexenTfog);
	S_StartMobjSound(fog, SfxId::HexenTeleport);
	return (true);
}

extern "C" void A_PigLook(mobj_t* actor)
{
	if(P_UpdateMorphedMonster(actor, 10))
	{
		return;
	}
	A_Look(actor);
}

extern "C" void A_PigChase(mobj_t* actor)
{
	if(P_UpdateMorphedMonster(actor, 3))
	{
		return;
	}
	A_Chase(actor);
}

extern "C" void A_PigAttack(mobj_t* actor)
{
	if(P_UpdateMorphedMonster(actor, 18))
	{
		return;
	}
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 2 + (P_Random(RandomClass::Hexen) & 1));
		S_StartMobjSound(actor, SfxId::HexenPigAttack);
	}
}

extern "C" void A_PigPain(mobj_t* actor)
{
	A_Pain(actor);
	if(actor->z <= actor->floorz)
	{
		actor->momz = 3.5 * FRACUNIT;
	}
}

void FaceMovementDirection(mobj_t* actor)
{
	switch(static_cast<DirType>(actor->movedir))
	{
		case DirType::East:
			actor->angle = 0 << 24;
			break;
		case DirType::NorthEast:
			actor->angle = 32 << 24;
			break;
		case DirType::North:
			actor->angle = 64 << 24;
			break;
		case DirType::NorthWest:
			actor->angle = 96 << 24;
			break;
		case DirType::West:
			actor->angle = 128 << 24;
			break;
		case DirType::SouthWest:
			actor->angle = 160 << 24;
			break;
		case DirType::South:
			actor->angle = 192 << 24;
			break;
		case DirType::SouthEast:
			actor->angle = 224 << 24;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
//
// Minotaur variables
//
//      special1                pointer to player that spawned it (mobj_t)
//      special2                internal to minotaur AI
//      args[0]                 args[0]-args[3] together make up minotaur start time
//      args[1]                 |
//      args[2]                 |
//      args[3]                 V
//      args[4]                 charge duration countdown
//----------------------------------------------------------------------------

extern "C" void A_MinotaurFade0(mobj_t* actor)
{
	actor->flags -= MobjFlag::AltShadow;
	actor->flags |= MobjFlag::Shadow;
}

extern "C" void A_MinotaurFade1(mobj_t* actor)
{
	// Second level of transparency
	actor->flags -= MobjFlag::Shadow;
	actor->flags |= MobjFlag::AltShadow;
}

extern "C" void A_MinotaurFade2(mobj_t* actor)
{
	// Make fully visible
	actor->flags -= MobjFlag::Shadow;
	actor->flags -= MobjFlag::AltShadow;
}

extern "C" void A_MinotaurLook(mobj_t* actor);

// Check the age of the minotaur and stomp it after MAULATORTICS of time
// have passed. Returns false if killed.
static dboolean CheckMinotaurAge(mobj_t* mo)
{
	byte args[5];
	unsigned int starttime;

	COLLAPSE_SPECIAL_ARGS(args, mo->special_args);
	memcpy(&starttime, args, sizeof(unsigned int));
	if(leveltime - LittleLong(starttime) >= std::to_underlying(PowerDuration::Maulatortics))
	{
		P_DamageMobj(mo, nullptr, nullptr, 10000);
		return false;
	}

	return true;
}

extern "C" void A_MinotaurRoam(mobj_t* actor)
{
	actor->flags -= MobjFlag::Shadow;    // In case pain caused him to
	actor->flags -= MobjFlag::AltShadow; // skip his fade in.

	if(!CheckMinotaurAge(actor))
	{
		return;
	}

	if(P_Random(RandomClass::Hexen) < 30)
		A_MinotaurLook(actor); // adjust to closest target

	if(P_Random(RandomClass::Hexen) < 6)
	{
		//Choose new direction
		actor->movedir = P_Random(RandomClass::Hexen) % 8;
		FaceMovementDirection(actor);
	}
	if(!P_Move(actor, false))
	{
		// Turn
		if(P_Random(RandomClass::Hexen) & 1)
			actor->movedir = (actor->movedir + 1) % 8;
		else
			actor->movedir = (actor->movedir + 7) % 8;
		FaceMovementDirection(actor);
	}
}

#define MINOTAUR_LOOK_DIST		(16*54*FRACUNIT)

extern "C" void A_MinotaurLook(mobj_t* actor)
{
	mobj_t* mo = nullptr;
	player_t* player;
	thinker_t* think;
	fixed_t dist;
	int i;
	mobj_t* master = actor->special1.m;

	P_SetTarget(&actor->target, nullptr);
	if(deathmatch) // Quick search for players
	{
		for(i = 0; i < g_maxplayers; i++)
		{
			if(!playeringame[i])
				continue;
			player = &players[i];
			mo = player->mo;
			if(mo == master)
				continue;
			if(mo->health <= 0)
				continue;
			dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
			if(dist > MINOTAUR_LOOK_DIST)
				continue;
			P_SetTarget(&actor->target, mo);
			break;
		}
	}

	if(!actor->target) // Near player monster search
	{
		if(master && (master->health > 0) && (master->player))
			mo = P_RoughTargetSearch(master, 0, 20);
		else
			mo = P_RoughTargetSearch(actor, 0, 20);
		P_SetTarget(&actor->target, mo);
	}

	if(!actor->target) // Normal monster search
	{
		for(think = thinkercap.next; think != &thinkercap;
			think = think->next)
		{
			if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
				continue;
			mo = (mobj_t*)think;
			if((mo->flags & MobjFlag::CountKill) == MobjFlag{})
				continue;
			if(mo->health <= 0)
				continue;
			if((mo->flags & MobjFlag::Shootable) == MobjFlag{})
				continue;
			dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
			if(dist > MINOTAUR_LOOK_DIST)
				continue;
			if((mo == master) || (mo == actor))
				continue;
			if((mo->type == MobjType::HexenMinotaur) &&
				(mo->special1.m == actor->special1.m))
				continue;
			P_SetTarget(&actor->target, mo);
			break; // Found mobj to attack
		}
	}

	if(actor->target)
	{
		P_SetMobjStateNF(actor, StateId::HexenMntrWalk1);
	}
	else
	{
		P_SetMobjStateNF(actor, StateId::HexenMntrRoam1);
	}
}

extern "C" void A_MinotaurChase(mobj_t* actor)
{
	actor->flags -= MobjFlag::Shadow;    // In case pain caused him to
	actor->flags -= MobjFlag::AltShadow; // skip his fade in.

	if(!CheckMinotaurAge(actor))
	{
		return;
	}

	if(P_Random(RandomClass::Hexen) < 30)
		A_MinotaurLook(actor); // adjust to closest target

	if(!actor->target || (actor->target->health <= 0) ||
		(actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// look for a new target
		P_SetMobjState(actor, StateId::HexenMntrLook1);
		return;
	}

	FaceMovementDirection(actor);
	actor->reactiontime = 0;

	// Melee attack
	if(actor->info->meleestate != StateId::Null && P_CheckMeleeRange(actor))
	{
		if(actor->info->attacksound != SfxId::None)
		{
			S_StartMobjSound(actor, actor->info->attacksound);
		}
		P_SetMobjState(actor, actor->info->meleestate);
		return;
	}

	// Missile attack
	if(actor->info->missilestate != StateId::Null && P_CheckMissileRange(actor))
	{
		P_SetMobjState(actor, actor->info->missilestate);
		return;
	}

	// chase towards target
	if(!P_Move(actor, false))
	{
		P_NewChaseDir(actor);
	}

	// Active sound
	if(actor->info->activesound != SfxId::None && P_Random(RandomClass::Hexen) < 6)
	{
		S_StartMobjSound(actor, actor->info->activesound);
	}
}

extern "C" void Hexen_A_Scream(mobj_t* actor)
{
	SfxId sound;

	S_StopSound(actor);
	if(actor->player)
	{
		if(actor->player->morphTics)
		{
			S_StartMobjSound(actor, actor->info->deathsound);
		}
		else
		{
			// Handle the different player death screams
			if(actor->momz <= -39 * FRACUNIT)
			{
				// Falling splat
				sound = SfxId::HexenPlayerFallingSplat;
			}
			else if(actor->health > -50)
			{
				// Normal death sound
				switch(actor->player->pclass)
				{
					case PClass::Fighter:
						sound = SfxId::HexenPlayerFighterNormalDeath;
						break;
					case PClass::Cleric:
						sound = SfxId::HexenPlayerClericNormalDeath;
						break;
					case PClass::Mage:
						sound = SfxId::HexenPlayerMageNormalDeath;
						break;
					default:
						sound = SfxId::None;
						break;
				}
			}
			else if(actor->health > -100)
			{
				// Crazy death sound
				switch(actor->player->pclass)
				{
					case PClass::Fighter:
						sound = SfxId::HexenPlayerFighterCrazyDeath;
						break;
					case PClass::Cleric:
						sound = SfxId::HexenPlayerClericCrazyDeath;
						break;
					case PClass::Mage:
						sound = SfxId::HexenPlayerMageCrazyDeath;
						break;
					default:
						sound = SfxId::None;
						break;
				}
			}
			else
			{
				// Extreme death sound
				switch(actor->player->pclass)
				{
					case PClass::Fighter:
						sound = SfxId::HexenPlayerFighterExtreme1Death;
						break;
					case PClass::Cleric:
						sound = SfxId::HexenPlayerClericExtreme1Death;
						break;
					case PClass::Mage:
						sound = SfxId::HexenPlayerMageExtreme1Death;
						break;
					default:
						sound = SfxId::None;
						break;
				}
				sound = SfxVariant(sound, P_Random(RandomClass::Hexen) % 3); // Three different extreme deaths
			}
			S_StartMobjSound(actor, sound);
		}
	}
	else
	{
		S_StartMobjSound(actor, actor->info->deathsound);
	}
}

extern "C" void A_SerpentUnHide(mobj_t* actor)
{
	actor->flags2 -= MobjFlag2::DontDraw;
	actor->floorclip = 24 * FRACUNIT;
}

extern "C" void A_SerpentHide(mobj_t* actor)
{
	actor->flags2 |= MobjFlag2::DontDraw;
	actor->floorclip = 0;
}

extern "C" void A_SerpentChase(mobj_t* actor)
{
	int delta;
	int oldX, oldY, oldFloor;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if((skill_info.flags & SkillFlag::FastMonsters) != SkillFlag{})
	{
		// Monsters move faster in nightmare mode
		actor->tics -= actor->tics / 2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

	//
	// turn towards movement direction if not there yet
	//
	if(actor->movedir < 8)
	{
		actor->angle &= (7 << 29);
		delta = actor->angle - (actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90 / 2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90 / 2;
		}
	}

	if(!actor->target || (actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// look for a new target
		if(P_LookForPlayers(actor, true))
		{
			// got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	//
	// don't attack twice in a row
	//
	if((actor->flags & MobjFlag::JustAttacked) != MobjFlag{})
	{
		actor->flags -= MobjFlag::JustAttacked;
		if((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{})
			P_NewChaseDir(actor);
		return;
	}

	//
	// check for melee attack
	//
	if(actor->info->meleestate != StateId::Null && P_CheckMeleeRange(actor))
	{
		if(actor->info->attacksound != SfxId::None)
		{
			S_StartMobjSound(actor, actor->info->attacksound);
		}
		P_SetMobjState(actor, actor->info->meleestate);
		return;
	}

	//
	// possibly choose another target
	//
	if(netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
	{
		if(P_LookForPlayers(actor, true))
			return; // got a new target
	}

	//
	// chase towards player
	//
	oldX = actor->x;
	oldY = actor->y;
	oldFloor = actor->subsector->sector->floorpic;
	if(--actor->movecount < 0 || !P_Move(actor, false))
	{
		P_NewChaseDir(actor);
	}
	if(actor->subsector->sector->floorpic != oldFloor)
	{
		P_TryMove(actor, oldX, oldY, 0);
		P_NewChaseDir(actor);
	}

	//
	// make active sound
	//
	if(actor->info->activesound != SfxId::None && P_Random(RandomClass::Hexen) < 3)
	{
		S_StartMobjSound(actor, actor->info->activesound);
	}
}

extern "C" void A_SerpentRaiseHump(mobj_t* actor)
{
	actor->floorclip -= 4 * FRACUNIT;
}

extern "C" void A_SerpentLowerHump(mobj_t* actor)
{
	actor->floorclip += 4 * FRACUNIT;
}

extern "C" void A_SerpentHumpDecide(mobj_t* actor)
{
	if(actor->type == MobjType::HexenSerpentleader)
	{
		if(P_Random(RandomClass::Hexen) > 30)
		{
			return;
		}
		else if(P_Random(RandomClass::Hexen) < 40)
		{
			// Missile attack
			P_SetMobjState(actor, StateId::HexenSerpentSurface1);
			return;
		}
	}
	else if(P_Random(RandomClass::Hexen) > 3)
	{
		return;
	}
	if(!P_CheckMeleeRange(actor))
	{
		// The hump shouldn't occur when within melee range
		if(actor->type == MobjType::HexenSerpentleader && P_Random(RandomClass::Hexen) < 128)
		{
			P_SetMobjState(actor, StateId::HexenSerpentSurface1);
		}
		else
		{
			P_SetMobjState(actor, StateId::HexenSerpentHump1);
			S_StartMobjSound(actor, SfxId::HexenSerpentActive);
		}
	}
}

extern "C" void A_SerpentBirthScream(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::HexenSerpentBirth);
}

extern "C" void A_SerpentDiveSound(mobj_t* actor)
{
	S_StartMobjSound(actor, SfxId::HexenSerpentActive);
}

extern "C" void A_SerpentWalk(mobj_t* actor)
{
	int delta;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if((skill_info.flags & SkillFlag::FastMonsters) != SkillFlag{})
	{
		// Monsters move faster in nightmare mode
		actor->tics -= actor->tics / 2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

	//
	// turn towards movement direction if not there yet
	//
	if(actor->movedir < 8)
	{
		actor->angle &= (7 << 29);
		delta = actor->angle - (actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90 / 2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90 / 2;
		}
	}

	if(!actor->target || (actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// look for a new target
		if(P_LookForPlayers(actor, true))
		{
			// got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	//
	// don't attack twice in a row
	//
	if((actor->flags & MobjFlag::JustAttacked) != MobjFlag{})
	{
		actor->flags -= MobjFlag::JustAttacked;
		if((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{})
			P_NewChaseDir(actor);
		return;
	}

	//
	// check for melee attack
	//
	if(actor->info->meleestate != StateId::Null && P_CheckMeleeRange(actor))
	{
		if(actor->info->attacksound != SfxId::None)
		{
			S_StartMobjSound(actor, actor->info->attacksound);
		}
		P_SetMobjState(actor, StateId::HexenSerpentAtk1);
		return;
	}
	//
	// possibly choose another target
	//
	if(netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
	{
		if(P_LookForPlayers(actor, true))
			return; // got a new target
	}

	//
	// chase towards player
	//
	if(--actor->movecount < 0 || !P_Move(actor, false))
	{
		P_NewChaseDir(actor);
	}
}

extern "C" void A_SerpentCheckForAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	if(actor->type == MobjType::HexenSerpentleader)
	{
		if(!P_CheckMeleeRange(actor))
		{
			P_SetMobjState(actor, StateId::HexenSerpentAtk1);
			return;
		}
	}
	if(P_CheckMeleeRange2(actor))
	{
		P_SetMobjState(actor, StateId::HexenSerpentWalk1);
	}
	else if(P_CheckMeleeRange(actor))
	{
		if(P_Random(RandomClass::Hexen) < 32)
		{
			P_SetMobjState(actor, StateId::HexenSerpentWalk1);
		}
		else
		{
			P_SetMobjState(actor, StateId::HexenSerpentAtk1);
		}
	}
}

extern "C" void A_SerpentChooseAttack(mobj_t* actor)
{
	if(!actor->target || P_CheckMeleeRange(actor))
	{
		return;
	}
	if(actor->type == MobjType::HexenSerpentleader)
	{
		P_SetMobjState(actor, StateId::HexenSerpentMissile1);
	}
}

extern "C" void A_SerpentMeleeAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(5));
		S_StartMobjSound(actor, SfxId::HexenSerpentMeleehit);
	}
	if(P_Random(RandomClass::Hexen) < 96)
	{
		A_SerpentCheckForAttack(actor);
	}
}

extern "C" void A_SerpentMissileAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}

	P_SpawnMissile(actor, actor->target, MobjType::HexenSerpentfx);
}

extern "C" void A_SerpentHeadPop(mobj_t* actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenSerpentHead);
}

extern "C" void A_SerpentSpawnGibs(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_Random(RandomClass::Hexen);
	r2 = P_Random(RandomClass::Hexen);
	mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
		actor->y + ((r1 - 128) << 12),
		actor->floorz + FRACUNIT, MobjType::HexenSerpentGib1);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->floorclip = 6 * FRACUNIT;
	}
	r1 = P_Random(RandomClass::Hexen);
	r2 = P_Random(RandomClass::Hexen);
	mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
		actor->y + ((r1 - 128) << 12),
		actor->floorz + FRACUNIT, MobjType::HexenSerpentGib2);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->floorclip = 6 * FRACUNIT;
	}
	r1 = P_Random(RandomClass::Hexen);
	r2 = P_Random(RandomClass::Hexen);
	mo = P_SpawnMobj(actor->x + ((r2 - 128) << 12),
		actor->y + ((r1 - 128) << 12),
		actor->floorz + FRACUNIT, MobjType::HexenSerpentGib3);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 6;
		mo->floorclip = 6 * FRACUNIT;
	}
}

extern "C" void A_FloatGib(mobj_t* actor)
{
	actor->floorclip -= FRACUNIT;
}

extern "C" void A_SinkGib(mobj_t* actor)
{
	actor->floorclip += FRACUNIT;
}

extern "C" void A_DelayGib(mobj_t* actor)
{
	actor->tics -= P_Random(RandomClass::Hexen) >> 2;
}

extern "C" void A_SerpentHeadCheck(mobj_t* actor)
{
	if(actor->z <= actor->floorz)
	{
		if(P_GetThingFloorType(actor) >= FloorType::Liquid)
		{
			P_HitFloor(actor);
			P_SetMobjState(actor, StateId::HexenNull);
		}
		else
		{
			P_SetMobjState(actor, StateId::HexenSerpentHeadX1);
		}
	}
}

extern "C" void A_CentaurAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, P_Random(RandomClass::Hexen) % 7 + 3);
	}
}

extern "C" void A_CentaurAttack2(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	P_SpawnMissile(actor, actor->target, MobjType::HexenCentaurFx);
	S_StartMobjSound(actor, SfxId::HexenCentaurleaderAttack);
}

extern "C" void A_CentaurDropStuff(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenCentaurShield);
	if(mo)
	{
		angle = actor->angle + ANG90;
		mo->momz = FRACUNIT * 8 + (P_Random(RandomClass::Hexen) << 10);
		mo->momx = FixedMul(((P_Random(RandomClass::Hexen) - 128) << 11) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul(((P_Random(RandomClass::Hexen) - 128) << 11) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenCentaurSword);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = FRACUNIT * 8 + (P_Random(RandomClass::Hexen) << 10);
		mo->momx = FixedMul(((P_Random(RandomClass::Hexen) - 128) << 11) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul(((P_Random(RandomClass::Hexen) - 128) << 11) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
}

extern "C" void A_CentaurDefend(mobj_t* actor)
{
	A_FaceTarget(actor);
	if(P_CheckMeleeRange(actor) && P_Random(RandomClass::Hexen) < 32)
	{
		A_UnSetInvulnerable(actor);
		P_SetMobjState(actor, actor->info->meleestate);
	}
}

extern "C" void A_BishopAttack(mobj_t* actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartMobjSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		return;
	}
	actor->special1.i = (P_Random(RandomClass::Hexen) & 3) + 5;
}

extern "C" void A_BishopAttack2(mobj_t* actor)
{
	mobj_t* mo;

	if(!actor->target || !actor->special1.i)
	{
		actor->special1.i = 0;
		P_SetMobjState(actor, StateId::HexenBishopWalk1);
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MobjType::HexenBishFx);
	if(mo)
	{
		P_SetTarget(&mo->special1.m, actor->target);
		mo->special2.i = 16; // High word == x/y, Low word == z
	}
	actor->special1.i--;
}

extern "C" void A_BishopMissileWeave(mobj_t* actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2.i >> 16;
	weaveZ = actor->special2.i & 0xFFFF;
	angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
	newX = actor->x - FixedMul(finecosine[angle],
		FloatBobOffsets[weaveXY] << 1);
	newY = actor->y - FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY] << 1);
	weaveXY = (weaveXY + 2) & 63;
	newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 1);
	newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 1);
	P_TryMove(actor, newX, newY, false);
	actor->z -= FloatBobOffsets[weaveZ];
	weaveZ = (weaveZ + 2) & 63;
	actor->z += FloatBobOffsets[weaveZ];
	actor->special2.i = weaveZ + (weaveXY << 16);
}

extern "C" void A_BishopMissileSeek(mobj_t* actor)
{
	P_SeekerMissile(actor, &actor->special1.m, ANG1 * 2, ANG1 * 3, false);
}

extern "C" void A_BishopDecide(mobj_t* actor)
{
	if(P_Random(RandomClass::Hexen) < 220)
	{
		return;
	}
	else
	{
		P_SetMobjState(actor, StateId::HexenBishopBlur1);
	}
}

extern "C" void A_BishopDoBlur(mobj_t* actor)
{
	actor->special1.i = (P_Random(RandomClass::Hexen) & 3) + 3; // Random number of blurs
	if(P_Random(RandomClass::Hexen) < 120)
	{
		P_ThrustMobj(actor, actor->angle + ANG90, 11 * FRACUNIT);
	}
	else if(P_Random(RandomClass::Hexen) > 125)
	{
		P_ThrustMobj(actor, actor->angle - ANG90, 11 * FRACUNIT);
	}
	else
	{
		// Thrust forward
		P_ThrustMobj(actor, actor->angle, 11 * FRACUNIT);
	}
	S_StartMobjSound(actor, SfxId::HexenBishopBlur);
}

extern "C" void A_BishopSpawnBlur(mobj_t* actor)
{
	mobj_t* mo;

	if(!--actor->special1.i)
	{
		actor->momx = 0;
		actor->momy = 0;
		if(P_Random(RandomClass::Hexen) > 96)
		{
			P_SetMobjState(actor, StateId::HexenBishopWalk1);
		}
		else
		{
			P_SetMobjState(actor, StateId::HexenBishopAtk1);
		}
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenBishopblur);
	if(mo)
	{
		mo->angle = actor->angle;
	}
}

extern "C" void A_BishopChase(mobj_t* actor)
{
	actor->z -= FloatBobOffsets[actor->special2.i] >> 1;
	actor->special2.i = (actor->special2.i + 4) & 63;
	actor->z += FloatBobOffsets[actor->special2.i] >> 1;
}

extern "C" void A_BishopPuff(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 40 * FRACUNIT,
		MobjType::HexenBishopPuff);
	if(mo)
	{
		mo->momz = FRACUNIT / 2;
	}
}

extern "C" void A_BishopPainBlur(mobj_t* actor)
{
	mobj_t* mo;
	int r1, r2, r3;

	if(P_Random(RandomClass::Hexen) < 64)
	{
		P_SetMobjState(actor, StateId::HexenBishopBlur1);
		return;
	}

	r1 = P_SubRandom();
	r2 = P_SubRandom();
	r3 = P_SubRandom();

	mo = P_SpawnMobj(actor->x + (r3 << 12), actor->y
		+ (r2 << 12),
		actor->z + (r1 << 11),
		MobjType::HexenBishoppainblur);
	if(mo)
	{
		mo->angle = actor->angle;
	}
}

static void DragonSeek(mobj_t* actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t* target;
	int search;
	int i;
	int bestArg;
	angle_t bestAngle;
	angle_t angleToSpot, angleToTarget;
	mobj_t* mo;

	target = actor->special1.m;
	if(target == nullptr)
	{
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{
		// Turn clockwise
		actor->angle += delta;
	}
	else
	{
		// Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);
	if(actor->z + actor->height < target->z
		|| target->z + target->height < actor->z)
	{
		dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
		dist = dist / actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = (target->z - actor->z) / dist;
	}
	else
	{
		dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
		dist = dist / actor->info->speed;
	}
	if((target->flags & MobjFlag::Shootable) != MobjFlag{} && P_Random(RandomClass::Hexen) < 64)
	{
		// attack the destination mobj if it's attackable
		mobj_t* oldTarget;

		if(AngleAbs(AngleDifference(actor->angle,
			R_PointToAngle2(actor->x, actor->y, target->x, target->y))) < ANG45 / 2)
		{
			oldTarget = actor->target;
			actor->target = target;
			if(P_CheckMeleeRange(actor))
			{
				P_DamageMobj(actor->target, actor, actor, HITDICE(10));
				S_StartMobjSound(actor, SfxId::HexenDragonAttack);
			}
			else if(P_Random(RandomClass::Hexen) < 128 && P_CheckMissileRange(actor))
			{
				P_SpawnMissile(actor, target, MobjType::HexenDragonFx);
				S_StartMobjSound(actor, SfxId::HexenDragonAttack);
			}
			actor->target = oldTarget;
		}
	}
	if(dist < 4)
	{
		// Hit the target thing
		if(actor->target && P_Random(RandomClass::Hexen) < 200)
		{
			bestArg = -1;
			bestAngle = ANGLE_MAX;
			angleToTarget = R_PointToAngle2(actor->x, actor->y,
				actor->target->x,
				actor->target->y);
			for(i = 0; i < 5; i++)
			{
				int mo_x, mo_y;
				if(!target->special_args[i])
				{
					continue;
				}
				search = -1;
				mo = P_FindMobjFromTID(target->special_args[i], &search);
				// [crispy] fix wyvern + porkalator bug
				if(mo == nullptr)
				{
					Log::Warn("DragonSeek: P_FindMobjFromTID() returned NULL mobj!\n");
					mo_x = 0;
					mo_y = 0;
				}
				else
				{
					mo_x = mo->x;
					mo_y = mo->y;
				}
				angleToSpot = R_PointToAngle2(actor->x, actor->y,
					mo_x, mo_y);
				if(AngleAbs(AngleDifference(angleToSpot, angleToTarget)) < bestAngle)
				{
					bestAngle = AngleAbs(AngleDifference(angleToSpot, angleToTarget));
					bestArg = i;
				}
			}
			if(bestArg != -1)
			{
				search = -1;
				P_SetTarget(
					&actor->special1.m,
					P_FindMobjFromTID(target->special_args[bestArg], &search)
				);
			}
		}
		else
		{
			do
			{
				i = (P_Random(RandomClass::Hexen) >> 2) % 5;
			}
			while(!target->special_args[i]);
			search = -1;
			P_SetTarget(
				&actor->special1.m,
				P_FindMobjFromTID(target->special_args[i], &search)
			);
		}
	}
}

extern "C" void A_DragonInitFlight(mobj_t* actor)
{
	int search;

	search = -1;
	do
	{
		// find the first tid identical to the dragon's tid
		P_SetTarget(
			&actor->special1.m,
			P_FindMobjFromTID(actor->tid, &search)
		);
		if(search == -1)
		{
			P_SetMobjState(actor, actor->info->spawnstate);
			return;
		}
	}
	while(actor->special1.m == actor);
	map_format.remove_mobj_thing_id(actor);
}

extern "C" void A_DragonFlight(mobj_t* actor)
{
	angle_t angle;

	DragonSeek(actor, 4 * ANG1, 8 * ANG1);
	if(actor->target)
	{
		if((actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
		{
			// target died
			P_SetTarget(&actor->target, nullptr);
			return;
		}
		angle = R_PointToAngle2(actor->x, actor->y, actor->target->x,
			actor->target->y);
		if(AngleAbs(AngleDifference(actor->angle, angle)) < ANG45 / 2
			&& P_CheckMeleeRange(actor))
		{
			P_DamageMobj(actor->target, actor, actor, HITDICE(8));
			S_StartMobjSound(actor, SfxId::HexenDragonAttack);
		}
		else if(AngleAbs(AngleDifference(actor->angle, angle)) <= ANG1 * 20)
		{
			P_SetMobjState(actor, actor->info->missilestate);
			S_StartMobjSound(actor, SfxId::HexenDragonAttack);
		}
	}
	else
	{
		P_LookForPlayers(actor, true);
	}
}

extern "C" void A_DragonFlap(mobj_t* actor)
{
	A_DragonFlight(actor);
	if(P_Random(RandomClass::Hexen) < 240)
	{
		S_StartMobjSound(actor, SfxId::HexenDragonWingflap);
	}
	else
	{
		S_StartMobjSound(actor, actor->info->activesound);
	}
}

extern "C" void A_DragonAttack(mobj_t* actor)
{
	P_SpawnMissile(actor, actor->target, MobjType::HexenDragonFx);
}

extern "C" void A_DragonFX2(mobj_t* actor)
{
	mobj_t* mo;
	int i;
	int r1, r2, r3;
	int delay;

	delay = 16 + (P_Random(RandomClass::Hexen) >> 3);
	for(i = 1 + (P_Random(RandomClass::Hexen) & 3); i; i--)
	{
		r1 = P_Random(RandomClass::Hexen);
		r2 = P_Random(RandomClass::Hexen);
		r3 = P_Random(RandomClass::Hexen);
		mo = P_SpawnMobj(actor->x + ((r3 - 128) << 14),
			actor->y + ((r2 - 128) << 14),
			actor->z + ((r1 - 128) << 12),
			MobjType::HexenDragonFx2);
		if(mo)
		{
			mo->tics = delay + (P_Random(RandomClass::Hexen) & 3) * i * 2;
			P_SetTarget(&mo->target, actor->target);
		}
	}
}

extern "C" void A_DragonPain(mobj_t* actor)
{
	A_Pain(actor);
	if(!actor->special1.m)
	{
		// no destination spot yet
		P_SetMobjState(actor, StateId::HexenDragonInit);
	}
}

extern "C" void A_DragonCheckCrash(mobj_t* actor)
{
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, StateId::HexenDragonCrash1);
	}
}

extern "C" void A_DemonAttack1(mobj_t* actor)
{
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
	}
}

extern "C" void A_DemonAttack2(mobj_t* actor)
{
	mobj_t* mo;
	MobjType fireBall;

	if(actor->type == MobjType::HexenDemon)
	{
		fireBall = MobjType::HexenDemonfx1;
	}
	else
	{
		fireBall = MobjType::HexenDemon2fx1;
	}
	mo = P_SpawnMissile(actor, actor->target, fireBall);
	if(mo)
	{
		mo->z += 30 * FRACUNIT;
		S_StartMobjSound(actor, SfxId::HexenDemonMissileFire);
	}
}

extern "C" void A_DemonDeath(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemonchunk1);
	if(mo)
	{
		angle = actor->angle + ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemonchunk2);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemonchunk3);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemonchunk4);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemonchunk5);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
}

extern "C" void A_Demon2Death(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemon2chunk1);
	if(mo)
	{
		angle = actor->angle + ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemon2chunk2);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemon2chunk3);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemon2chunk4);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 45 * FRACUNIT,
		MobjType::HexenDemon2chunk5);
	if(mo)
	{
		angle = actor->angle - ANG90;
		mo->momz = 8 * FRACUNIT;
		mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finecosine[angle >> ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 10) + FRACUNIT,
			finesine[angle >> ANGLETOFINESHIFT]);
		P_SetTarget(&mo->target, actor);
	}
}

extern "C" dboolean A_SinkMobj(mobj_t* actor)
{
	if(actor->floorclip < actor->info->height)
	{
		switch(actor->type)
		{
			case MobjType::HexenThrustfloorDown:
			case MobjType::HexenThrustfloorUp:
				actor->floorclip += 6 * FRACUNIT;
				break;
			default:
				actor->floorclip += FRACUNIT;
				break;
		}
		return false;
	}
	return true;
}

extern "C" dboolean A_RaiseMobj(mobj_t* actor)
{
	int done = true;

	// Raise a mobj from the ground
	if(actor->floorclip > 0)
	{
		switch(actor->type)
		{
			case MobjType::HexenWraithb:
				actor->floorclip -= 2 * FRACUNIT;
				break;
			case MobjType::HexenThrustfloorDown:
			case MobjType::HexenThrustfloorUp:
				actor->floorclip -= actor->special2.i * FRACUNIT;
				break;
			default:
				actor->floorclip -= 2 * FRACUNIT;
				break;
		}
		if(actor->floorclip <= 0)
		{
			actor->floorclip = 0;
			done = true;
		}
		else
		{
			done = false;
		}
	}
	return done; // Reached target height
}

extern "C" void A_WraithInit(mobj_t* actor)
{
	actor->z += 48 << FRACBITS;
	actor->special1.i = 0; // index into floatbob
}

extern "C" void A_WraithRaiseInit(mobj_t* actor)
{
	actor->flags2 -= MobjFlag2::DontDraw;
	actor->flags2 -= MobjFlag2::NonShootable;
	actor->flags |= MobjFlag::Shootable | MobjFlag::Solid;
	actor->floorclip = actor->info->height;
}

extern "C" void A_WraithRaise(mobj_t* actor)
{
	if(A_RaiseMobj(actor))
	{
		// Reached it's target height
		P_SetMobjState(actor, StateId::HexenWraithChase1);
	}

	P_SpawnDirt(actor, actor->radius);
}

extern "C" void A_WraithMelee(mobj_t* actor)
{
	int amount;

	// Steal health from target and give to player
	if(P_CheckMeleeRange(actor) && (P_Random(RandomClass::Hexen) < 220))
	{
		amount = HITDICE(2);
		P_DamageMobj(actor->target, actor, actor, amount);
		actor->health += amount;
	}
}

extern "C" void A_WraithMissile(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMissile(actor, actor->target, MobjType::HexenWraithfx1);
	if(mo)
	{
		S_StartMobjSound(actor, SfxId::HexenWraithMissileFire);
	}
}

extern "C" void A_WraithFX2(mobj_t* actor)
{
	mobj_t* mo;
	angle_t angle;
	int i;

	for(i = 0; i < 2; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenWraithfx2);
		if(mo)
		{
			if(P_Random(RandomClass::Hexen) < 128)
			{
				angle = actor->angle + (P_Random(RandomClass::Hexen) << 22);
			}
			else
			{
				angle = actor->angle - (P_Random(RandomClass::Hexen) << 22);
			}
			mo->momz = 0;
			mo->momx = FixedMul((P_Random(RandomClass::Hexen) << 7) + FRACUNIT,
				finecosine[angle >> ANGLETOFINESHIFT]);
			mo->momy = FixedMul((P_Random(RandomClass::Hexen) << 7) + FRACUNIT,
				finesine[angle >> ANGLETOFINESHIFT]);
			P_SetTarget(&mo->target, actor);
			mo->floorclip = 10 * FRACUNIT;
		}
	}
}

extern "C" void A_WraithFX3(mobj_t* actor)
{
	mobj_t* mo;
	int numdropped = P_Random(RandomClass::Hexen) % 15;
	int i;

	for(i = 0; i < numdropped; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenWraithfx3);
		if(mo)
		{
			mo->x += (P_Random(RandomClass::Hexen) - 128) << 11;
			mo->y += (P_Random(RandomClass::Hexen) - 128) << 11;
			mo->z += (P_Random(RandomClass::Hexen) << 10);
			P_SetTarget(&mo->target, actor);
		}
	}
}

extern "C" void A_WraithFX4(mobj_t* actor)
{
	mobj_t* mo;
	int chance = P_Random(RandomClass::Hexen);
	int spawn4, spawn5;

	if(chance < 10)
	{
		spawn4 = true;
		spawn5 = false;
	}
	else if(chance < 20)
	{
		spawn4 = false;
		spawn5 = true;
	}
	else if(chance < 25)
	{
		spawn4 = true;
		spawn5 = true;
	}
	else
	{
		spawn4 = false;
		spawn5 = false;
	}

	if(spawn4)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenWraithfx4);
		if(mo)
		{
			mo->x += (P_Random(RandomClass::Hexen) - 128) << 12;
			mo->y += (P_Random(RandomClass::Hexen) - 128) << 12;
			mo->z += (P_Random(RandomClass::Hexen) << 10);
			P_SetTarget(&mo->target, actor);
		}
	}
	if(spawn5)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenWraithfx5);
		if(mo)
		{
			mo->x += (P_Random(RandomClass::Hexen) - 128) << 11;
			mo->y += (P_Random(RandomClass::Hexen) - 128) << 11;
			mo->z += (P_Random(RandomClass::Hexen) << 10);
			P_SetTarget(&mo->target, actor);
		}
	}
}

extern "C" void A_WraithLook(mobj_t* actor)
{
	//  A_WraithFX4(actor);             // too expensive
	A_Look(actor);
}

extern "C" void A_WraithChase(mobj_t* actor)
{
	int weaveindex = actor->special1.i;
	actor->z += FloatBobOffsets[weaveindex];
	actor->special1.i = (weaveindex + 2) & 63;
	A_Chase(actor);
	A_WraithFX4(actor);
}

extern "C" void A_EttinAttack(mobj_t* actor)
{
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
	}
}

extern "C" void A_DropMace(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMobj(actor->x, actor->y,
		actor->z + (actor->height >> 1), MobjType::HexenEttinMace);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momz = FRACUNIT * 10 + (P_Random(RandomClass::Hexen) << 10);
		P_SetTarget(&mo->target, actor);
	}
}

extern "C" void A_FiredSpawnRock(mobj_t* actor)
{
	mobj_t* mo;
	int x, y, z;
	MobjType rtype = MobjType::Null;

	switch(P_Random(RandomClass::Hexen) % 5)
	{
		case 0:
			rtype = MobjType::HexenFiredemonFx1;
			break;
		case 1:
			rtype = MobjType::HexenFiredemonFx2;
			break;
		case 2:
			rtype = MobjType::HexenFiredemonFx3;
			break;
		case 3:
			rtype = MobjType::HexenFiredemonFx4;
			break;
		case 4:
			rtype = MobjType::HexenFiredemonFx5;
			break;
	}

	x = actor->x + ((P_Random(RandomClass::Hexen) - 128) << 12);
	y = actor->y + ((P_Random(RandomClass::Hexen) - 128) << 12);
	z = actor->z + ((P_Random(RandomClass::Hexen)) << 11);
	mo = P_SpawnMobj(x, y, z, rtype);
	if(mo)
	{
		P_SetTarget(&mo->target, actor);
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 10;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 10;
		mo->momz = (P_Random(RandomClass::Hexen) << 10);
		mo->special1.i = 2; // Number bounces
	}

	// Initialize fire demon
	actor->special2.i = 0;
	actor->flags -= MobjFlag::JustAttacked;
}

extern "C" void A_FiredRocks(mobj_t* actor)
{
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
}

extern "C" void A_FiredAttack(mobj_t* actor)
{
	mobj_t* mo;
	mo = P_SpawnMissile(actor, actor->target, MobjType::HexenFiredemonFx6);
	if(mo)
		S_StartMobjSound(actor, SfxId::HexenFiredAttack);
}

extern "C" void A_SmBounce(mobj_t* actor)
{
	// give some more momentum (x,y,&z)
	actor->z = actor->floorz + FRACUNIT;
	actor->momz = (2 * FRACUNIT) + (P_Random(RandomClass::Hexen) << 10);
	actor->momx = P_Random(RandomClass::Hexen) % 3 << FRACBITS;
	actor->momy = P_Random(RandomClass::Hexen) % 3 << FRACBITS;
}

#define FIREDEMON_ATTACK_RANGE	64*8*FRACUNIT

extern "C" void A_FiredChase(mobj_t* actor)
{
	int weaveindex = actor->special1.i;
	mobj_t* target = actor->target;
	angle_t ang;
	fixed_t dist;

	if(actor->reactiontime)
		actor->reactiontime--;
	if(actor->threshold)
		actor->threshold--;

	// Float up and down
	actor->z += FloatBobOffsets[weaveindex];
	actor->special1.i = (weaveindex + 2) & 63;

	// Insure it stays above certain height
	if(actor->z < actor->floorz + (64 * FRACUNIT))
	{
		actor->z += 2 * FRACUNIT;
	}

	if(!actor->target || (actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// Invalid target
		P_LookForPlayers(actor, true);
		return;
	}

	// Strafe
	if(actor->special2.i > 0)
	{
		actor->special2.i--;
	}
	else
	{
		actor->special2.i = 0;
		actor->momx = actor->momy = 0;
		dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
		if(dist < FIREDEMON_ATTACK_RANGE)
		{
			if(P_Random(RandomClass::Hexen) < 30)
			{
				ang =
					R_PointToAngle2(actor->x, actor->y, target->x, target->y);
				if(P_Random(RandomClass::Hexen) < 128)
					ang += ANG90;
				else
					ang -= ANG90;
				ang >>= ANGLETOFINESHIFT;
				actor->momx = FixedMul(8 * FRACUNIT, finecosine[ang]);
				actor->momy = FixedMul(8 * FRACUNIT, finesine[ang]);
				actor->special2.i = 3; // strafe time
			}
		}
	}

	FaceMovementDirection(actor);

	// Normal movement
	if(!actor->special2.i)
	{
		if(--actor->movecount < 0 || !P_Move(actor, false))
		{
			P_NewChaseDir(actor);
		}
	}

	// Do missile attack
	if((actor->flags & MobjFlag::JustAttacked) == MobjFlag{})
	{
		if(P_CheckMissileRange(actor) && (P_Random(RandomClass::Hexen) < 20))
		{
			P_SetMobjState(actor, actor->info->missilestate);
			actor->flags |= MobjFlag::JustAttacked;
			return;
		}
	}
	else
	{
		actor->flags -= MobjFlag::JustAttacked;
	}

	// make active sound
	if(actor->info->activesound != SfxId::None && P_Random(RandomClass::Hexen) < 3)
	{
		S_StartMobjSound(actor, actor->info->activesound);
	}
}

extern "C" void A_FiredSplotch(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenFiredemonSplotch1);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momz = FRACUNIT * 3 + (P_Random(RandomClass::Hexen) << 10);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenFiredemonSplotch2);
	if(mo)
	{
		mo->momx = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momy = (P_Random(RandomClass::Hexen) - 128) << 11;
		mo->momz = FRACUNIT * 3 + (P_Random(RandomClass::Hexen) << 10);
	}
}

#include "heretic/sb_bar.hpp"

extern "C" void A_FreezeDeath(mobj_t* actor)
{
	int r = P_Random(RandomClass::Hexen);
	actor->tics = 75 + r + P_Random(RandomClass::Hexen);
	actor->flags |= MobjFlag::Solid | MobjFlag::Shootable | MobjFlag::NoBlood;
	actor->flags2 |= MobjFlag2::Pushable | MobjFlag2::TeleStomp | MobjFlag2::PassMobj | MobjFlag2::Slide;
	actor->height <<= 2;
	S_StartMobjSound(actor, SfxId::HexenFreezeDeath);

	if(actor->player)
	{
		actor->player->damagecount = 0;
		actor->player->poisoncount = 0;
		actor->player->bonuscount = 0;
		if(actor->player == &players[consoleplayer])
		{
			SB_PaletteFlash(false);
		}
	}
	else if((actor->flags & MobjFlag::CountKill) != MobjFlag{} && actor->special)
	{
		// Initiate monster death actions.
		map_format.execute_line_special(actor->special, actor->special_args, nullptr, 0, actor);
	}
}

extern "C" void A_IceSetTics(mobj_t* actor)
{

	actor->tics = 70 + (P_Random(RandomClass::Hexen) & 63);
	const FloorType floor = P_GetThingFloorType(actor);
	if(floor == FloorType::Lava)
	{
		actor->tics >>= 2;
	}
	else if(floor == FloorType::Ice)
	{
		actor->tics <<= 1;
	}
}

extern "C" void A_IceCheckHeadDone(mobj_t* actor)
{
	if(actor->special2.i == 666)
	{
		P_SetMobjState(actor, StateId::HexenIcechunkHead2);
	}
}

extern "C" void A_FreezeDeathChunks(mobj_t* actor)
{
	int i;
	int r1, r2, r3;
	mobj_t* mo;

	if(actor->momx || actor->momy || actor->momz)
	{
		actor->tics = 105;
		return;
	}
	S_StartMobjSound(actor, SfxId::HexenFreezeShatter);

	for(i = 12 + (P_Random(RandomClass::Hexen) & 15); i >= 0; i--)
	{
		r1 = P_Random(RandomClass::Hexen);
		r2 = P_Random(RandomClass::Hexen);
		r3 = P_Random(RandomClass::Hexen);
		mo = P_SpawnMobj(actor->x +
			(((r3 - 128) * actor->radius) >> 7),
			actor->y +
			(((r2 - 128) * actor->radius) >> 7),
			actor->z + (r1 * actor->height / 255),
			MobjType::HexenIcechunk);
		P_SetMobjState(mo, StateVariant(mo->info->spawnstate, (P_Random(RandomClass::Hexen) % 3)));
		mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
		mo->momx = P_SubRandom() << (FRACBITS - 7);
		mo->momy = P_SubRandom() << (FRACBITS - 7);
		A_IceSetTics(mo); // set a random tic wait
	}
	for(i = 12 + (P_Random(RandomClass::Hexen) & 15); i >= 0; i--)
	{
		r1 = P_Random(RandomClass::Hexen);
		r2 = P_Random(RandomClass::Hexen);
		r3 = P_Random(RandomClass::Hexen);
		mo = P_SpawnMobj(actor->x +
			(((r3 - 128) * actor->radius) >> 7),
			actor->y +
			(((r2 - 128) * actor->radius) >> 7),
			actor->z + (r1 * actor->height / 255),
			MobjType::HexenIcechunk);
		P_SetMobjState(mo, StateVariant(mo->info->spawnstate, (P_Random(RandomClass::Hexen) % 3)));
		mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
		mo->momx = P_SubRandom() << (FRACBITS - 7);
		mo->momy = P_SubRandom() << (FRACBITS - 7);
		A_IceSetTics(mo); // set a random tic wait
	}
	if(actor->player)
	{
		// attach the player's view to a chunk of ice
		mo = P_SpawnMobj(actor->x, actor->y, actor->z + g_viewheight,
			MobjType::HexenIcechunk);
		P_SetMobjState(mo, StateId::HexenIcechunkHead);
		mo->momz = FixedDiv(mo->z - actor->z, actor->height) << 2;
		mo->momx = P_SubRandom() << (FRACBITS - 7);
		mo->momy = P_SubRandom() << (FRACBITS - 7);
		mo->flags2 |= MobjFlag2::IceDamage; // used to force blue palette
		mo->flags2 -= MobjFlag2::FootClip;
		mo->player = actor->player;
		actor->player = nullptr;
		mo->health = actor->health;
		mo->angle = actor->angle;
		mo->player->mo = mo;
		mo->player->lookdir = 0;
	}
	map_format.remove_mobj_thing_id(actor);
	P_SetMobjState(actor, StateId::HexenFreetargmobj);
	actor->flags2 |= MobjFlag2::DontDraw;
}

extern "C" void A_IceGuyLook(mobj_t* actor)
{
	fixed_t dist;
	fixed_t an;

	A_Look(actor);
	if(P_Random(RandomClass::Hexen) < 64)
	{
		dist = ((P_Random(RandomClass::Hexen) - 128) * actor->radius) >> 7;
		an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;

		P_SpawnMobj(actor->x + FixedMul(dist, finecosine[an]),
			actor->y + FixedMul(dist, finesine[an]),
			actor->z + 60 * FRACUNIT,
			MobjVariant(MobjType::HexenIceguyWisp1, (P_Random(RandomClass::Hexen) & 1)));
	}
}

extern "C" void A_IceGuyChase(mobj_t* actor)
{
	fixed_t dist;
	fixed_t an;
	mobj_t* mo;

	A_Chase(actor);
	if(P_Random(RandomClass::Hexen) < 128)
	{
		dist = ((P_Random(RandomClass::Hexen) - 128) * actor->radius) >> 7;
		an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;

		mo = P_SpawnMobj(actor->x + FixedMul(dist, finecosine[an]),
			actor->y + FixedMul(dist, finesine[an]),
			actor->z + 60 * FRACUNIT,
			MobjVariant(MobjType::HexenIceguyWisp1, (P_Random(RandomClass::Hexen) & 1)));
		if(mo)
		{
			mo->momx = actor->momx;
			mo->momy = actor->momy;
			mo->momz = actor->momz;
			P_SetTarget(&mo->target, actor);
		}
	}
}

extern "C" void A_IceGuyAttack(mobj_t* actor)
{
	fixed_t an;

	if(!actor->target)
	{
		return;
	}
	an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
	P_SpawnMissileXYZ(actor->x + FixedMul(actor->radius >> 1,
			finecosine[an]),
		actor->y + FixedMul(actor->radius >> 1, finesine[an]),
		actor->z + 40 * FRACUNIT, actor, actor->target,
		MobjType::HexenIceguyFx);
	an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
	P_SpawnMissileXYZ(actor->x + FixedMul(actor->radius >> 1,
			finecosine[an]),
		actor->y + FixedMul(actor->radius >> 1, finesine[an]),
		actor->z + 40 * FRACUNIT, actor, actor->target,
		MobjType::HexenIceguyFx);
	S_StartMobjSound(actor, actor->info->attacksound);
}

extern "C" void A_IceGuyMissilePuff(mobj_t* actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z + 2 * FRACUNIT, MobjType::HexenIcefxPuff);
}

extern "C" void A_IceGuyDie(mobj_t* actor)
{
	actor->momx = 0;
	actor->momy = 0;
	actor->momz = 0;
	actor->height <<= 2;
	A_FreezeDeathChunks(actor);
}

extern "C" void A_IceGuyMissileExplode(mobj_t* actor)
{
	mobj_t* mo;
	unsigned int i;

	for(i = 0; i < 8; i++)
	{
		mo = P_SpawnMissileAngle(actor, MobjType::HexenIceguyFx2, i * ANG45,
			-0.3 * FRACUNIT);
		if(mo)
		{
			P_SetTarget(&mo->target, actor->target);
		}
	}
}

//============================================================================
//
//      Sorcerer stuff
//
// Sorcerer Variables
//              special1                Angle of ball 1 (all others relative to that)
//              special2                which ball to stop at in stop mode (HEXEN_MT_???)
//              args[0]                 Denfense time
//              args[1]                 Number of full rotations since stopping mode
//              args[2]                 Target orbit speed for acceleration/deceleration
//              args[3]                 Movement mode (see SORC_ macros)
//              args[4]                 Current ball orbit speed
//      Sorcerer Ball Variables
//              special1                Previous angle of ball (for woosh)
//              special2                Countdown of rapid fire (FX4)
//              args[0]                 If set, don't play the bounce sound when bouncing
//============================================================================

#define SORCBALL_INITIAL_SPEED 		7
#define SORCBALL_TERMINAL_SPEED		25
#define SORCBALL_SPEED_ROTATIONS 	5
#define SORC_DEFENSE_TIME			255
#define SORC_DEFENSE_HEIGHT			45
#define BOUNCE_TIME_UNIT			(TICRATE/2)
#define SORCFX4_RAPIDFIRE_TIME		(6*3)   // 3 seconds
#define SORCFX4_SPREAD_ANGLE		20

#define SORC_DECELERATE		0
#define SORC_ACCELERATE 	1
#define SORC_STOPPING		2
#define SORC_FIRESPELL		3
#define SORC_STOPPED		4
#define SORC_NORMAL			5
#define SORC_FIRING_SPELL	6

#define BALL1_ANGLEOFFSET	0
#define BALL2_ANGLEOFFSET	(ANGLE_MAX/3)
#define BALL3_ANGLEOFFSET	((ANGLE_MAX/3)*2)

extern "C" void A_SorcBallOrbit(mobj_t* actor);
extern "C" void A_SorcSpinBalls(mobj_t* actor);
extern "C" void A_SpeedBalls(mobj_t* actor);
extern "C" void A_SlowBalls(mobj_t* actor);
extern "C" void A_StopBalls(mobj_t* actor);
extern "C" void A_AccelBalls(mobj_t* actor);
extern "C" void A_DecelBalls(mobj_t* actor);
extern "C" void A_SorcBossAttack(mobj_t* actor);
extern "C" void A_SpawnFizzle(mobj_t* actor);
extern "C" void A_CastSorcererSpell(mobj_t* actor);
extern "C" void A_SorcUpdateBallAngle(mobj_t* actor);
extern "C" void A_BounceCheck(mobj_t* actor);
extern "C" void A_SorcFX1Seek(mobj_t* actor);
extern "C" void A_SorcOffense1(mobj_t* actor);
extern "C" void A_SorcOffense2(mobj_t* actor);

extern "C" void A_SorcSpinBalls(mobj_t* actor)
{
	mobj_t* mo;
	fixed_t z;

	A_SlowBalls(actor);
	actor->special_args[0] = 0; // Currently no defense
	actor->special_args[3] = SORC_NORMAL;
	actor->special_args[4] = SORCBALL_INITIAL_SPEED; // Initial orbit speed
	actor->special1.i = ANG1;
	z = actor->z - actor->floorclip + actor->info->height;

	mo = P_SpawnMobj(actor->x, actor->y, z, MobjType::HexenSorcball1);
	if(mo)
	{
		P_SetTarget(&mo->target, actor);
		mo->special2.i = SORCFX4_RAPIDFIRE_TIME;
	}
	mo = P_SpawnMobj(actor->x, actor->y, z, MobjType::HexenSorcball2);
	if(mo)
		P_SetTarget(&mo->target, actor);
	mo = P_SpawnMobj(actor->x, actor->y, z, MobjType::HexenSorcball3);
	if(mo)
		P_SetTarget(&mo->target, actor);
}

extern "C" void A_SorcBallOrbit(mobj_t* actor)
{
	int x, y;
	angle_t angle, baseangle;
	int mode = actor->target->special_args[3];
	mobj_t* parent = (mobj_t*)actor->target;
	int dist = parent->radius - (actor->radius << 1);
	angle_t prevangle = actor->special1.i;

	if(actor->target->health <= 0)
		P_SetMobjState(actor, actor->info->painstate);

	baseangle = (angle_t)parent->special1.i;
	switch(actor->type)
	{
		case MobjType::HexenSorcball1:
			angle = baseangle + BALL1_ANGLEOFFSET;
			break;
		case MobjType::HexenSorcball2:
			angle = baseangle + BALL2_ANGLEOFFSET;
			break;
		case MobjType::HexenSorcball3:
			angle = baseangle + BALL3_ANGLEOFFSET;
			break;
		default:
			Log::Fatal("corrupted sorcerer");
			return;
	}
	actor->angle = angle;
	angle >>= ANGLETOFINESHIFT;

	switch(mode)
	{
		case SORC_NORMAL: // Balls rotating normally
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_DECELERATE: // Balls decelerating
			A_DecelBalls(actor);
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_ACCELERATE: // Balls accelerating
			A_AccelBalls(actor);
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_STOPPING: // Balls stopping
			if((parent->special2.i == std::to_underlying(actor->type)) &&
				(parent->special_args[1] > SORCBALL_SPEED_ROTATIONS) &&
				(abs((int)angle - (int)(parent->angle >> ANGLETOFINESHIFT)) <
					(30 << 5)))
			{
				// Can stop now
				actor->target->special_args[3] = SORC_FIRESPELL;
				actor->target->special_args[4] = 0;
				// Set angle so ball angle == sorcerer angle
				switch(actor->type)
				{
					case MobjType::HexenSorcball1:
						parent->special1.i = (int)(parent->angle -
							BALL1_ANGLEOFFSET);
						break;
					case MobjType::HexenSorcball2:
						parent->special1.i = (int)(parent->angle -
							BALL2_ANGLEOFFSET);
						break;
					case MobjType::HexenSorcball3:
						parent->special1.i = (int)(parent->angle -
							BALL3_ANGLEOFFSET);
						break;
					default:
						break;
				}
			}
			else
			{
				A_SorcUpdateBallAngle(actor);
			}
			break;
		case SORC_FIRESPELL: // Casting spell
			if(parent->special2.i == std::to_underlying(actor->type))
			{
				// Put sorcerer into special throw spell anim
				if(parent->health > 0)
					P_SetMobjStateNF(parent, StateId::HexenSorcAttack1);

				if(actor->type == MobjType::HexenSorcball1 && P_Random(RandomClass::Hexen) < 200)
				{
					S_StartVoidSound(SfxId::HexenSorcererSpellcast);
					actor->special2.i = SORCFX4_RAPIDFIRE_TIME;
					actor->special_args[4] = 128;
					parent->special_args[3] = SORC_FIRING_SPELL;
				}
				else
				{
					A_CastSorcererSpell(actor);
					parent->special_args[3] = SORC_STOPPED;
				}
			}
			break;
		case SORC_FIRING_SPELL:
			if(parent->special2.i == std::to_underlying(actor->type))
			{
				if(actor->special2.i-- <= 0)
				{
					// Done rapid firing
					parent->special_args[3] = SORC_STOPPED;
					// Back to orbit balls
					if(parent->health > 0)
						P_SetMobjStateNF(parent, StateId::HexenSorcAttack4);
				}
				else
				{
					// Do rapid fire spell
					A_SorcOffense2(actor);
				}
			}
			break;
		case SORC_STOPPED: // Balls stopped
		default:
			break;
	}

	if((angle < prevangle) && (parent->special_args[4] == SORCBALL_TERMINAL_SPEED))
	{
		parent->special_args[1]++; // Bump rotation counter
		// Completed full rotation - make woosh sound
		S_StartMobjSound(actor, SfxId::HexenSorcererBallwoosh);
	}
	actor->special1.i = angle; // Set previous angle
	x = parent->x + FixedMul(dist, finecosine[angle]);
	y = parent->y + FixedMul(dist, finesine[angle]);
	actor->x = x;
	actor->y = y;
	actor->z = parent->z - parent->floorclip + parent->info->height;
}

extern "C" void A_SpeedBalls(mobj_t* actor)
{
	actor->special_args[3] = SORC_ACCELERATE;         // speed mode
	actor->special_args[2] = SORCBALL_TERMINAL_SPEED; // target speed
}

extern "C" void A_SlowBalls(mobj_t* actor)
{
	actor->special_args[3] = SORC_DECELERATE;        // slow mode
	actor->special_args[2] = SORCBALL_INITIAL_SPEED; // target speed
}

extern "C" void A_StopBalls(mobj_t* actor)
{
	int chance = P_Random(RandomClass::Hexen);
	actor->special_args[3] = SORC_STOPPING; // stopping mode
	actor->special_args[1] = 0;             // Reset rotation counter

	if((actor->special_args[0] <= 0) && (chance < 200))
	{
		actor->special2.i = std::to_underlying(MobjType::HexenSorcball2); // Blue
	}
	else if((actor->health < (P_MobjSpawnHealth(actor) >> 1)) &&
		(chance < 200))
	{
		actor->special2.i = std::to_underlying(MobjType::HexenSorcball3); // Green
	}
	else
	{
		actor->special2.i = std::to_underlying(MobjType::HexenSorcball1); // Yellow
	}
}

extern "C" void A_AccelBalls(mobj_t* actor)
{
	mobj_t* sorc = actor->target;

	if(sorc->special_args[4] < sorc->special_args[2])
	{
		sorc->special_args[4]++;
	}
	else
	{
		sorc->special_args[3] = SORC_NORMAL;
		if(sorc->special_args[4] >= SORCBALL_TERMINAL_SPEED)
		{
			// Reached terminal velocity - stop balls
			A_StopBalls(sorc);
		}
	}
}

extern "C" void A_DecelBalls(mobj_t* actor)
{
	mobj_t* sorc = actor->target;

	if(sorc->special_args[4] > sorc->special_args[2])
	{
		sorc->special_args[4]--;
	}
	else
	{
		sorc->special_args[3] = SORC_NORMAL;
	}
}

extern "C" void A_SorcUpdateBallAngle(mobj_t* actor)
{
	if(actor->type == MobjType::HexenSorcball1)
	{
		actor->target->special1.i += ANG1 * actor->target->special_args[4];
	}
}

extern "C" void A_CastSorcererSpell(mobj_t* actor)
{
	mobj_t* mo;
	MobjType spell = actor->type;
	angle_t ang1, ang2;
	fixed_t z;
	mobj_t* parent = actor->target;

	S_StartVoidSound(SfxId::HexenSorcererSpellcast);

	// Put sorcerer into throw spell animation
	if(parent->health > 0)
		P_SetMobjStateNF(parent, StateId::HexenSorcAttack4);

	switch(spell)
	{
		case MobjType::HexenSorcball1: // Offensive
			A_SorcOffense1(actor);
			break;
		case MobjType::HexenSorcball2: // Defensive
			z = parent->z - parent->floorclip +
				SORC_DEFENSE_HEIGHT * FRACUNIT;
			mo = P_SpawnMobj(actor->x, actor->y, z, MobjType::HexenSorcfx2);
			parent->flags2 |= MobjFlag2::Reflective | MobjFlag2::Invulnerable;
			parent->special_args[0] = SORC_DEFENSE_TIME;
			if(mo)
				P_SetTarget(&mo->target, parent);
			break;
		case MobjType::HexenSorcball3: // Reinforcements
			ang1 = actor->angle - ANG45;
			ang2 = actor->angle + ANG45;
			if(actor->health < (P_MobjSpawnHealth(actor) / 3))
			{
				// Spawn 2 at a time
				mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx3, ang1,
					4 * FRACUNIT);
				if(mo)
					P_SetTarget(&mo->target, parent);
				mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx3, ang2,
					4 * FRACUNIT);
				if(mo)
					P_SetTarget(&mo->target, parent);
			}
			else
			{
				if(P_Random(RandomClass::Hexen) < 128)
					ang1 = ang2;
				mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx3, ang1,
					4 * FRACUNIT);
				if(mo)
					P_SetTarget(&mo->target, parent);
			}
			break;
		default:
			break;
	}
}

extern "C" void A_SorcOffense1(mobj_t* actor)
{
	mobj_t* mo;
	angle_t ang1, ang2;
	mobj_t* parent = (mobj_t*)actor->target;

	ang1 = actor->angle + ANG1 * 70;
	ang2 = actor->angle - ANG1 * 70;
	mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx1, ang1, 0);
	if(mo)
	{
		P_SetTarget(&mo->target, parent);
		P_SetTarget(&mo->special1.m, parent->target);
		mo->special_args[4] = BOUNCE_TIME_UNIT;
		mo->special_args[3] = 15; // Bounce time in seconds
	}
	mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx1, ang2, 0);
	if(mo)
	{
		P_SetTarget(&mo->target, parent);
		P_SetTarget(&mo->special1.m, parent->target);
		mo->special_args[4] = BOUNCE_TIME_UNIT;
		mo->special_args[3] = 15; // Bounce time in seconds
	}
}

extern "C" void A_SorcOffense2(mobj_t* actor)
{
	angle_t ang1;
	mobj_t* mo;
	int delta, index;
	mobj_t* parent = actor->target;
	mobj_t* dest = parent->target;
	int dist;

	index = actor->special_args[4] << 5;
	actor->special_args[4] += 15;
	actor->special_args[4] &= 0xff;
	delta = (finesine[index]) * SORCFX4_SPREAD_ANGLE;
	delta = (delta >> FRACBITS) * ANG1;
	ang1 = actor->angle + delta;
	mo = P_SpawnMissileAngle(parent, MobjType::HexenSorcfx4, ang1, 0);
	if(mo)
	{
		mo->special2.i = TICRATE * 5 / 2; // 5 seconds
		dist = P_AproxDistance(dest->x - mo->x, dest->y - mo->y);
		dist = dist / mo->info->speed;
		if(dist < 1)
			dist = 1;
		mo->momz = (dest->z - mo->z) / dist;
	}
}

extern "C" void A_SorcBossAttack(mobj_t* actor)
{
	actor->special_args[3] = SORC_ACCELERATE;
	actor->special_args[2] = SORCBALL_INITIAL_SPEED;
}

extern "C" void A_SpawnFizzle(mobj_t* actor)
{
	fixed_t x, y, z;
	fixed_t dist = 5 * FRACUNIT;
	angle_t angle = actor->angle >> ANGLETOFINESHIFT;
	fixed_t speed = actor->info->speed;
	angle_t rangle;
	mobj_t* mo;
	int ix;

	x = actor->x + FixedMul(dist, finecosine[angle]);
	y = actor->y + FixedMul(dist, finesine[angle]);
	z = actor->z - actor->floorclip + (actor->height >> 1);
	for(ix = 0; ix < 5; ix++)
	{
		mo = P_SpawnMobj(x, y, z, MobjType::HexenSorcspark1);
		if(mo)
		{
			rangle = angle + ((P_Random(RandomClass::Hexen) % 5) << 1);
			mo->momx = FixedMul(P_Random(RandomClass::Hexen) % speed, finecosine[rangle]);
			mo->momy = FixedMul(P_Random(RandomClass::Hexen) % speed, finesine[rangle]);
			mo->momz = FRACUNIT * 2;
		}
	}
}

//============================================================================
// Yellow spell - offense
//============================================================================

extern "C" void A_SorcFX1Seek(mobj_t* actor)
{
	A_BounceCheck(actor);
	P_SeekerMissile(actor, &actor->special1.m, ANG1 * 2, ANG1 * 6, false);
}

//============================================================================
// Blue spell - defense
//============================================================================
//
// FX2 Variables
//              special1                current angle
//              special2
//              args[0]         0 = CW,  1 = CCW
//              args[1]
//============================================================================

extern "C" void A_SorcFX2Split(mobj_t* actor)
{
	mobj_t* mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenSorcfx2);
	if(mo)
	{
		P_SetTarget(&mo->target, actor->target);
		mo->special_args[0] = 0;       // CW
		mo->special1.i = actor->angle; // Set angle
		P_SetMobjStateNF(mo, StateId::HexenSorcfx2Orbit1);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenSorcfx2);
	if(mo)
	{
		P_SetTarget(&mo->target, actor->target);
		mo->special_args[0] = 1;       // CCW
		mo->special1.i = actor->angle; // Set angle
		P_SetMobjStateNF(mo, StateId::HexenSorcfx2Orbit1);
	}
	P_SetMobjStateNF(actor, StateId::HexenNull);
}

extern "C" void A_SorcFX2Orbit(mobj_t* actor)
{
	angle_t angle;
	fixed_t x, y, z;
	mobj_t* parent = actor->target;
	fixed_t dist = parent->info->radius;

	if((parent->health <= 0) ||     // Sorcerer is dead
		(!parent->special_args[0])) // Time expired
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
		parent->special_args[0] = 0;
		parent->flags2 -= MobjFlag2::Reflective;
		parent->flags2 -= MobjFlag2::Invulnerable;
	}

	if(actor->special_args[0] && (parent->special_args[0]-- <= 0)) // Time expired
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
		parent->special_args[0] = 0;
		parent->flags2 -= MobjFlag2::Reflective;
	}

	// Move to new position based on angle
	if(actor->special_args[0]) // Counter clock-wise
	{
		actor->special1.i += ANG1 * 10;
		angle = ((angle_t)actor->special1.i) >> ANGLETOFINESHIFT;
		x = parent->x + FixedMul(dist, finecosine[angle]);
		y = parent->y + FixedMul(dist, finesine[angle]);
		z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT * FRACUNIT;
		z += FixedMul(15 * FRACUNIT, finecosine[angle]);
		// Spawn trailer
		P_SpawnMobj(x, y, z, MobjType::HexenSorcfx2T1);
	}
	else // Clock wise
	{
		actor->special1.i -= ANG1 * 10;
		angle = ((angle_t)actor->special1.i) >> ANGLETOFINESHIFT;
		x = parent->x + FixedMul(dist, finecosine[angle]);
		y = parent->y + FixedMul(dist, finesine[angle]);
		z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT * FRACUNIT;
		z += FixedMul(20 * FRACUNIT, finesine[angle]);
		// Spawn trailer
		P_SpawnMobj(x, y, z, MobjType::HexenSorcfx2T1);
	}

	actor->x = x;
	actor->y = y;
	actor->z = z;
}

//============================================================================
// Green spell - spawn bishops
//============================================================================

extern "C" void A_SpawnBishop(mobj_t* actor)
{
	mobj_t* mo;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenBishop);
	if(mo)
	{
		if(!P_TestMobjLocation(mo))
		{
			P_SetMobjState(mo, StateId::HexenNull);
		}
	}
	P_SetMobjState(actor, StateId::HexenNull);
}

extern "C" void A_SmokePuffExit(mobj_t* actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenMntrsmokeexit);
}

extern "C" void A_SorcererBishopEntry(mobj_t* actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MobjType::HexenSorcfx3Explosion);
	S_StartMobjSound(actor, actor->info->seesound);
}

extern "C" void A_SorcFX4Check(mobj_t* actor)
{
	if(actor->special2.i-- <= 0)
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
	}
}

extern "C" void A_SorcBallPop(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HexenSorcererBallpop);
	actor->flags -= MobjFlag::NoGravity;
	actor->flags2 |= MobjFlag2::LoGrav;
	actor->momx = ((P_Random(RandomClass::Hexen) % 10) - 5) << FRACBITS;
	actor->momy = ((P_Random(RandomClass::Hexen) % 10) - 5) << FRACBITS;
	actor->momz = (2 + (P_Random(RandomClass::Hexen) % 3)) << FRACBITS;
	actor->special2.i = 4 * FRACUNIT;          // Initial bounce factor
	actor->special_args[4] = BOUNCE_TIME_UNIT; // Bounce time unit
	actor->special_args[3] = 5;                // Bounce time in seconds
}

extern "C" void A_BounceCheck(mobj_t* actor)
{
	if(actor->special_args[4]-- <= 0)
	{
		if(actor->special_args[3]-- <= 0)
		{
			P_SetMobjState(actor, actor->info->deathstate);
			switch(actor->type)
			{
				case MobjType::HexenSorcball1:
				case MobjType::HexenSorcball2:
				case MobjType::HexenSorcball3:
					S_StartVoidSound(SfxId::HexenSorcererBigballexplode);
					break;
				case MobjType::HexenSorcfx1:
					S_StartVoidSound(SfxId::HexenSorcererHeadscream);
					break;
				default:
					break;
			}
		}
		else
		{
			actor->special_args[4] = BOUNCE_TIME_UNIT;
		}
	}
}

#define CLASS_BOSS_STRAFE_RANGE	64*10*FRACUNIT

extern "C" void A_FastChase(mobj_t* actor)
{
	int delta;
	fixed_t dist;
	angle_t ang;
	mobj_t* target;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if((skill_info.flags & SkillFlag::FastMonsters) != SkillFlag{})
	{
		// Monsters move faster in nightmare mode
		actor->tics -= actor->tics / 2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

	//
	// turn towards movement direction if not there yet
	//
	if(actor->movedir < 8)
	{
		actor->angle &= (7 << 29);
		delta = actor->angle - (actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90 / 2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90 / 2;
		}
	}

	if(!actor->target || (actor->target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// look for a new target
		if(P_LookForPlayers(actor, true))
		{
			// got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

	//
	// don't attack twice in a row
	//
	if((actor->flags & MobjFlag::JustAttacked) != MobjFlag{})
	{
		actor->flags -= MobjFlag::JustAttacked;
		if((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{})
			P_NewChaseDir(actor);
		return;
	}

	// Strafe
	if(actor->special2.i > 0)
	{
		actor->special2.i--;
	}
	else
	{
		target = actor->target;
		actor->special2.i = 0;
		actor->momx = actor->momy = 0;
		dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
		if(dist < CLASS_BOSS_STRAFE_RANGE)
		{
			if(P_Random(RandomClass::Hexen) < 100)
			{
				ang = R_PointToAngle2(actor->x, actor->y,
					target->x, target->y);
				if(P_Random(RandomClass::Hexen) < 128)
					ang += ANG90;
				else
					ang -= ANG90;
				ang >>= ANGLETOFINESHIFT;
				actor->momx = FixedMul(13 * FRACUNIT, finecosine[ang]);
				actor->momy = FixedMul(13 * FRACUNIT, finesine[ang]);
				actor->special2.i = 3; // strafe time
			}
		}
	}

	//
	// check for missile attack
	//
	if(actor->info->missilestate != StateId::Null)
	{
		if((skill_info.flags & SkillFlag::FastMonsters) == SkillFlag{} && actor->movecount)
			goto nomissile;
		if(!P_CheckMissileRange(actor))
			goto nomissile;
		P_SetMobjState(actor, actor->info->missilestate);
		actor->flags |= MobjFlag::JustAttacked;
		return;
	}
nomissile:

	//
	// possibly choose another target
	//
	if(netgame && !actor->threshold && !P_CheckSight(actor, actor->target))
	{
		if(P_LookForPlayers(actor, true))
			return; // got a new target
	}

	//
	// chase towards player
	//
	if(!actor->special2.i)
	{
		if(--actor->movecount < 0 || !P_Move(actor, false))
		{
			P_NewChaseDir(actor);
		}
	}
}

extern "C" void A_FSwordAttack2(mobj_t* actor);
extern "C" void A_FighterAttack(mobj_t* actor)
{

	if(!actor->target)
		return;
	A_FSwordAttack2(actor);
}

extern "C" void A_CHolyAttack3(mobj_t* actor);
extern "C" void A_ClericAttack(mobj_t* actor)
{

	if(!actor->target)
		return;
	A_CHolyAttack3(actor);
}

extern "C" void A_MStaffAttack2(mobj_t* actor);
extern "C" void A_MageAttack(mobj_t* actor)
{

	if(!actor->target)
		return;
	A_MStaffAttack2(actor);
}

extern "C" void A_ClassBossHealth(mobj_t* actor)
{
	if(netgame && !deathmatch) // co-op only
	{
		if(!actor->special1.i)
		{
			actor->health *= 5;
			actor->special1.i = true; // has been initialized
		}
	}
}

extern "C" void A_CheckFloor(mobj_t* actor)
{
	if(actor->z <= actor->floorz)
	{
		actor->z = actor->floorz;
		actor->flags2 -= MobjFlag2::LoGrav;
		P_SetMobjState(actor, actor->info->deathstate);
	}
}

//===========================================================================
// Korax Variables
//      special1        last teleport destination
//      special2        set if "below half" script not yet run
//
// Korax Scripts (reserved)
//      249             Tell scripts that we are below half health
//      250-254         Control scripts
//      255             Death script
//
// Korax TIDs (reserved)
//      245             Reserved for Korax himself
//      248             Initial teleport destination
//      249             Teleport destination
//      250-254         For use in respective control scripts
//      255             For use in death script (spawn spots)
//===========================================================================
#define KORAX_SPIRIT_LIFETIME	(5*(TICRATE/5))      // 5 seconds
#define KORAX_COMMAND_HEIGHT	(120*FRACUNIT)
#define KORAX_COMMAND_OFFSET	(27*FRACUNIT)

extern "C" void KoraxFire1(mobj_t* actor, MobjType type);
extern "C" void KoraxFire2(mobj_t* actor, MobjType type);
extern "C" void KoraxFire3(mobj_t* actor, MobjType type);
extern "C" void KoraxFire4(mobj_t* actor, MobjType type);
extern "C" void KoraxFire5(mobj_t* actor, MobjType type);
extern "C" void KoraxFire6(mobj_t* actor, MobjType type);
extern "C" void KSpiritInit(mobj_t* spirit, mobj_t* korax);

#define KORAX_TID					(245)
#define KORAX_FIRST_TELEPORT_TID	(248)
#define KORAX_TELEPORT_TID			(249)

extern "C" void A_KoraxChase(mobj_t* actor)
{
	mobj_t* spot;
	int lastfound;
	byte args[3] = {0, 0, 0};

	if((!actor->special2.i) &&
		(actor->health <= (P_MobjSpawnHealth(actor) / 2)))
	{
		lastfound = 0;
		spot = P_FindMobjFromTID(KORAX_FIRST_TELEPORT_TID, &lastfound);
		if(spot)
		{
			P_Teleport(actor, spot->x, spot->y, spot->angle, true);
		}

		CheckACSPresent(249);
		P_StartACS(249, 0, args, actor, nullptr, 0);
		actor->special2.i = 1; // Don't run again

		return;
	}

	if(!actor->target)
		return;
	if(P_Random(RandomClass::Hexen) < 30)
	{
		P_SetMobjState(actor, actor->info->missilestate);
	}
	else if(P_Random(RandomClass::Hexen) < 30)
	{
		S_StartVoidSound(SfxId::HexenKoraxActive);
	}

	// Teleport away
	if(actor->health < (P_MobjSpawnHealth(actor) >> 1))
	{
		if(P_Random(RandomClass::Hexen) < 10)
		{
			lastfound = actor->special1.i;
			spot = P_FindMobjFromTID(KORAX_TELEPORT_TID, &lastfound);
			actor->special1.i = lastfound;
			if(spot)
			{
				P_Teleport(actor, spot->x, spot->y, spot->angle, true);
			}
		}
	}
}

extern "C" void A_KoraxStep(mobj_t* actor)
{
	A_Chase(actor);
}

extern "C" void A_KoraxStep2(mobj_t* actor)
{
	S_StartVoidSound(SfxId::HexenKoraxStep);
	A_Chase(actor);
}

extern "C" void A_KoraxBonePop(mobj_t* actor)
{
	mobj_t* mo;
	byte args[5];

	args[0] = args[1] = args[2] = args[3] = args[4] = 0;

	// Spawn 6 spirits equalangularly
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit1, ANG60 * 0,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit2, ANG60 * 1,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit3, ANG60 * 2,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit4, ANG60 * 3,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit5, ANG60 * 4,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);
	mo = P_SpawnMissileAngle(actor, MobjType::HexenKoraxSpirit6, ANG60 * 5,
		5 * FRACUNIT);
	if(mo)
		KSpiritInit(mo, actor);

	CheckACSPresent(255);
	P_StartACS(255, 0, args, actor, nullptr, 0); // Death script
}

extern "C" void KSpiritInit(mobj_t* spirit, mobj_t* korax)
{
	int i;
	mobj_t *tail, *next;

	spirit->health = KORAX_SPIRIT_LIFETIME;

	P_SetTarget(&spirit->special1.m, korax);            // Swarm around korax
	spirit->special2.i = 32 + (P_Random(RandomClass::Hexen) & 7); // Float bob index
	spirit->special_args[0] = 10;                       // initial turn value
	spirit->special_args[1] = 0;                        // initial look angle

	// Spawn a tail for spirit
	tail = P_SpawnMobj(spirit->x, spirit->y, spirit->z, MobjType::HexenHolyTail);
	P_SetTarget(&tail->special2.m, spirit); // parent
	for(i = 1; i < 3; i++)
	{
		next = P_SpawnMobj(spirit->x, spirit->y, spirit->z, MobjType::HexenHolyTail);
		P_SetMobjState(next, StateVariant(next->info->spawnstate, 1));
		P_SetTarget(&tail->special1.m, next);
		tail = next;
	}
	P_SetTarget(&tail->special1.m, nullptr); // last tail bit
}

extern "C" void A_KoraxDecide(mobj_t* actor)
{
	if(P_Random(RandomClass::Hexen) < 220)
	{
		P_SetMobjState(actor, StateId::HexenKoraxMissile1);
	}
	else
	{
		P_SetMobjState(actor, StateId::HexenKoraxCommand1);
	}
}

extern "C" void A_KoraxMissile(mobj_t* actor)
{
	MobjType type = MobjType::Null;
	SfxId sound = SfxId::None;

	S_StartMobjSound(actor, SfxId::HexenKoraxAttack);

	switch(P_Random(RandomClass::Hexen) % 6)
	{
		case 0:
			type = MobjType::HexenWraithfx1;
			sound = SfxId::HexenWraithMissileFire;
			break;
		case 1:
			type = MobjType::HexenDemonfx1;
			sound = SfxId::HexenDemonMissileFire;
			break;
		case 2:
			type = MobjType::HexenDemon2fx1;
			sound = SfxId::HexenDemonMissileFire;
			break;
		case 3:
			type = MobjType::HexenFiredemonFx6;
			sound = SfxId::HexenFiredAttack;
			break;
		case 4:
			type = MobjType::HexenCentaurFx;
			sound = SfxId::HexenCentaurleaderAttack;
			break;
		case 5:
			type = MobjType::HexenSerpentfx;
			sound = SfxId::HexenCentaurleaderAttack;
			break;
	}

	// Fire all 6 missiles at once
	S_StartVoidSound(sound);
	KoraxFire1(actor, type);
	KoraxFire2(actor, type);
	KoraxFire3(actor, type);
	KoraxFire4(actor, type);
	KoraxFire5(actor, type);
	KoraxFire6(actor, type);
}

extern "C" void A_KoraxCommand(mobj_t* actor)
{
	byte args[5];
	fixed_t x, y, z;
	angle_t ang;
	int numcommands;

	S_StartMobjSound(actor, SfxId::HexenKoraxCommand);

	// Shoot stream of lightning to ceiling
	ang = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_COMMAND_OFFSET, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_COMMAND_OFFSET, finesine[ang]);
	z = actor->z + KORAX_COMMAND_HEIGHT;
	P_SpawnMobj(x, y, z, MobjType::HexenKoraxBolt);

	args[0] = args[1] = args[2] = args[3] = args[4] = 0;

	if(actor->health <= (P_MobjSpawnHealth(actor) >> 1))
	{
		numcommands = 5;
	}
	else
	{
		numcommands = 4;
	}

	switch(P_Random(RandomClass::Hexen) % numcommands)
	{
		case 0:
			CheckACSPresent(250);
			P_StartACS(250, 0, args, actor, nullptr, 0);
			break;
		case 1:
			CheckACSPresent(251);
			P_StartACS(251, 0, args, actor, nullptr, 0);
			break;
		case 2:
			CheckACSPresent(252);
			P_StartACS(252, 0, args, actor, nullptr, 0);
			break;
		case 3:
			CheckACSPresent(253);
			P_StartACS(253, 0, args, actor, nullptr, 0);
			break;
		case 4:
			CheckACSPresent(254);
			P_StartACS(254, 0, args, actor, nullptr, 0);
			break;
	}
}

#define KORAX_DELTAANGLE			(85*ANG1)
#define KORAX_ARM_EXTENSION_SHORT	(40*FRACUNIT)
#define KORAX_ARM_EXTENSION_LONG	(55*FRACUNIT)

#define KORAX_ARM1_HEIGHT			(108*FRACUNIT)
#define KORAX_ARM2_HEIGHT			(82*FRACUNIT)
#define KORAX_ARM3_HEIGHT			(54*FRACUNIT)
#define KORAX_ARM4_HEIGHT			(104*FRACUNIT)
#define KORAX_ARM5_HEIGHT			(86*FRACUNIT)
#define KORAX_ARM6_HEIGHT			(53*FRACUNIT)

// Arm projectiles
//              arm positions numbered:
//                      1       top left
//                      2       middle left
//                      3       lower left
//                      4       top right
//                      5       middle right
//                      6       lower right

extern "C" void KoraxFire1(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM1_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void KoraxFire2(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM2_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void KoraxFire3(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM3_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void KoraxFire4(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM4_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void KoraxFire5(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM5_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void KoraxFire6(mobj_t* actor, MobjType type)
{
	angle_t ang;
	fixed_t x, y, z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM6_HEIGHT;
	P_SpawnKoraxMissile(x, y, z, actor, actor->target, static_cast<MobjType>(type));
}

extern "C" void A_KSpiritWeave(mobj_t* actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2.i >> 16;
	weaveZ = actor->special2.i & 0xFFFF;
	angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
	newX = actor->x - FixedMul(finecosine[angle],
		FloatBobOffsets[weaveXY] << 2);
	newY = actor->y - FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY] << 2);
	weaveXY = (weaveXY + (P_Random(RandomClass::Hexen) % 5)) & 63;
	newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 2);
	newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 2);
	P_TryMove(actor, newX, newY, false);
	actor->z -= FloatBobOffsets[weaveZ] << 1;
	weaveZ = (weaveZ + (P_Random(RandomClass::Hexen) % 5)) & 63;
	actor->z += FloatBobOffsets[weaveZ] << 1;
	actor->special2.i = weaveZ + (weaveXY << 16);
}

extern "C" void A_KSpiritSeeker(mobj_t* actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t* target;
	fixed_t newZ;
	fixed_t deltaZ;

	target = actor->special1.m;
	if(target == nullptr)
	{
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{
		// Turn clockwise
		actor->angle += delta;
	}
	else
	{
		// Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);

	if(!(leveltime & 15)
		|| actor->z > target->z + (target->info->height)
		|| actor->z + actor->height < target->z)
	{
		newZ = target->z + ((P_Random(RandomClass::Hexen) * target->info->height) >> 8);
		deltaZ = newZ - actor->z;
		if(abs(deltaZ) > 15 * FRACUNIT)
		{
			if(deltaZ > 0)
			{
				deltaZ = 15 * FRACUNIT;
			}
			else
			{
				deltaZ = -15 * FRACUNIT;
			}
		}
		dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
		dist = dist / actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = deltaZ / dist;
	}
	return;
}

extern "C" void A_KSpiritRoam(mobj_t* actor)
{
	if(actor->health-- <= 0)
	{
		S_StartMobjSound(actor, SfxId::HexenSpiritDie);
		P_SetMobjState(actor, StateId::HexenKspiritDeath1);
	}
	else
	{
		if(actor->special1.m)
		{
			A_KSpiritSeeker(actor, actor->special_args[0] * ANG1,
				actor->special_args[0] * ANG1 * 2);
		}
		A_KSpiritWeave(actor);
		if(P_Random(RandomClass::Hexen) < 50)
		{
			S_StartVoidSound(SfxId::HexenSpiritActive);
		}
	}
}

extern "C" void A_KBolt(mobj_t* actor)
{
	// Countdown lifetime
	if(actor->special1.i-- <= 0)
	{
		P_SetMobjState(actor, StateId::HexenNull);
	}
}

#define KORAX_BOLT_HEIGHT		48*FRACUNIT
#define KORAX_BOLT_LIFETIME		3

extern "C" void A_KBoltRaise(mobj_t* actor)
{
	mobj_t* mo;
	fixed_t z;

	// Spawn a child upward
	z = actor->z + KORAX_BOLT_HEIGHT;

	if((z + KORAX_BOLT_HEIGHT) < actor->ceilingz)
	{
		mo = P_SpawnMobj(actor->x, actor->y, z, MobjType::HexenKoraxBolt);
		if(mo)
		{
			mo->special1.i = KORAX_BOLT_LIFETIME;
		}
	}
	else
	{
		// Maybe cap it off here
	}
}
