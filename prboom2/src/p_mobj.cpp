// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Moving object handling. Spawn functions.
 */

#include <array>
#include <utility>

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "m_random.hpp"
#include "r_main.hpp"
#include "p_maputl.hpp"
#include "p_map.hpp"
#include "p_tick.hpp"
#include "sounds.hpp"
#include "s_sound.hpp"
#include "s_advsound.hpp"
#include "info.hpp"
#include "g_game.hpp"
#include "p_inter.hpp"
#include "p_user.hpp"
#include "lprintf.hpp"
#include "smooth.hpp"
#include "p_enemy.hpp"
#include "p_spec.hpp"
#include "g_overflow.hpp"
#include "e6y.hpp"//e6y

#include "dsda.hpp"
#include "dsda/aim.hpp"
#include "dsda/ambient.hpp"
#include "dsda/excmd.hpp"
#include "dsda/map_format.hpp"
#include "dsda/line_special.hpp"
#include "dsda/messenger.hpp"
#include "dsda/settings.hpp"
#include "dsda/skill_info.hpp"
#include "dsda/spawn_number.hpp"
#include "dsda/thing_id.hpp"
#include "dsda/tranmap.hpp"
#include "dsda/utility.hpp"

#include "heretic/sb_bar.hpp"

#include "hexen/po_man.hpp"

// heretic_note: static NUMSTATES arrays here - probably fine?
// NUMSTATES > HERETIC_NUMSTATES

//
// P_SetMobjState
// Returns true if the mobj is still present.
//

dboolean P_SetMobjState(mobj_t* mobj, StateId state)
{
	state_t* st;

	// killough 4/9/98: remember states seen, to detect cycles:

	extern StateId* seenstate_tab; // fast transition table
	StateId* seenstate;            // pointer to table
	static int recursion;             // detects recursion
	StateId i;                     // initial state
	dboolean ret;                     // return value
	StateId* tempstate = nullptr;     // for use with recursion

	if(raven) return Raven_P_SetMobjState(mobj, state);

	seenstate = seenstate_tab;
	i = state;
	ret = true;

	if(recursion++)                                                       // if recursion detected,
		seenstate = tempstate = static_cast<StateId*>(Z_Calloc(num_states, sizeof(StateId))); // allocate state table

	do
	{
		if(state == g_s_null)
		{
			mobj->state = nullptr;
			P_RemoveMobj(mobj);
			ret = false;
			break; // killough 4/9/98
		}

		st = &states[std::to_underlying(state)];
		mobj->state = st;
		mobj->tics = st->tics;
		mobj->sprite = st->sprite;
		mobj->frame = st->frame;

		// Modified handling.
		// Call action functions when the state is set

		if(st->action)
			reinterpret_cast<void (*)(mobj_t*)>(st->action)(mobj);

		seenstate[std::to_underlying(state)] = StateVariant(st->nextstate, 1); // killough 4/9/98

		state = st->nextstate;
	}
	while(!mobj->tics && seenstate[std::to_underlying(state)] == StateId::Null); // killough 4/9/98

	if(ret && !mobj->tics) // killough 4/9/98: detect state cycles
		Message::Add("Warning: State Cycle Detected");

	if(!--recursion)
		for(; (state = seenstate[std::to_underlying(i)]) != StateId::Null;
			i = StateVariant(state, -1))
			seenstate[std::to_underlying(i)] = StateId::Null; // killough 4/9/98: erase memory of states

	if(tempstate)
		Z_Free(tempstate);

	return ret;
}


//
// P_ExplodeMissile
//

void P_ExplodeMissile(mobj_t* mo)
{
	if(heretic && mo->type == MobjType::HereticWhirlwind)
	{
		if(++mo->special2.i < 60)
		{
			return;
		}
	}

	mo->momx = mo->momy = mo->momz = 0;

	P_SetMobjState(mo, static_cast<StateId>(mobjinfo[std::to_underlying(mo->type)].deathstate));

	if(!raven)
	{
		mo->tics -= P_Random(RandomClass::Explode) & 3;

		if(mo->tics < 1)
			mo->tics = 1;
	}

	mo->flags -= MobjFlag::Missile;

	if(!hexen)
	{
		if(mo->info->deathsound != SfxId::None)
			S_StartMobjSound(mo, mo->info->deathsound);
	}
	else
	{
		switch(mo->type)
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
				if(mo->info->deathsound != SfxId::None)
					S_StartMobjSound(mo, mo->info->deathsound);
				break;
		}
	}
}


//
// P_XYMovement
//
// Attempts to move something if it has momentum.
//

extern "C" void P_ApplyCompatibleSectorMovementSpecial(mobj_t* mo, int special)
{
	// nothing in doom
}

// heretic, hexen, and zdoom all use the same values
extern "C" void P_ApplyHereticSectorMovementSpecial(mobj_t* mo, int special)
{
	static int windTab[3] = {2048 * 5, 2048 * 10, 2048 * 25};

	if((mo->flags2 & MobjFlag2::WindThrust) != MobjFlag2{})
	{
		switch(special)
		{
			case std::to_underlying(ZDoomSectorSpecial::WindEastWeak):
			case std::to_underlying(ZDoomSectorSpecial::WindEastMedium):
			case std::to_underlying(ZDoomSectorSpecial::WindEastStrong):
				P_ThrustMobj(mo, 0, windTab[special - 40]);
				break;
			case std::to_underlying(ZDoomSectorSpecial::WindNorthWeak):
			case std::to_underlying(ZDoomSectorSpecial::WindNorthMedium):
			case std::to_underlying(ZDoomSectorSpecial::WindNorthStrong):
				P_ThrustMobj(mo, ANG90, windTab[special - 43]);
				break;
			case std::to_underlying(ZDoomSectorSpecial::WindSouthWeak):
			case std::to_underlying(ZDoomSectorSpecial::WindSouthMedium):
			case std::to_underlying(ZDoomSectorSpecial::WindSouthStrong):
				P_ThrustMobj(mo, ANG270, windTab[special - 46]);
				break;
			case std::to_underlying(ZDoomSectorSpecial::WindWestWeak):
			case std::to_underlying(ZDoomSectorSpecial::WindWestMedium):
			case std::to_underlying(ZDoomSectorSpecial::WindWestStrong):
				P_ThrustMobj(mo, ANG180, windTab[special - 49]);
				break;
		}
	}
}

static void P_XYMovement(mobj_t* mo)
{
	player_t* player;
	fixed_t xmove, ymove;

	//e6y
	fixed_t oldx, oldy; // phares 9/10/98: reducing bobbing/momentum on ice

	// heretic
	int special;

	if(!(mo->momx | mo->momy)) // Any momentum?
	{
		if((mo->flags & MobjFlag::SkullFly) != MobjFlag{})
		{
			StateId new_state;
			// the skull slammed into something

			mo->flags -= MobjFlag::SkullFly;
			mo->momz = 0;

			if(raven)
				new_state = mo->info->seestate;
			else
				new_state = mo->info->spawnstate;

			P_SetMobjState(mo, static_cast<StateId>(new_state));
		}
		return;
	}

	special = mo->subsector->sector->special;
	map_format.apply_sector_movement_special(mo, special);

	player = mo->player;

	if(mo->momx > MAXMOVE)
		mo->momx = MAXMOVE;
	else if(mo->momx < -MAXMOVE)
		mo->momx = -MAXMOVE;

	if(mo->momy > MAXMOVE)
		mo->momy = MAXMOVE;
	else if(mo->momy < -MAXMOVE)
		mo->momy = -MAXMOVE;

	xmove = mo->momx;
	ymove = mo->momy;

	oldx = mo->x; // phares 9/10/98: new code to reduce bobbing/momentum
	oldy = mo->y; // when on ice & up against wall. These will be compared
	// to your x,y values later to see if you were able to move

	do
	{
		fixed_t ptryx, ptryy;
		angle_t angle;

		// killough 8/9/98: fix bug in original Doom source:
		// Large negative displacements were never considered.
		// This explains the tendency for Mancubus fireballs
		// to pass through walls.
		// CPhipps - compatibility optioned

		if(xmove > MAXMOVE / 2 ||
			ymove > MAXMOVE / 2 ||
			(!comp[std::to_underlying(CompOption::MoveBlock)] && (xmove < -MAXMOVE / 2 || ymove < -MAXMOVE / 2)))
		{
			ptryx = mo->x + xmove / 2;
			ptryy = mo->y + ymove / 2;
			xmove >>= 1;
			ymove >>= 1;
		}
		else
		{
			ptryx = mo->x + xmove;
			ptryy = mo->y + ymove;
			xmove = ymove = 0;
		}

		// killough 3/15/98: Allow objects to drop off

		if(!P_TryMove(mo, ptryx, ptryy, true))
		{
			// blocked move

			// killough 8/11/98: bouncing off walls
			// killough 10/98:
			// Add ability for objects other than players to bounce on ice

			if(
				(mo->flags & MobjFlag::Missile) == MobjFlag{} &&
				mbf_features &&
				(
					(mo->flags & MobjFlag::Bounces) != MobjFlag{} ||
					(
						!player &&
						blockline &&
						variable_friction &&
						mo->z <= mo->floorz &&
						P_GetFriction(mo, nullptr) > ORIG_FRICTION
					)
				)
			)
			{
				if(blockline)
				{
					fixed_t r = ((blockline->dx >> FRACBITS) * mo->momx +
							(blockline->dy >> FRACBITS) * mo->momy) /
						((blockline->dx >> FRACBITS) * (blockline->dx >> FRACBITS) +
							(blockline->dy >> FRACBITS) * (blockline->dy >> FRACBITS));
					fixed_t x = FixedMul(r, blockline->dx);
					fixed_t y = FixedMul(r, blockline->dy);

					// reflect momentum away from wall

					mo->momx = x * 2 - mo->momx;
					mo->momy = y * 2 - mo->momy;

					// if under gravity, slow down in
					// direction perpendicular to wall.

					if((mo->flags & MobjFlag::NoGravity) == MobjFlag{})
					{
						mo->momx = (mo->momx + x) / 2;
						mo->momy = (mo->momy + y) / 2;
					}
				}
				else
					mo->momx = mo->momy = 0;
			}
			else if(player || (mo->flags2 & MobjFlag2::Slide) != MobjFlag2{}) // try to slide along it
			{
				if(BlockingMobj == nullptr || map_format.zdoom)
				{
					P_SlideMove(mo);
				}
				else
				{
					if(P_TryMove(mo, mo->x, ptryy, 0))
					{
						mo->momx = 0;
					}
					else if(P_TryMove(mo, ptryx, mo->y, 0))
					{
						mo->momy = 0;
					}
					else
					{
						mo->momx = mo->momy = 0;
					}
				}
			}
			else if((mo->flags & MobjFlag::Missile) != MobjFlag{})
			{
				if(hexen && (mo->flags2 & MobjFlag2::FloorBounce) != MobjFlag2{})
				{
					if(BlockingMobj)
					{
						if((BlockingMobj->flags2 & MobjFlag2::Reflective) != MobjFlag2{} ||
							((!BlockingMobj->player) &&
								((BlockingMobj->flags & MobjFlag::CountKill) == MobjFlag{})))
						{
							fixed_t speed;

							angle = R_PointToAngle2(BlockingMobj->x, BlockingMobj->y, mo->x, mo->y) +
								ANG1 * ((P_Random(RandomClass::Hexen) % 16) - 8);
							speed = P_AproxDistance(mo->momx, mo->momy);
							speed = FixedMul(speed, 0.75 * FRACUNIT);
							mo->angle = angle;
							angle >>= ANGLETOFINESHIFT;
							mo->momx = FixedMul(speed, finecosine[angle]);
							mo->momy = FixedMul(speed, finesine[angle]);
							if(mo->info->seesound != SfxId::None)
							{
								S_StartMobjSound(mo, mo->info->seesound);
							}
							return;
						}
						else
						{
							// Struck a player/creature
							P_ExplodeMissile(mo);
						}
					}
					else
					{
						// Struck a wall
						P_BounceWall(mo);
						switch(mo->type)
						{
							case MobjType::HexenSorcball1:
							case MobjType::HexenSorcball2:
							case MobjType::HexenSorcball3:
							case MobjType::HexenSorcfx1:
								break;
							default:
								if(mo->info->seesound != SfxId::None)
								{
									S_StartMobjSound(mo, mo->info->seesound);
								}
								break;
						}
						return;
					}
				}

				if(BlockingMobj && (BlockingMobj->flags2 & MobjFlag2::Reflective) != MobjFlag2{})
				{
					angle = R_PointToAngle2(BlockingMobj->x,
						BlockingMobj->y, mo->x, mo->y);

					// Change angle for delflection/reflection
					switch(BlockingMobj->type)
					{
						case MobjType::HexenCentaur:
						case MobjType::HexenCentaurleader:
							if(AngleAbs(AngleDifference(angle, BlockingMobj->angle)) >> 24 > 45)
								goto explode;
							if(mo->type == MobjType::HexenHolyFx)
								goto explode;
						// Drop through to sorcerer full reflection
						case MobjType::HexenSorcboss:
							// Deflection
							if(P_Random(RandomClass::Hexen) < 128)
								angle += ANG45;
							else
								angle -= ANG45;
							break;
						default:
							// Reflection
							angle += ANG1 * ((P_Random(RandomClass::Hexen) % 16) - 8);
							break;
					}

					// Reflect the missile along angle
					mo->angle = angle;
					angle >>= ANGLETOFINESHIFT;
					mo->momx = FixedMul(mo->info->speed >> 1, finecosine[angle]);
					mo->momy = FixedMul(mo->info->speed >> 1, finesine[angle]);
					if((mo->flags2 & MobjFlag2::SeekerMissile) != MobjFlag2{})
					{
						P_SetTarget(&mo->special1.m, mo->target);
					}
					P_SetTarget(&mo->target, BlockingMobj);
					return;
				}

			explode:
				// explode a missile
				if(ceilingline &&
					ceilingline->backsector &&
					ceilingline->backsector->ceilingpic == skyflatnum)
				{
					if(raven && mo->type == g_skullpop_mt)
					{
						mo->momx = mo->momy = 0;
						mo->momz = -FRACUNIT;
						return;
					}
					else if(hexen && mo->type == MobjType::HexenHolyFx)
					{
						P_ExplodeMissile(mo);
						return;
					}
					else if(demo_compatibility || // killough
						mo->z > ceilingline->backsector->ceilingheight)
					{
						// Hack to prevent missiles exploding
						// against the sky.
						// Does not handle sky floors.

						P_RemoveMobj(mo);
						return;
					}
				}

				// [RH] Don't explode on horizon lines.
				if(map_format.zdoom && blockline && blockline->special == std::to_underlying(ZDoomLineSpecial::LineHorizon))
				{
					P_RemoveMobj(mo);
					return;
				}

				P_ExplodeMissile(mo);
			}
			else // whatever else it is, it is now standing still in (x,y)
			{
				mo->momx = mo->momy = 0;
			}
		}
	}
	while(xmove || ymove);

	/* no friction for missiles or skulls ever, no friction when airborne */
	if((mo->flags & (MobjFlag::Missile | MobjFlag::SkullFly)) != MobjFlag{})
		return;

	if(
		mo->z > mo->floorz && (mo->flags2 & MobjFlag2::OnMobj) == MobjFlag2{} && (mo->flags & MobjFlag::Fly) == MobjFlag{} &&
		player && mo->player && map_aircontrol > 256
	)
	{
		mo->momx = FixedMul(mo->momx, map_airfriction);
		mo->momy = FixedMul(mo->momy, map_airfriction);

		player->momx = FixedMul(player->momx, map_airfriction);
		player->momy = FixedMul(player->momy, map_airfriction);
		return;
	}

	if(mo->z > mo->floorz &&
		(mo->flags & MobjFlag::Fly) == MobjFlag{} &&
		(mo->flags2 & MobjFlag2::Fly) == MobjFlag2{} &&
		(mo->flags2 & MobjFlag2::OnMobj) == MobjFlag2{} &&
		(!hexen || mo->type != MobjType::HexenBlasteffect))
		return;

	/* killough 8/11/98: add bouncers
	* killough 9/15/98: add objects falling off ledges
	* killough 11/98: only include bouncers hanging off ledges
	*/
	if(
		(
			((mo->flags & MobjFlag::Bounces) != MobjFlag{} && mo->z > mo->dropoffz) ||
			(mo->flags & MobjFlag::Corpse) != MobjFlag{} ||
			(mo->intflags & MobjIntFlag::Falling) != MobjIntFlag{}
		) &&
		(
			mo->momx > FRACUNIT / 4 ||
			mo->momx < -FRACUNIT / 4 ||
			mo->momy > FRACUNIT / 4 ||
			mo->momy < -FRACUNIT / 4
		) &&
		mo->floorz != mo->subsector->sector->floorheight
	)
		return; // do not stop sliding if halfway off a step with some momentum

	// killough 11/98:
	// Stop voodoo dolls that have come to rest, despite any
	// moving corresponding player, except in old demos:

	if(
		mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
		mo->momy > -STOPSPEED && mo->momy < STOPSPEED &&
		!(map_format.zdoom && (mo->intflags & MobjIntFlag::Scrolling) != MobjIntFlag{}) &&
		(
			!player ||
			!(player->cmd.forwardmove | player->cmd.sidemove) ||
			(
				player->mo != mo &&
				compatibility_level >= CompLevel::Lxdoom1 &&
				(comp[std::to_underlying(CompOption::VoodooScroller)] || (mo->intflags & MobjIntFlag::Scrolling) == MobjIntFlag{})
			)
		)
	)
	{
		// if in a walking frame, stop moving

		// killough 10/98:
		// Don't affect main player when voodoo dolls stop, except in old demos:

		if(player)
		{
			if(player->chickenTics)
			{
				if((unsigned)(player->mo->state - states - std::to_underlying(StateId::HereticChicplayRun1)) < 4)
				{
					P_SetMobjState(player->mo, StateId::HereticChicplay);
				}
			}
			else
			{
				if((unsigned)(player->mo->state - states - std::to_underlying(pclass[player->pclass].run_state)) < 4)
				{
					if(raven || player->mo == mo || compatibility_level >= CompLevel::Lxdoom1)
					{
						P_SetMobjState(player->mo, static_cast<StateId>(pclass[player->pclass].normal_state));
					}
				}
			}
		}

		mo->momx = mo->momy = 0;

		/* killough 10/98: kill any bobbing momentum too (except in voodoo dolls)
		* cph - DEMOSYNC - needs compatibility check?
		*/
		if(!raven && player && player->mo == mo)
			player->momx = player->momy = 0;
	}
	else
	{
		/* phares 3/17/98
		*
		* Friction will have been adjusted by friction thinkers for
		* icy or muddy floors. Otherwise it was never touched and
		* remained set at ORIG_FRICTION
		*
		* killough 8/28/98: removed inefficient thinker algorithm,
		* instead using touching_sectorlist in P_GetFriction() to
		* determine friction (and thus only when it is needed).
		*
		* killough 10/98: changed to work with new bobbing method.
		* Reducing player momentum is no longer needed to reduce
		* bobbing, so ice works much better now.
		*
		* cph - DEMOSYNC - need old code for Boom demos?
		*/

		//e6y
		if(compatibility_level <= CompLevel::Boom201 && !prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state)
		{
			if((mo->flags2 & MobjFlag2::Fly) != MobjFlag2{} && !(mo->z <= mo->floorz)
				&& (mo->flags2 & MobjFlag2::OnMobj) == MobjFlag2{})
			{
				mo->momx = FixedMul(mo->momx, FRICTION_FLY);
				mo->momy = FixedMul(mo->momy, FRICTION_FLY);
			}
			else if(
				hexen ? P_GetThingFloorType(mo) == FloorType::Ice : heretic ? special == 15 : false
			)
			{
				mo->momx = FixedMul(mo->momx, FRICTION_LOW);
				mo->momy = FixedMul(mo->momy, FRICTION_LOW);
			}
			else
			{
				// phares 3/17/98
				// Friction will have been adjusted by friction thinkers for icy
				// or muddy floors. Otherwise it was never touched and
				// remained set at ORIG_FRICTION
				mo->momx = FixedMul(mo->momx, mo->friction);
				mo->momy = FixedMul(mo->momy, mo->friction);
			}

			mo->friction = ORIG_FRICTION; // reset to normal for next tic
		}
		else if(compatibility_level <= CompLevel::Lxdoom1 && !prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state)
		{
			// phares 9/10/98: reduce bobbing/momentum when on ice & up against wall

			if((oldx == mo->x) && (oldy == mo->y)) // Did you go anywhere?
			{
				// No. Use original friction. This allows you to not bob so much
				// if you're on ice, but keeps enough momentum around to break free
				// when you're mildly stuck in a wall.
				mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
				mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
			}
			else
			{
				// Yes. Use stored friction.
				mo->momx = FixedMul(mo->momx, mo->friction);
				mo->momy = FixedMul(mo->momy, mo->friction);
			}
			mo->friction = ORIG_FRICTION; // reset to normal for next tic
		}
		else
		{
			fixed_t friction = P_GetFriction(mo, nullptr);

			mo->momx = FixedMul(mo->momx, friction);
			mo->momy = FixedMul(mo->momy, friction);

			/* killough 10/98: Always decrease player bobbing by ORIG_FRICTION.
			* This prevents problems with bobbing on ice, where it was not being
			* reduced fast enough, leading to all sorts of kludges being developed.
			*/

			if(player && player->mo == mo) /* Not voodoo dolls */
			{
				player->momx = FixedMul(player->momx, ORIG_FRICTION);
				player->momy = FixedMul(player->momy, ORIG_FRICTION);
			}
		}
	}
}

