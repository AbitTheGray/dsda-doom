// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Plats (i.e. elevator platforms) code, raising/lowering.
 */

#include <utility>

#include "doomstat.hpp"
#include "m_random.hpp"
#include "r_main.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"//e6y

#include "dsda/id_list.hpp"
#include "dsda/map_format.hpp"

#include "hexen/p_acs.hpp"
#include "hexen/sn_sonix.hpp"

platlist_t* activeplats; // killough 2/14/98: made global again

//
// T_PlatRaise()
//
// Action routine to move a plat up and down
//
// Passed a plat structure containing all pertinent information about the move
// No return
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

extern "C" void T_CompatiblePlatRaise(plat_t* plat)
{
	MoveResult res;

	// handle plat moving, up, down, waiting, or in stasis,
	switch(plat->status)
	{
		case PlatState::Up: // plat moving up
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, false);

			if(heretic && !(leveltime & 31))
			{
				S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 32);
			}

			// if a pure raise type, make the plat moving sound
			if(plat->type == PlatType::RaiseAndChange
				|| plat->type == PlatType::RaiseToNearestAndChange)
			{
				if(!(leveltime & 7))
					S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 8);
			}

			// if encountered an obstacle, and not a crush type, reverse direction
			if(res == MoveResult::Crushed && plat->crush == NO_CRUSH)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Down;
				S_StartSectorSound(plat->sector, g_sfx_pstart);

				if(demo_compatibility &&
					(plat->type == PlatType::RaiseToNearestAndChange ||
						plat->type == PlatType::RaiseAndChange))
				{
					// For these types vanilla did not initialize plat->low in EV_DoPlat,
					// so they may descend to any depth, or not at all.
					// See https://sourceforge.net/p/prboom-plus/bugs/211/ .
					Log::Warn("T_PlatRaise: raise-and-change type has reversed "
						"direction in compatibility mode - may lead to desync\n"
						" gametic: {} sector: {} complevel: {}\n",
						gametic, plat->sector->iSectorID, std::to_underlying(compatibility_level));
				}
			}
			else // else handle reaching end of up stroke
			{
				if(res == MoveResult::PastDest) // end of stroke
				{
					// if not an instant toggle type, wait, make plat stop sound
					if(plat->type != PlatType::ToggleUpDn)
					{
						plat->count = plat->wait;
						plat->status = PlatState::Waiting;
						S_StartSectorSound(plat->sector, g_sfx_pstop);
					}
					else // else go into stasis awaiting next toggle activation
					{
						plat->oldstatus = plat->status; //jff 3/14/98 after action wait
						plat->status = PlatState::InStasis;       //for reactivation of toggle
					}

					// lift types and pure raise types are done at end of up stroke
					// only the perpetual type waits then goes back up
					switch(plat->type)
					{
						case PlatType::BlazeDWUS:
						case PlatType::DownWaitUpStay:
						case PlatType::RaiseAndChange:
						case PlatType::RaiseToNearestAndChange:
						case PlatType::GenLift:
							if(heretic && plat->type == PlatType::RaiseToNearestAndChange) break;
							P_RemoveActivePlat(plat); // killough
						default:
							break;
					}
				}
			}
			break;

		case PlatState::Down: // plat moving down
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, false);

			// handle reaching end of down stroke
			if(res == MoveResult::PastDest)
			{
				// if not an instant toggle, start waiting, make plat stop sound
				if(plat->type != PlatType::ToggleUpDn) //jff 3/14/98 toggle up down
				{
					// is silent, instant, no waiting
					plat->count = plat->wait;
					plat->status = PlatState::Waiting;
					S_StartSectorSound(plat->sector, g_sfx_pstop);
				}
				else // instant toggles go into stasis awaiting next activation
				{
					plat->oldstatus = plat->status; //jff 3/14/98 after action wait
					plat->status = PlatState::InStasis;       //for reactivation of toggle
				}

				//jff 1/26/98 remove the plat if it bounced so it can be tried again
				//only affects plats that raise and bounce
				//killough 1/31/98: relax compatibility to demo_compatibility

				// remove the plat if its a pure raise type
				if(!comp[std::to_underlying(CompOption::Floors)])
				{
					switch(plat->type)
					{
						case PlatType::RaiseAndChange:
						case PlatType::RaiseToNearestAndChange:
							P_RemoveActivePlat(plat);
						default:
							break;
					}
				}
			}
			else if(heretic && !(leveltime & 31))
			{
				S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 32);
			}
			break;

		case PlatState::Waiting:          // plat is waiting
			if(!--plat->count) // downcount and check for delay elapsed
			{
				if(plat->sector->floorheight == plat->low)
					plat->status = PlatState::Up; // if at bottom, start up
				else
					plat->status = PlatState::Down; // if at top, start down

				// make plat start sound
				S_StartSectorSound(plat->sector, g_sfx_pstart);
			}
			break; //jff 1/27/98 don't pickup code added later to in_stasis

		case PlatState::InStasis: // do nothing if in stasis
			break;
	}
}

