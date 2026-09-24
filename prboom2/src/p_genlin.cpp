// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Generalized linedef type handlers
 *  Floors, Ceilings, Doors, Locked Doors, Lifts, Stairs, Crushers
 */

#include <utility>

#include "doomstat.hpp" //jff 6/19/98 for demo_compatibility
#include "r_main.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "m_random.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "e6y.hpp"

#include "dsda/id_list.hpp"

// check if a manual trigger, if so do just the sector on the backside
#define FIND_GENLIN_SECTORS if (Trig == GenTriggerType::PushOnce || Trig == GenTriggerType::PushMany) \
                            { \
                              if (!(sec = line->backsector)) \
                                return rtn; \
                              manual_list[0] = sec->iSectorID; \
                              id_p = manual_list; \
                            } \
                            else \
                            { \
                              id_p = dsda_FindSectorsFromID(line->special_args[0]); \
                            }

//////////////////////////////////////////////////////////
//
// Generalized Linedef Type handlers
//
//////////////////////////////////////////////////////////

//
// EV_DoGenFloor()
//
// Handle generalized floor types
//
// Passed the line activating the generalized floor function
// Returns true if a thinker is created
//
// jff 02/04/98 Added this routine (and file) to handle generalized
// floor movers using bit fields in the line special type.
//
int EV_DoGenFloor
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int rtn;
	sector_t* sec;
	floormove_t* floor;
	unsigned value = (unsigned)line->special - GenFloorBase;

	// parse the bit fields in the line's special type

	int Crsh = (value & FloorCrush) >> FloorCrushShift;
	GenFloorChange ChgT = static_cast<GenFloorChange>((value & FloorChange) >> FloorChangeShift);
	GenFloorTarget Targ = static_cast<GenFloorTarget>((value & FloorTarget) >> FloorTargetShift);
	int Dirn = (value & FloorDirection) >> FloorDirectionShift;
	GenFloorModel ChgM = static_cast<GenFloorModel>((value & FloorModel) >> FloorModelShift);
	MotionSpeed Sped = static_cast<MotionSpeed>((value & FloorSpeed) >> FloorSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	FIND_GENLIN_SECTORS;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if floor already moving
		if(P_FloorActive(sec))
			continue;

		// new floor thinker
		rtn = 1;
		floor = static_cast<floormove_t*>(Z_MallocLevel(sizeof(*floor)));
		memset(floor, 0, sizeof(*floor));
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function = reinterpret_cast<think_t>(T_MoveFloor);
		floor->crush = (Crsh ? DOOM_CRUSH : NO_CRUSH);
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;
		floor->texture = sec->floorpic;
		P_CopyTransferSpecial(&floor->newspecial, sec);
		floor->type = FloorKind::GenFloor;

		// set the speed of motion
		switch(Sped)
		{
			case MotionSpeed::Slow:
				floor->speed = FLOORSPEED;
				break;
			case MotionSpeed::Normal:
				floor->speed = FLOORSPEED * 2;
				break;
			case MotionSpeed::Fast:
				floor->speed = FLOORSPEED * 4;
				break;
			case MotionSpeed::Turbo:
				floor->speed = FLOORSPEED * 8;
				break;
			default:
				break;
		}

		// set the destination height
		switch(Targ)
		{
			case GenFloorTarget::ToHnF:
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				break;
			case GenFloorTarget::ToLnF:
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;
			case GenFloorTarget::ToNnF:
				floor->floordestheight = Dirn ? P_FindNextHighestFloor(sec, sec->floorheight) : P_FindNextLowestFloor(sec, sec->floorheight);
				break;
			case GenFloorTarget::ToLnC:
				floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
				break;
			case GenFloorTarget::ToC:
				floor->floordestheight = sec->ceilingheight;
				break;
			case GenFloorTarget::ByST:
				floor->floordestheight = (floor->sector->floorheight >> FRACBITS) +
					floor->direction * (P_FindShortestTextureAround(*id_p) >> FRACBITS);
				if(floor->floordestheight > 32000)  //jff 3/13/98 prevent overflow
					floor->floordestheight = 32000; // wraparound in floor height
				if(floor->floordestheight < -32000)
					floor->floordestheight = -32000;
				floor->floordestheight <<= FRACBITS;
				break;
			case GenFloorTarget::By24:
				floor->floordestheight = floor->sector->floorheight +
					floor->direction * 24 * FRACUNIT;
				break;
			case GenFloorTarget::By32:
				floor->floordestheight = floor->sector->floorheight +
					floor->direction * 32 * FRACUNIT;
				break;
			default:
				break;
		}

		// set texture/type change properties
		if(ChgT != GenFloorChange::NoChg) // if a texture change is indicated
		{
			if(ChgM == GenFloorModel::NumericModel) // if a numeric model change
			{
				sector_t* sec;

				//jff 5/23/98 find model with ceiling at target height if target
				//is a ceiling type
				sec = (Targ == GenFloorTarget::ToLnC || Targ == GenFloorTarget::ToC) ? P_FindModelCeilingSector(floor->floordestheight, *id_p) : P_FindModelFloorSector(floor->floordestheight, *id_p);
				if(sec)
				{
					floor->texture = sec->floorpic;
					switch(ChgT)
					{
						case GenFloorChange::ChgZero: // zero type
							P_ResetTransferSpecial(&floor->newspecial);
							floor->type = FloorKind::GenFloorChg0;
							break;
						case GenFloorChange::ChgTyp: // copy type
							P_CopyTransferSpecial(&floor->newspecial, sec);
							floor->type = FloorKind::GenFloorChgT;
							break;
						case GenFloorChange::ChgTxt: // leave type be
							floor->type = FloorKind::GenFloorChg;
							break;
						default:
							break;
					}
				}
			}
			else // else if a trigger model change
			{
				floor->texture = line->frontsector->floorpic;
				switch(ChgT)
				{
					case GenFloorChange::ChgZero: // zero type
						P_ResetTransferSpecial(&floor->newspecial);
						floor->type = FloorKind::GenFloorChg0;
						break;
					case GenFloorChange::ChgTyp: // copy type
						P_CopyTransferSpecial(&floor->newspecial, line->frontsector);
						floor->type = FloorKind::GenFloorChgT;
						break;
					case GenFloorChange::ChgTxt: // leave type be
						floor->type = FloorKind::GenFloorChg;
					default:
						break;
				}
			}
		}
	}
	return rtn;
}