fixed_t P_MobjGravity(mobj_t* mo)
{
	return FixedMul(mo->subsector->sector->gravity, mo->gravity);
}

void P_AutoCorrectLookDir(player_t* player)
{
	if(allow_incompatibility && dsda_MouseLook())
	{
		return;
	}

	player->centering = true;
}

//
// P_ZMovement
//
// Attempt vertical movement.

static void P_ZMovement(mobj_t* mo)
{
	fixed_t gravity = P_MobjGravity(mo);

	/* killough 7/11/98:
	* BFG fireballs bounced on floors and ceilings in Pre-Beta Doom
	* killough 8/9/98: added support for non-missile objects bouncing
	* (e.g. grenade, mine, pipebomb)
	*/

	if((mo->flags & MobjFlag::Bounces) != MobjFlag{} && mo->momz)
	{
		mo->z += mo->momz;
		if(mo->z <= mo->floorz) /* bounce off floors */
		{
			mo->z = mo->floorz;
			if(mo->momz < 0)
			{
				mo->momz = -mo->momz;
				if((mo->flags & MobjFlag::NoGravity) == MobjFlag{}) /* bounce back with decay */
				{
					mo->momz = (mo->flags & MobjFlag::Float) != MobjFlag{}
						? // floaters fall slowly
						(mo->flags & MobjFlag::DropOff) != MobjFlag{}
						? // DROPOFF indicates rate
						FixedMul(mo->momz, (fixed_t)(FRACUNIT * .85))
						: FixedMul(mo->momz, (fixed_t)(FRACUNIT * .70))
						: FixedMul(mo->momz, (fixed_t)(FRACUNIT * .45));

					/* Bring it to rest below a certain speed */
					if(D_abs(mo->momz) <= mo->info->mass * (gravity * 4 / 256))
						mo->momz = 0;
				}

				/* killough 11/98: touchy objects explode on impact */
				if((mo->flags & MobjFlag::Touchy) != MobjFlag{} && (mo->intflags & MobjIntFlag::Armed) != MobjIntFlag{}
					&& mo->health > 0)
					P_DamageMobj(mo, nullptr, nullptr, mo->health);
				else if((mo->flags & MobjFlag::Float) != MobjFlag{} && sentient(mo))
					goto floater;
				return;
			}
		}
		else if(mo->z >= mo->ceilingz - mo->height)
		{
			/* bounce off ceilings */
			mo->z = mo->ceilingz - mo->height;
			if(mo->momz > 0)
			{
				if(mo->subsector->sector->ceilingpic != skyflatnum)
					mo->momz = -mo->momz; /* always bounce off non-sky ceiling */
				else if((mo->flags & MobjFlag::Missile) != MobjFlag{})
					P_RemoveMobj(mo); /* missiles don't bounce off skies */
				else if((mo->flags & MobjFlag::NoGravity) != MobjFlag{})
					mo->momz = -mo->momz; // bounce unless under gravity

				if((mo->flags & MobjFlag::Float) != MobjFlag{} && sentient(mo))
					goto floater;

				return;
			}
		}
		else
		{
			if((mo->flags & MobjFlag::NoGravity) == MobjFlag{}) /* free-fall under gravity */
				mo->momz -= mo->info->mass * (gravity / 256);

			if((mo->flags & MobjFlag::Float) != MobjFlag{} && sentient(mo)) goto floater;
			return;
		}

		/* came to a stop */
		mo->momz = 0;

		if((mo->flags & MobjFlag::Missile) != MobjFlag{})
		{
			if(ceilingline &&
				ceilingline->backsector &&
				ceilingline->backsector->ceilingpic == skyflatnum &&
				mo->z > ceilingline->backsector->ceilingheight)
				P_RemoveMobj(mo); /* don't explode on skies */
			else
				P_ExplodeMissile(mo);
		}

		if((mo->flags & MobjFlag::Float) != MobjFlag{} && sentient(mo)) goto floater;
		return;
	}

	/* killough 8/9/98: end bouncing object code */

	// check for smooth step up

	if(mo->player &&                                    //e6y: restoring original visual behaviour for demo_compatibility
		(demo_compatibility || mo->player->mo == mo) && // killough 5/12/98: exclude voodoo dolls
		mo->z < mo->floorz)
	{
		mo->player->viewheight -= mo->floorz - mo->z;
		mo->player->deltaviewheight = (g_viewheight - mo->player->viewheight) >> 3;
	}

	// adjust altitude

	mo->z += mo->momz;

floater:
	if((mo->flags & MobjFlag::Float) != MobjFlag{} && mo->target)

		// float down towards target if too close

		if((mo->flags & MobjFlag::SkullFly) == MobjFlag{} && (mo->flags & MobjFlag::InFloat) == MobjFlag{})
		{
			fixed_t delta;
			if(P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) <
				D_abs(delta = mo->target->z + (mo->height >> 1) - mo->z) * 3)
				mo->z += delta < 0 ? -FLOATSPEED : FLOATSPEED;
		}

	if(mo->player && (mo->flags & MobjFlag::Fly) != MobjFlag{} && (mo->z > mo->floorz))
	{
		mo->z += finesine[(FINEANGLES / 80 * gametic) & FINEMASK] / 8;
		mo->momz = FixedMul(mo->momz, FRICTION_FLY);
	}

	if(mo->player && (mo->flags2 & MobjFlag2::Fly) != MobjFlag2{} && !(mo->z <= mo->floorz)
		&& leveltime & 2)
	{
		mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
	}

	// clip movement

	if(mo->z <= mo->floorz)
	{
		// hit the floor

		if(raven)
		{
			if((mo->flags & MobjFlag::Missile) != MobjFlag{})
			{
				mo->z = mo->floorz;
				if((mo->flags2 & MobjFlag2::FloorBounce) != MobjFlag2{})
				{
					P_FloorBounceMissile(mo);
					return;
				}
				else if(
					raven && (
						mo->type == MobjType::HereticMntrfx2 ||
						mo->type == MobjType::HexenMntrfx2 ||
						mo->type == MobjType::HexenLightningFloor
					)
				)
				{
					// Minotaur floor fire can go up steps
					return;
				}
				else if(hexen && mo->type == MobjType::HexenHolyFx)
				{
					// The spirit struck the ground
					mo->momz = 0;
					P_HitFloor(mo);
					return;
				}
				else
				{
					if(hexen)
						P_HitFloor(mo);
					P_ExplodeMissile(mo);
					return;
				}
			}

			if(hexen && (mo->flags & MobjFlag::CountKill) != MobjFlag{}) // Blasted mobj falling
			{
				if(mo->momz < -(23 * FRACUNIT))
				{
					P_DamageMobj(mo, nullptr, nullptr, 10000);
				}
			}

			if(mo->z - mo->momz > mo->floorz)
			{
				// Spawn splashes, etc.
				P_HitFloor(mo);
			}
		}

		/* Note (id):
		*  somebody left this after the setting momz to 0,
		*  kinda useless there.
		* cph - This was the a bug in the linuxdoom-1.10 source which
		*  caused it not to sync Doom 2 v1.9 demos. Someone
		*  added the above comment and moved up the following code. So
		*  demos would desync in close lost soul fights.
		* cph - revised 2001/04/15 -
		* This was a bug in the Doom/Doom 2 source; the following code
		*  is meant to make charging lost souls bounce off of floors, but it
		*  was incorrectly placed after momz was set to 0.
		*  However, this bug was fixed in Doom95, Final/Ultimate Doom, and
		*  the v1.10 source release (which is one reason why it failed to sync
		*  some Doom2 v1.9 demos)
		* I've added a comp_soul compatibility option to make this behavior
		*  selectable for PrBoom v2.3+. For older demos, we do this here only
		*  if we're in a compatibility level above Doom 2 v1.9 (in which case we
		*  mimic the bug and do it further down instead)
		*/

		if(
			(mo->flags & MobjFlag::SkullFly) != MobjFlag{} &&
			(
				!comp[std::to_underlying(CompOption::Soul)] ||
				(
					compatibility_level > CompLevel::Doom219 &&
					compatibility_level < CompLevel::Prboom4
				)
			)
		)
			mo->momz = -mo->momz; // the skull slammed into something

		if(hexen) mo->z = mo->floorz;
		if(mo->momz < 0)
		{
			/* killough 11/98: touchy objects explode on impact */
			if((mo->flags & MobjFlag::Touchy) != MobjFlag{} && (mo->intflags & MobjIntFlag::Armed) != MobjIntFlag{} && mo->health > 0)
				P_DamageMobj(mo, nullptr, nullptr, mo->health);
			else
			{
				if((mo->flags2 & MobjFlag2::IceDamage) != MobjFlag2{} && mo->momz < -gravity * 8)
				{
					mo->tics = 1;
					mo->momx = 0;
					mo->momy = 0;
					mo->momz = 0;
					return;
				}
				// heretic_note: probably not necessary?
				if(!heretic && mo->player)
					mo->player->jumpTics = 7;
				if(
					mo->player && /* killough 5/12/98: exclude voodoo dolls */
					// e6y
					// Restoring original visual behaviour for demo_compatibility.
					// Viewheight of consoleplayer should be decreased for a moment
					// after voodoo doll hits the ground.
					// This additional condition makes sense only for plutonia complevel
					// when voodoo doll falls down after teleporting,
					// but can be applied globally for all demo_compatibility complevels,
					// because original sources do not exclude voodoo dolls from condition above,
					// but Boom does it.
					(demo_compatibility || mo->player->mo == mo) &&
					mo->momz < -gravity * 8 &&
					(mo->flags2 & MobjFlag2::Fly) == MobjFlag2{}
				)
				{
					// Squat down.
					// Decrease viewheight for a moment
					// after hitting the ground (hard),
					// and utter appropriate sound.

					mo->player->deltaviewheight = mo->momz >> 3;

					if(heretic)
					{
						S_StartMobjSound(mo, SfxId::HereticPlroof);
						P_AutoCorrectLookDir(mo->player);
					}
					else if(hexen)
					{
						if(mo->momz < -23 * FRACUNIT)
						{
							P_FallingDamage(mo->player);
							P_NoiseAlert(mo, mo);
						}
						else if(mo->momz < -gravity * 12 && !mo->player->morphTics)
						{
							S_StartMobjSound(mo, SfxId::HexenPlayerLand);
							switch(mo->player->pclass)
							{
								case PClass::Fighter:
									S_StartMobjSound(mo, SfxId::HexenPlayerFighterGrunt);
									break;
								case PClass::Cleric:
									S_StartMobjSound(mo, SfxId::HexenPlayerClericGrunt);
									break;
								case PClass::Mage:
									S_StartMobjSound(mo, SfxId::HexenPlayerMageGrunt);
									break;
								default:
									break;
							}
						}
						else if(P_GetThingFloorType(mo) < FloorType::Liquid && !mo->player->morphTics)
						{
							S_StartMobjSound(mo, SfxId::HexenPlayerLand);
						}
						P_AutoCorrectLookDir(mo->player);
					}
					//e6y: compatibility optioned
					else if(comp[std::to_underlying(CompOption::Sound)] || (mo->health > 0)) /* cph - prevent "oof" when dead */
						S_StartSound(mo, SfxId::Oof);
				}
				else if(hexen && mo->type >= MobjType::HexenPottery1 && mo->type <= MobjType::HexenPottery3)
				{
					P_DamageMobj(mo, nullptr, nullptr, 25);
				}
			}
			mo->momz = 0;
		}
		if(!hexen) mo->z = mo->floorz;

		/* cph 2001/04/15 -
		* This is the buggy lost-soul bouncing code referenced above.
		* We've already set momz = 0 normally by this point, so it's useless.
		* However we might still have upward momentum, in which case this will
		* incorrectly reverse it, so we might still need this for demo sync
		*/
		if((mo->flags & MobjFlag::SkullFly) != MobjFlag{} &&
			compatibility_level <= CompLevel::Doom219)
			mo->momz = -mo->momz; // the skull slammed into something

		if(mo->info->crashstate != StateId::Null && (mo->flags & MobjFlag::Corpse) != MobjFlag{} && (mo->flags2 & MobjFlag2::IceDamage) == MobjFlag2{})
		{
			P_SetMobjState(mo, mo->info->crashstate);
			return;
		}

		if(!raven && (mo->flags & MobjFlag::Missile) != MobjFlag{} && (mo->flags & MobjFlag::NoClip) == MobjFlag{})
		{
			P_ExplodeMissile(mo);
			return;
		}
	}
	else if((mo->flags2 & MobjFlag2::LoGrav) != MobjFlag2{})
	{
		if(mo->momz == 0)
			mo->momz = -(gravity >> 3) * 2;
		else
			mo->momz -= gravity >> 3;
	}
	else if((mo->flags & MobjFlag::NoGravity) == MobjFlag{})
	{
		if(mo->momz == 0)
			mo->momz = -gravity;
		mo->momz -= gravity;
	}

	if(mo->z + mo->height > mo->ceilingz)
	{
		/* cph 2001/04/15 -
		* Lost souls were meant to bounce off of ceilings;
		*  new comp_soul compatibility option added
		*/
		if(!comp[std::to_underlying(CompOption::Soul)] && (mo->flags & MobjFlag::SkullFly) != MobjFlag{})
			mo->momz = -mo->momz; // the skull slammed into something

		// hit the ceiling

		if(mo->momz > 0)
			mo->momz = 0;

		mo->z = mo->ceilingz - mo->height;

		if(hexen && (mo->flags2 & MobjFlag2::FloorBounce) != MobjFlag2{})
		{
			if(mo->info->seesound != SfxId::None)
			{
				S_StartMobjSound(mo, mo->info->seesound);
			}
			return;
		}

		/* cph 2001/04/15 -
		* We might have hit a ceiling but had downward momentum (e.g. ceiling is
		*  lowering on us), so for old demos we must still do the buggy
		*  momentum reversal here
		*/
		if(comp[std::to_underlying(CompOption::Soul)] && (mo->flags & MobjFlag::SkullFly) != MobjFlag{})
			mo->momz = -mo->momz; // the skull slammed into something

		if((mo->flags & MobjFlag::Missile) != MobjFlag{} && (hexen || (mo->flags & MobjFlag::NoClip) == MobjFlag{}))
		{
			if(hexen && mo->type == MobjType::HexenLightningCeiling)
			{
				return;
			}
			if(raven && mo->subsector->sector->ceilingpic == skyflatnum)
			{
				if(mo->type == g_skullpop_mt)
				{
					mo->momx = mo->momy = 0;
					mo->momz = -FRACUNIT;
				}
				else if(mo->type == MobjType::HexenHolyFx)
				{
					P_ExplodeMissile(mo);
				}
				else
				{
					P_RemoveMobj(mo);
				}
				return;
			}
			P_ExplodeMissile(mo);
			return;
		}
	}
}

