// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Ceiling aninmation (lowering, crushing, raising)
 */

#include <utility>

#include "doomstat.hpp"
#include "r_main.hpp"
#include "p_map.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "e6y.hpp"//e6y

#include "hexen/p_acs.hpp"
#include "hexen/sn_sonix.hpp"

#include "dsda/id_list.hpp"
#include "dsda/map_format.hpp"

// the list of ceilings moving currently, including crushers
ceilinglist_t* activeceilings;

/////////////////////////////////////////////////////////////////
//
// Ceiling action routine and linedef type handler
//
/////////////////////////////////////////////////////////////////

//
// T_MoveCeilingPlane()
//
// Move a ceiling plane and check for crushing. Called
// every tick by all actions that move ceiling.
//
// Passed the sector to move a plane in, the speed to move it at,
// the dest height it is to achieve, whether it crushes obstacles,
// and the direction up or down to move.
//
// Returns a result_e:
//  ok - plane moved normally, has not achieved destination yet
//  pastdest - plane moved normally and is now at destination height
//  crushed - plane encountered an obstacle, is holding until removed
//
MoveResult T_MoveCeilingPlane
(sector_t* sector,
	fixed_t speed,
	fixed_t dest,
	int crush,
	int direction,
	dboolean hexencrush)
{
	dboolean flag;
	fixed_t lastpos;
	fixed_t destheight; //jff 02/04/98 used to keep ceilings from moving thru each other

	if(V_IsOpenGLMode())
	{
		gld_UpdateSplitData(sector);
	}

	switch(direction)
	{
		case -1:
			// moving a ceiling down
			// jff 02/04/98 keep ceiling from moving thru floors
			// jff 2/22/98 weaken check to demo_compatibility
			destheight = (comp[std::to_underlying(CompOption::Floors)] || dest > sector->floorheight) ? dest : sector->floorheight;
			if(sector->ceilingheight - speed < destheight)
			{
				lastpos = sector->ceilingheight;
				sector->ceilingheight = destheight;
				flag = P_CheckSector(sector, crush); //jff 3/19/98 use faster chk

				if(flag == true)
				{
					sector->ceilingheight = lastpos;
					P_CheckSector(sector, crush); //jff 3/19/98 use faster chk
				}
				return MoveResult::PastDest;
			}
			else
			{
				// crushing is possible
				lastpos = sector->ceilingheight;
				sector->ceilingheight -= speed;
				flag = P_CheckSector(sector, crush); //jff 3/19/98 use faster chk

				if(flag == true)
				{
					if(!hexencrush && crush >= 0)
						return MoveResult::Crushed;
					sector->ceilingheight = lastpos;
					P_CheckSector(sector, crush); //jff 3/19/98 use faster chk
					return MoveResult::Crushed;
				}
			}
			break;

		case 1:
			// moving a ceiling up
			if(sector->ceilingheight + speed > dest)
			{
				lastpos = sector->ceilingheight;
				sector->ceilingheight = dest;
				flag = P_CheckSector(sector, crush); //jff 3/19/98 use faster chk
				if(flag == true)
				{
					sector->ceilingheight = lastpos;
					P_CheckSector(sector, crush); //jff 3/19/98 use faster chk
				}
				return MoveResult::PastDest;
			}
			else
			{
				lastpos = sector->ceilingheight;
				sector->ceilingheight += speed;
				flag = P_CheckSector(sector, crush); //jff 3/19/98 use faster chk
			}
			break;
	}

	return MoveResult::Ok;
}

//
// T_MoveCeiling
//
// Action routine that moves ceilings. Called once per tick.
//
// Passed a ceiling_t structure that contains all the info about the move.
// see P_SPEC.H for fields. No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.
//

