// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "p_mobj.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "p_map.hpp"
#include "p_inter.hpp"
#include "p_tick.hpp"

#include "hexen/a_action.hpp"

#include "p_things.hpp"

static dboolean ActivateThing(mobj_t* mobj);
static dboolean DeactivateThing(mobj_t* mobj);

MobjType TranslateThingType[] = {
	MobjType::HexenMapspot,           // T_NONE
	MobjType::HexenCentaur,           // T_CENTAUR
	MobjType::HexenCentaurleader,     // T_CENTAURLEADER
	MobjType::HexenDemon,             // T_DEMON
	MobjType::HexenEttin,             // T_ETTIN
	MobjType::HexenFiredemon,         // T_FIREGARGOYLE
	MobjType::HexenSerpent,           // T_WATERLURKER
	MobjType::HexenSerpentleader,     // T_WATERLURKERLEADER
	MobjType::HexenWraith,            // T_WRAITH
	MobjType::HexenWraithb,           // T_WRAITHBURIED
	MobjType::HexenFireball1,         // T_FIREBALL1
	MobjType::HexenMana1,             // T_MANA1
	MobjType::HexenMana2,             // T_MANA2
	MobjType::HexenSpeedboots,        // T_ITEMBOOTS
	MobjType::HexenArtiegg,           // T_ITEMEGG
	MobjType::HexenArtifly,           // T_ITEMFLIGHT
	MobjType::HexenSummonmaulator,    // T_ITEMSUMMON
	MobjType::HexenTeleportother,     // T_ITEMTPORTOTHER
	MobjType::HexenArtiteleport,      // T_ITEMTELEPORT
	MobjType::HexenBishop,            // T_BISHOP
	MobjType::HexenIceguy,            // T_ICEGOLEM
	MobjType::HexenBridge,            // T_BRIDGE
	MobjType::HexenBoostarmor,        // T_DRAGONSKINBRACERS
	MobjType::HexenHealingbottle,     // T_ITEMHEALTHPOTION
	MobjType::HexenHealthflask,       // T_ITEMHEALTHFLASK
	MobjType::HexenArtisuperheal,     // T_ITEMHEALTHFULL
	MobjType::HexenBoostmana,         // T_ITEMBOOSTMANA
	MobjType::HexenFwAxe,            // T_FIGHTERAXE
	MobjType::HexenFwHammer,         // T_FIGHTERHAMMER
	MobjType::HexenFwSword1,         // T_FIGHTERSWORD1
	MobjType::HexenFwSword2,         // T_FIGHTERSWORD2
	MobjType::HexenFwSword3,         // T_FIGHTERSWORD3
	MobjType::HexenCwSerpstaff,      // T_CLERICSTAFF
	MobjType::HexenCwHoly1,          // T_CLERICHOLY1
	MobjType::HexenCwHoly2,          // T_CLERICHOLY2
	MobjType::HexenCwHoly3,          // T_CLERICHOLY3
	MobjType::HexenMwCone,           // T_MAGESHARDS
	MobjType::HexenMwStaff1,         // T_MAGESTAFF1
	MobjType::HexenMwStaff2,         // T_MAGESTAFF2
	MobjType::HexenMwStaff3,         // T_MAGESTAFF3
	MobjType::HexenEggfx,             // T_MORPHBLAST
	MobjType::HexenRock1,             // T_ROCK1
	MobjType::HexenRock2,             // T_ROCK2
	MobjType::HexenRock3,             // T_ROCK3
	MobjType::HexenDirt1,             // T_DIRT1
	MobjType::HexenDirt2,             // T_DIRT2
	MobjType::HexenDirt3,             // T_DIRT3
	MobjType::HexenDirt4,             // T_DIRT4
	MobjType::HexenDirt5,             // T_DIRT5
	MobjType::HexenDirt6,             // T_DIRT6
	MobjType::HexenArrow,             // T_ARROW
	MobjType::HexenDart,              // T_DART
	MobjType::HexenPoisondart,        // T_POISONDART
	MobjType::HexenRipperball,        // T_RIPPERBALL
	MobjType::HexenSgshard1,          // T_STAINEDGLASS1
	MobjType::HexenSgshard2,          // T_STAINEDGLASS2
	MobjType::HexenSgshard3,          // T_STAINEDGLASS3
	MobjType::HexenSgshard4,          // T_STAINEDGLASS4
	MobjType::HexenSgshard5,          // T_STAINEDGLASS5
	MobjType::HexenSgshard6,          // T_STAINEDGLASS6
	MobjType::HexenSgshard7,          // T_STAINEDGLASS7
	MobjType::HexenSgshard8,          // T_STAINEDGLASS8
	MobjType::HexenSgshard9,          // T_STAINEDGLASS9
	MobjType::HexenSgshard0,          // T_STAINEDGLASS0
	MobjType::HexenProjectileBlade,  // T_BLADE
	MobjType::HexenIceshard,          // T_ICESHARD
	MobjType::HexenFlameSmall,       // T_FLAME_SMALL
	MobjType::HexenFlameLarge,       // T_FLAME_LARGE
	MobjType::HexenArmor1,           // T_MESHARMOR
	MobjType::HexenArmor2,           // T_FALCONSHIELD
	MobjType::HexenArmor3,           // T_PLATINUMHELM
	MobjType::HexenArmor4,           // T_AMULETOFWARDING
	MobjType::HexenArtipoisonbag,     // T_ITEMFLECHETTE
	MobjType::HexenArtitorch,         // T_ITEMTORCH
	MobjType::HexenBlastradius,       // T_ITEMREPULSION
	MobjType::HexenMana3,             // T_MANA3
	MobjType::HexenArtipuzzskull,     // T_PUZZSKULL
	MobjType::HexenArtipuzzgembig,    // T_PUZZGEMBIG
	MobjType::HexenArtipuzzgemred,    // T_PUZZGEMRED
	MobjType::HexenArtipuzzgemgreen1, // T_PUZZGEMGREEN1
	MobjType::HexenArtipuzzgemgreen2, // T_PUZZGEMGREEN2
	MobjType::HexenArtipuzzgemblue1,  // T_PUZZGEMBLUE1
	MobjType::HexenArtipuzzgemblue2,  // T_PUZZGEMBLUE2
	MobjType::HexenArtipuzzbook1,     // T_PUZZBOOK1
	MobjType::HexenArtipuzzbook2,     // T_PUZZBOOK2
	MobjType::HexenKey1,              // T_METALKEY
	MobjType::HexenKey2,              // T_SMALLMETALKEY
	MobjType::HexenKey3,              // T_AXEKEY
	MobjType::HexenKey4,              // T_FIREKEY
	MobjType::HexenKey5,              // T_GREENKEY
	MobjType::HexenKey6,              // T_MACEKEY
	MobjType::HexenKey7,              // T_SILVERKEY
	MobjType::HexenKey8,              // T_RUSTYKEY
	MobjType::HexenKey9,              // T_HORNKEY
	MobjType::HexenKeya,              // T_SERPENTKEY
	MobjType::HexenWaterDrip,        // T_WATERDRIP
	MobjType::HexenFlameSmallTemp,  // T_TEMPSMALLFLAME
	MobjType::HexenFlameSmall,       // T_PERMSMALLFLAME
	MobjType::HexenFlameLargeTemp,  // T_TEMPLARGEFLAME
	MobjType::HexenFlameLarge,       // T_PERMLARGEFLAME
	MobjType::HexenDemonMash,        // T_DEMON_MASH
	MobjType::HexenDemon2Mash,       // T_DEMON2_MASH
	MobjType::HexenEttinMash,        // T_ETTIN_MASH
	MobjType::HexenCentaurMash,      // T_CENTAUR_MASH
	MobjType::HexenThrustfloorUp,    // T_THRUSTSPIKEUP
	MobjType::HexenThrustfloorDown,  // T_THRUSTSPIKEDOWN
	MobjType::HexenWraithfx4,         // T_FLESH_DRIP1
	MobjType::HexenWraithfx5,         // T_FLESH_DRIP2
	MobjType::HexenWraithfx2          // T_SPARK_DRIP
};