//
// EV_DoGenCeiling()
//
// Handle generalized ceiling types
//
// Passed the linedef activating the ceiling function
// Returns true if a thinker created
//
// jff 02/04/98 Added this routine (and file) to handle generalized
// floor movers using bit fields in the line special type.
//
int EV_DoGenCeiling
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int rtn;
	fixed_t targheight;
	sector_t* sec;
	ceiling_t* ceiling;
	unsigned value = (unsigned)line->special - GenCeilingBase;

	// parse the bit fields in the line's special type

	int Crsh = (value & CeilingCrush) >> CeilingCrushShift;
	GenCeilingChange ChgT = static_cast<GenCeilingChange>((value & CeilingChange) >> CeilingChangeShift);
	GenCeilingTarget Targ = static_cast<GenCeilingTarget>((value & CeilingTarget) >> CeilingTargetShift);
	int Dirn = (value & CeilingDirection) >> CeilingDirectionShift;
	GenCeilingModel ChgM = static_cast<GenCeilingModel>((value & CeilingModel) >> CeilingModelShift);
	MotionSpeed Sped = static_cast<MotionSpeed>((value & CeilingSpeed) >> CeilingSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	FIND_GENLIN_SECTORS;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if ceiling already moving
		if(P_CeilingActive(sec)) //jff 2/22/98
			continue;

		// new ceiling thinker
		rtn = 1;
		ceiling = static_cast<ceiling_t*>(Z_MallocLevel(sizeof(*ceiling)));
		memset(ceiling, 0, sizeof(*ceiling));
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling; //jff 2/22/98
		ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
		ceiling->crush = (Crsh ? DOOM_CRUSH : NO_CRUSH);
		ceiling->direction = Dirn ? 1 : -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		P_CopyTransferSpecial(&ceiling->newspecial, sec);
		ceiling->tag = sec->tag;
		ceiling->type = CeilingKind::GenCeiling;

		// set speed of motion
		switch(Sped)
		{
			case MotionSpeed::Slow:
				ceiling->speed = CEILSPEED;
				break;
			case MotionSpeed::Normal:
				ceiling->speed = CEILSPEED * 2;
				break;
			case MotionSpeed::Fast:
				ceiling->speed = CEILSPEED * 4;
				break;
			case MotionSpeed::Turbo:
				ceiling->speed = CEILSPEED * 8;
				break;
			default:
				break;
		}

		// set destination target height
		targheight = sec->ceilingheight;
		switch(Targ)
		{
			case GenCeilingTarget::ToHnC:
				targheight = P_FindHighestCeilingSurrounding(sec);
				break;
			case GenCeilingTarget::ToLnC:
				targheight = P_FindLowestCeilingSurrounding(sec);
				break;
			case GenCeilingTarget::ToNnC:
				targheight = Dirn ? P_FindNextHighestCeiling(sec, sec->ceilingheight) : P_FindNextLowestCeiling(sec, sec->ceilingheight);
				break;
			case GenCeilingTarget::ToHnF:
				targheight = P_FindHighestFloorSurrounding(sec);
				break;
			case GenCeilingTarget::ToF:
				targheight = sec->floorheight;
				break;
			case GenCeilingTarget::ByST:
				targheight = (ceiling->sector->ceilingheight >> FRACBITS) +
					ceiling->direction * (P_FindShortestUpperAround(*id_p) >> FRACBITS);
				if(targheight > 32000)  //jff 3/13/98 prevent overflow
					targheight = 32000; // wraparound in ceiling height
				if(targheight < -32000)
					targheight = -32000;
				targheight <<= FRACBITS;
				break;
			case GenCeilingTarget::By24:
				targheight = ceiling->sector->ceilingheight +
					ceiling->direction * 24 * FRACUNIT;
				break;
			case GenCeilingTarget::By32:
				targheight = ceiling->sector->ceilingheight +
					ceiling->direction * 32 * FRACUNIT;
				break;
			default:
				break;
		}
		if(Dirn) ceiling->topheight = targheight;
		else ceiling->bottomheight = targheight;

		// set texture/type change properties
		if(ChgT != GenCeilingChange::NoChg) // if a texture change is indicated
		{
			if(ChgM == GenCeilingModel::NumericModel) // if a numeric model change
			{
				sector_t* sec;

				//jff 5/23/98 find model with floor at target height if target
				//is a floor type
				sec = (Targ == GenCeilingTarget::ToHnF || Targ == GenCeilingTarget::ToF) ? P_FindModelFloorSector(targheight, *id_p) : P_FindModelCeilingSector(targheight, *id_p);
				if(sec)
				{
					ceiling->texture = sec->ceilingpic;
					switch(ChgT)
					{
						case GenCeilingChange::ChgZero: // type is zeroed
							P_ResetTransferSpecial(&ceiling->newspecial);
							ceiling->type = CeilingKind::GenCeilingChg0;
							break;
						case GenCeilingChange::ChgTyp: // type is copied
							P_CopyTransferSpecial(&ceiling->newspecial, sec);
							ceiling->type = CeilingKind::GenCeilingChgT;
							break;
						case GenCeilingChange::ChgTxt: // type is left alone
							ceiling->type = CeilingKind::GenCeilingChg;
							break;
						default:
							break;
					}
				}
			}
			else // else if a trigger model change
			{
				ceiling->texture = line->frontsector->ceilingpic;
				switch(ChgT)
				{
					case GenCeilingChange::ChgZero: // type is zeroed
						P_ResetTransferSpecial(&ceiling->newspecial);
						ceiling->type = CeilingKind::GenCeilingChg0;
						break;
					case GenCeilingChange::ChgTyp: // type is copied
						P_CopyTransferSpecial(&ceiling->newspecial, line->frontsector);
						ceiling->type = CeilingKind::GenCeilingChgT;
						break;
					case GenCeilingChange::ChgTxt: // type is left alone
						ceiling->type = CeilingKind::GenCeilingChg;
						break;
					default:
						break;
				}
			}
		}
		P_AddActiveCeiling(ceiling); // add this ceiling to the active list
	}
	return rtn;
}