extern "C" void T_MoveCompatibleCeiling(ceiling_t* ceiling)
{
	MoveResult res;

	switch(ceiling->direction)
	{
		case 0:
			// If ceiling in stasis, do nothing
			break;

		case 1:
			// Ceiling is moving up
			res = T_MoveCeilingPlane
			(
				ceiling->sector,
				ceiling->speed,
				ceiling->topheight,
				NO_CRUSH,
				ceiling->direction,
				false
			);

			// if not silent, make moving sound
			if(!(leveltime & 7) && !ceiling->silent)
				S_LoopSectorSound(ceiling->sector, g_sfx_stnmov, 8);

			// handle reaching destination height
			if(res == MoveResult::PastDest)
			{
				switch(ceiling->type)
				{
					// plain movers are just removed
					case CeilingKind::RaiseToHighest:
					case CeilingKind::GenCeiling:
						P_RemoveActiveCeiling(ceiling);
						break;

					// movers with texture change, change the texture then get removed
					case CeilingKind::GenCeilingChgT:
					case CeilingKind::GenCeilingChg0:
						P_TransferSpecial(ceiling->sector, &ceiling->newspecial);
					// fallthrough
					case CeilingKind::GenCeilingChg:
						ceiling->sector->ceilingpic = ceiling->texture;
						P_RemoveActiveCeiling(ceiling);
						break;

					// crushers reverse direction at the top
					case CeilingKind::SilentCrushAndRaise:
						S_StartSectorSound(ceiling->sector, SfxId::Pstop);
					// fallthrough
					case CeilingKind::GenSilentCrusher:
					case CeilingKind::GenCrusher:
					case CeilingKind::FastCrushAndRaise:
					case CeilingKind::CrushAndRaise:
						ceiling->direction = -1;
						break;

					case CeilingKind::CeilCrushAndRaise:
						ceiling->direction = -1;
						ceiling->speed = ceiling->oldspeed;
						if(ceiling->silent == 1)
							S_StartSectorSound(ceiling->sector, SfxId::Pstop);
						break;

					default:
						if(map_format.zdoom)
							P_RemoveActiveCeiling(ceiling);
						break;
				}
			}
			break;

		case -1:
			// Ceiling moving down
			res = T_MoveCeilingPlane
			(
				ceiling->sector,
				ceiling->speed,
				ceiling->bottomheight,
				ceiling->crush,
				ceiling->direction,
				ceiling->crushmode == CrushMode::Hexen
			);

			// if not silent, make moving sound
			if(!(leveltime & 7) && !ceiling->silent)
				S_LoopSectorSound(ceiling->sector, g_sfx_stnmov, 8);

			// handle reaching destination height
			if(res == MoveResult::PastDest)
			{
				switch(ceiling->type)
				{
					// 02/09/98 jff change slow crushers' speed back to normal
					// start back up
					case CeilingKind::GenSilentCrusher:
					case CeilingKind::GenCrusher:
						if(ceiling->oldspeed < CEILSPEED * 3)
							ceiling->speed = ceiling->oldspeed;
						ceiling->direction = 1; //jff 2/22/98 make it go back up!
						break;

					// make platform stop at bottom of all crusher strokes
					// except generalized ones, reset speed, start back up
					case CeilingKind::SilentCrushAndRaise:
						S_StartSectorSound(ceiling->sector, SfxId::Pstop);
					// fallthrough
					case CeilingKind::CrushAndRaise:
						ceiling->speed = CEILSPEED;
					// fallthrough
					case CeilingKind::FastCrushAndRaise:
						ceiling->direction = 1;
						break;

					// in the case of ceiling mover/changer, change the texture
					// then remove the active ceiling
					case CeilingKind::GenCeilingChgT:
					case CeilingKind::GenCeilingChg0:
						P_TransferSpecial(ceiling->sector, &ceiling->newspecial);
					// fallthrough
					case CeilingKind::GenCeilingChg:
						ceiling->sector->ceilingpic = ceiling->texture;
						P_RemoveActiveCeiling(ceiling);
						break;

					// all other case, just remove the active ceiling
					case CeilingKind::LowerAndCrush:
					case CeilingKind::LowerToFloor:
					case CeilingKind::LowerToLowest:
					case CeilingKind::LowerToMaxFloor:
					case CeilingKind::GenCeiling:
						P_RemoveActiveCeiling(ceiling);
						break;

					case CeilingKind::CeilCrushAndRaise:
					case CeilingKind::CeilCrushRaiseAndStay:
						ceiling->speed = ceiling->speed2;
						ceiling->direction = 1;
						if(ceiling->silent == 1)
							S_StartSectorSound(ceiling->sector, SfxId::Pstop);
						break;

					default:
						if(map_format.zdoom)
							P_RemoveActiveCeiling(ceiling);
						break;
				}
			}
			else // ( res != pastdest )
			{
				// handle the crusher encountering an obstacle
				if(res == MoveResult::Crushed)
				{
					switch(ceiling->type)
					{
						//jff 02/08/98 slow down slow crushers on obstacle
						case CeilingKind::GenCrusher:
						case CeilingKind::GenSilentCrusher:
							if(ceiling->oldspeed < CEILSPEED * 3)
								ceiling->speed = CEILSPEED / 8;
							break;
						case CeilingKind::SilentCrushAndRaise:
						case CeilingKind::CrushAndRaise:
						case CeilingKind::LowerAndCrush:
							ceiling->speed = CEILSPEED / 8;
							break;

						case CeilingKind::CeilCrushAndRaise:
						case CeilingKind::CeilLowerAndCrush:
							if(ceiling->crushmode == CrushMode::Slowdown)
								ceiling->speed = FRACUNIT / 8;
							break;

						default:
							break;
					}
				}
			}
			break;
	}
}