extern "C" void T_ZDoomPlatRaise(plat_t* plat)
{
	MoveResult res;

	switch(plat->status)
	{
		case PlatState::Up:
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, false);

			// if a pure raise type, make the plat moving sound
			if(plat->type == PlatType::PlatUpByValueStay
				|| plat->type == PlatType::PlatRaiseAndStay
				|| plat->type == PlatType::PlatRaiseAndStayLockout)
			{
				if(!(leveltime & 7))
					S_LoopSectorSound(plat->sector, g_sfx_stnmov_plats, 8);
			}

			if(res == MoveResult::Crushed && plat->crush == NO_CRUSH)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Down;
				S_StartSectorSound(plat->sector, g_sfx_pstart);
			}
			else if(res == MoveResult::PastDest)
			{
				if(plat->type != PlatType::PlatToggle)
				{
					plat->count = plat->wait;
					plat->status = PlatState::Waiting;
					S_StartSectorSound(plat->sector, g_sfx_pstop);

					switch(plat->type)
					{
						case PlatType::PlatRaiseAndStayLockout:
						case PlatType::PlatRaiseAndStay:
						case PlatType::PlatDownByValue:
						case PlatType::PlatDownWaitUpStay:
						case PlatType::PlatDownWaitUpStayStone:
						case PlatType::PlatUpByValueStay:
						case PlatType::PlatDownToNearestFloor:
						case PlatType::PlatDownToLowestCeiling:
							P_RemoveActivePlat(plat);
							break;
						default:
							break;
					}
				}
				else
				{
					plat->oldstatus = plat->status;
					plat->status = PlatState::InStasis;
				}
			}
			break;
		case PlatState::Down:
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, false);

			if(res == MoveResult::PastDest)
			{
				if(plat->type != PlatType::PlatToggle)
				{
					plat->count = plat->wait;
					plat->status = PlatState::Waiting;
					S_StartSectorSound(plat->sector, g_sfx_pstop);

					switch(plat->type)
					{
						case PlatType::PlatUpWaitDownStay:
						case PlatType::PlatUpNearestWaitDownStay:
						case PlatType::PlatUpByValue:
							P_RemoveActivePlat(plat);
							break;
						default:
							break;
					}
				}
				else
				{
					plat->oldstatus = plat->status;
					plat->status = PlatState::InStasis;
				}
			}
			else if(res == MoveResult::Crushed && plat->crush == NO_CRUSH && plat->type != PlatType::PlatToggle)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Up;
				S_StartSectorSound(plat->sector, g_sfx_pstart);
			}

			// jff 1/26/98 remove the plat if it bounced so it can be tried again
			// only affects plats that raise and bounce
			// remove the plat if it's a pure raise type
			switch(plat->type)
			{
				case PlatType::PlatUpByValueStay:
				case PlatType::PlatRaiseAndStay:
				case PlatType::PlatRaiseAndStayLockout:
					P_RemoveActivePlat(plat);
					break;
				default:
					break;
			}

			break;
		case PlatState::Waiting:
			if(!plat->count || !--plat->count)
			{
				if(plat->sector->floorheight == plat->low)
					plat->status = PlatState::Up;
				else
					plat->status = PlatState::Down;

				if(plat->type != PlatType::PlatToggle)
					S_StartSectorSound(plat->sector, g_sfx_pstart);
			}
			break;
		case PlatState::InStasis:
			break;
	}
}