//
// EV_DoGenLift()
//
// Handle generalized lift types
//
// Passed the linedef activating the lift
// Returns true if a thinker is created
//
int EV_DoGenLift
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	plat_t* plat;
	int rtn;
	sector_t* sec;
	unsigned value = (unsigned)line->special - GenLiftBase;

	// parse the bit fields in the line's special type

	GenLiftTarget Targ = static_cast<GenLiftTarget>((value & LiftTarget) >> LiftTargetShift);
	int Dely = (value & LiftDelay) >> LiftDelayShift;
	MotionSpeed Sped = static_cast<MotionSpeed>((value & LiftSpeed) >> LiftSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	// Activate all <type> plats that are in_stasis

	if(Targ == GenLiftTarget::LnF2HnF)
		P_ActivateInStasis(line->special_args[0]);

	FIND_GENLIN_SECTORS;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if floor already moving
		if(P_FloorActive(sec))
			continue;

		// Setup the plat thinker
		rtn = 1;
		plat = static_cast<plat_t*>(Z_MallocLevel(sizeof(*plat)));
		memset(plat, 0, sizeof(*plat));
		P_AddThinker(&plat->thinker);

		plat->sector = sec;
		plat->sector->floordata = plat;
		plat->thinker.function = reinterpret_cast<think_t>(T_PlatRaise);
		plat->crush = NO_CRUSH;
		plat->tag = line->special_args[0];

		plat->type = PlatType::GenLift;
		plat->high = sec->floorheight;
		plat->status = PlatState::Down;

		// setup the target destination height
		switch(Targ)
		{
			case GenLiftTarget::F2LnF:
				plat->low = P_FindLowestFloorSurrounding(sec);
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				break;
			case GenLiftTarget::F2NnF:
				plat->low = P_FindNextLowestFloor(sec, sec->floorheight);
				break;
			case GenLiftTarget::F2LnC:
				plat->low = P_FindLowestCeilingSurrounding(sec);
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				break;
			case GenLiftTarget::LnF2HnF:
				plat->type = PlatType::GenPerpetual;
				plat->low = P_FindLowestFloorSurrounding(sec);
				if(plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if(plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->status = static_cast<PlatState>(P_Random(RandomClass::Genlift) & 1);
				break;
			default:
				break;
		}

		// setup the speed of motion
		switch(Sped)
		{
			case MotionSpeed::Slow:
				plat->speed = PLATSPEED * 2;
				break;
			case MotionSpeed::Normal:
				plat->speed = PLATSPEED * 4;
				break;
			case MotionSpeed::Fast:
				plat->speed = PLATSPEED * 8;
				break;
			case MotionSpeed::Turbo:
				plat->speed = PLATSPEED * 16;
				break;
			default:
				break;
		}

		// setup the delay time before the floor returns
		switch(Dely)
		{
			case 0:
				plat->wait = 1 * TICRATE;
				break;
			case 1:
				plat->wait = PLATWAIT * TICRATE;
				break;
			case 2:
				plat->wait = 5 * TICRATE;
				break;
			case 3:
				plat->wait = 10 * TICRATE;
				break;
		}

		S_StartSectorSound(sec, SfxId::Pstart);
		P_AddActivePlat(plat); // add this plat to the list of active plats
	}
	return rtn;
}

//
// EV_DoGenStairs()
//
// Handle generalized stair building
//
// Passed the linedef activating the stairs
// Returns true if a thinker is created
//
int EV_DoGenStairs
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int height;
	int i;
	int oldsecnum;
	int newsecnum;
	int texture;
	int ok;
	int rtn;

	sector_t* sec;
	sector_t* tsec;

	floormove_t* floor;

	fixed_t stairsize;
	fixed_t speed;

	unsigned value = (unsigned)line->special - GenStairsBase;

	// parse the bit fields in the line's special type

	int Igno = (value & StairIgnore) >> StairIgnoreShift;
	int Dirn = (value & StairDirection) >> StairDirectionShift;
	int Step = (value & StairStep) >> StairStepShift;
	MotionSpeed Sped = static_cast<MotionSpeed>((value & StairSpeed) >> StairSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	FIND_GENLIN_SECTORS;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		//Do not start another function if floor already moving
		//jff 2/26/98 add special lockout condition to wait for entire
		//staircase to build before retriggering
		if(P_FloorActive(sec) || sec->stairlock)
			continue;

		// new floor thinker
		rtn = 1;
		floor = static_cast<floormove_t*>(Z_MallocLevel(sizeof(*floor)));
		memset(floor, 0, sizeof(*floor));
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function = reinterpret_cast<think_t>(T_MoveFloor);
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;

		// setup speed of stair building
		switch(Sped)
		{
			default:
			case MotionSpeed::Slow:
				floor->speed = FLOORSPEED / 4;
				break;
			case MotionSpeed::Normal:
				floor->speed = FLOORSPEED / 2;
				break;
			case MotionSpeed::Fast:
				floor->speed = FLOORSPEED * 2;
				break;
			case MotionSpeed::Turbo:
				floor->speed = FLOORSPEED * 4;
				break;
		}

		// setup stepsize for stairs
		switch(Step)
		{
			default:
			case 0:
				stairsize = 4 * FRACUNIT;
				break;
			case 1:
				stairsize = 8 * FRACUNIT;
				break;
			case 2:
				stairsize = 16 * FRACUNIT;
				break;
			case 3:
				stairsize = 24 * FRACUNIT;
				break;
		}

		speed = floor->speed;
		height = sec->floorheight + floor->direction * stairsize;
		floor->floordestheight = height;
		texture = sec->floorpic;
		floor->crush = NO_CRUSH;
		floor->type = FloorKind::GenBuildStair; // jff 3/31/98 do not leave uninited

		sec->stairlock = -2; // jff 2/26/98 set up lock on current sector
		sec->nextsec = -1;
		sec->prevsec = -1;

		oldsecnum = *id_p;
		// Find next sector to raise
		// 1.     Find 2-sided line with same sector side[0]
		// 2.     Other side is the next sector to raise
		do
		{
			ok = 0;
			for(i = 0; i < sec->linecount; i++)
			{
				if(!((sec->lines[i])->backsector))
					continue;

				tsec = (sec->lines[i])->frontsector;
				newsecnum = tsec->iSectorID;

				if(oldsecnum != newsecnum)
					continue;

				tsec = (sec->lines[i])->backsector;
				newsecnum = tsec->iSectorID;

				if(!Igno && tsec->floorpic != texture)
					continue;

				/* jff 6/19/98 prevent double stepsize */
				if(compatibility_level < CompLevel::Boom202)
					height += floor->direction * stairsize;

				//jff 2/26/98 special lockout condition for retriggering
				if(P_FloorActive(tsec) || tsec->stairlock)
					continue;

				/* jff 6/19/98 increase height AFTER continue */
				if(compatibility_level >= CompLevel::Boom202)
					height += floor->direction * stairsize;

				// jff 2/26/98
				// link the stair chain in both directions
				// lock the stair sector until building complete
				sec->nextsec = newsecnum;  // link step to next
				tsec->prevsec = oldsecnum; // link next back
				tsec->nextsec = -1;        // set next forward link as end
				tsec->stairlock = -2;      // lock the step

				sec = tsec;
				oldsecnum = newsecnum;
				floor = static_cast<floormove_t*>(Z_MallocLevel(sizeof(*floor)));

				memset(floor, 0, sizeof(*floor));
				P_AddThinker(&floor->thinker);

				sec->floordata = floor;
				floor->thinker.function = reinterpret_cast<think_t>(T_MoveFloor);
				floor->direction = Dirn ? 1 : -1;
				floor->sector = sec;
				floor->speed = speed;
				floor->floordestheight = height;
				floor->crush = NO_CRUSH;
				floor->type = FloorKind::GenBuildStair; // jff 3/31/98 do not leave uninited

				ok = 1;
				break;
			}
		}
		while(ok);
	}
	// retriggerable generalized stairs build up or down alternately
	if(rtn)
		line->special ^= StairDirection; // alternate dir on succ activations
	return rtn;
}