extern "C" void T_MoveHexenCeiling(ceiling_t* ceiling)
{
	MoveResult res;

	switch(ceiling->direction)
	{
		//              case 0:         // IN STASIS
		//                      break;
		case 1: // UP
			res = T_MoveCeilingPlane(ceiling->sector, ceiling->speed,
				ceiling->topheight, NO_CRUSH,
				ceiling->direction, true);
			if(res == MoveResult::PastDest)
			{
				SN_StopSequence((mobj_t*)&ceiling->sector->soundorg);
				switch(ceiling->type)
				{
					case CeilingKind::ClevCrushandraise:
						ceiling->direction = -1;
						ceiling->speed = ceiling->speed * 2;
						break;
					default:
						P_RemoveActiveCeiling(ceiling);
						break;
				}
			}
			break;
		case -1: // DOWN
			res = T_MoveCeilingPlane(ceiling->sector, ceiling->speed,
				ceiling->bottomheight, ceiling->crush,
				ceiling->direction, true);
			if(res == MoveResult::PastDest)
			{
				SN_StopSequence((mobj_t*)&ceiling->sector->soundorg);
				switch(ceiling->type)
				{
					case CeilingKind::ClevCrushandraise:
					case CeilingKind::ClevCrushraiseandstay:
						ceiling->direction = 1;
						ceiling->speed = ceiling->speed / 2;
						break;
					default:
						P_RemoveActiveCeiling(ceiling);
						break;
				}
			}
			else if(res == MoveResult::Crushed)
			{
				switch(ceiling->type)
				{
					case CeilingKind::ClevCrushandraise:
					case CeilingKind::ClevLowerandcrush:
					case CeilingKind::ClevCrushraiseandstay:
						//ceiling->speed = ceiling->speed/4;
						break;
					default:
						break;
				}
			}
			break;
	}
}

void T_MoveCeiling(ceiling_t* ceiling)
{
	map_format.t_move_ceiling(ceiling);
}