//
// P_NightmareRespawn
//

static void P_NightmareRespawn(mobj_t* mobj)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	sector_t* sec;
	mobj_t* mo;
	mapthing_t* mthing;

	x = mobj->spawnpoint.x;
	y = mobj->spawnpoint.y;

	/* haleyjd: stupid nightmare respawning bug fix
	*
	* 08/09/00: compatibility added, time to ramble :)
	* This fixes the notorious nightmare respawning bug that causes monsters
	* that didn't spawn at level startup to respawn at the point (0,0)
	* regardless of that point's nature. SMMU and Eternity need this for
	* script-spawned things like Halif Swordsmythe, as well.
	*
	* cph - copied from eternity, alias comp_respawnfix
	*/
	if(!comp[std::to_underlying(CompOption::Respawn)] && !x && !y)
	{
		// spawnpoint was zeroed out, so use point of death instead
		x = mobj->x;
		y = mobj->y;
	}

	// something is occupying its position?

	if(!P_CheckPosition(mobj, x, y))
		return; // no respwan

	// spawn a teleport fog at old spot
	// because of removal of the body?

	mo = P_SpawnMobj(mobj->x,
		mobj->y,
		mobj->subsector->sector->floorheight + g_telefog_height,
		static_cast<MobjType>(g_mt_tfog));

	// initiate teleport sound

	S_StartSound(mo, g_sfx_telept);

	// spawn a teleport fog at the new spot

	sec = R_PointInSector(x, y);

	mo = P_SpawnMobj(x, y, sec->floorheight + g_telefog_height, static_cast<MobjType>(g_mt_tfog));

	S_StartSound(mo, g_sfx_telept);

	// spawn the new monster

	mthing = &mobj->spawnpoint;
	if((mobj->info->flags & MobjFlag::SpawnCeiling) != MobjFlag{})
		z = ONCEILINGZ;
	else
		z = ONFLOORZ;

	// inherit attributes from deceased one

	mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(mobj->type));
	mo->spawnpoint = mobj->spawnpoint;
	mo->angle = ANG45 * (mthing->angle / 45);
	mo->index = mobj->index;

	// "bug" in the respawn code for heretic
	// the chicken's return type is stored in special2.i
	// that value didn't exist in doom so is left uninitialized on respawn
	// we have to set this to the MT zero value for heretic
	if(heretic && mo->type == MobjType::HereticChicken)
		mo->special2.i = std::to_underlying(MobjType::HereticZero);

	if(hexen && mo->type == MobjType::HexenPig)
		mo->special2.i = std::to_underlying(MobjType::HexenZero);

	if((mthing->options & MapThingFlag::Ambush) != MapThingFlag{})
		mo->flags |= MobjFlag::Ambush;

	/* killough 11/98: transfer friendliness from deceased */
	mo->flags = (mo->flags - MobjFlag::Friend) | (mobj->flags & MobjFlag::Friend);
	mo->flags = mo->flags | MobjFlag::Ressurected; //e6y

	mo->reactiontime = 18;

	// remove the old monster,

	P_RemoveMobj(mobj);
}

fixed_t FloatBobOffsets[64] = {
	0, 51389, 102283, 152192,
	200636, 247147, 291278, 332604,
	370727, 405280, 435929, 462380,
	484378, 501712, 514213, 521763,
	524287, 521763, 514213, 501712,
	484378, 462380, 435929, 405280,
	370727, 332604, 291278, 247147,
	200636, 152192, 102283, 51389,
	-1, -51390, -102284, -152193,
	-200637, -247148, -291279, -332605,
	-370728, -405281, -435930, -462381,
	-484380, -501713, -514215, -521764,
	-524288, -521764, -514214, -501713,
	-484379, -462381, -435930, -405280,
	-370728, -332605, -291279, -247148,
	-200637, -152193, -102284, -51389
};

//
// P_MobjInterpolation
//

static dboolean mobj_interp_capture;

void P_UpdateMobjInterpolations()
{
	mobj_interp_capture = !mobj_interp_capture;
}

// [AR] Save mobj interpolation once per tic
// This fixes out-of-sync thinker order of operations (i.e. lift thinkers)
void P_MobjInterpolation(mobj_t* mobj)
{
	dboolean captured = (mobj->intflags & MobjIntFlag::InterpCapture) != MobjIntFlag{};

	if(captured == mobj_interp_capture)
		return;

	mobj->PrevX = mobj->x;
	mobj->PrevY = mobj->y;
	mobj->PrevZ = mobj->z;

	mobj->intflags = static_cast<MobjIntFlag>(std::to_underlying(mobj->intflags) ^ std::to_underlying(MobjIntFlag::InterpCapture));
}

//
// P_MobjThinker
//

static void PlayerLandedOnThing(mobj_t* mo, mobj_t* onmobj, fixed_t gravity);