//
// EV_DoGenCrusher()
//
// Handle generalized crusher types
//
// Passed the linedef activating the crusher
// Returns true if a thinker created
//
int EV_DoGenCrusher
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int rtn;
	sector_t* sec;
	ceiling_t* ceiling;
	unsigned value = (unsigned)line->special - GenCrusherBase;

	// parse the bit fields in the line's special type

	int Slnt = (value & CrusherSilent) >> CrusherSilentShift;
	MotionSpeed Sped = static_cast<MotionSpeed>((value & CrusherSpeed) >> CrusherSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	//jff 2/22/98  Reactivate in-stasis ceilings...for certain types.
	//jff 4/5/98 return if activated
	rtn = P_ActivateInStasisCeiling(line->special_args[0]);

	FIND_GENLIN_SECTORS;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if ceiling already moving
		if(P_CeilingActive(sec)) //jff 2/22/98
			continue;

		// new ceiling thinker
		rtn = 1;
		ceiling = static_cast<ceiling_t*>(Z_MallocLevel(sizeof(*ceiling)));
		memset(ceiling, 0, sizeof(*ceiling));
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling; //jff 2/22/98
		ceiling->thinker.function = reinterpret_cast<think_t>(T_MoveCeiling);
		ceiling->crush = DOOM_CRUSH;
		ceiling->direction = -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		P_CopyTransferSpecial(&ceiling->newspecial, sec);
		ceiling->tag = sec->tag;
		ceiling->type = Slnt ? CeilingKind::GenSilentCrusher : CeilingKind::GenCrusher;
		ceiling->silent = (ceiling->type == CeilingKind::GenSilentCrusher);
		ceiling->topheight = sec->ceilingheight;
		ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);

		// setup ceiling motion speed
		switch(Sped)
		{
			case MotionSpeed::Slow:
				ceiling->speed = CEILSPEED;
				break;
			case MotionSpeed::Normal:
				ceiling->speed = CEILSPEED * 2;
				break;
			case MotionSpeed::Fast:
				ceiling->speed = CEILSPEED * 4;
				break;
			case MotionSpeed::Turbo:
				ceiling->speed = CEILSPEED * 8;
				break;
			default:
				break;
		}
		ceiling->oldspeed = ceiling->speed;

		P_AddActiveCeiling(ceiling); // add to list of active ceilings
	}
	return rtn;
}