//
// EV_DoCeiling
//
// Move a ceiling up/down or start a crusher
//
// Passed the linedef activating the function and the type of function desired
// returns true if a thinker started
//
int EV_DoCeiling
(line_t* line,
	CeilingKind type)
{
	const int* id_p;
	int rtn;
	sector_t* sec;
	ceiling_t* ceiling;

	rtn = 0;

	// Reactivate in-stasis ceilings...for certain types.
	// This restarts a crusher after it has been stopped
	switch(type)
	{
		case CeilingKind::FastCrushAndRaise:
		case CeilingKind::SilentCrushAndRaise:
		case CeilingKind::CrushAndRaise:
			//jff 4/5/98 return if activated
			rtn = P_ActivateInStasisCeiling(line->special_args[0]); // heretic_note: rtn not set in heretic
		default:
			break;
	}

	// affects all sectors with the same tag as the linedef
	FIND_SECTORS(id_p, line->special_args[0])
	{
		sec = &sectors[*id_p];

		// if ceiling already moving, don't start a second function on it
		if(P_CeilingActive(sec)) //jff 2/22/98
			continue;

		// create a new ceiling thinker
		rtn = 1;
		ceiling = static_cast<ceiling_t*>(Z_MallocLevel(sizeof(*ceiling)));
		memset(ceiling, 0, sizeof(*ceiling));
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling; //jff 2/22/98
		ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
		ceiling->sector = sec;
		ceiling->crush = NO_CRUSH;

		// setup ceiling structure according to type of function
		switch(type)
		{
			case CeilingKind::FastCrushAndRaise:
				ceiling->crush = DOOM_CRUSH;
				ceiling->topheight = sec->ceilingheight;
				ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED * 2;
				break;

			case CeilingKind::SilentCrushAndRaise:
				ceiling->silent = 1;
			case CeilingKind::CrushAndRaise:
				ceiling->crush = DOOM_CRUSH;
				ceiling->topheight = sec->ceilingheight;
			// fallthrough
			case CeilingKind::LowerAndCrush:
			case CeilingKind::LowerToFloor:
				ceiling->bottomheight = sec->floorheight;
				if(type != CeilingKind::LowerToFloor)
					ceiling->bottomheight += 8 * FRACUNIT;
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED;
				break;

			case CeilingKind::RaiseToHighest:
				ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
				ceiling->direction = 1;
				ceiling->speed = CEILSPEED;
				break;

			case CeilingKind::LowerToLowest:
				ceiling->bottomheight = P_FindLowestCeilingSurrounding(sec);
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED;
				break;

			case CeilingKind::LowerToMaxFloor:
				ceiling->bottomheight = P_FindHighestFloorSurrounding(sec);
				ceiling->direction = -1;
				ceiling->speed = CEILSPEED;
				break;

			default:
				break;
		}

		// add the ceiling to the active list
		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
	}
	return rtn;
}

//////////////////////////////////////////////////////////////////////
//
// Active ceiling list primitives
//
/////////////////////////////////////////////////////////////////////

// jff 2/22/98 - modified Lee's plat code to work for ceilings
//
// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active ceilings. It also avoids spending as much
// time searching for active ceilings. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_ActivateInStasisCeiling()
//
// Reactivates all stopped crushers with the right tag
//
// Passed the line reactivating the crusher
// Returns true if a ceiling reactivated
//
//jff 4/5/98 return if activated
int P_ActivateInStasisCeiling(int tag)
{
	ceilinglist_t* cl;
	int rtn = 0;

	for(cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		if(ceiling->tag == tag && ceiling->direction == 0)
		{
			ceiling->direction = ceiling->olddirection;
			ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
			//jff 4/5/98 return if activated
			rtn = 1;
		}
	}
	return rtn;
}

// TODO: without reflection, we don't know if the ceilingdata needs special handling
int EV_ZDoomCeilingStop(int tag, line_t* line)
{
	const int* id_p;
	ceilinglist_t* cl;

	for(cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		if(ceiling->tag == tag)
		{
			P_RemoveActiveCeiling(ceiling);
		}
	}

	FIND_SECTORS2(id_p, tag, line)
	{
		sector_t* sec = &sectors[*id_p];
		ceiling_t* ceiling = (ceiling_t*)sec->ceilingdata;

		if(ceiling)
		{
			sec->ceilingdata = nullptr;
			P_RemoveThinker(&ceiling->thinker);
		}
	}

	return true;
}

int EV_ZDoomCeilingCrushStop(int tag, dboolean remove)
{
	dboolean rtn = 0;
	ceilinglist_t* cl;

	for(cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		if(ceiling->direction != 0 && ceiling->tag == tag)
		{
			if(!remove)
			{
				ceiling->olddirection = ceiling->direction;
				ceiling->direction = 0;
			}
			else
			{
				P_RemoveActiveCeiling(ceiling);
			}
			rtn = 1;
		}
	}

	return rtn;
}

