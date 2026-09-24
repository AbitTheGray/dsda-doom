// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Door animation code (opening/closing)
 */

#include <utility>

#include "doomstat.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "r_main.hpp"
#include "dstrings.hpp"
#include "d_deh.hpp"  // Ty 03/27/98 - externalized
#include "lprintf.hpp"
#include "e6y.hpp"//e6y

#include "dsda/id_list.hpp"
#include "dsda/map_format.hpp"
#include "dsda/messenger.hpp"

#include "hexen/p_acs.hpp"
#include "hexen/sn_sonix.hpp"

///////////////////////////////////////////////////////////////
//
// Door action routines, called once per tick
//
///////////////////////////////////////////////////////////////

//
// T_VerticalDoor
//
// Passed a door structure containing all info about the door.
// See P_SPEC.H for fields.
// Returns nothing.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

extern "C" void T_VerticalCompatibleDoor(vldoor_t* door)
{
	MoveResult res;

	// Is the door waiting, going up, or going down?
	switch(door->direction)
	{
		case 0:
			// Door is waiting
			if(!--door->topcountdown) // downcount and check
			{
				switch(door->type)
				{
					case VerticalDoorType::BlazeRaise:
					case VerticalDoorType::GenBlazeRaise:
						door->direction = -1; // time to go back down
						S_StartSectorSound(door->sector, SfxId::Bdcls);
						break;

					case VerticalDoorType::Normal:
					case VerticalDoorType::GenRaise:
					case VerticalDoorType::VldNormal:
						door->direction = -1; // time to go back down
						S_StartSectorSound(door->sector, g_sfx_dorcls);
						break;

					case VerticalDoorType::Close30ThenOpen:
					case VerticalDoorType::GenCdO:
					case VerticalDoorType::VldClose30ThenOpen:
						door->direction = 1; // time to go back up
						S_StartSectorSound(door->sector, g_sfx_doropn);
						break;

					case VerticalDoorType::GenBlazeCdO:
						door->direction = 1; // time to go back up
						S_StartSectorSound(door->sector, SfxId::Bdopn);
						break;

					default:
						break;
				}
			}
			break;

		case 2:
			// Special case for sector type door that waits before starting
			if(!--door->topcountdown)
			{
				switch(door->type)
				{
					case VerticalDoorType::WaitRaiseDoor:
					case VerticalDoorType::VldRaiseIn5Mins:
						door->direction = 1;        // time to raise then
						door->type = static_cast<VerticalDoorType>(g_door_normal); // door acts just like normal 1 DR door now
						S_StartSectorSound(door->sector, g_sfx_doropn);
						break;

					case VerticalDoorType::WaitCloseDoor:
						door->direction = -1;
						door->type = VerticalDoorType::CloseDoor;
						S_StartSectorSound(door->sector, g_sfx_dorcls);

					default:
						break;
				}
			}
			break;

		case -1:
			// Door is moving down
			res = T_MoveCeilingPlane
			(
				door->sector,
				door->speed,
				door->sector->floorheight,
				NO_CRUSH,
				door->direction,
				false
			);

			/* killough 10/98: implement gradual lighting effects */
			// e6y: "Tagged doors don't trigger special lighting" handled wrong
			// http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
			// Old code: if (door->lighttag && door->topheight - door->sector->floorheight)
			if(
				door->lighttag &&
				door->topheight - door->sector->floorheight &&
				compatibility_level >= CompLevel::Mbf
			)
				EV_LightTurnOnPartway(door->line,
					FixedDiv(door->sector->ceilingheight -
						door->sector->floorheight,
						door->topheight -
						door->sector->floorheight));

			// handle door reaching bottom
			if(res == MoveResult::PastDest)
			{
				switch(door->type)
				{
					// regular open and close doors are all done, remove them
					case VerticalDoorType::BlazeRaise:
					case VerticalDoorType::BlazeClose:
					case VerticalDoorType::GenBlazeRaise:
					case VerticalDoorType::GenBlazeClose:
						door->sector->ceilingdata = nullptr; //jff 2/22/98
						P_RemoveThinker(&door->thinker);  // unlink and free
						// killough 4/15/98: remove double-closing sound of blazing doors
						if(comp[std::to_underlying(CompOption::Blazing)])
							S_StartSectorSound(door->sector, SfxId::Bdcls);
						break;

					case VerticalDoorType::Normal:
					case VerticalDoorType::CloseDoor:
					case VerticalDoorType::GenRaise:
					case VerticalDoorType::GenClose:
						door->sector->ceilingdata = nullptr; //jff 2/22/98
						P_RemoveThinker(&door->thinker);  // unlink and free
						break;

					case VerticalDoorType::VldNormal:
					case VerticalDoorType::VldClose:
						door->sector->ceilingdata = nullptr;
						P_RemoveThinker(&door->thinker); // unlink and free
						S_StartSectorSound(door->sector, g_sfx_dorlnd);
						break;

					// close then open doors start waiting
					case VerticalDoorType::Close30ThenOpen:
					case VerticalDoorType::VldClose30ThenOpen:
						door->direction = 0;
						door->topcountdown = TICRATE * 30;
						break;

					case VerticalDoorType::GenCdO:
					case VerticalDoorType::GenBlazeCdO:
						door->direction = 0;
						door->topcountdown = door->topwait; // jff 5/8/98 insert delay
						break;

					default:
						break;
				}
				// e6y: "Tagged doors don't trigger special lighting" handled wrong
				// http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
				if(
					!heretic &&
					door->lighttag &&
					door->topheight - door->sector->floorheight &&
					compatibility_level < CompLevel::Mbf
				)
					EV_LightTurnOnPartway(door->line, 0);
			}
			/* jff 1/31/98 turn lighting off in tagged sectors of manual doors
			* killough 10/98: replaced with gradual lighting code
			*/
			else if(res == MoveResult::Crushed) // handle door meeting obstruction on way down
			{
				switch(door->type)
				{
					case VerticalDoorType::GenClose:
					case VerticalDoorType::GenBlazeClose:
					case VerticalDoorType::BlazeClose:
					case VerticalDoorType::CloseDoor:
					case VerticalDoorType::VldClose: // Close types do not bounce, merely wait
						break;

					case VerticalDoorType::BlazeRaise:
					case VerticalDoorType::GenBlazeRaise:
						door->direction = 1;
						if(!comp[std::to_underlying(CompOption::Blazing)])
						{
							S_StartSectorSound(door->sector, SfxId::Bdopn);
							break;
						}
					// fallthrough

					default: // other types bounce off the obstruction
						door->direction = 1;
						S_StartSectorSound(door->sector, g_sfx_doropn);
						break;
				}
			}
			break;

		case 1:
			// Door is moving up
			res = T_MoveCeilingPlane
			(
				door->sector,
				door->speed,
				door->topheight,
				NO_CRUSH,
				door->direction,
				false
			);

			/* killough 10/98: implement gradual lighting effects */
			// e6y: "Tagged doors don't trigger special lighting" handled wrong
			// http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
			// Old code: if (door->lighttag && door->topheight - door->sector->floorheight)
			if(
				door->lighttag &&
				door->topheight - door->sector->floorheight &&
				compatibility_level >= CompLevel::Mbf
			)
				EV_LightTurnOnPartway(door->line,
					FixedDiv(door->sector->ceilingheight -
						door->sector->floorheight,
						door->topheight -
						door->sector->floorheight));

			// handle door reaching the top
			if(res == MoveResult::PastDest)
			{
				switch(door->type)
				{
					case VerticalDoorType::BlazeRaise: // regular open/close doors start waiting
					case VerticalDoorType::Normal:
					case VerticalDoorType::GenRaise:
					case VerticalDoorType::GenBlazeRaise:
					case VerticalDoorType::VldNormal:
						door->direction = 0; // wait at top with delay
						door->topcountdown = door->topwait;
						break;

					case VerticalDoorType::Close30ThenOpen: // close and close/open doors are done
					case VerticalDoorType::BlazeOpen:
					case VerticalDoorType::OpenDoor:
					case VerticalDoorType::GenBlazeOpen:
					case VerticalDoorType::GenOpen:
					case VerticalDoorType::GenCdO:
					case VerticalDoorType::GenBlazeCdO:
						door->sector->ceilingdata = nullptr; //jff 2/22/98
						P_RemoveThinker(&door->thinker);  // unlink and free
						break;

					case VerticalDoorType::VldClose30ThenOpen:
					case VerticalDoorType::VldOpen:
						door->sector->ceilingdata = nullptr;
						P_RemoveThinker(&door->thinker); // unlink and free
						S_StopSound(&door->sector->soundorg);
						break;

					default:
						break;
				}

				/* jff 1/31/98 turn lighting on in tagged sectors of manual doors
			* killough 10/98: replaced with gradual lighting code */
				// e6y: "Tagged doors don't trigger special lighting" handled wrong
				// http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
				if(
					!heretic &&
					door->lighttag &&
					door->topheight - door->sector->floorheight &&
					compatibility_level < CompLevel::Mbf
				)
					EV_LightTurnOnPartway(door->line,FRACUNIT);
			}
			break;
	}
}