//
// EV_DoGenLockedDoor()
//
// Handle generalized locked door types
//
// Passed the linedef activating the generalized locked door
// Returns true if a thinker created
//
int EV_DoGenLockedDoor
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int rtn;
	sector_t* sec;
	vldoor_t* door;
	unsigned value = (unsigned)line->special - GenLockedBase;

	// parse the bit fields in the line's special type

	GenDoorKind Kind = static_cast<GenDoorKind>((value & LockedKind) >> LockedKindShift);
	MotionSpeed Sped = static_cast<MotionSpeed>((value & LockedSpeed) >> LockedSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	FIND_GENLIN_SECTORS;

	rtn = 0;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if ceiling already moving
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
		door->topwait = VDOORWAIT;
		door->line = line;
		door->topheight = P_FindLowestCeilingSurrounding(sec);
		door->topheight -= 4 * FRACUNIT;
		door->direction = 1;

		/* killough 10/98: implement gradual lighting */
		door->lighttag = !comp[std::to_underlying(CompOption::DoorLight)] &&
			(line->special & 6) == 6 &&
			line->special > GenLockedBase
			? line->special_args[0]
			: 0;

		// setup speed of door motion
		switch(Sped)
		{
			default:
			case MotionSpeed::Slow:
				door->type = Kind != GenDoorKind::OpenDelayClose ? VerticalDoorType::GenOpen : VerticalDoorType::GenRaise;
				door->speed = VDOORSPEED;
				break;
			case MotionSpeed::Normal:
				door->type = Kind != GenDoorKind::OpenDelayClose ? VerticalDoorType::GenOpen : VerticalDoorType::GenRaise;
				door->speed = VDOORSPEED * 2;
				break;
			case MotionSpeed::Fast:
				door->type = Kind != GenDoorKind::OpenDelayClose ? VerticalDoorType::GenBlazeOpen : VerticalDoorType::GenBlazeRaise;
				door->speed = VDOORSPEED * 4;
				break;
			case MotionSpeed::Turbo:
				door->type = Kind != GenDoorKind::OpenDelayClose ? VerticalDoorType::GenBlazeOpen : VerticalDoorType::GenBlazeRaise;
				door->speed = VDOORSPEED * 8;

				break;
		}

		// killough 4/15/98: fix generalized door opening sounds
		// (previously they always had the blazing door close sound)
		S_StartSectorSound(door->sector, door->speed >= VDOORSPEED * 4 ? SfxId::Bdopn : SfxId::Doropn);
	}
	return rtn;
}