//
// EV_CeilingCrushStop()
//
// Stops all active ceilings with the right tag
//
// Passed the linedef stopping the ceilings
// Returns true if a ceiling put in stasis
//
int EV_CeilingCrushStop(line_t* line)
{
	int rtn = 0;

	ceilinglist_t* cl;
	for(cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		if(ceiling->direction != 0 && ceiling->tag == line->special_args[0])
		{
			ceiling->olddirection = ceiling->direction;
			ceiling->direction = 0;
			ceiling->thinker.function = nullptr;
			rtn = 1;
		}
	}
	return rtn;
}

//
// P_AddActiveCeiling()
//
// Adds a ceiling to the head of the list of active ceilings
//
// Passed the ceiling motion structure
// Returns nothing
//
void P_AddActiveCeiling(ceiling_t* ceiling)
{
	ceilinglist_t* list = static_cast<ceilinglist_t*>(Z_Malloc(sizeof *list));
	list->ceiling = ceiling;
	ceiling->list = list;
	if((list->next = activeceilings))
		list->next->prev = &list->next;
	list->prev = &activeceilings;
	activeceilings = list;
}

//
// P_RemoveActiveCeiling()
//
// Removes a ceiling from the list of active ceilings
//
// Passed the ceiling motion structure
// Returns nothing
//
void P_RemoveActiveCeiling(ceiling_t* ceiling)
{
	ceilinglist_t* list = ceiling->list;
	ceiling->sector->ceilingdata = nullptr; //jff 2/22/98
	P_RemoveThinker(&ceiling->thinker);
	P_TagFinished(ceiling->sector->tag);
	if((*list->prev = list->next))
		list->next->prev = list->prev;
	Z_Free(list);
}

//
// P_RemoveAllActiveCeilings()
//
// Removes all ceilings from the active ceiling list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActiveCeilings()
{
	while(activeceilings)
	{
		ceilinglist_t* next = activeceilings->next;
		Z_Free(activeceilings);
		activeceilings = next;
	}
}

// hexen

int Hexen_EV_CeilingCrushStop(line_t* line, byte* args)
{
	ceilinglist_t* cl;
	for(cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		if(ceiling->tag == args[0])
		{
			SN_StopSequence((mobj_t*)&ceiling->sector->soundorg);
			P_RemoveActiveCeiling(ceiling);

			return 1;
		}
	}
	return 0;
}