dboolean EV_ThingProjectile(byte* args, dboolean gravity)
{
	int tid;
	angle_t angle;
	int fineAngle;
	fixed_t speed;
	fixed_t vspeed;
	MobjType moType;
	mobj_t* mobj;
	mobj_t* newMobj;
	int searcher;
	dboolean success;

	success = false;
	searcher = -1;
	tid = args[0];
	moType = TranslateThingType[args[1]];
	if(nomonsters && (mobjinfo[std::to_underlying(moType)].flags & MF_COUNTKILL))
	{
		// Don't spawn monsters if -nomonsters
		return false;
	}
	angle = (int)args[2] << 24;
	fineAngle = angle >> ANGLETOFINESHIFT;
	speed = (int)args[3] << 13;
	vspeed = (int)args[4] << 13;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		newMobj = P_SpawnMobj(mobj->x, mobj->y, mobj->z, moType);
		if(newMobj->info->seesound != SfxId::None)
		{
			S_StartMobjSound(newMobj, newMobj->info->seesound);
		}
		P_SetTarget(&newMobj->target, mobj); // Originator
		newMobj->angle = angle;
		newMobj->momx = FixedMul(speed, finecosine[fineAngle]);
		newMobj->momy = FixedMul(speed, finesine[fineAngle]);
		newMobj->momz = vspeed;
		newMobj->flags |= MF_DROPPED; // Don't respawn
		if(gravity == true)
		{
			newMobj->flags &= ~MF_NOGRAVITY;
			newMobj->flags2 |= MobjFlag2::LoGrav;
		}
		if(P_CheckMissileSpawn(newMobj) == true)
		{
			success = true;
		}
	}
	return success;
}