void P_MobjThinker(mobj_t* mobj)
{
	// killough 11/98:
	// removed old code which looked at target references
	// (we use pointer reference counting now)

	if(mobj->type == MobjType::Musicsource)
	{
		MusInfoThinker(mobj);
		return;
	}

	P_MobjInterpolation(mobj);

	// momentum movement
	BlockingMobj = nullptr;
	if(mobj->momx | mobj->momy || (mobj->flags & MobjFlag::SkullFly) != MobjFlag{})
	{
		P_XYMovement(mobj);
		mobj->intflags -= MobjIntFlag::Scrolling;
		if(mobj->thinker.function != reinterpret_cast<think_t>(P_MobjThinker)) // cph - Must've been removed
			return;                                 // killough - mobj was removed
	}
	else if((mobj->flags2 & MobjFlag2::Blasted) != MobjFlag2{})
	{
		// Reset to not blasted when momentums are gone
		ResetBlasted(mobj);
	}

	if((mobj->flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
	{
		// Floating item bobbing motion
		mobj->z = mobj->floorz +
			(hexen ? mobj->special1.i : 0) + FloatBobOffsets[(mobj->health++) & 63];
	}
	else if(mobj->z != mobj->floorz || mobj->momz || BlockingMobj)
	{
		if((mobj->flags2 & MobjFlag2::PassMobj) != MobjFlag2{})
		{
			mobj_t* onmo;

			if(!(onmo = P_CheckOnmobj(mobj)))
			{
				P_ZMovement(mobj);

				// This bug is part of the original source
				if(hexen && mobj->player && (std::to_underlying(mobj->flags) & std::to_underlying(MobjFlag2::OnMobj)) != 0)
				{
					mobj->flags2 -= MobjFlag2::OnMobj;
				}

				if(map_format.zdoom)
				{
					mobj->flags2 -= MobjFlag2::OnMobj;
				}
			}
			else
			{
				if(mobj->player)
				{
					if(map_format.hexen)
					{
						fixed_t gravity = P_MobjGravity(mobj);

						if(hexen && mobj->momz < -gravity * 8 && (mobj->flags2 & MobjFlag2::Fly) == MobjFlag2{})
						{
							PlayerLandedOnThing(mobj, onmo, gravity);
						}
						if(onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
						{
							mobj->player->viewheight -= onmo->z + onmo->height - mobj->z;
							mobj->player->deltaviewheight = (g_viewheight - mobj->player->viewheight) >> 3;
							mobj->z = onmo->z + onmo->height;
							mobj->flags2 |= MobjFlag2::OnMobj;
							mobj->momz = 0;
						}
						else
						{
							// hit the bottom of the blocking mobj
							mobj->momz = 0;
						}
					}
					else
					{
						if(mobj->momz < 0)
						{
							mobj->flags2 |= MobjFlag2::OnMobj;
							mobj->momz = 0;
						}
						if(onmo->player || onmo->type == MobjType::HereticPod)
						{
							mobj->momx = onmo->momx;
							mobj->momy = onmo->momy;
							if(onmo->z < onmo->floorz)
							{
								mobj->z += onmo->floorz - onmo->z;
								if(onmo->player)
								{
									onmo->player->viewheight -= onmo->floorz - onmo->z;
									onmo->player->deltaviewheight = (g_viewheight - onmo->player->viewheight) >> 3;
								}
								onmo->z = onmo->floorz;
							}
						}
					}
				}
				else if(map_format.zdoom)
				{
					mobj->momz = 0;

					if(onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
					{
						mobj->z = onmo->z + onmo->height;
						mobj->flags2 |= MobjFlag2::OnMobj;
					}
				}
			}
		}
		else
			P_ZMovement(mobj);
		if(mobj->thinker.function != reinterpret_cast<think_t>(P_MobjThinker)) // cph - Must've been removed
			return;                                 // killough - mobj was removed
	}
	// raven_note: are the intflags irrelevant when compatibility is enabled?
	else if(!raven && !(mobj->momx | mobj->momy) && !sentient(mobj))
	{
		// non-sentient objects at rest
		mobj->intflags |= MobjIntFlag::Armed; // arm a mine which has come to rest

		// killough 9/12/98: objects fall off ledges if they are hanging off
		// slightly push off of ledge if hanging more than halfway off

		if(mobj->z > mobj->dropoffz &&       // Only objects contacting dropoff
			(mobj->flags & MobjFlag::NoGravity) == MobjFlag{} && // Only objects which fall
			!comp[std::to_underlying(CompOption::FallOff)])             // Not in old demos
			P_ApplyTorque(mobj);             // Apply torque
		else
			mobj->intflags -= MobjIntFlag::Falling, mobj->gear = 0; // Reset torque
	}

	if(map_format.mobj_in_special_sector(mobj))
		return;

	// cycle through states,
	// calling action functions at transitions
	if(mobj->tics != -1)
	{
		mobj->tics--;

		// you can cycle through multiple states in a tic

		// raven's cycle code is only here (i.e., not in every call to P_SetMobjState)
		// not sure about ramifications of moving loop inside all state calls
		if(raven)
		{
			while(!mobj->tics)
				if(!P_SetMobjState(mobj, static_cast<StateId>(mobj->state->nextstate)))
					return; // freed itself
		}
		else
		{
			if(!mobj->tics)
				if(!P_SetMobjState(mobj, static_cast<StateId>(mobj->state->nextstate)))
					return; // freed itself
		}
	}
	else
	{
		// check for nightmare respawn

		if((mobj->flags & MobjFlag::CountKill) == MobjFlag{})
			return;

		if(!skill_info.respawn_time)
			return;

		mobj->movecount++;

		if(mobj->movecount < skill_info.respawn_time * TICRATE)
			return;

		if(leveltime & 31)
			return;

		if(P_Random(RandomClass::Respawn) > 4)
			return;

		P_NightmareRespawn(mobj);
	}
}


// Certain functions assume that a mobj_t pointer is non-NULL,
// causing a crash in some situations where it is NULL.  Vanilla
// Doom did not crash because of the lack of proper memory
// protection. This function substitutes NULL pointers for
// pointers to a dummy mobj, to avoid a crash.
mobj_t* P_SubstNullMobj(mobj_t* mobj)
{
	if(mobj == nullptr)
	{
		static mobj_t dummy_mobj;

		dummy_mobj.x = 0;
		dummy_mobj.y = 0;
		dummy_mobj.z = 0;
		dummy_mobj.flags = MobjFlag{};

		mobj = &dummy_mobj;
	}

	return mobj;
}

/*
 * P_FindDoomedNum
 *
 * Finds a mobj type with a matching doomednum
 *
 * killough 8/24/98: rewrote to use hashing
 */

static dboolean P_IsTypeMatch(unsigned doomednum, int type)
{
	return (unsigned)mobjinfo[type].doomednum == doomednum &&
		(mobjinfo[type].visibility & map_format.visibility) != ThingVisibility{};
}

static PUREFUNC MobjType P_FindDoomedNum(unsigned type)
{
	static struct
	{
		int first, next;
	}* hash;
	int i;

	if(!hash)
	{
		hash = static_cast<decltype(hash)>(Z_Malloc(sizeof(*hash) * num_mobj_types));

		for(i = 0; i < num_mobj_types; i++)
			hash[i].first = num_mobj_types;

		for(i = mobj_types_zero; i < num_mobj_types; i++)
			if(mobjinfo[i].doomednum != -1)
			{
				unsigned h = (unsigned)mobjinfo[i].doomednum % num_mobj_types;
				hash[i].next = hash[h].first;
				hash[h].first = i;
			}
	}

	i = hash[type % num_mobj_types].first;
	while(i < num_mobj_types && !P_IsTypeMatch(type, i))
		i = hash[i].next;

	return static_cast<MobjType>(i);
}

dboolean P_SpawnProjectile(short thing_id, mobj_t* source, int spawn_num, angle_t angle,
	fixed_t speed, fixed_t vspeed, short dest_id, mobj_t* forcedest,
	int gravity, short new_thing_id)
{
	MobjType type;
	dboolean is_monster;
	dboolean success = false;
	thing_id_search_t search;
	thing_id_search_t dest_search;
	mobj_t* spawn_location;
	mobj_t* destination;
	mobj_t* new_mobj;

	type = dsda_ThingTypeFromSpawnNumber(spawn_num);

	if(std::to_underlying(type) == num_mobj_types)
		return false;

	is_monster = (type == MobjType::Skull || (mobjinfo[std::to_underlying(type)].flags & MobjFlag::CountKill) != MobjFlag{});

	if(nomonsters && is_monster)
		return false;

	dsda_ResetThingIDSearch(&search);
	while((spawn_location = dsda_FindMobjFromThingIDOrMobj(thing_id, source, &search)))
	{
		dsda_ResetThingIDSearch(&dest_search);
		destination = dsda_FindMobjFromThingIDOrMobj(dest_id, forcedest, &dest_search);

		if(!dest_id || destination)
		{
			do
			{
				new_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y, spawn_location->z, static_cast<MobjType>(type));
				if(new_mobj)
				{
					if(new_thing_id)
						dsda_AddMobjThingID(new_mobj, new_thing_id);

					if(new_mobj->info->seesound != SfxId::None)
					{
						S_StartMobjSound(new_mobj, new_mobj->info->seesound);
					}

					if(gravity)
					{
						new_mobj->flags -= MobjFlag::NoGravity;
						if(!is_monster && gravity == 1)
						{
							new_mobj->flags2 |= MobjFlag2::LoGrav;
						}
					}
					else
					{
						new_mobj->flags |= MobjFlag::NoGravity;
					}

					P_SetTarget(&new_mobj->target, spawn_location);

					if(destination)
					{
						fixed_t aimx, aimy, aimz;
						fixed_t slope;

						aimx = destination->x - new_mobj->x;
						aimy = destination->y - new_mobj->y;
						aimz = destination->z + destination->height / 2 - new_mobj->z;
						slope = FixedDiv(aimz, P_AproxDistance(aimx, aimy));
						new_mobj->angle = R_PointToAngle2(0, 0, aimx, aimy);
						new_mobj->momx = FixedMul(speed, finecosine[new_mobj->angle >> ANGLETOFINESHIFT]);
						new_mobj->momy = FixedMul(speed, finesine[new_mobj->angle >> ANGLETOFINESHIFT]);
						new_mobj->momz = FixedMul(speed, slope);
					}
					else
					{
						new_mobj->angle = angle;
						new_mobj->momx = FixedMul(speed, finecosine[new_mobj->angle >> ANGLETOFINESHIFT]);
						new_mobj->momy = FixedMul(speed, finesine[new_mobj->angle >> ANGLETOFINESHIFT]);
						new_mobj->momz = vspeed;
					}

					new_mobj->flags |= MobjFlag::Dropped; // Don't respawn

					if((new_mobj->flags & MobjFlag::Missile) != MobjFlag{})
					{
						if(P_CheckMissileSpawn(new_mobj))
						{
							success = true;
						}
					}
					else if(P_TestMobjLocation(new_mobj))
					{
						success = true;
					}
					else
					{
						dsda_WatchFailedSpawn(new_mobj);
						P_RemoveMobj(new_mobj);
					}
				}
			}
			while(dest_id && (destination = dsda_FindMobjFromThingIDOrMobj(dest_id, forcedest, &dest_search)));
		}
	}

	return success;
}

dboolean P_SpawnThing(short thing_id, mobj_t* source, int spawn_num,
	angle_t angle, dboolean fog, short new_thing_id)
{
	MobjType type;
	dboolean success = false;
	thing_id_search_t search;
	mobj_t* spawn_location;
	mobj_t* new_mobj;

	type = dsda_ThingTypeFromSpawnNumber(spawn_num);

	if(std::to_underlying(type) == num_mobj_types)
		return false;

	if(nomonsters && (type == MobjType::Skull || (mobjinfo[std::to_underlying(type)].flags & MobjFlag::CountKill) != MobjFlag{}))
		return false;

	dsda_ResetThingIDSearch(&search);
	while((spawn_location = dsda_FindMobjFromThingIDOrMobj(thing_id, source, &search)))
	{
		new_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y, spawn_location->z, static_cast<MobjType>(type));
		if(!P_TestMobjLocation(new_mobj))
		{
			dsda_WatchFailedSpawn(new_mobj);
			P_RemoveMobj(new_mobj);
		}
		else
		{
			new_mobj->angle = angle == ANGLE_MAX ? spawn_location->angle : angle;
			if(fog)
			{
				mobj_t* fog_mobj;
				fog_mobj = P_SpawnMobj(spawn_location->x, spawn_location->y,
					spawn_location->z + TELEFOGHEIGHT, static_cast<MobjType>(g_mt_tfog));
				S_StartMobjSound(fog_mobj, g_sfx_telept);
			}
			if(new_thing_id)
				dsda_AddMobjThingID(new_mobj, new_thing_id);
			new_mobj->flags |= MobjFlag::Dropped; // Don't respawn
			success = true;
		}
	}

	return success;
}

int P_MobjSpawnHealth(const mobj_t* mobj)
{
	int result;

	if(mobj->type == MobjType::Skull || (mobj->flags & MobjFlag::CountKill) != MobjFlag{})
	{
		if((mobj->flags & MobjFlag::Friend) != MobjFlag{})
		{
			if(skill_info.friend_health_factor)
			{
				result = FixedMul(mobj->info->spawnhealth, skill_info.friend_health_factor);

				return result > 0 ? result : 1;
			}
		}
		else if(skill_info.monster_health_factor)
		{
			result = FixedMul(mobj->info->spawnhealth, skill_info.monster_health_factor);

			return result > 0 ? result : 1;
		}
	}

	return mobj->info->spawnhealth;
}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, MobjType type)
{
	mobj_t* mobj;
	state_t* st;
	mobjinfo_t* info;

	mobj = static_cast<mobj_t*>(Z_MallocLevel(sizeof(*mobj)));
	memset(mobj, 0, sizeof (*mobj));
	info = &mobjinfo[std::to_underlying(type)];
	mobj->type = type;
	mobj->info = info;
	mobj->x = x;
	mobj->y = y;
	mobj->radius = info->radius;
	mobj->height = info->height; // phares
	mobj->flags = info->flags;
	mobj->flags2 = info->flags2;
	if(raven) mobj->damage = info->damage;

	/* killough 8/23/98: no friends, bouncers, or touchy things in old demos */
	if(!mbf_features)
		mobj->flags -= (MobjFlag::Bounces | MobjFlag::Friend | MobjFlag::Touchy);
	else if(type == g_mt_player)  // Except in old demos, players
		mobj->flags |= MobjFlag::Friend; // are always friends.

	// TODO: possible mapinfo "passover" flag
	//if (mobj->flags & MF_SOLID)
	//mobj->flags2 |= MF2_PASSMOBJ;

	mobj->health = P_MobjSpawnHealth(mobj);

	if(!(skill_info.flags & SI_INSTANT_REACTION))
		mobj->reactiontime = info->reactiontime;

	if(type != ZMT_AMBIENTSOUND)
		mobj->lastlook = P_Random(RandomClass::Lastlook) % g_maxplayers;

	// do not set the state with P_SetMobjState,
	// because action routines can not be called yet

	st = &states[std::to_underlying(info->spawnstate)];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
	mobj->touching_sectorlist = nullptr; // NULL head of sector list // phares 3/13/98

	// set subsector and/or block links

	P_SetThingPosition(mobj);

	mobj->dropoffz = /* killough 11/98: for tracking dropoffs */
		mobj->floorz = mobj->subsector->sector->floorheight;
	mobj->ceilingz = mobj->subsector->sector->ceilingheight;

	if(z == ONFLOORZ)
	{
		mobj->z = mobj->floorz;
	}
	else if(z == ONCEILINGZ)
	{
		mobj->z = mobj->ceilingz - mobj->height;
	}
	else if(raven && z == FLOATRANDZ)
	{
		fixed_t space;

		space = ((mobj->ceilingz) - (mobj->height)) - mobj->floorz;
		if(space > 48 * FRACUNIT)
		{
			space -= 40 * FRACUNIT;
			mobj->z =
				((space * P_Random(RandomClass::Heretic)) >> 8) + mobj->floorz + 40 * FRACUNIT;
		}
		else
		{
			mobj->z = mobj->floorz;
		}
	}
	else if(hexen && (mobj->flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
	{
		mobj->z = mobj->floorz + z; // artifact z passed in as height
	}
	else
	{
		mobj->z = z;
	}

	if(hexen)
	{
		if((mobj->flags2 & MobjFlag2::FootClip) != MobjFlag2{}
			&& P_GetThingFloorType(mobj) >= FloorType::Liquid
			&& mobj->z == mobj->subsector->sector->floorheight)
		{
			mobj->floorclip = 10 * FRACUNIT;
		}
		else
		{
			mobj->floorclip = 0;
		}
	}
	else
	{
		if((mobj->flags2 & MobjFlag2::FootClip) != MobjFlag2{}
			&& P_GetThingFloorType(mobj) != FloorType::Solid
			&& mobj->floorz == mobj->subsector->sector->floorheight)
		{
			mobj->flags2 |= MobjFlag2::FeetAreClipped;
		}
		else
		{
			mobj->flags2 -= MobjFlag2::FeetAreClipped;
		}
	}

	mobj->PrevX = mobj->x;
	mobj->PrevY = mobj->y;
	mobj->PrevZ = mobj->z;

	if(mobj_interp_capture)
		mobj->intflags |= MobjIntFlag::InterpCapture;

	mobj->thinker.function = reinterpret_cast<think_t>(P_MobjThinker);

	//e6y
	mobj->friction = ORIG_FRICTION; // phares 3/17/98
	mobj->gravity = map_gravity;
	mobj->alpha = 1.f;
	mobj->index = -1;

	mobj->target = mobj->tracer = mobj->lastenemy = nullptr;
	P_AddThinker(&mobj->thinker);
	if(((mobj->flags ^ MobjFlag::CountKill) & (MobjFlag::Friend | MobjFlag::CountKill)) == MobjFlag{})
		totallive++;

	dsda_WatchSpawn(mobj);

	return mobj;
}


static mapthing_t itemrespawnque[ITEMQUESIZE];
static int itemrespawntime[ITEMQUESIZE];
int iquehead;
int iquetail;


//
// P_RemoveMobj
//

void P_RemoveMobj(mobj_t* mobj)
{
	if(raven) // so short, just putting it here
	{
		if(hexen)
		{
			// Remove from creature queue
			if((mobj->flags & MobjFlag::CountKill) != MobjFlag{} && (mobj->flags & MobjFlag::Corpse) != MobjFlag{})
			{
				A_DeQueueCorpse(mobj);
			}
		}

		if(map_format.thing_id && mobj->tid)
		{
			map_format.remove_mobj_thing_id(mobj);
		}

		P_UnsetThingPosition(mobj);
		if(sector_list)
		{
			P_DelSeclist(sector_list);
			sector_list = nullptr;
		}
		S_StopSound(mobj);
		P_RemoveThinker((thinker_t*)mobj);
		return;
	}

	if((mobj->flags & MobjFlag::Special) != MobjFlag{}
		&& (mobj->flags & MobjFlag::Dropped) == MobjFlag{}
		&& (mobj->type != MobjType::Inv)
		&& (mobj->type != MobjType::Ins))
	{
		itemrespawnque[iquehead] = mobj->spawnpoint;
		itemrespawntime[iquehead] = leveltime;
		iquehead = (iquehead + 1) & (ITEMQUESIZE - 1);

		// lose one off the end?

		if(iquehead == iquetail)
			iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
	}

	if(map_format.thing_id && mobj->tid)
	{
		map_format.remove_mobj_thing_id(mobj);
	}

	// unlink from sector and block lists

	P_UnsetThingPosition(mobj);

	// Delete all nodes on the current sector_list               phares 3/16/98

	if(sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = nullptr;
	}

	// stop any playing sound

	// [FG] removed map objects may finish their sounds
	if(full_sounds)
		S_UnlinkSound(mobj);
	else
		S_StopSound(mobj);

	// killough 11/98:
	//
	// Remove any references to other mobjs.
	//
	// Older demos might depend on the fields being left alone, however,
	// if multiple thinkers reference each other indirectly before the
	// end of the current tic.
	// CPhipps - only leave dead references in old demos; I hope lxdoom_1 level
	// demos are rare and don't rely on this. I hope.

	if(compatibility_level >= CompLevel::Lxdoom1 || allow_incompatibility)
	{
		P_SetTarget(&mobj->target, nullptr);
		P_SetTarget(&mobj->tracer, nullptr);
		P_SetTarget(&mobj->lastenemy, nullptr);
	}
	// free block

	P_RemoveThinker(&mobj->thinker);
}

void P_RemoveMonsters()
{
	thinker_t* th;
	mobj_t* mobj;

	for(th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if(th->function != reinterpret_cast<think_t>(P_MobjThinker))
			continue;

		mobj = (mobj_t*)th;
		if(mobj->player)
			continue;

		if(
			mobj->type == MobjType::Skull ||
			(mobj->flags & MobjFlag::CountKill) != MobjFlag{} ||
			(mobj->target && !mobj->target->player)
		)
			P_RemoveMobj(mobj);
	}

	P_CleanThinkers();
}

//
// P_RespawnSpecials
//

void P_RespawnSpecials()
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	sector_t* sec;
	mobj_t* mo;
	mapthing_t* mthing;
	MobjType i;

	// only respawn items in deathmatch

	if(deathmatch != 2)
		return;

	// nothing left to respawn?

	if(iquehead == iquetail)
		return;

	// wait at least 30 seconds

	if(leveltime - itemrespawntime[iquetail] < 30 * TICRATE)
		return;

	mthing = &itemrespawnque[iquetail];

	x = mthing->x;
	y = mthing->y;

	// spawn a teleport fog at the new spot

	sec = R_PointInSector(x, y);
	mo = P_SpawnMobj(x, y, sec->floorheight, MobjType::Ifog);
	S_StartSound(mo, SfxId::Itmbk);

	// find which type to spawn

	/* killough 8/23/98: use table for faster lookup */
	i = P_FindDoomedNum(mthing->type);

	// spawn it

	if((mobjinfo[std::to_underlying(i)].flags & MobjFlag::SpawnCeiling) != MobjFlag{})
		z = ONCEILINGZ;
	else
		z = ONFLOORZ;

	mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(i));
	mo->spawnpoint = *mthing;
	mo->angle = ANG45 * (mthing->angle / 45);

	// pull it from the queue

	iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//

extern byte playernumtotrans[MAX_MAXPLAYERS];

void P_SpawnPlayer(int n, const mapthing_t* mthing)
{
	player_t* p;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	mobj_t* mobj;
	int i;

	// e6y
	// playeringame overflow detection
	// it detects and emulates overflows on vex6d.wad\bug_wald(toke).lmp, etc.
	// http://www.doom2.net/doom2/research/runningbody.zip
	if(PlayeringameOverrun(mthing))
		return;

	// not playing?

	if(!playeringame[n])
		return;

	p = &players[n];

	if(p->playerstate == PlayerState::Reborn)
		G_PlayerReborn(n);

	/* cph 2001/08/14 - use the options field of memorised player starts to
	* indicate whether the start really exists in the level.
	*/
	if(mthing->options == MapThingFlag{})
		Log::Fatal("P_SpawnPlayer: attempt to spawn player at unavailable start point");

	x = mthing->x;
	y = mthing->y;
	z = ONFLOORZ;

	if(hexen)
	{
		if(randomclass && deathmatch)
		{
			p->pclass = static_cast<PClass>(1 + P_Random(RandomClass::Hexen) % 3);
			if(p->pclass == PlayerClass[mthing->type - 1])
			{
				p->pclass = static_cast<PClass>(1 + ((std::to_underlying(p->pclass) + 1) % 3));
			}
			PlayerClass[mthing->type - 1] = p->pclass;
			SB_SetClassData();
		}
		else
		{
			p->pclass = PlayerClass[mthing->type - 1];
		}
		switch(p->pclass)
		{
			case PClass::Fighter:
				mobj = P_SpawnMobj(x, y, z, MobjType::HexenPlayerFighter);
				break;
			case PClass::Cleric:
				mobj = P_SpawnMobj(x, y, z, MobjType::HexenPlayerCleric);
				break;
			case PClass::Mage:
				mobj = P_SpawnMobj(x, y, z, MobjType::HexenPlayerMage);
				break;
			default:
				Log::Fatal("P_SpawnPlayer: Unknown class type");
				return;
		}
	}
	else
		mobj = P_SpawnMobj(x, y, z, static_cast<MobjType>(g_mt_player));

	// TODO: possible "use player start z" mapinfo flag
	//   mobj->z += mthing->height;

	if(map_format.zdoom)
		P_AdjustZLimits(mobj);

	// set color translations for player sprites
	if(hexen)
	{
		if(p->pclass == PClass::Fighter && (mthing->type == 1 || mthing->type == 3))
		{
			// The first type should be blue, and the third should be the
			// Fighter's original gold color
			if(mthing->type == 1)
			{
				mobj->flags |= MobjFlag::Translation2;
			}
		}
		else if(mthing->type > 1)
		{
			// Set color translation bits for player sprites
			mobj->flags |= MobjTranslationFlags(mthing->type - 1);
		}
	}
	else
		mobj->flags |= MobjTranslationFlags(playernumtotrans[n]);

	if(leave_data.flags & LF_SET_ANGLE)
		mobj->angle = leave_data.angle;
	else
		mobj->angle = ANG45 * (mthing->angle / 45);

	mobj->player = p;
	mobj->health = p->health;
	mobj->player->prev_viewangle = mobj->angle;

	p->mo = mobj;
	p->playerstate = PlayerState::Live;
	p->refire = 0;
	p->damagecount = 0;
	p->bonuscount = 0;
	p->poisoncount = 0;
	p->chickenTics = 0;
	p->morphTics = 0;
	p->rain1 = nullptr;
	p->rain2 = nullptr;
	p->extralight = 0;
	p->fixedcolormap = 0;
	p->viewheight = g_viewheight;

	p->momx = p->momy = 0; // killough 10/98: initialize bobbing to 0.

	// setup gun psprite

	P_SetupPsprites(p);

	// give all cards in death match mode

	if(deathmatch)
	{
		for(i = 0; i < std::to_underlying(Card::Count); i++)
			p->cards[i] = true;
		if(p == &players[consoleplayer])
			p->ravenkeys = 7;
	}
	else if(p == &players[consoleplayer] && !hexen)
		p->ravenkeys = 0;

	R_SmoothPlaying_Reset(p); // e6y
}

/*
 * P_IsDoomnumAllowed()
 * Based on code taken from P_LoadThings() in src/p_setup.c  Return TRUE
 * if the thing in question is expected to be available in the gamemode used.
 */

dboolean P_IsDoomnumAllowed(int doomnum)
{
	// Do not spawn cool, new monsters if !commercial
	if(!raven && gamemode != GameMode::Commercial)
		switch(doomnum)
		{
			case 64: // Archvile
			case 65: // Former Human Commando
			case 66: // Revenant
			case 67: // Mancubus
			case 68: // Arachnotron
			case 69: // Hell Knight
			case 71: // Pain Elemental
			case 84: // Wolf SS
			case 88: // Boss Brain
			case 89: // Boss Shooter
				return false;
		}

	return true;
}

//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//

static dboolean P_ShouldSpawnPlayer(const mapthing_t* mthing)
{
	return !deathmatch && (map_format.zdoom ? mthing->special_args[0] == leave_data.position : !mthing->special_args[0]);
}

static dboolean P_ShouldSpawnMapThing(MapThingFlag options)
{
	MapThingFlag spawnMask;
	dboolean spawn_multi;

	spawn_multi = skill_info.flags & SI_SPAWN_MULTI;

	if(map_format.hexen)
	{
		// Check current game type with spawn flags
		if(netgame == false)
		{
			spawnMask = MapThingFlag::GSingle;

			if(spawn_multi)
				spawnMask |= MapThingFlag::GCoop;
		}
		else if(deathmatch)
		{
			spawnMask = MapThingFlag::GDeathmatch;
		}
		else
		{
			spawnMask = MapThingFlag::GCoop;
		}

		if((options & spawnMask) == MapThingFlag{})
		{
			return false;
		}
	}
	else
	{
		/* jff "not single" thing flag */
		if(!spawn_multi && !netgame && (options & MapThingFlag::NotSingle) != MapThingFlag{})
			return false;

		//jff 3/30/98 implement "not deathmatch" thing flag
		if(netgame && deathmatch && (options & MapThingFlag::NotDm) != MapThingFlag{})
			return false;

		//jff 3/30/98 implement "not cooperative" thing flag
		if((spawn_multi || netgame) && !deathmatch && (options & MapThingFlag::NotCoop) != MapThingFlag{})
			return false;
	}

	// check for appropriate skill level
	if(
		skill_info.spawn_filter == 1 ? (options & MapThingFlag::Skill1) == MapThingFlag{} : skill_info.spawn_filter == 2 ? (options & MapThingFlag::Skill2) == MapThingFlag{} : skill_info.spawn_filter == 3 ? (options & MapThingFlag::Skill3) == MapThingFlag{} : skill_info.spawn_filter == 4 ? (options & MapThingFlag::Skill4) == MapThingFlag{} : (options & MapThingFlag::Skill5) == MapThingFlag{}
	)
		return false;

	if(hexen)
	{
		static constexpr std::array classFlags {
			MapThingFlag{}, // null class
			MapThingFlag::Fighter,
			MapThingFlag::Cleric,
			MapThingFlag::Mage
		};

		int i;

		// Check current character classes with spawn flags
		if(netgame == false)
		{
			// Single player
			if((options & classFlags[std::to_underlying(PlayerClass[0])]) == MapThingFlag{})
			{
				// Not for current class
				return false;
			}
		}
		else if(deathmatch == false)
		{
			// Cooperative
			spawnMask = MapThingFlag{};
			for(i = 0; i < g_maxplayers; i++)
			{
				if(playeringame[i])
				{
					spawnMask |= classFlags[std::to_underlying(PlayerClass[i])];
				}
			}
			if((options & spawnMask) == MapThingFlag{})
			{
				return false;
			}
		}
	}

	return true;
}

void P_TrySpawnPlayer(const mapthing_t* mthing, int player)
{
	mapthing_t* player_start;

	// Hexen stored these regardless of arg1, but only used the relevant ones depending on game type
	// this caused a crash - HEXDD MAP39 players have arg1 of 99
	if(mthing->special_args[0] < MAX_PLAYER_STARTS)
		player_start = &playerstarts[mthing->special_args[0]][player];
	else
		player_start = &playerstarts[0][player];

	*player_start = *mthing;
	player_start->type = player + 1;

	/* cph 2006/07/24 - use the otherwise-unused options field to flag that
	* this start is present (so we know which elements of the array are filled
	* in, in effect). Also note that the call below to P_SpawnPlayer must use
	* the playerstarts version with this field set */
	player_start->options = MapThingFlag::Easy;

	if(P_ShouldSpawnPlayer(mthing))
		P_SpawnPlayer(player, player_start);
}

static int P_TypeToPlayer(int type)
{
	if(type <= 4 && type > 0)
		return type - 1;

	if(map_format.hexen && type >= 9100 && type <= 9103)
		return 4 + type - 9100;

	return -1;
}

mobj_t* P_SpawnMapThing(const mapthing_t* mthing, int index)
{
	MobjType i;
	int player;
	mobj_t* mobj;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	MapThingFlag options = mthing->options; /* cph 2001/07/07 - make writable copy */
	short thingtype = mthing->type;
	int iden_num = 0;

	// killough 2/26/98: Ignore type-0 things as NOPs
	// phares 5/14/98: Ignore Player 5-8 starts (for now)

	if(!hexen)
	{
		switch(thingtype)
		{
			case 0:
			case DEN_PLAYER5:
			case DEN_PLAYER6:
			case DEN_PLAYER7:
			case DEN_PLAYER8:
				return nullptr;
		}
	}

	// killough 11/98: clear flags unused by Doom
	//
	// We clear the flags unused in Doom if we see flag mask 256 set, since
	// it is reserved to be 0 under the new scheme. A 1 in this reserved bit
	// indicates it's a Doom wad made by a Doom editor which puts 1's in
	// bits that weren't used in Doom (such as HellMaker wads). So we should
	// then simply ignore all upper bits.

	if(
		!map_format.hexen &&
		(
			demo_compatibility ||
			(
				compatibility_level >= CompLevel::Lxdoom1 &&
				(options & MapThingFlag::Reserved) != MapThingFlag{}
			)
		)
	)
	{
		if(!demo_compatibility) // cph - Add warning about bad thing flags
			Log::Warn("P_SpawnMapThing: correcting bad flags ({}) (thing type {})\n", std::to_underlying(options), thingtype);
		options = options & (MapThingFlag::Skill1 | MapThingFlag::Skill2 | MapThingFlag::Skill3 | MapThingFlag::Skill4 | MapThingFlag::Skill5 | MapThingFlag::Ambush | MapThingFlag::NotSingle);
	}

	// count deathmatch start positions

	// doom2.exe has at most 10 deathmatch starts
	if(thingtype == 11)
	{
		if(compatibility && deathmatch_p - deathmatchstarts >= 10)
		{
			return nullptr;
		}
		else
		{
			// 1/11/98 killough -- new code removes limit on deathmatch starts:

			size_t offset = deathmatch_p - deathmatchstarts;

			if(offset >= num_deathmatchstarts)
			{
				num_deathmatchstarts = num_deathmatchstarts ? num_deathmatchstarts * 2 : 16;
				deathmatchstarts = static_cast<mapthing_t*>(Z_Realloc(deathmatchstarts,
					num_deathmatchstarts *
					sizeof(*deathmatchstarts)));
				deathmatch_p = deathmatchstarts + offset;
			}
			memcpy(deathmatch_p++, mthing, sizeof(*mthing));
			(deathmatch_p - 1)->options = MapThingFlag::Easy;

			return nullptr;
		}
	}

	if(PO_Detect(mthing->type))
	{
		return nullptr;
	}

	// check for players specially
	if((player = P_TypeToPlayer(thingtype)) >= 0)
	{
		// TODO: possible "filter starts" mapinfo flag
		//   if (!P_ShouldSpawnMapThing(options))
		//     return nullptr;

		// killough 7/19/98: Marine's best friend :)
		if(
			!netgame &&
			player > 0 && player <= dogs &&
			!players[player].secretcount
		)
		{
			// use secretcount to avoid multiple dogs in case of multiple starts
			players[player].secretcount = 1;

			// killough 10/98: force it to be a friend
			options |= (map_format.zdoom ? MapThingFlag::Friendly : MapThingFlag::Friend);
			if(HelperThing != -1) // haleyjd 9/22/99: deh substitution
			{
				int type = HelperThing - 1;
				if(type >= 0 && type < num_mobj_types)
				{
					i = static_cast<MobjType>(type);
				}
				else
				{
					Message::Add("Invalid value {} for helper, ignored.", HelperThing);
					i = MobjType::Dogs;
				}
			}
			else
			{
				i = MobjType::Dogs;
			}
			goto spawnit;
		}

		P_TrySpawnPlayer(mthing, player);

		return nullptr;
	}

	// MAP_FORMAT_TODO: need to verify heretic types
	if(heretic)
	{
		// Ambient sound sequences
		if(mthing->type >= 1200 && mthing->type < 1300)
		{
			P_AddAmbientSfx(mthing->type - 1200);
			return nullptr;
		}

		// Check for boss spots
		if(mthing->type == 56) // Monster_BossSpot
		{
			P_AddBossSpot(mthing->x, mthing->y,
				ANG45 * (mthing->angle / 45));
			return nullptr;
		}
	}

	if(map_format.hexen)
	{
		if(mthing->type >= 1400 && mthing->type < 1410)
		{
			R_PointInSector(
				mthing->x, mthing->y
			)->seqType = static_cast<SeqType>(mthing->type - 1400);
			return nullptr;
		}
	}

	if(!P_ShouldSpawnMapThing(options))
		return nullptr;

	if(!raven && thingtype >= 14001 && thingtype <= 14064)
	{
		iden_num = thingtype - 14000; // Ambient sound id
		thingtype = 14064;            // ZMT_AMBIENTSOUND
	}

	if(!raven && thingtype == 14065)
	{
		iden_num = mthing->special_args[0]; // Ambient sound id
		thingtype = 14064;                  // ZMT_AMBIENTSOUND
	}

	if(!raven && thingtype >= 14100 && thingtype <= 14164)
	{
		iden_num = thingtype - 14100; // Mus change
		thingtype = 14164;            // MT_MUSICSOURCE
	}

	if(!raven && thingtype == 14165 && map_format.hexen)
	{
		iden_num = BETWEEN(0, 64, mthing->special_args[0]); // Mus change
		thingtype = 14164;                                  // MT_MUSICSOURCE
	}

	// find which type to spawn

	// killough 8/23/98: use table for faster lookup
	i = P_FindDoomedNum(thingtype);

	// phares 5/16/98:
	// Do not abort because of an unknown thing. Ignore it, but post a
	// warning message for the player.

	if(std::to_underlying(i) == num_mobj_types)
	{
		Log::Info("P_SpawnMapThing: Unknown Thing type {} at ({}, {})\n", thingtype, mthing->x, mthing->y);
		return nullptr;
	}

	// don't spawn keycards and players in deathmatch

	if(deathmatch && (mobjinfo[std::to_underlying(i)].flags & MobjFlag::NotDMatch) != MobjFlag{})
		return nullptr;

	// don't spawn any monsters if -nomonsters

	if(nomonsters && (i == MobjType::Skull || (mobjinfo[std::to_underlying(i)].flags & MobjFlag::CountKill) != MobjFlag{}))
		return nullptr;

	// spawn it
spawnit:

	if(heretic && i == MobjType::HereticWmace)
	{
		P_AddMaceSpot(mthing);
		return nullptr;
	}

	x = mthing->x;
	y = mthing->y;

	if((mobjinfo[std::to_underlying(i)].flags & MobjFlag::SpawnCeiling) != MobjFlag{})
		z = ONCEILINGZ;
	else if((mobjinfo[std::to_underlying(i)].flags2 & MobjFlag2::SpawnFloat) != MobjFlag2{})
		z = FLOATRANDZ;
	else if(hexen && (mobjinfo[std::to_underlying(i)].flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
		z = mthing->height;
	else
		z = ONFLOORZ;

	if(hexen && i == MobjType::HexenZlynchedNoheart)
		P_SpawnMobj(x, y, ONFLOORZ, MobjType::HexenBloodpool);

	mobj = P_SpawnMobj(x, y, z, static_cast<MobjType>(i));

	if((mobj->flags & MobjFlag::Friend) == MobjFlag{} &&
		(options & (map_format.zdoom ? MapThingFlag::Friendly : MapThingFlag::Friend)) != MapThingFlag{} &&
		mbf_features)
	{
		mobj->flags |= MobjFlag::Friend;        // killough 10/98:
		P_UpdateThinker(&mobj->thinker); // transfer friendliness flag

		// Friends can have a different spawn health
		mobj->health = P_MobjSpawnHealth(mobj);
	}

	if(mthing->health != FRACUNIT)
	{
		if(mthing->health < 0)
			mobj->health = -mthing->health >> FRACBITS;
		else
			mobj->health = FixedMul(mobj->health, mthing->health);
	}

	if(mthing->gravity != FRACUNIT)
	{
		if(mthing->gravity < 0)
			mobj->gravity = -mthing->gravity;
		else
			mobj->gravity = FixedMul(map_gravity, mthing->gravity);
	}

	mobj->alpha = mthing->alpha;
	if(mobj->alpha < 1.f)
		mobj->tranmap = dsda_TranMap(dsda_FloatToPercent(mobj->alpha));
	else
		mobj->tranmap = nullptr;

	mobj->spawnpoint = *mthing; // heretic_note: this is only done with totalkills++ in heretic
	mobj->index = index;        //e6y
	mobj->iden_nums = iden_num;

	if(map_format.hexen)
	{
		if(z == ONFLOORZ)
		{
			mobj->z += mthing->height;
		}
		else if(z == ONCEILINGZ)
		{
			mobj->z -= mthing->height;
		}

		if(map_format.zdoom)
			P_AdjustZLimits(mobj);

		mobj->tid = mthing->tid;
		mobj->special = mthing->special;
		mobj->special_args[0] = mthing->special_args[0];
		mobj->special_args[1] = mthing->special_args[1];
		mobj->special_args[2] = mthing->special_args[2];
		mobj->special_args[3] = mthing->special_args[3];
		mobj->special_args[4] = mthing->special_args[4];
	}

	if((mobj->flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
	{
		// Seed random starting index for bobbing motion
		mobj->health = P_Random(RandomClass::Heretic);
		if(hexen) mobj->special1.i = mthing->height;
	}

	if(mobj->tics > 0)
		mobj->tics = 1 + (P_Random(RandomClass::Spawnthing) % mobj->tics);

	if(map_format.zdoom)
	{
		if((options & MapThingFlag::Translucent) != MapThingFlag{})
			mobj->flags |= MobjFlag::Translucent;

		if((options & MapThingFlag::Invisible) != MapThingFlag{})
		{
			P_UnsetThingPosition(mobj);
			mobj->flags |= MobjFlag::NoSector;
			P_SetThingPosition(mobj);
		}

		if((options & MapThingFlag::CountSecret) != MapThingFlag{})
			P_AddMobjSecret(mobj);
	}

	/* killough 7/20/98: exclude friends */
	if(((mobj->flags ^ MobjFlag::CountKill) & (MobjFlag::Friend | MobjFlag::CountKill)) == MobjFlag{})
		totalkills++;

	if((mobj->flags & MobjFlag::CountItem) != MobjFlag{})
		totalitems++;

	if(map_format.hexen)
	{
		if((mobj->flags & MobjFlag::CountKill) != MobjFlag{})
		{
			// Quantize angle to 45 degree increments
			mobj->angle = ANG45 * (mthing->angle / 45);
		}
		else
		{
			// Scale angle correctly (source is 0..359)
			mobj->angle = ((mthing->angle << 8) / 360) << 24;
		}
	}
	else
		mobj->angle = ANG45 * (mthing->angle / 45);

	if((options & MapThingFlag::Ambush) != MapThingFlag{})
		mobj->flags |= MobjFlag::Ambush;

	if(map_format.hexen && (mthing->options & MapThingFlag::Dormant) != MapThingFlag{})
	{
		mobj->flags2 |= MobjFlag2::Dormant;
		if(hexen && mobj->type == MobjType::HexenIceguy)
		{
			P_SetMobjState(mobj, StateId::HexenIceguyDormant);
		}
		mobj->tics = -1;
	}

	if(!raven && thingtype == 14064)
	{
		dsda_SpawnAmbientSource(mobj);
	}

	return mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//

extern fixed_t attackrange;

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
	mobj_t* th;
	int t;

	if(raven) return Raven_P_SpawnPuff(x, y, z);

	// killough 5/5/98: remove dependence on order of evaluation:
	t = P_Random(RandomClass::Spawnpuff);
	z += (t - P_Random(RandomClass::Spawnpuff)) << 10;

	th = P_SpawnMobj(x, y, z, MobjType::Puff);
	th->momz = FRACUNIT;
	th->tics -= P_Random(RandomClass::Spawnpuff) & 3;

	if(th->tics < 1)
		th->tics = 1;

	// don't make punches spark on the wall

	if(attackrange == MELEERANGE)
		P_SetMobjState(th, StateId::Puff3);
}


//
// P_SpawnBlood
//
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t* bleeder)
{
	mobj_t* th;
	// killough 5/5/98: remove dependence on order of evaluation:
	int t = P_Random(RandomClass::Spawnblood);
	z += (t - P_Random(RandomClass::Spawnblood)) << 10;
	th = P_SpawnMobj(x, y, z, MobjType::Blood);
	th->momz = FRACUNIT * 2;
	th->tics -= P_Random(RandomClass::Spawnblood) & 3;
	th->color = bleeder->info->bloodcolor;

	if(th->tics < 1)
		th->tics = 1;

	if(damage <= 12 && damage >= 9)
		P_SetMobjState(th, StateId::Blood2);
	else if(damage < 9)
		P_SetMobjState(th, StateId::Blood3);
}


//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//

dboolean P_CheckMissileSpawn(mobj_t* th)
{
	if(!raven)
	{
		th->tics -= P_Random(RandomClass::Missile) & 3;
		if(th->tics < 1)
			th->tics = 1;
	}

	// move a little forward so an angle can
	// be computed if it immediately explodes

	if(heretic && th->type == MobjType::HereticBlasterfx1)
	{
		// Ultra-fast ripper spawning missile
		th->x += (th->momx >> 3);
		th->y += (th->momy >> 3);
		th->z += (th->momz >> 3);
	}
	else
	{
		th->x += (th->momx >> 1);
		th->y += (th->momy >> 1);
		th->z += (th->momz >> 1);
	}

	// killough 8/12/98: for non-missile objects (e.g. grenades)
	if((th->flags & MobjFlag::Missile) == MobjFlag{} && mbf_features)
		return true;

	// killough 3/15/98: no dropoff (really = don't care for missiles)

	if(!P_TryMove(th, th->x, th->y, false))
	{
		P_ExplodeMissile(th);
		return false;
	}

	return true;
}


//
// P_SpawnMissile
//

mobj_t* P_SpawnMissile(mobj_t* source, mobj_t* dest, MobjType type)
{
	fixed_t z;
	mobj_t* th;
	angle_t an;
	int dist;

	if(!raven)
	{
		z = source->z + 32 * FRACUNIT;
	}
	else
	{
		switch(type)
		{
			case MobjType::HereticMntrfx1: // Minotaur swing attack missile
			case MobjType::HexenMntrfx1:   // Minotaur swing attack missile
			case MobjType::HexenIceguyFx:
			case MobjType::HexenHolyMissile:
				z = source->z + 40 * FRACUNIT;
				break;
			case MobjType::HereticMntrfx2: // Minotaur floor fire missile
				z = ONFLOORZ;
				break;
			case MobjType::HexenMntrfx2: // Minotaur floor fire missile
				z = ONFLOORZ + source->floorclip;
				break;
			case MobjType::HereticSrcrfx1: // Sorcerer Demon fireball
				z = source->z + 48 * FRACUNIT;
				break;
			case MobjType::HereticKnightaxe: // Knight normal axe
			case MobjType::HereticRedaxe:    // Knight red power axe
				z = source->z + 36 * FRACUNIT;
				break;
			case MobjType::HexenCentaurFx:
				z = source->z + 45 * FRACUNIT;
				break;
			default:
				z = source->z + 32 * FRACUNIT;
				break;
		}

		if(hexen)
			z -= source->floorclip;
		else if((source->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
			z -= FOOTCLIPSIZE;
	}

	th = P_SpawnMobj(source->x, source->y, z, static_cast<MobjType>(type));

	if(th->info->seesound != SfxId::None)
		S_StartMobjSound(th, th->info->seesound);

	P_SetTarget(&th->target, source); // where it came from
	an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

	// fuzzy player
	if((dest->flags & MobjFlag::Shadow) != MobjFlag{})
	{
		// killough 5/5/98: remove dependence on order of evaluation:
		int t = P_Random(RandomClass::Shadow);
		an += (t - P_Random(RandomClass::Shadow)) << g_fuzzy_aim_shift;
	}

	th->angle = an;
	an >>= ANGLETOFINESHIFT;
	th->momx = FixedMul(th->info->speed, finecosine[an]);
	th->momy = FixedMul(th->info->speed, finesine[an]);

	dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
	dist = dist / th->info->speed;

	if(dist < 1)
		dist = 1;

	th->momz = (dest->z - source->z) / dist;

	if(!raven)
	{
		P_CheckMissileSpawn(th);
		return th;
	}

	return (P_CheckMissileSpawn(th) ? th : nullptr);
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//

mobj_t* P_SpawnPlayerMissile(mobj_t* source, MobjType type)
{
	mobj_t* th;
	fixed_t x, y, z;
	aim_t aim;

	// see which target is to be aimed at
	dsda_PlayerAim(source, source->angle, &aim, mbf_features ? MobjFlag::Friend : MobjFlag{});

	x = source->x;
	y = source->y;

	if(!raven)
	{
		z = source->z + 4 * 8 * FRACUNIT + aim.z_offset;
	}
	else
	{
		if(type == MobjType::HexenLightningFloor)
		{
			z = ONFLOORZ;
			aim.slope = 0;
		}
		else if(type == MobjType::HexenLightningCeiling)
		{
			z = ONCEILINGZ;
			aim.slope = 0;
		}
		else
		{
			z = source->z + 4 * 8 * FRACUNIT + aim.z_offset;

			if(hexen)
			{
				z -= source->floorclip;
			}
			else if((source->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
			{
				z -= FOOTCLIPSIZE;
			}
		}
	}

	// heretic global MissileMobj
	MissileMobj = th = P_SpawnMobj(x, y, z, static_cast<MobjType>(type));

	if(!hexen && th->info->seesound != SfxId::None)
		S_StartMobjSound(th, th->info->seesound);

	P_SetTarget(&th->target, source);
	th->angle = aim.angle;

	if(dsda_FreeAim())
	{
		fixed_t horizontal_speed;

		horizontal_speed = FixedMul(th->info->speed, finecosine[source->pitch >> ANGLETOFINESHIFT]);
		th->momx = FixedMul(horizontal_speed, finecosine[aim.angle >> ANGLETOFINESHIFT]);
		th->momy = FixedMul(horizontal_speed, finesine[aim.angle >> ANGLETOFINESHIFT]);
		th->momz = FixedMul(th->info->speed, -finesine[source->pitch >> ANGLETOFINESHIFT]);
	}
	else
	{
		th->momx = FixedMul(th->info->speed, finecosine[aim.angle >> ANGLETOFINESHIFT]);
		th->momy = FixedMul(th->info->speed, finesine[aim.angle >> ANGLETOFINESHIFT]);
		th->momz = FixedMul(th->info->speed, aim.slope);
	}

	if(hexen)
	{
		if(th->type == MobjType::HexenMwandMissile || th->type == MobjType::HexenCflameMissile)
		{
			// Ultra-fast ripper spawning missile
			th->x += (th->momx >> 3);
			th->y += (th->momy >> 3);
			th->z += (th->momz >> 3);
		}
		else
		{
			// Normal missile
			th->x += (th->momx >> 1);
			th->y += (th->momy >> 1);
			th->z += (th->momz >> 1);
		}
		if(!P_TryMove(th, th->x, th->y, false))
		{
			// Exploded immediately
			P_ExplodeMissile(th);
			return (nullptr);
		}
		return (th);
	}

	// heretic - return missile if it's ok
	return P_CheckMissileSpawn(th) ? th : nullptr;
}

// heretic

#include "p_spec.hpp"

MobjType PuffType;
mobj_t* MissileMobj;

void P_BlasterMobjThinker(mobj_t* mobj)
{
	int i;
	fixed_t xfrac;
	fixed_t yfrac;
	fixed_t zfrac;
	fixed_t z;
	dboolean changexy;

	P_MobjInterpolation(mobj);

	// Handle movement
	if(mobj->momx || mobj->momy || (mobj->z != mobj->floorz) || mobj->momz)
	{
		xfrac = mobj->momx >> 3;
		yfrac = mobj->momy >> 3;
		zfrac = mobj->momz >> 3;
		changexy = xfrac || yfrac;
		for(i = 0; i < 8; i++)
		{
			if(changexy)
			{
				if(!P_TryMove(mobj, mobj->x + xfrac, mobj->y + yfrac, false))
				{
					// Blocked move
					P_ExplodeMissile(mobj);
					return;
				}
			}
			mobj->z += zfrac;
			if(mobj->z <= mobj->floorz)
			{
				// Hit the floor
				mobj->z = mobj->floorz;
				P_HitFloor(mobj);
				P_ExplodeMissile(mobj);
				return;
			}
			if(mobj->z + mobj->height > mobj->ceilingz)
			{
				// Hit the ceiling
				mobj->z = mobj->ceilingz - mobj->height;
				P_ExplodeMissile(mobj);
				return;
			}
			if(changexy)
			{
				if(hexen)
				{
					if(mobj->type == MobjType::HexenMwandMissile && (P_Random(RandomClass::Hexen) < 128))
					{
						z = mobj->z - 8 * FRACUNIT;
						if(z < mobj->floorz)
						{
							z = mobj->floorz;
						}
						P_SpawnMobj(mobj->x, mobj->y, z, MobjType::HexenMwandsmoke);
					}
					else if(!--mobj->special1.i)
					{
						mobj_t* mo;
						mobj->special1.i = 4;
						z = mobj->z - 12 * FRACUNIT;
						if(z < mobj->floorz)
						{
							z = mobj->floorz;
						}
						mo = P_SpawnMobj(mobj->x, mobj->y, z, MobjType::HexenCflamefloor);
						if(mo)
						{
							mo->angle = mobj->angle;
						}
					}
				}
				else if(P_Random(RandomClass::Heretic) < 64)
				{
					z = mobj->z - 8 * FRACUNIT;
					if(z < mobj->floorz)
					{
						z = mobj->floorz;
					}
					P_SpawnMobj(mobj->x, mobj->y, z, MobjType::HereticBlastersmoke);
				}
			}
		}
	}
	// Advance the state
	if(mobj->tics != -1)
	{
		mobj->tics--;
		while(!mobj->tics)
		{
			if(!P_SetMobjState(mobj, static_cast<StateId>(mobj->state->nextstate)))
			{
				// mobj was removed
				return;
			}
		}
	}
}

extern "C" void A_ContMobjSound(mobj_t* actor)
{
	switch(actor->type)
	{
		case MobjType::HereticKnightaxe:
			S_StartMobjSound(actor, SfxId::HereticKgtatk);
			break;
		case MobjType::HereticMummyfx1:
			S_StartMobjSound(actor, SfxId::HereticMumhed);
			break;
		case MobjType::HexenSerpentfx:
			S_StartMobjSound(actor, SfxId::HexenSerpentfxContinuous);
			break;
		case MobjType::HexenHammerMissile:
			S_StartMobjSound(actor, SfxId::HexenFighterHammerContinuous);
			break;
		case MobjType::HexenQuakeFocus:
			S_StartMobjSound(actor, SfxId::HexenEarthquake);
			break;
		default:
			break;
	}
}

mobj_t* P_SpawnMissileAngle(mobj_t* source, MobjType type, angle_t angle, fixed_t momz)
{
	fixed_t z;
	mobj_t* mo;

	switch(type)
	{
		case MobjType::HereticMntrfx1: // Minotaur swing attack missile
		case MobjType::HexenMntrfx1:   // Minotaur swing attack missile
		case MobjType::HexenMstaffFx2:
			z = source->z + 40 * FRACUNIT;
			break;
		case MobjType::HexenMntrfx2: // Minotaur floor fire missile
			z = ONFLOORZ + source->floorclip;
			break;
		case MobjType::HereticMntrfx2: // Minotaur floor fire missile
			z = ONFLOORZ;
			break;
		case MobjType::HereticSrcrfx1: // Sorcerer Demon fireball
			z = source->z + 48 * FRACUNIT;
			break;
		case MobjType::HexenIceguyFx2: // Secondary Projectiles of the Ice Guy
			z = source->z + 3 * FRACUNIT;
			break;
		default:
			z = source->z + 32 * FRACUNIT;
			break;
	}

	if(hexen)
	{
		z -= source->floorclip;
	}
	else if((source->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
	{
		z -= FOOTCLIPSIZE;
	}

	mo = P_SpawnMobj(source->x, source->y, z, static_cast<MobjType>(type));
	if(mo->info->seesound != SfxId::None)
	{
		S_StartMobjSound(mo, mo->info->seesound);
	}
	P_SetTarget(&mo->target, source); // Originator
	mo->angle = angle;
	angle >>= ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[angle]);
	mo->momy = FixedMul(mo->info->speed, finesine[angle]);
	mo->momz = momz;
	return (P_CheckMissileSpawn(mo) ? mo : nullptr);
}

dboolean P_SetMobjStateNF(mobj_t* mobj, StateId state)
{
	state_t* st;

	if(state == g_s_null)
	{
		// Remove mobj
		mobj->state = nullptr;
		P_RemoveMobj(mobj);
		return (false);
	}
	st = &states[std::to_underlying(state)];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
	return (true);
}

void P_ThrustMobj(mobj_t* mo, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;
	mo->momx += FixedMul(move, finecosine[angle]);
	mo->momy += FixedMul(move, finesine[angle]);
}

dboolean P_SeekerMissile(mobj_t* actor, mobj_t** seekTarget, angle_t thresh, angle_t turnMax, dboolean seekcenter)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t* target;

	target = *seekTarget;
	if(target == nullptr)
	{
		return (false);
	}
	if((target->flags & MobjFlag::Shootable) == MobjFlag{})
	{
		// Target died
		*seekTarget = nullptr;
		return (false);
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
	if(actor->z + actor->height < target->z ||
		target->z + target->height < actor->z || seekcenter)
	{
		// Need to seek vertically
		dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
		dist = dist / actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		if(hexen)
			actor->momz = (target->z + (target->height >> 1) - (actor->z + (actor->height >> 1))) / dist;
		else
			actor->momz = (target->z + (seekcenter ? target->height / 2 : 0) - actor->z) / dist;
	}
	return (true);
}

mobj_t* P_SPMAngle(mobj_t* source, MobjType type, angle_t angle)
{
	mobj_t* th;
	fixed_t x, y, z;
	aim_t aim;

	//
	// see which target is to be aimed at
	//
	dsda_PlayerAim(source, angle, &aim, MobjFlag{});

	x = source->x;
	y = source->y;
	z = source->z + 4 * 8 * FRACUNIT + aim.z_offset;
	if(hexen)
	{
		z -= source->floorclip;
	}
	else if((source->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
	{
		z -= FOOTCLIPSIZE;
	}
	th = P_SpawnMobj(x, y, z, static_cast<MobjType>(type));
	if(!hexen && th->info->seesound != SfxId::None)
	{
		S_StartMobjSound(th, th->info->seesound);
	}
	P_SetTarget(&th->target, source);
	th->angle = aim.angle;
	th->momx = FixedMul(th->info->speed, finecosine[aim.angle >> ANGLETOFINESHIFT]);
	th->momy = FixedMul(th->info->speed, finesine[aim.angle >> ANGLETOFINESHIFT]);
	th->momz = FixedMul(th->info->speed, aim.slope);
	return (P_CheckMissileSpawn(th) ? th : nullptr);
}

static FloorType Hexen_P_HitFloor(mobj_t* thing);

FloorType P_HitFloor(mobj_t* thing)
{
	mobj_t* mo;

	if(thing->floorz != thing->subsector->sector->floorheight)
	{
		// don't splash if landing on the edge above water/lava/etc....
		return (FloorType::Solid);
	}

	if(hexen) return Hexen_P_HitFloor(thing);

	switch(P_GetThingFloorType(thing))
	{
		case FloorType::Water:
			P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticSplashbase);
			mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticSplash);
			P_SetTarget(&mo->target, thing);
			mo->momx = P_SubRandom() << 8;
			mo->momy = P_SubRandom() << 8;
			mo->momz = 2 * FRACUNIT + (P_Random(RandomClass::Heretic) << 8);
			S_StartMobjSound(mo, SfxId::HereticGloop);
			return (FloorType::Water);
		case FloorType::Lava:
			P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticLavasplash);
			mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticLavasmoke);
			mo->momz = FRACUNIT + (P_Random(RandomClass::Heretic) << 7);
			S_StartMobjSound(mo, SfxId::HereticBurn);
			return (FloorType::Lava);
		case FloorType::Sludge:
			P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticSludgesplash);
			mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HereticSludgechunk);
			P_SetTarget(&mo->target, thing);
			mo->momx = P_SubRandom() << 8;
			mo->momy = P_SubRandom() << 8;
			mo->momz = FRACUNIT + (P_Random(RandomClass::Heretic) << 8);
			return (FloorType::Sludge);
	}
	return (FloorType::Solid);
}

FloorType P_GetThingFloorType(mobj_t* thing)
{
	if(hexen && thing->floorpic)
	{
		return static_cast<FloorType>(TerrainTypes[thing->floorpic]);
	}
	else
	{
		return static_cast<FloorType>(TerrainTypes[thing->subsector->sector->floorpic]);
	}
}

// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
int P_FaceMobj(mobj_t* source, mobj_t* target, angle_t* delta)
{
	angle_t diff;
	angle_t angle1;
	angle_t angle2;

	angle1 = source->angle;
	angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);
	if(angle2 > angle1)
	{
		diff = angle2 - angle1;
		if(diff > ANG180)
		{
			*delta = ANGLE_MAX - diff;
			return (0);
		}
		else
		{
			*delta = diff;
			return (1);
		}
	}
	else
	{
		diff = angle1 - angle2;
		if(diff > ANG180)
		{
			*delta = ANGLE_MAX - diff;
			return (1);
		}
		else
		{
			*delta = diff;
			return (0);
		}
	}
}

dboolean Raven_P_SetMobjState(mobj_t* mobj, StateId state)
{
	state_t* st;

	if(state == g_s_null)
	{
		// Remove mobj
		mobj->state = nullptr;
		P_RemoveMobj(mobj);
		return (false);
	}
	st = &states[std::to_underlying(state)];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
	if(st->action)
	{
		// Call action function
		reinterpret_cast<void (*)(mobj_t*)>(st->action)(mobj);
	}
	return (true);
}

void P_FloorBounceMissile(mobj_t* mo)
{
	if(hexen)
	{
		if(P_HitFloor(mo) >= FloorType::Liquid)
		{
			switch(mo->type)
			{
				case MobjType::HexenSorcfx1:
				case MobjType::HexenSorcball1:
				case MobjType::HexenSorcball2:
				case MobjType::HexenSorcball3:
					break;
				default:
					P_RemoveMobj(mo);
					return;
			}
		}
		switch(mo->type)
		{
			case MobjType::HexenSorcfx1:
				mo->momz = -mo->momz; // no energy absorbed
				break;
			case MobjType::HexenSgshard1:
			case MobjType::HexenSgshard2:
			case MobjType::HexenSgshard3:
			case MobjType::HexenSgshard4:
			case MobjType::HexenSgshard5:
			case MobjType::HexenSgshard6:
			case MobjType::HexenSgshard7:
			case MobjType::HexenSgshard8:
			case MobjType::HexenSgshard9:
			case MobjType::HexenSgshard0:
				mo->momz = FixedMul(mo->momz, -0.3 * FRACUNIT);
				if(abs(mo->momz) < (FRACUNIT / 2))
				{
					P_SetMobjState(mo, StateId::HexenNull);
					return;
				}
				break;
			default:
				mo->momz = FixedMul(mo->momz, -0.7 * FRACUNIT);
				break;
		}
		mo->momx = 2 * mo->momx / 3;
		mo->momy = 2 * mo->momy / 3;
		if(mo->info->seesound != SfxId::None)
		{
			switch(mo->type)
			{
				case MobjType::HexenSorcball1:
				case MobjType::HexenSorcball2:
				case MobjType::HexenSorcball3:
					if(!mo->special_args[0])
						S_StartMobjSound(mo, mo->info->seesound);
					break;
				default:
					S_StartMobjSound(mo, mo->info->seesound);
					break;
			}
			S_StartMobjSound(mo, mo->info->seesound);
		}
	}
	else
	{
		mo->momz = -mo->momz;
		P_SetMobjState(mo, static_cast<StateId>(mobjinfo[std::to_underlying(mo->type)].deathstate));
	}
}

extern mobj_t* PuffSpawned;

void Raven_P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
	mobj_t* puff;

	z += (P_SubRandom() << 10);
	puff = P_SpawnMobj(x, y, z, static_cast<MobjType>(PuffType));
	if(hexen && linetarget && puff->info->seesound != SfxId::None)
	{
		// Hit thing sound
		S_StartMobjSound(puff, puff->info->seesound);
	}
	else if(puff->info->attacksound != SfxId::None)
	{
		S_StartMobjSound(puff, puff->info->attacksound);
	}
	switch(PuffType)
	{
		case MobjType::HereticBeakpuff:
		case MobjType::HereticStaffpuff:
		case MobjType::HexenPunchpuff:
			puff->momz = FRACUNIT;
			break;
		case MobjType::HereticGauntletpuff1:
		case MobjType::HereticGauntletpuff2:
		case MobjType::HexenHammerpuff:
			puff->momz = (fixed_t)(.8 * FRACUNIT);
		default:
			break;
	}
	PuffSpawned = puff;
}

void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t* originator)
{
	mobj_t* mo;

	mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(g_mt_bloodsplatter));
	P_SetTarget(&mo->target, originator);
	mo->momx = P_SubRandom() << g_bloodsplatter_shift;
	mo->momy = P_SubRandom() << g_bloodsplatter_shift;
	mo->momz = FRACUNIT * g_bloodsplatter_weight;
}

void P_RipperBlood(mobj_t* mo, mobj_t* bleeder)
{
	mobj_t* th;
	fixed_t x, y, z;

	x = mo->x + (P_SubRandom() << 12);
	y = mo->y + (P_SubRandom() << 12);
	z = mo->z + (P_SubRandom() << 12);
	th = P_SpawnMobj(x, y, z, static_cast<MobjType>(g_mt_blood));
	if(!hexen) th->flags |= MobjFlag::NoGravity;
	th->momx = mo->momx >> 1;
	th->momy = mo->momy >> 1;
	th->tics += P_Random(RandomClass::Heretic) & 3;
	th->color = bleeder->info->bloodcolor;
}

// hexen

#define MAX_TID_COUNT 200

extern mobj_t LavaInflictor;

static int TIDList[MAX_TID_COUNT + 1]; // +1 for termination marker
static mobj_t* TIDMobj[MAX_TID_COUNT];

mobj_t* P_SpawnMissileAngleSpeed(mobj_t* source, MobjType type,
	angle_t angle, fixed_t momz, fixed_t speed)
{
	fixed_t z;
	mobj_t* mo;

	z = source->z;
	z -= source->floorclip;
	mo = P_SpawnMobj(source->x, source->y, z, static_cast<MobjType>(type));
	P_SetTarget(&mo->target, source); // Originator
	mo->angle = angle;
	angle >>= ANGLETOFINESHIFT;
	mo->momx = FixedMul(speed, finecosine[angle]);
	mo->momy = FixedMul(speed, finesine[angle]);
	mo->momz = momz;
	return (P_CheckMissileSpawn(mo) ? mo : nullptr);
}

mobj_t* P_SPMAngleXYZ(mobj_t* source, fixed_t x, fixed_t y,
	fixed_t z, MobjType type, angle_t angle)
{
	mobj_t* th;
	aim_t aim;

	//
	// see which target is to be aimed at
	//
	dsda_PlayerAim(source, angle, &aim, MobjFlag{});

	z += 4 * 8 * FRACUNIT + aim.z_offset;
	z -= source->floorclip;
	th = P_SpawnMobj(x, y, z, static_cast<MobjType>(type));
	P_SetTarget(&th->target, source);
	th->angle = aim.angle;
	th->momx = FixedMul(th->info->speed, finecosine[aim.angle >> ANGLETOFINESHIFT]);
	th->momy = FixedMul(th->info->speed, finesine[aim.angle >> ANGLETOFINESHIFT]);
	th->momz = FixedMul(th->info->speed, aim.slope);
	return (P_CheckMissileSpawn(th) ? th : nullptr);
}

static void PlayerLandedOnThing(mobj_t* mo, mobj_t* onmobj, fixed_t gravity)
{
	mo->player->deltaviewheight = mo->momz >> 3;
	if(mo->momz < -23 * FRACUNIT)
	{
		P_FallingDamage(mo->player);
		P_NoiseAlert(mo, mo);
	}
	else if(mo->momz < -gravity * 12 && !mo->player->morphTics)
	{
		S_StartMobjSound(mo, SfxId::HexenPlayerLand);
		switch(mo->player->pclass)
		{
			case PClass::Fighter:
				S_StartMobjSound(mo, SfxId::HexenPlayerFighterGrunt);
				break;
			case PClass::Cleric:
				S_StartMobjSound(mo, SfxId::HexenPlayerClericGrunt);
				break;
			case PClass::Mage:
				S_StartMobjSound(mo, SfxId::HexenPlayerMageGrunt);
				break;
			default:
				break;
		}
	}
	else if(!mo->player->morphTics)
	{
		S_StartMobjSound(mo, SfxId::HexenPlayerLand);
	}
	P_AutoCorrectLookDir(mo->player);
}

extern "C" void P_CreateTIDList()
{
	int i;
	mobj_t* mobj;
	thinker_t* t;

	i = 0;
	for(t = thinkercap.next; t != &thinkercap; t = t->next)
	{
		// Search all current thinkers
		if(t->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mobj = (mobj_t*)t;
		if(mobj->tid != 0)
		{
			// Add to list
			if(i == MAX_TID_COUNT)
			{
				Log::Fatal("P_CreateTIDList: MAX_TID_COUNT ({}) exceeded.",
					MAX_TID_COUNT);
			}
			TIDList[i] = mobj->tid;
			TIDMobj[i++] = mobj;
		}
	}
	// Add termination marker
	TIDList[i] = 0;
}

extern "C" void P_InsertMobjIntoTIDList(mobj_t* mobj, short tid)
{
	int i;
	int index;

	index = -1;
	for(i = 0; TIDList[i] != 0; i++)
	{
		if(TIDList[i] == -1)
		{
			// Found empty slot
			index = i;
			break;
		}
	}
	if(index == -1)
	{
		// Append required
		if(i == MAX_TID_COUNT)
		{
			Log::Fatal("P_InsertMobjIntoTIDList: MAX_TID_COUNT ({})"
				"exceeded.", MAX_TID_COUNT);
		}
		index = i;
		TIDList[index + 1] = 0;
	}
	mobj->tid = tid;
	TIDList[index] = tid;
	TIDMobj[index] = mobj;
}

extern "C" void P_RemoveMobjFromTIDList(mobj_t* mobj)
{
	int i;

	for(i = 0; TIDList[i] != 0; i++)
	{
		if(TIDMobj[i] == mobj)
		{
			TIDList[i] = -1;
			TIDMobj[i] = nullptr;
			mobj->tid = 0;
			return;
		}
	}
	mobj->tid = 0;
}

mobj_t* P_FindMobjFromTID(short tid, int* searchPosition)
{
	int i;

	for(i = *searchPosition + 1; TIDList[i] != 0; i++)
	{
		if(TIDList[i] == tid)
		{
			*searchPosition = i;
			return TIDMobj[i];
		}
	}
	*searchPosition = -1;
	return nullptr;
}

void P_BloodSplatter2(fixed_t x, fixed_t y, fixed_t z, mobj_t* originator)
{
	mobj_t* mo;
	int r1, r2;

	r1 = P_Random(RandomClass::Hexen);
	r2 = P_Random(RandomClass::Hexen);
	mo = P_SpawnMobj(x + ((r2 - 128) << 11),
		y + ((r1 - 128) << 11), z, MobjType::HexenAxeblood);
	P_SetTarget(&mo->target, originator);
}

#define SMALLSPLASHCLIP 12<<FRACBITS;

static FloorType Hexen_P_HitFloor(mobj_t* thing)
{
	mobj_t* mo;
	int smallsplash = false;

	if(thing->floorz != thing->subsector->sector->floorheight)
	{
		// don't splash if landing on the edge above water/lava/etc....
		return (FloorType::Solid);
	}

	// Things that don't splash go here
	switch(thing->type)
	{
		case MobjType::HexenLeaf1:
		case MobjType::HexenLeaf2:
		case MobjType::HexenSplash:
		case MobjType::HexenSludgechunk:
			return (FloorType::Solid);
		default:
			break;
	}

	// Small splash for small masses
	if(thing->info->mass < 10)
		smallsplash = true;

	switch(P_GetThingFloorType(thing))
	{
		case FloorType::Water:
			if(smallsplash)
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenSplashbase);
				if(mo)
					mo->floorclip += SMALLSPLASHCLIP;
				S_StartMobjSound(mo, SfxId::HexenAmbient10); // small drip
			}
			else
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenSplash);
				P_SetTarget(&mo->target, thing);
				mo->momx = P_SubRandom() << 8;
				mo->momy = P_SubRandom() << 8;
				mo->momz = 2 * FRACUNIT + (P_Random(RandomClass::Hexen) << 8);
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenSplashbase);
				if(thing->player)
					P_NoiseAlert(thing, thing);
				S_StartMobjSound(mo, SfxId::HexenWaterSplash);
			}
			return (FloorType::Water);
		case FloorType::Lava:
			if(smallsplash)
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenLavasplash);
				if(mo)
					mo->floorclip += SMALLSPLASHCLIP;
			}
			else
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenLavasmoke);
				mo->momz = FRACUNIT + (P_Random(RandomClass::Hexen) << 7);
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, MobjType::HexenLavasplash);
				if(thing->player)
					P_NoiseAlert(thing, thing);
			}
			S_StartMobjSound(mo, SfxId::HexenLavaSizzle);
			if(thing->player && leveltime & 31)
			{
				P_DamageMobj(thing, &LavaInflictor, nullptr, 5);
			}
			return (FloorType::Lava);
		case FloorType::Sludge:
			if(smallsplash)
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
					MobjType::HexenSludgesplash);
				if(mo)
					mo->floorclip += SMALLSPLASHCLIP;
			}
			else
			{
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
					MobjType::HexenSludgechunk);
				P_SetTarget(&mo->target, thing);
				mo->momx = P_SubRandom() << 8;
				mo->momy = P_SubRandom() << 8;
				mo->momz = FRACUNIT + (P_Random(RandomClass::Hexen) << 8);
				mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
					MobjType::HexenSludgesplash);
				if(thing->player)
					P_NoiseAlert(thing, thing);
			}
			S_StartMobjSound(mo, SfxId::HexenSludgeGloop);
			return (FloorType::Sludge);
	}
	return (FloorType::Solid);
}