static void P_SpawnZDoomCeiling(sector_t* sec, CeilingKind type, line_t* line, int tag,
	fixed_t speed, fixed_t speed2, fixed_t height, int crush,
	byte silent, int change, CrushMode crushmode)
{
	ceiling_t* ceiling;
	fixed_t targheight = 0;

	ceiling = static_cast<ceiling_t*>(Z_MallocLevel(sizeof(*ceiling)));
	memset(ceiling, 0, sizeof(*ceiling));
	P_AddThinker(&ceiling->thinker);
	sec->ceilingdata = ceiling;
	ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
	ceiling->sector = sec;
	ceiling->speed = speed;
	ceiling->oldspeed = speed;
	ceiling->speed2 = speed2;
	ceiling->silent = (silent & ~4);
	ceiling->texture = NO_TEXTURE;

	switch(type)
	{
		case CeilingKind::CeilCrushAndRaise:
		case CeilingKind::CeilCrushRaiseAndStay:
			ceiling->topheight = sec->ceilingheight;
		case CeilingKind::CeilLowerAndCrush:
			targheight = sec->floorheight + height;
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseToHighest:
			targheight = P_FindHighestCeilingSurrounding(sec);
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilLowerByValue:
			targheight = sec->ceilingheight - height;
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseByValue:
			targheight = sec->ceilingheight + height;
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilMoveToValue:
		{
			fixed_t diff = height - sec->ceilingheight;

			targheight = height;
			if(diff < 0)
			{
				ceiling->bottomheight = height;
				ceiling->direction = -1;
			}
			else
			{
				ceiling->topheight = height;
				ceiling->direction = 1;
			}
		}
		break;
		case CeilingKind::CeilLowerToHighestFloor:
			targheight = P_FindHighestFloorSurrounding(sec) + height;
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseToHighestFloor:
			targheight = P_FindHighestFloorSurrounding(sec);
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilLowerInstant:
			targheight = sec->ceilingheight - height;
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			ceiling->speed = height;
			break;
		case CeilingKind::CeilRaiseInstant:
			targheight = sec->ceilingheight + height;
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			ceiling->speed = height;
			break;
		case CeilingKind::CeilLowerToNearest:
			targheight = P_FindNextLowestCeiling(sec, sec->ceilingheight);
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseToNearest:
			targheight = P_FindNextHighestCeiling(sec, sec->ceilingheight);
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilLowerToLowest:
			targheight = P_FindLowestCeilingSurrounding(sec);
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseToLowest:
			targheight = P_FindLowestCeilingSurrounding(sec);
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilLowerToFloor:
			targheight = sec->floorheight + height;
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseToFloor:
			targheight = sec->floorheight + height;
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		case CeilingKind::CeilLowerToHighest:
			targheight = P_FindHighestCeilingSurrounding(sec);
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilLowerByTexture:
			targheight = sec->ceilingheight - P_FindShortestUpperAround(sec->iSectorID);
			ceiling->bottomheight = targheight;
			ceiling->direction = -1;
			break;
		case CeilingKind::CeilRaiseByTexture:
			targheight = sec->ceilingheight + P_FindShortestUpperAround(sec->iSectorID);
			ceiling->topheight = targheight;
			ceiling->direction = 1;
			break;
		default:
			break;
	}

	ceiling->tag = tag;
	ceiling->type = type;
	ceiling->crush = crush;
	ceiling->crushmode = crushmode;

	// Don't make noise for instant movement ceilings
	if(ceiling->direction < 0)
	{
		if(ceiling->speed >= sec->ceilingheight - ceiling->bottomheight)
			if(silent & 4)
				ceiling->silent = 2;
	}
	else
	{
		if(ceiling->speed >= ceiling->topheight - sec->ceilingheight)
			if(silent & 4)
				ceiling->silent = 2;
	}

	// set texture/type change properties
	if(change & 3) // if a texture change is indicated
	{
		if(change & 4) // if a numeric model change
		{
			sector_t* modelsec;

			// jff 5/23/98 find model with floor at target height if target is a floor type
			modelsec = (type == CeilingKind::CeilRaiseToFloor || type == CeilingKind::CeilLowerToFloor) ? P_FindModelFloorSector(targheight, sec->iSectorID) : P_FindModelCeilingSector(targheight, sec->iSectorID);

			if(modelsec != nullptr)
			{
				ceiling->texture = modelsec->ceilingpic;
				switch(change & 3)
				{
					case 0:
						break;
					case 1: // type is zeroed
						P_ResetTransferSpecial(&ceiling->newspecial);
						ceiling->type = CeilingKind::GenCeilingChg0;
						break;
					case 2: // type is copied
						P_CopyTransferSpecial(&ceiling->newspecial, sec);
						ceiling->type = CeilingKind::GenCeilingChgT;
						break;
					case 3: // type is left alone
						ceiling->type = CeilingKind::GenCeilingChg;
						break;
				}
			}
		}
		else if(line) // else if a trigger model change
		{
			ceiling->texture = line->frontsector->ceilingpic;
			switch(change & 3)
			{
				case 0:
					break;
				case 1: // type is zeroed
					P_ResetTransferSpecial(&ceiling->newspecial);
					ceiling->type = CeilingKind::GenCeilingChg0;
					break;
				case 2: // type is copied
					P_CopyTransferSpecial(&ceiling->newspecial, line->frontsector);
					ceiling->type = CeilingKind::GenCeilingChgT;
					break;
				case 3: // type is left alone
					ceiling->type = CeilingKind::GenCeilingChg;
					break;
			}
		}
	}

	P_AddActiveCeiling(ceiling);

	return;
}