dboolean EV_ThingSpawn(byte* args, dboolean fog)
{
	int tid;
	angle_t angle;
	mobj_t* mobj;
	mobj_t* newMobj;
	mobj_t* fogMobj;
	MobjType moType;
	int searcher;
	dboolean success;
	fixed_t z;

	success = false;
	searcher = -1;
	tid = args[0];
	moType = TranslateThingType[args[1]];
	if(nomonsters && (mobjinfo[std::to_underlying(moType)].flags & MF_COUNTKILL))
	{
		// Don't spawn monsters if -nomonsters
		return false;
	}
	angle = (int)args[2] << 24;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		if((mobjinfo[std::to_underlying(moType)].flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
		{
			z = mobj->z - mobj->floorz;
		}
		else
		{
			z = mobj->z;
		}
		newMobj = P_SpawnMobj(mobj->x, mobj->y, z, moType);
		if(P_TestMobjLocation(newMobj) == false)
		{
			// Didn't fit
			P_RemoveMobj(newMobj);
		}
		else
		{
			newMobj->angle = angle;
			if(fog == true)
			{
				fogMobj = P_SpawnMobj(mobj->x, mobj->y,
					mobj->z + TELEFOGHEIGHT, MobjType::HexenTfog);
				S_StartMobjSound(fogMobj, SfxId::HexenTeleport);
			}
			newMobj->flags |= MF_DROPPED; // Don't respawn
			if((newMobj->flags2 & MobjFlag2::FloatBob) != MobjFlag2{})
			{
				newMobj->special1.i = newMobj->z - newMobj->floorz;
			}
			success = true;
		}
	}
	return success;
}

dboolean EV_ThingActivate(int tid)
{
	mobj_t* mobj;
	int searcher;
	dboolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		if(ActivateThing(mobj) == true)
		{
			success = true;
		}
	}
	return success;
}

dboolean EV_ThingDeactivate(int tid)
{
	mobj_t* mobj;
	int searcher;
	dboolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		if(DeactivateThing(mobj) == true)
		{
			success = true;
		}
	}
	return success;
}

dboolean EV_ThingRemove(int tid)
{
	mobj_t* mobj;
	int searcher;
	dboolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		if(mobj->type == MobjType::HexenBridge)
		{
			A_BridgeRemove(mobj);
			return true;
		}
		P_RemoveMobj(mobj);
		success = true;
	}
	return success;
}

dboolean EV_ThingDestroy(int tid)
{
	mobj_t* mobj;
	int searcher;
	dboolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		if(mobj->flags & MF_SHOOTABLE)
		{
			P_DamageMobj(mobj, nullptr, nullptr, 10000);
			success = true;
		}
	}
	return success;
}