mobj_t* P_SpawnMissileXYZ(fixed_t x, fixed_t y, fixed_t z,
	mobj_t* source, mobj_t* dest, MobjType type)
{
	mobj_t* th;
	angle_t an;
	int dist;

	z -= source->floorclip;
	th = P_SpawnMobj(x, y, z, static_cast<MobjType>(type));
	if(th->info->seesound != SfxId::None)
	{
		S_StartMobjSound(th, th->info->seesound);
	}
	P_SetTarget(&th->target, source); // Originator
	an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
	if((dest->flags & MobjFlag::Shadow) != MobjFlag{})
	{
		// Invisible target
		an += P_SubRandom() << 21;
	}
	th->angle = an;
	an >>= ANGLETOFINESHIFT;
	th->momx = FixedMul(th->info->speed, finecosine[an]);
	th->momy = FixedMul(th->info->speed, finesine[an]);
	dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
	dist = dist / th->info->speed;
	if(dist < 1)
	{
		dist = 1;
	}
	th->momz = (dest->z - source->z) / dist;
	return (P_CheckMissileSpawn(th) ? th : nullptr);
}

mobj_t* P_SpawnKoraxMissile(fixed_t x, fixed_t y, fixed_t z,
	mobj_t* source, mobj_t* dest, MobjType type)
{
	mobj_t* th;
	angle_t an;
	int dist;

	z -= source->floorclip;
	th = P_SpawnMobj(x, y, z, static_cast<MobjType>(type));
	if(th->info->seesound != SfxId::None)
	{
		S_StartMobjSound(th, th->info->seesound);
	}
	P_SetTarget(&th->target, source); // Originator
	an = R_PointToAngle2(x, y, dest->x, dest->y);
	if((dest->flags & MobjFlag::Shadow) != MobjFlag{})
	{
		// Invisible target
		an += P_SubRandom() << 21;
	}
	th->angle = an;
	an >>= ANGLETOFINESHIFT;
	th->momx = FixedMul(th->info->speed, finecosine[an]);
	th->momy = FixedMul(th->info->speed, finesine[an]);
	dist = P_AproxDistance(dest->x - x, dest->y - y);
	dist = dist / th->info->speed;
	if(dist < 1)
	{
		dist = 1;
	}
	th->momz = (dest->z - z + (30 * FRACUNIT)) / dist;
	return (P_CheckMissileSpawn(th) ? th : nullptr);
}