int EV_DoZDoomCeiling(CeilingKind type, line_t* line, int tag, fixed_t speed, fixed_t speed2,
	fixed_t height, int crush, byte silent, int change, CrushMode crushmode)
{
	sector_t* sec;
	const int* id_p;
	int retcode = 0;

	height *= FRACUNIT;

	// check if a manual trigger, if so do just the sector on the backside
	if(tag == 0)
	{
		int secnum;

		if(!line || !(sec = line->backsector))
			return 0;

		secnum = sec - sectors;
		// [RH] Hack to let manual crushers be retriggerable, too
		tag ^= secnum | 0x1000000;
		P_ActivateInStasisCeiling(tag);

		if(sec->ceilingdata)
			return 0;

		P_SpawnZDoomCeiling(sec, type, line, tag, speed, speed2,
			height, crush, silent, change, crushmode);
		return 1;
	}

	// Reactivate in-stasis ceilings...for certain types.
	// This restarts a crusher after it has been stopped
	if(type == CeilingKind::CeilCrushAndRaise)
	{
		P_ActivateInStasisCeiling(tag);
	}

	FIND_SECTORS(id_p, tag)
	{
		sec = &sectors[*id_p];
		if(sec->ceilingdata)
		{
			continue;
		}
		retcode = 1;
		P_SpawnZDoomCeiling(sec, type, line, tag, speed, speed2,
			height, crush, silent, change, crushmode);
	}

	return retcode;
}

int Hexen_EV_DoCeiling(line_t* line, byte* arg, CeilingKind type)
{
	const int* id_p;
	int rtn;
	sector_t* sec;
	ceiling_t* ceiling;

	rtn = 0;

	FIND_SECTORS(id_p, arg[0])
	{
		sec = &sectors[*id_p];
		if(sec->floordata || sec->ceilingdata)
			continue;

		//
		// new door thinker
		//
		rtn = 1;
		ceiling = static_cast<ceiling_t*>(Z_MallocLevel(sizeof(*ceiling)));
		memset(ceiling, 0, sizeof(*ceiling));
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling;
		ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
		ceiling->sector = sec;
		ceiling->crush = NO_CRUSH;
		ceiling->speed = arg[1] * (FRACUNIT / 8);
		switch(type)
		{
			case CeilingKind::ClevCrushraiseandstay:
				ceiling->crush = P_ConvertHexenCrush(arg[2]); // arg[2] = crushing value
				ceiling->topheight = sec->ceilingheight;
				ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
				ceiling->direction = -1;
				break;
			case CeilingKind::ClevCrushandraise:
				ceiling->topheight = sec->ceilingheight;
			case CeilingKind::ClevLowerandcrush:
				ceiling->crush = P_ConvertHexenCrush(arg[2]); // arg[2] = crushing value
			case CeilingKind::ClevLowertofloor:
				ceiling->bottomheight = sec->floorheight;
				if(type != CeilingKind::ClevLowertofloor)
				{
					ceiling->bottomheight += 8 * FRACUNIT;
				}
				ceiling->direction = -1;
				break;
			case CeilingKind::ClevRaisetohighest:
				ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
				ceiling->direction = 1;
				break;
			case CeilingKind::ClevLowerbyvalue:
				ceiling->bottomheight =
					sec->ceilingheight - arg[2] * FRACUNIT;
				ceiling->direction = -1;
				break;
			case CeilingKind::ClevRaisebyvalue:
				ceiling->topheight = sec->ceilingheight + arg[2] * FRACUNIT;
				ceiling->direction = 1;
				break;
			case CeilingKind::ClevMovetovaluetimes8:
			{
				int destHeight = arg[2] * FRACUNIT * 8;

				if(arg[3])
				{
					destHeight = -destHeight;
				}
				if(sec->ceilingheight <= destHeight)
				{
					ceiling->direction = 1;
					ceiling->topheight = destHeight;
					if(sec->ceilingheight == destHeight)
					{
						rtn = 0;
					}
				}
				else if(sec->ceilingheight > destHeight)
				{
					ceiling->direction = -1;
					ceiling->bottomheight = destHeight;
				}
				break;
			}
			default:
				rtn = 0;
				break;
		}
		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
		if(rtn)
		{
			SN_StartSequence((mobj_t*)&ceiling->sector->soundorg,
				std::to_underlying(SoundSequence::Platform) + static_cast<int>(ceiling->sector->seqType));
		}
	}
	return rtn;
}