static dboolean ActivateThing(mobj_t* mobj)
{
	if(mobj->flags & MF_COUNTKILL)
	{
		// Monster
		if((mobj->flags2 & MobjFlag2::Dormant) != MobjFlag2{})
		{
			mobj->flags2 -= MobjFlag2::Dormant;
			mobj->tics = 1;
			return true;
		}
		return false;
	}
	switch(mobj->type)
	{
		case MobjType::HexenZtwinedtorch:
		case MobjType::HexenZtwinedtorchUnlit:
			P_SetMobjState(mobj, StateId::HexenZtwinedtorch1);
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			break;
		case MobjType::HexenZwalltorch:
		case MobjType::HexenZwalltorchUnlit:
			P_SetMobjState(mobj, StateId::HexenZwalltorch1);
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			break;
		case MobjType::HexenZgempedestal:
			P_SetMobjState(mobj, StateId::HexenZgempedestal2);
			break;
		case MobjType::HexenZwingedstatuenoskull:
			P_SetMobjState(mobj, StateId::HexenZwingedstatuenoskull2);
			break;
		case MobjType::HexenThrustfloorUp:
		case MobjType::HexenThrustfloorDown:
			if(mobj->special_args[0] == 0)
			{
				S_StartMobjSound(mobj, SfxId::HexenThrustspikeLower);
				mobj->flags2 -= MobjFlag2::DontDraw;
				if(mobj->special_args[1])
					P_SetMobjState(mobj, StateId::HexenBthrustraise1);
				else
					P_SetMobjState(mobj, StateId::HexenThrustraise1);
			}
			break;
		case MobjType::HexenZfirebull:
		case MobjType::HexenZfirebullUnlit:
			P_SetMobjState(mobj, StateId::HexenZfirebullBirth);
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			break;
		case MobjType::HexenZbell:
			if(mobj->health > 0)
			{
				P_DamageMobj(mobj, nullptr, nullptr, 10); // 'ring' the bell
			}
			break;
		case MobjType::HexenZcauldron:
		case MobjType::HexenZcauldronUnlit:
			P_SetMobjState(mobj, StateId::HexenZcauldron1);
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			break;
		case MobjType::HexenFlameSmall:
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			P_SetMobjState(mobj, StateId::HexenFlameSmall1);
			break;
		case MobjType::HexenFlameLarge:
			S_StartMobjSound(mobj, SfxId::HexenIgnite);
			P_SetMobjState(mobj, StateId::HexenFlameLarge1);
			break;
		case MobjType::HexenBatSpawner:
			P_SetMobjState(mobj, StateId::HexenSpawnbats1);
			break;
		default:
			return false;
			break;
	}
	return true;
}

static dboolean DeactivateThing(mobj_t* mobj)
{
	if(mobj->flags & MF_COUNTKILL)
	{
		// Monster
		if((mobj->flags2 & MobjFlag2::Dormant) == MobjFlag2{})
		{
			mobj->flags2 |= MobjFlag2::Dormant;
			mobj->tics = -1;
			return true;
		}
		return false;
	}
	switch(mobj->type)
	{
		case MobjType::HexenZtwinedtorch:
		case MobjType::HexenZtwinedtorchUnlit:
			P_SetMobjState(mobj, StateId::HexenZtwinedtorchUnlit);
			break;
		case MobjType::HexenZwalltorch:
		case MobjType::HexenZwalltorchUnlit:
			P_SetMobjState(mobj, StateId::HexenZwalltorchU);
			break;
		case MobjType::HexenThrustfloorUp:
		case MobjType::HexenThrustfloorDown:
			if(mobj->special_args[0] == 1)
			{
				S_StartMobjSound(mobj, SfxId::HexenThrustspikeRaise);
				if(mobj->special_args[1])
					P_SetMobjState(mobj, StateId::HexenBthrustlower);
				else
					P_SetMobjState(mobj, StateId::HexenThrustlower);
			}
			break;
		case MobjType::HexenZfirebull:
		case MobjType::HexenZfirebullUnlit:
			P_SetMobjState(mobj, StateId::HexenZfirebullDeath);
			break;
		case MobjType::HexenZcauldron:
		case MobjType::HexenZcauldronUnlit:
			P_SetMobjState(mobj, StateId::HexenZcauldronU);
			break;
		case MobjType::HexenFlameSmall:
			P_SetMobjState(mobj, StateId::HexenFlameSdorm1);
			break;
		case MobjType::HexenFlameLarge:
			P_SetMobjState(mobj, StateId::HexenFlameLdorm1);
			break;
		case MobjType::HexenBatSpawner:
			P_SetMobjState(mobj, StateId::HexenSpawnbatsOff);
			break;
		default:
			return false;
			break;
	}
	return true;
}