void T_PlatRaise(plat_t* plat)
{
	map_format.t_plat_raise(plat);
}


//
// EV_DoPlat
//
// Handle Plat linedef types
//
// Passed the linedef that activated the plat, the type of plat action,
// and for some plat types, an amount to raise
// Returns true if a thinker is started, or restarted from stasis
//

int EV_DoZDoomPlat(int tag, line_t* line, PlatType type, fixed_t height,
	fixed_t speed, int delay, fixed_t lip, int change)
{
	plat_t* plat;
	const int* id_p;
	int rtn = 0;
	sector_t* sec;

	height *= FRACUNIT;
	lip *= FRACUNIT;

	if(tag)
	{
		// Activate all <type> plats that are in_stasis
		switch(type)
		{
			case PlatType::PlatToggle:
				rtn = 1;
			case PlatType::PlatPerpetualRaise:
				P_ActivateInStasis(tag);
				break;
			default:
				break;
		}
	}

	FIND_SECTORS2(id_p, tag, line)
	{
		sec = &sectors[*id_p];

		if(P_FloorActive(sec))
			continue;

		rtn = 1;

		plat = static_cast<plat_t*>(Z_MallocLevel(sizeof(*plat)));
		memset(plat, 0, sizeof(*plat));
		P_AddThinker(&plat->thinker);

		plat->sector = sec;
		sec->floordata = plat;
		plat->thinker.function = reinterpret_cast<think_t>(T_PlatRaise);
		plat->type = type;
		plat->crush = NO_CRUSH;
		plat->tag = tag;
		plat->speed = speed;
		plat->wait = delay;
		plat->low = sec->floorheight;
		plat->high = sec->floorheight;

		if(change)
		{
			if(line)
				sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
			if(change == 1)
				P_ResetSectorSpecial(sec);
		}

		switch(type)
		{
			case PlatType::PlatRaiseAndStay:
			case PlatType::PlatRaiseAndStayLockout:
				plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
				plat->status = PlatState::Up;
				P_ResetSectorSpecial(sec);
				S_StartSectorSound(sec, g_sfx_stnmov_plats);
				break;
			case PlatType::PlatUpByValue:
			case PlatType::PlatUpByValueStay:
				plat->high = sec->floorheight + height;
				plat->status = PlatState::Up;
				S_StartSectorSound(sec, g_sfx_stnmov_plats);
				break;
			case PlatType::PlatDownByValue:
				plat->low = sec->floorheight - height;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, g_sfx_stnmov_plats);
				break;
			case PlatType::PlatDownWaitUpStay:
			case PlatType::PlatDownWaitUpStayStone:
				plat->low = P_FindLowestFloorSurrounding(sec) + lip;
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, type == PlatType::PlatDownWaitUpStay ? g_sfx_pstart : g_sfx_stnmov_plats);
				break;
			case PlatType::PlatUpNearestWaitDownStay:
				plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
				plat->status = PlatState::Up;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;
			case PlatType::PlatUpWaitDownStay:
				plat->high = P_FindHighestFloorSurrounding(sec);
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->status = PlatState::Up;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;
			case PlatType::PlatPerpetualRaise:
				plat->low = P_FindLowestFloorSurrounding(sec) + lip;
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->status = (P_Random(RandomClass::Plats) & 1) ? PlatState::Up : PlatState::Down;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;
			case PlatType::PlatToggle:
				plat->crush = DOOM_CRUSH;
				plat->low = sec->ceilingheight;
				plat->status = PlatState::Down;
				break;
			case PlatType::PlatDownToNearestFloor:
				plat->low = P_FindNextLowestFloor(sec, sec->floorheight) + lip;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;
			case PlatType::PlatDownToLowestCeiling:
				plat->low = P_FindLowestCeilingSurrounding(sec);
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;
			default:
				break;
		}

		P_AddActivePlat(plat);
	}

	return rtn;
}