extern "C" void T_VerticalHexenDoor(vldoor_t* door)
{
	MoveResult res;

	switch(door->direction)
	{
		case 0: // WAITING
			if(!--door->topcountdown)
				switch(door->type)
				{
					case VerticalDoorType::DrevNormal:
						door->direction = -1; // time to go back down
						SN_StartSequence((mobj_t*)&door->sector->soundorg,
							std::to_underlying(SoundSequence::DoorStone) +
							static_cast<int>(door->sector->seqType));
						break;
					case VerticalDoorType::DrevClose30thenopen:
						door->direction = 1;
						break;
					default:
						break;
				}
			break;
		case 2: // INITIAL WAIT
			if(!--door->topcountdown)
			{
				switch(door->type)
				{
					case VerticalDoorType::DrevRaisein5mins:
						door->direction = 1;
						door->type = VerticalDoorType::DrevNormal;
						break;
					default:
						break;
				}
			}
			break;
		case -1: // DOWN
			res = T_MoveCeilingPlane(door->sector, door->speed,
				door->sector->floorheight, NO_CRUSH,
				door->direction, true);
			if(res == MoveResult::PastDest)
			{
				SN_StopSequence((mobj_t*)&door->sector->soundorg);
				switch(door->type)
				{
					case VerticalDoorType::DrevNormal:
					case VerticalDoorType::DrevClose:
						door->sector->ceilingdata = nullptr;
						P_TagFinished(door->sector->tag);
						P_RemoveThinker(&door->thinker); // unlink and free
						break;
					case VerticalDoorType::DrevClose30thenopen:
						door->direction = 0;
						door->topcountdown = TICRATE * 30;
						break;
					default:
						break;
				}
			}
			else if(res == MoveResult::Crushed)
			{
				switch(door->type)
				{
					case VerticalDoorType::DrevClose: // DON'T GO BACK UP!
						break;
					default:
						door->direction = 1;
						break;
				}
			}
			break;
		case 1: // UP
			res = T_MoveCeilingPlane(door->sector, door->speed,
				door->topheight, NO_CRUSH, door->direction, true);
			if(res == MoveResult::PastDest)
			{
				SN_StopSequence((mobj_t*)&door->sector->soundorg);
				switch(door->type)
				{
					case VerticalDoorType::DrevNormal:
						door->direction = 0; // wait at top
						door->topcountdown = door->topwait;
						break;
					case VerticalDoorType::DrevClose30thenopen:
					case VerticalDoorType::DrevOpen:
						door->sector->ceilingdata = nullptr;
						P_TagFinished(door->sector->tag);
						P_RemoveThinker(&door->thinker); // unlink and free
						break;
					default:
						break;
				}
			}
			break;
	}
}