//
// EV_DoGenDoor()
//
// Handle generalized door types
//
// Passed the linedef activating the generalized door
// Returns true if a thinker created
//
int EV_DoGenDoor
(line_t* line)
{
	const int* id_p;
	int manual_list[2] = {-1, -1};
	int rtn;
	sector_t* sec;
	vldoor_t* door;
	unsigned value = (unsigned)line->special - GenDoorBase;

	// parse the bit fields in the line's special type

	int Dely = (value & DoorDelay) >> DoorDelayShift;
	GenDoorKind Kind = static_cast<GenDoorKind>((value & DoorKind) >> DoorKindShift);
	MotionSpeed Sped = static_cast<MotionSpeed>((value & DoorSpeed) >> DoorSpeedShift);
	GenTriggerType Trig = static_cast<GenTriggerType>((value & TriggerType) >> TriggerTypeShift);

	rtn = 0;

	FIND_GENLIN_SECTORS;

	rtn = 0;

	for(; *id_p >= 0; id_p++)
	{
		sec = &sectors[*id_p];

		// Do not start another function if ceiling already moving
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
		// setup delay for door remaining open/closed
		switch(Dely)
		{
			default:
			case 0:
				door->topwait = TICRATE;
				break;
			case 1:
				door->topwait = VDOORWAIT;
				break;
			case 2:
				door->topwait = 2 * VDOORWAIT;
				break;
			case 3:
				door->topwait = 7 * VDOORWAIT;
				break;
		}

		// setup speed of door motion
		switch(Sped)
		{
			default:
			case MotionSpeed::Slow:
				door->speed = VDOORSPEED;
				break;
			case MotionSpeed::Normal:
				door->speed = VDOORSPEED * 2;
				break;
			case MotionSpeed::Fast:
				door->speed = VDOORSPEED * 4;
				break;
			case MotionSpeed::Turbo:
				door->speed = VDOORSPEED * 8;
				break;
		}
		door->line = line; // jff 1/31/98 remember line that triggered us

		/* killough 10/98: implement gradual lighting */
		door->lighttag = !comp[std::to_underlying(CompOption::DoorLight)] &&
			(line->special & 6) == 6 &&
			line->special > GenLockedBase
			? line->special_args[0]
			: 0;

		// set kind of door, whether it opens then close, opens, closes etc.
		// assign target heights accordingly
		switch(Kind)
		{
			case GenDoorKind::OpenDelayClose:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if(door->topheight != sec->ceilingheight)
					S_StartSectorSound(door->sector,
						Sped >= MotionSpeed::Fast || comp[std::to_underlying(CompOption::Sound)] ? SfxId::Bdopn : SfxId::Doropn);
				door->type = Sped >= MotionSpeed::Fast ? VerticalDoorType::GenBlazeRaise : VerticalDoorType::GenRaise;
				break;
			case GenDoorKind::Open:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if(door->topheight != sec->ceilingheight)
					S_StartSectorSound(door->sector,
						Sped >= MotionSpeed::Fast || comp[std::to_underlying(CompOption::Sound)] ? SfxId::Bdopn : SfxId::Doropn);
				door->type = Sped >= MotionSpeed::Fast ? VerticalDoorType::GenBlazeOpen : VerticalDoorType::GenOpen;
				break;
			case GenDoorKind::CloseDelayOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSectorSound(door->sector,
					Sped >= MotionSpeed::Fast && !comp[std::to_underlying(CompOption::Sound)] ? SfxId::Bdcls : SfxId::Dorcls);
				door->type = Sped >= MotionSpeed::Fast ? VerticalDoorType::GenBlazeCdO : VerticalDoorType::GenCdO;
				break;
			case GenDoorKind::Close:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSectorSound(door->sector,
					Sped >= MotionSpeed::Fast && !comp[std::to_underlying(CompOption::Sound)] ? SfxId::Bdcls : SfxId::Dorcls);
				door->type = Sped >= MotionSpeed::Fast ? VerticalDoorType::GenBlazeClose : VerticalDoorType::GenClose;
				break;
			default:
				break;
		}
	}
	return rtn;
}