int EV_DoPlat
(line_t* line,
	PlatType type,
	int amount)
{
	plat_t* plat;
	const int* id_p;
	int rtn;
	sector_t* sec;

	rtn = 0;

	// Activate all <type> plats that are in_stasis
	switch(type)
	{
		case PlatType::PerpetualRaise:
			P_ActivateInStasis(line->special_args[0]);
			break;

		case PlatType::ToggleUpDn:
			P_ActivateInStasis(line->special_args[0]);
			rtn = 1;
			break;

		default:
			break;
	}

	// act on all sectors tagged the same as the activating linedef
	FIND_SECTORS(id_p, line->special_args[0])
	{
		sec = &sectors[*id_p];

		// don't start a second floor function if already moving
		if(P_FloorActive(sec)) //jff 2/23/98 multiple thinkers
			continue;

		// Create a thinker
		rtn = 1;
		plat = static_cast<plat_t*>(Z_MallocLevel(sizeof(*plat)));
		memset(plat, 0, sizeof(*plat));
		P_AddThinker(&plat->thinker);

		plat->type = type;
		plat->sector = sec;
		plat->sector->floordata = plat; //jff 2/23/98 multiple thinkers
		plat->thinker.function = reinterpret_cast<think_t>(T_PlatRaise);
		plat->crush = NO_CRUSH;
		plat->tag = line->special_args[0];

		//jff 1/26/98 Avoid raise plat bouncing a head off a ceiling and then
		//going down forever -- default low to plat height when triggered
		plat->low = sec->floorheight; // heretic_note: not in heretic

		// set up plat according to type
		switch(type)
		{
			case PlatType::RaiseToNearestAndChange:
				plat->speed = PLATSPEED / 2;
				sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
				plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
				plat->wait = 0;
				plat->status = PlatState::Up;
				P_ResetSectorSpecial(sec);

				S_StartSectorSound(sec, g_sfx_stnmov_plats);
				break;

			case PlatType::RaiseAndChange:
				plat->speed = PLATSPEED / 2;
				sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
				plat->high = sec->floorheight + amount * FRACUNIT;
				plat->wait = 0;
				plat->status = PlatState::Up;

				S_StartSectorSound(sec, g_sfx_stnmov_plats);
				break;

			case PlatType::DownWaitUpStay:
				plat->speed = PLATSPEED * 4;
				plat->low = P_FindLowestFloorSurrounding(sec);

				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;

				plat->high = sec->floorheight;
				plat->wait = TICRATE * PLATWAIT;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, g_sfx_pstart);
				break;

			case PlatType::BlazeDWUS:
				plat->speed = PLATSPEED * 8;
				plat->low = P_FindLowestFloorSurrounding(sec);

				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;

				plat->high = sec->floorheight;
				plat->wait = TICRATE * PLATWAIT;
				plat->status = PlatState::Down;
				S_StartSectorSound(sec, SfxId::Pstart);
				break;

			case PlatType::PerpetualRaise:
				plat->speed = PLATSPEED;
				plat->low = P_FindLowestFloorSurrounding(sec);

				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;

				plat->high = P_FindHighestFloorSurrounding(sec);

				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;

				plat->wait = TICRATE * PLATWAIT;
				plat->status = static_cast<PlatState>(P_Random(RandomClass::Plats) & 1);

				S_StartSectorSound(sec, g_sfx_pstart);
				break;

			case PlatType::ToggleUpDn:                //jff 3/14/98 add new type to support instant toggle
				plat->speed = PLATSPEED;    //not used
				plat->wait = TICRATE * PLATWAIT; //not used
				plat->crush = DOOM_CRUSH;   //jff 3/14/98 crush anything in the way

				// set up toggling between ceiling, floor inclusive
				plat->low = sec->ceilingheight;
				plat->high = sec->floorheight;
				plat->status = PlatState::Down;
				break;

			default:
				break;
		}
		P_AddActivePlat(plat); // add plat to list of active plats
	}
	return rtn;
}

// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active plats. It also avoids spending as much
// time searching for active plats. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_ActivateInStasis()
//
// Activate a plat that has been put in stasis
// (stopped perpetual floor, instant floor/ceil toggle)
//
// Passed the tag of the plat that should be reactivated
// Returns nothing
//
void P_ActivateInStasis(int tag)
{
	platlist_t* pl;
	for(pl = activeplats; pl; pl = pl->next) // search the active plats
	{
		plat_t* plat = pl->plat; // for one in stasis with right tag
		if(plat->tag == tag && plat->status == PlatState::InStasis)
		{
			if(plat->type == PlatType::ToggleUpDn) //jff 3/14/98 reactivate toggle type
				plat->status = plat->oldstatus == PlatState::Up ? PlatState::Down : PlatState::Up;
			else
				plat->status = plat->oldstatus;
			plat->thinker.function = reinterpret_cast<think_t>(T_PlatRaise);
		}
	}
}

//
// EV_StopPlat()
//
// Handler for "stop perpetual floor" linedef type
//
// Passed the linedef that stopped the plat
// Returns true if a plat was put in stasis
//
// jff 2/12/98 added int return value, fixed return
//

void EV_StopZDoomPlat(int tag, dboolean remove)
{
	platlist_t* pl;

	for(pl = activeplats; pl; pl = pl->next)
	{
		plat_t* plat = pl->plat;
		if(plat->status != PlatState::InStasis && plat->tag == tag)
		{
			if(!remove)
			{
				plat->oldstatus = plat->status;
				plat->status = PlatState::InStasis;
			}
			else
			{
				P_RemoveActivePlat(plat);
			}
		}
	}
}

int EV_StopPlat(line_t* line)
{
	platlist_t* pl;
	for(pl = activeplats; pl; pl = pl->next) // search the active plats
	{
		plat_t* plat = pl->plat; // for one with the tag not in stasis
		if(plat->status != PlatState::InStasis && plat->tag == line->special_args[0])
		{
			plat->oldstatus = plat->status; // put it in stasis
			plat->status = PlatState::InStasis;
			plat->thinker.function = nullptr;
		}
	}
	return 1;
}

//
// P_AddActivePlat()
//
// Add a plat to the head of the active plat list
//
// Passed a pointer to the plat to add
// Returns nothing
//
void P_AddActivePlat(plat_t* plat)
{
	platlist_t* list = static_cast<platlist_t*>(Z_Malloc(sizeof *list));
	list->plat = plat;
	plat->list = list;
	if((list->next = activeplats))
		list->next->prev = &list->next;
	list->prev = &activeplats;
	activeplats = list;
}

//
// P_RemoveActivePlat()
//
// Remove a plat from the active plat list
//
// Passed a pointer to the plat to remove
// Returns nothing
//
void P_RemoveActivePlat(plat_t* plat)
{
	platlist_t* list = plat->list;
	plat->sector->floordata = nullptr; //jff 2/23/98 multiple thinkers
	P_TagFinished(plat->sector->tag);
	P_RemoveThinker(&plat->thinker);
	if((*list->prev = list->next))
		list->next->prev = list->prev;
	Z_Free(list);
}

//
// P_RemoveAllActivePlats()
//
// Remove all plats from the active plat list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActivePlats()
{
	while(activeplats)
	{
		platlist_t* next = activeplats->next;
		Z_Free(activeplats);
		activeplats = next;
	}
}

// hexen