void T_VerticalDoor(vldoor_t* door)
{
	map_format.t_vertical_door(door);
}

///////////////////////////////////////////////////////////////
//
// Door linedef handlers
//
///////////////////////////////////////////////////////////////

//
// EV_DoLockedDoor
//
// Handle opening a tagged locked door
//
// Passed the line activating the door, the type of door,
// and the thing that activated the line
// Returns true if a thinker created
//
int EV_DoLockedDoor
(line_t* line,
	VerticalDoorType type,
	mobj_t* thing)
{
	player_t* p;

	// only players can open locked doors
	p = thing->player;
	if(!p)
		return 0;

	// check type of linedef, and if key is possessed to open it
	switch(line->special)
	{
		case 99: // Blue Lock
		case 133:
			if(!p->cards[std::to_underlying(Card::BlueCard)] && !p->cards[std::to_underlying(Card::BlueSkull)])
			{
				dsda_AddPlayerMessage(s_PD_BLUEO, p);
				S_StartMobjSound(p->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;

		case 134: // Red Lock
		case 135:
			if(!p->cards[std::to_underlying(Card::RedCard)] && !p->cards[std::to_underlying(Card::RedSkull)])
			{
				dsda_AddPlayerMessage(s_PD_REDO, p);
				S_StartMobjSound(p->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;

		case 136: // Yellow Lock
		case 137:
			if(!p->cards[std::to_underlying(Card::YellowCard)] && !p->cards[std::to_underlying(Card::YellowSkull)])
			{
				dsda_AddPlayerMessage(s_PD_YELLOWO, p);
				S_StartMobjSound(p->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;
	}

	// got the key, so open the door
	return EV_DoDoor(line, static_cast<VerticalDoorType>(type));
}


//
// EV_DoDoor
//
// Handle opening a tagged door
//
// Passed the line activating the door and the type of door
// Returns true if a thinker created
//
int EV_DoDoor
(line_t* line,
	VerticalDoorType type)
{
	const int* id_p;
	int rtn;
	sector_t* sec;
	vldoor_t* door;

	rtn = 0;

	// open all doors with the same tag as the activating line
	FIND_SECTORS(id_p, line->special_args[0])
	{
		sec = &sectors[*id_p];
		// if the ceiling already moving, don't start the door action
		if(P_CeilingActive(sec)) //jff 2/22/98
			continue;

		// new door thinker
		rtn = 1;
		door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
		memset(door, 0, sizeof(*door));
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door; //jff 2/22/98

		door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
		door->sector = sec;
		door->type = type;
		door->topwait = VDOORWAIT;
		door->speed = VDOORSPEED;
		door->line = line;  // jff 1/31/98 remember line that triggered us
		door->lighttag = 0; /* killough 10/98: no light effects with tagged doors */

		// setup door parameters according to type of door
		switch(type)
		{
			case VerticalDoorType::BlazeClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				door->speed = VDOORSPEED * 4;
				S_StartSectorSound(door->sector, SfxId::Bdcls);
				break;

			case VerticalDoorType::CloseDoor:
			case VerticalDoorType::VldClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSectorSound(door->sector, g_sfx_dorcls);
				break;

			case VerticalDoorType::Close30ThenOpen:
			case VerticalDoorType::VldClose30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSectorSound(door->sector, g_sfx_dorcls);
				break;

			case VerticalDoorType::BlazeRaise:
			case VerticalDoorType::BlazeOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->speed = VDOORSPEED * 4;
				if(door->topheight != sec->ceilingheight)
					S_StartSectorSound(door->sector, SfxId::Bdopn);
				break;

			case VerticalDoorType::VldNormalTurbo:
				door->type = VerticalDoorType::VldNormal;
				door->speed = VDOORSPEED * 3;
			// fall through

			case VerticalDoorType::Normal:
			case VerticalDoorType::OpenDoor:
			case VerticalDoorType::VldNormal:
			case VerticalDoorType::VldOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if(door->topheight != sec->ceilingheight)
					S_StartSectorSound(door->sector, g_sfx_doropn);
				break;

			default:
				break;
		}
	}
	return rtn;
}


//
// EV_VerticalDoor
//
// Handle opening a door manually, no tag value
//
// Passed the line activating the door and the thing activating it
// Returns true if a thinker created
//
// jff 2/12/98 added int return value, fixed all returns
//
int EV_VerticalDoor
(line_t* line,
	mobj_t* thing)
{
	player_t* player;
	sector_t* sec;
	vldoor_t* door;

	// heretic_note: I don't see where the return value is used...keeping heretic signature
	if(heretic)
	{
		Heretic_EV_VerticalDoor(line, thing);
		return 0;
	}

	//  Check for locks
	player = thing->player;

	switch(line->special)
	{
		case 26: // Blue Lock
		case 32:
			if(!player)
				return 0;
			if(!player->cards[std::to_underlying(Card::BlueCard)] && !player->cards[std::to_underlying(Card::BlueSkull)])
			{
				dsda_AddPlayerMessage(s_PD_BLUEK, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;

		case 27: // Yellow Lock
		case 34:
			if(!player)
				return 0;
			if(!player->cards[std::to_underlying(Card::YellowCard)] && !player->cards[std::to_underlying(Card::YellowSkull)])
			{
				dsda_AddPlayerMessage(s_PD_YELLOWK, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;

		case 28: // Red Lock
		case 33:
			if(!player)
				return 0;
			if(!player->cards[std::to_underlying(Card::RedCard)] && !player->cards[std::to_underlying(Card::RedSkull)])
			{
				dsda_AddPlayerMessage(s_PD_REDK, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return 0;
			}
			break;

		default:
			break;
	}

	// if the wrong side of door is pushed, give oof sound
	if(line->sidenum[1] == NO_INDEX) // killough
	{
		S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
		return 0;
	}

	// get the sector on the second side of activating linedef
	sec = sides[line->sidenum[1]].sector;

	/* if door already has a thinker, use it
	* cph 2001/04/05 -
	* Ok, this is a disaster area. We're assuming that sec->ceilingdata
	*  is a vldoor_t! What if this door is controlled by both DR lines
	*  and by switches? I don't know how to fix that.
	* Secondly, original Doom didn't distinguish floor/lighting/ceiling
	*  actions, so we need to do the same in demo compatibility mode.
	*/
	door = static_cast<vldoor_t *>(sec->ceilingdata);
	if(demo_compatibility)
	{
		if(!door) door = static_cast<vldoor_t *>(sec->floordata);
	}
	/* If this is a repeatable line, and the door is already moving, then we can just reverse the current action. Note that in prboom 2.3.0 I erroneously removed the if-this-is-repeatable check, hence the prboom_4_compatibility clause below (foolishly assumed that already moving implies repeatable - but it could be moving due to another switch, e.g. lv19-509) */
	if(
		door &&
		(
			(compatibility_level == CompLevel::Prboom4) ||
			(line->special == 1) || (line->special == 117) || (line->special == 26) || (line->special == 27) || (line->special == 28)
		)
	)
	{
		/* For old demos we have to emulate the old buggy behavior and
		* mess up non-T_VerticalDoor actions.
		*/
		if(compatibility_level < CompLevel::Prboom4 ||
			door->thinker.function == reinterpret_cast<think_t>(T_VerticalDoor))
		{
			/* cph - we are writing outval to door->direction iff it is non-zero */
			signed int outval = 0;

			/* An already moving repeatable door which is being re-pressed, or a
			* monster is trying to open a closing door - so change direction
			* DEMOSYNC: we only read door->direction now if it really is a door.
			*/
			if(door->thinker.function == reinterpret_cast<think_t>(T_VerticalDoor) && door->direction == -1)
			{
				outval = 1; /* go back up */
			}
			else if(player)
			{
				outval = -1; /* go back down */
			}

			/* Write this to the thinker. In demo compatibility mode, we might be
			*  overwriting a field of a non-vldoor_t thinker - we need to add any
			*  other thinker types here if any demos depend on specific fields
			*  being corrupted by this.
			*/
			if(outval)
			{
				if(door->thinker.function == reinterpret_cast<think_t>(T_VerticalDoor))
				{
					door->direction = outval;
				}
				else if(door->thinker.function == reinterpret_cast<think_t>(T_PlatRaise))
				{
					plat_t* p = (plat_t*)door;
					p->wait = outval;
				}
				else
				{
					lprintf(OutputLevels::Debug, "EV_VerticalDoor: unknown thinker.function in thinker corruption emulation");
				}

				return 1;
			}
		}
		/* Either we're in prboom >=v2.3 and it's not a door, or it's a door but
		* we're a monster and don't want to shut it; exit with no action.
		*/
		return 0;
	}

	// emit proper sound
	switch(line->special)
	{
		case 117: // blazing door raise
		case 118: // blazing door open
			S_StartSectorSound(sec, SfxId::Bdopn);
			break;

		default: // normal or locked door sound
			S_StartSectorSound(sec, SfxId::Doropn);
			break;
	}

	// new door thinker
	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door; //jff 2/22/98
	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->direction = 1;
	door->speed = VDOORSPEED;
	door->topwait = VDOORWAIT;
	door->line = line; // jff 1/31/98 remember line that triggered us

	/* killough 10/98: use gradual lighting changes if nonzero tag given */
	door->lighttag = comp[std::to_underlying(CompOption::DoorLight)] ? 0 : line->special_args[0];

	// set the type of door from the activating linedef type
	switch(line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = VerticalDoorType::Normal;
			break;

		case 31:
		case 32:
		case 33:
		case 34:
			door->type = VerticalDoorType::OpenDoor;
			line->special = 0;
			break;

		case 117: // blazing door raise
			door->type = VerticalDoorType::BlazeRaise;
			door->speed = VDOORSPEED * 4;
			break;
		case 118: // blazing door open
			door->type = VerticalDoorType::BlazeOpen;
			line->special = 0;
			door->speed = VDOORSPEED * 4;
			break;

		default:
			door->lighttag = 0; // killough 10/98
			break;
	}

	// find the top and bottom of the movement range
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	return 1;
}


///////////////////////////////////////////////////////////////
//
// Sector type door spawners
//
///////////////////////////////////////////////////////////////

//
// P_SpawnDoorCloseIn30()
//
// Spawn a door that closes after 30 seconds (called at level init)
//
// Passed the sector of the door, whose type specified the door action
// Returns nothing
//
void P_SpawnDoorCloseIn30(sector_t* sec)
{
	vldoor_t* door;

	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));

	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);

	sec->ceilingdata = door; //jff 2/22/98
	sec->special = 0;

	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->direction = 0;
	door->type = static_cast<VerticalDoorType>(g_door_normal);
	door->speed = VDOORSPEED;
	door->topcountdown = 30 * TICRATE;
	door->line = nullptr;  // jff 1/31/98 remember line that triggered us
	door->lighttag = 0; /* killough 10/98: no lighting changes */
}

//
// P_SpawnDoorRaiseIn5Mins()
//
// Spawn a door that opens after 5 minutes (called at level init)
//
// Passed the sector of the door, whose type specified the door action
// Returns nothing
//
void P_SpawnDoorRaiseIn5Mins
(sector_t* sec,
	int secnum)
{
	vldoor_t* door;

	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));

	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);

	sec->ceilingdata = door; //jff 2/22/98
	sec->special = 0;

	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->direction = 2;
	door->type = static_cast<VerticalDoorType>(g_door_raise_in_5_mins);
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	door->topwait = VDOORWAIT;
	door->topcountdown = 5 * 60 * TICRATE;
	door->line = nullptr;  // jff 1/31/98 remember line that triggered us
	door->lighttag = 0; /* killough 10/98: no lighting changes */
}

// heretic

#include "p_inter.hpp"
#include "heretic/dstrings.hpp"

void Heretic_EV_VerticalDoor(line_t* line, mobj_t* thing)
{
	player_t* player;
	sector_t* sec;
	vldoor_t* door;
	int side;

	side = 0; // only front sides can be used
	//
	//      Check for locks
	//
	player = thing->player;
	switch(line->special)
	{
		case 26: // Blue Lock
		case 32:
			if(!player)
			{
				return;
			}
			if(!player->cards[std::to_underlying(Card::KeyBlue)])
			{
				P_SetMessage(player, HERETIC_TXT_NEEDBLUEKEY, false);
				S_StartVoidSound(SfxId::HereticPlroof);
				return;
			}
			break;
		case 27: // Yellow Lock
		case 34:
			if(!player)
			{
				return;
			}
			if(!player->cards[std::to_underlying(Card::KeyYellow)])
			{
				P_SetMessage(player, HERETIC_TXT_NEEDYELLOWKEY, false);
				S_StartVoidSound(SfxId::HereticPlroof);
				return;
			}
			break;
		case 28: // Green Lock
		case 33:
			if(!player)
			{
				return;
			}
			if(!player->cards[std::to_underlying(Card::KeyGreen)])
			{
				P_SetMessage(player, HERETIC_TXT_NEEDGREENKEY, false);
				S_StartVoidSound(SfxId::HereticPlroof);
				return;
			}
			break;
	}

	// if the sector has an active thinker, use it
	sec = sides[line->sidenum[side ^ 1]].sector;
	if(sec->ceilingdata)
	{
		door = static_cast<vldoor_t *>(sec->ceilingdata);
		switch(line->special)
		{
			case 1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
			case 26:
			case 27:
			case 28:
				if(door->direction == -1)
				{
					door->direction = 1; // go back up
				}
				else
				{
					if(!thing->player)
					{
						// Monsters don't close doors
						return;
					}
					door->direction = -1; // start going down immediately
				}
				return;
		}
	}

	S_StartSectorSound(sec, SfxId::HereticDoropn);

	//
	// new door thinker
	//
	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door;
	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->direction = 1;
	door->speed = VDOORSPEED;
	door->topwait = VDOORWAIT;
	door->line = line; // heretic_note: this is from doom
	door->lighttag = 0;
	switch(line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = VerticalDoorType::VldNormal;
			break;
		case 31:
		case 32:
		case 33:
		case 34:
			door->type = VerticalDoorType::VldOpen;
			line->special = 0;
			break;
	}

	//
	// find the top and bottom of the movement range
	//
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
}

// hexen

static void P_SpawnZDoomDoor(sector_t* sec, VerticalDoorType type, line_t* line, fixed_t speed,
	int topwait, int lightTag, int topcountdown)
{
	vldoor_t* door;

	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door;

	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->type = type;
	door->topwait = topwait;
	door->topcountdown = topcountdown;
	door->speed = speed;
	door->line = line;
	door->lighttag = lightTag;

	switch(type)
	{
		case VerticalDoorType::CloseDoor:
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4 * FRACUNIT;
			door->direction = -1;
			S_StartSectorSound(door->sector, g_sfx_dorcls);
			break;
		case VerticalDoorType::GenCdO:
			door->topheight = sec->ceilingheight;
			door->direction = -1;
			door->topwait = topwait;
			S_StartSectorSound(door->sector, g_sfx_dorcls);
			break;
		case VerticalDoorType::Normal:
		case VerticalDoorType::OpenDoor:
			door->direction = 1;
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4 * FRACUNIT;
			S_StartSectorSound(door->sector, g_sfx_doropn);
			break;
		case VerticalDoorType::WaitRaiseDoor:
			door->direction = 2;
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4 * FRACUNIT;
			break;
		case VerticalDoorType::WaitCloseDoor:
			door->direction = 2;
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4 * FRACUNIT;
			break;
		default:
			break;
	}
}

int EV_DoZDoomDoor(VerticalDoorType type, line_t* line, mobj_t* mo, int tag, fixed_t speed, int topwait,
	ZDoomLock lock, int lightTag, dboolean boomgen, int topcountdown)
{
	sector_t* sec;
	vldoor_t* door;

	speed *= FRACUNIT / 8;

	if(lock != ZDoomLock::None && !P_CheckKeys(mo, lock, true))
		return 0;

	if(!tag)
	{
		if(!line)
			return 0;

		// if the wrong side of door is pushed, give oof sound
		if(line->sidenum[1] == NO_INDEX)
		{
			if(mo->player) // is this check necessary?
				S_StartMobjSound(mo, SfxId::Oof);
			return 0;
		}

		// get the sector on the second side of activating linedef
		sec = sides[line->sidenum[1]].sector;

		door = static_cast<vldoor_t *>(sec->ceilingdata);

		if(door)
		{
			// Boom used remote door logic for generalized doors, even if they are manual
			if(boomgen)
				return 0;

			if(door->thinker.function == reinterpret_cast<think_t>(T_VerticalDoor))
			{
				// ONLY FOR "RAISE" DOORS, NOT "OPEN"s
				if(door->type == VerticalDoorType::Normal && type == VerticalDoorType::Normal)
				{
					if(door->direction == -1)
					{
						door->direction = 1;

						S_StartSectorSound(door->sector, g_sfx_doropn);
						return 1;
					}
					else if(!(line->activation & (SPAC_PUSH | SPAC_MPUSH)))
					// [RH] activate push doors don't go back down when you
					//    run into them (otherwise opening them would be
					//    a real pain).
					{
						if(!mo->player)
							return 0; // JDC: bad guys never close doors

						door->direction = -1; // start going down immediately

						S_StartSectorSound(door->sector, g_sfx_dorcls);
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}

			return 0;
		}

		P_SpawnZDoomDoor(sec, type, line, speed, topwait, lightTag, topcountdown);
		return 1;
	}
	else
	{
		const int* id_p;
		int retcode = 0;

		FIND_SECTORS(id_p, tag)
		{
			sec = &sectors[*id_p];
			if(sec->ceilingdata)
			{
				continue;
			}
			retcode = 1;
			P_SpawnZDoomDoor(sec, type, line, speed, topwait, lightTag, topcountdown);
		}

		return retcode;
	}
}

int Hexen_EV_DoDoor(line_t* line, byte* args, VerticalDoorType type)
{
	const int* id_p;
	int retcode;
	sector_t* sec;
	vldoor_t* door;
	fixed_t speed;

	speed = args[1] * FRACUNIT / 8;
	retcode = 0;
	FIND_SECTORS(id_p, args[0])
	{
		sec = &sectors[*id_p];
		if(sec->floordata || sec->ceilingdata)
		{
			continue;
		}
		// Add new door thinker
		retcode = 1;
		door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
		memset(door, 0, sizeof(*door));
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door;
		door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
		door->sector = sec;
		door->line = line;
		switch(type)
		{
			case VerticalDoorType::DrevClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				break;
			case VerticalDoorType::DrevClose30thenopen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				break;
			case VerticalDoorType::DrevNormal:
			case VerticalDoorType::DrevOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				break;
			default:
				break;
		}
		door->type = type;
		door->speed = speed;
		door->topwait = args[2];
		SN_StartSequence((mobj_t*)&door->sector->soundorg,
			std::to_underlying(SoundSequence::DoorStone) + static_cast<int>(door->sector->seqType));
	}
	return (retcode);
}

dboolean Hexen_EV_VerticalDoor(line_t* line, mobj_t* thing)
{
	sector_t* sec;
	vldoor_t* door;
	int side;

	side = 0; // only front sides can be used

	// if the sector has an active thinker, use it
	sec = sides[line->sidenum[side ^ 1]].sector;
	if(sec->floordata || sec->ceilingdata)
	{
		return false;
	}
	//
	// new door thinker
	//
	door = static_cast<vldoor_t*>(Z_MallocLevel(sizeof(*door)));
	memset(door, 0, sizeof(*door));
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door;
	door->thinker.function = reinterpret_cast<think_t>(T_VerticalDoor);
	door->sector = sec;
	door->line = line;
	door->direction = 1;
	switch(line->special)
	{
		case 11:
			door->type = VerticalDoorType::DrevOpen;
			line->special = 0;
			break;
		case 12:
		case 13:
			door->type = VerticalDoorType::DrevNormal;
			break;
		default:
			door->type = VerticalDoorType::DrevNormal;
			break;
	}
	door->speed = line->special_args[1] * (FRACUNIT / 8);
	door->topwait = line->special_args[2];

	//
	// find the top and bottom of the movement range
	//
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	SN_StartSequence((mobj_t*)&door->sector->soundorg,
		std::to_underlying(SoundSequence::DoorStone) + static_cast<int>(door->sector->seqType));
	return true;
}