extern "C" void T_HexenPlatRaise(plat_t* plat)
{
	MoveResult res;

	switch(plat->status)
	{
		case PlatState::Up:
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->high, plat->crush, 1, true);
			if(res == MoveResult::Crushed && plat->crush == NO_CRUSH)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Down;
				SN_StartSequence((mobj_t*)&plat->sector->soundorg,
					std::to_underlying(SoundSequence::Platform) + static_cast<int>(plat->sector->seqType));
			}
			else if(res == MoveResult::PastDest)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Waiting;
				SN_StopSequence((mobj_t*)&plat->sector->soundorg);
				switch(plat->type)
				{
					case PlatType::PlatDownwaitupstay:
					case PlatType::PlatDownbyvaluewaitupstay:
						P_RemoveActivePlat(plat);
						break;
					default:
						break;
				}
			}
			break;
		case PlatState::Down:
			res = T_MoveFloorPlane(plat->sector, plat->speed, plat->low, NO_CRUSH, -1, true);
			if(res == MoveResult::PastDest)
			{
				plat->count = plat->wait;
				plat->status = PlatState::Waiting;
				switch(plat->type)
				{
					case PlatType::PlatUpwaitdownstay:
					case PlatType::PlatUpbyvaluewaitdownstay:
						P_RemoveActivePlat(plat);
						break;
					default:
						break;
				}
				SN_StopSequence((mobj_t*)&plat->sector->soundorg);
			}
			break;
		case PlatState::Waiting:
			if(!--plat->count)
			{
				if(plat->sector->floorheight == plat->low)
					plat->status = PlatState::Up;
				else
					plat->status = PlatState::Down;
				SN_StartSequence((mobj_t*)&plat->sector->soundorg,
					std::to_underlying(SoundSequence::Platform) + static_cast<int>(plat->sector->seqType));
			}
		default:
			break;
	}
}

int EV_DoHexenPlat(line_t* line, byte* args, PlatType type, int amount)
{
	plat_t* plat;
	const int* id_p;
	int rtn;
	sector_t* sec;

	rtn = 0;

	FIND_SECTORS(id_p, args[0])
	{
		sec = &sectors[*id_p];
		if(sec->floordata || sec->ceilingdata)
			continue;

		//
		// Find lowest & highest floors around sector
		//
		rtn = 1;
		plat = static_cast<plat_t*>(Z_MallocLevel(sizeof(*plat)));
		memset(plat, 0, sizeof(*plat));
		P_AddThinker(&plat->thinker);

		plat->type = type;
		plat->sector = sec;
		plat->sector->floordata = plat;
		plat->thinker.function = reinterpret_cast<think_t>(T_PlatRaise);
		plat->crush = NO_CRUSH;
		plat->tag = args[0];
		plat->speed = args[1] * (FRACUNIT / 8);
		switch(type)
		{
			case PlatType::PlatDownwaitupstay:
				plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = PlatState::Down;
				break;
			case PlatType::PlatDownbyvaluewaitupstay:
				plat->low = sec->floorheight - args[3] * 8 * FRACUNIT;
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = PlatState::Down;
				break;
			case PlatType::PlatUpwaitdownstay:
				plat->high = P_FindHighestFloorSurrounding(sec);
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->low = sec->floorheight;
				plat->wait = args[2];
				plat->status = PlatState::Up;
				break;
			case PlatType::PlatUpbyvaluewaitdownstay:
				plat->high = sec->floorheight + args[3] * 8 * FRACUNIT;
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->low = sec->floorheight;
				plat->wait = args[2];
				plat->status = PlatState::Up;
				break;
			case PlatType::PlatPerpetualraise:
				plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = static_cast<PlatState>(P_Random(RandomClass::Hexen) & 1);
				break;
			default:
				break;
		}
		P_AddActivePlat(plat);
		SN_StartSequence((mobj_t*)&sec->soundorg,
			std::to_underlying(SoundSequence::Platform) + static_cast<int>(sec->seqType));
	}
	return rtn;
}

// hexen_note: why set the tags? not sure if this is correct
void Hexen_EV_StopPlat(line_t* line, byte* args)
{
	platlist_t* pl;
	for(pl = activeplats; pl; pl = pl->next)
	{
		pl->plat->tag = args[0];

		if(pl->plat->tag != 0)
		{
			P_RemoveActivePlat(pl->plat);
			return;
		}
	}
}
