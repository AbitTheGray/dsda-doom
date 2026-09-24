// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Spawn Numbers

#include "info.hpp"

#include "spawn_number.hpp"

#define SPAWN_NUMBER_MAX 155

MobjType doom_spawn_numbers[SPAWN_NUMBER_MAX] = {
	[0] = MobjType::Null,
	[1] = MobjType::Shotguy,       // shotgun guy
	[2] = MobjType::Chainguy,      // chaingun guy
	[3] = MobjType::Bruiser,       // baron
	[4] = MobjType::Possessed,     // zombieman
	[5] = MobjType::Troop,         // imp
	[6] = MobjType::Baby,          // arachnotron
	[7] = MobjType::Spider,        // spider mastermind
	[8] = MobjType::Sergeant,      // demon
	[9] = MobjType::Shadows,       // spectre
	[10] = MobjType::Troopshot,    // imp fireball
	[11] = MobjType::Clip,         // ammo clip
	[12] = MobjType::Misc22,       // shotgun shells
	MobjType::Null,                // unused 13
	MobjType::Null,                // unused 14
	MobjType::Null,                // unused 15
	MobjType::Null,                // unused 16
	MobjType::Null,                // unused 17
	MobjType::Null,                // unused 18
	[19] = MobjType::Head,         // cacodemon
	[20] = MobjType::Undead,       // revenant
	[21] = MobjType::Null,         // bridge (x)
	[22] = MobjType::Misc3,        // armor bonus
	[23] = MobjType::Misc10,       // stimpack
	[24] = MobjType::Misc11,       // medkit
	[25] = MobjType::Misc12,       // soul sphere
	MobjType::Null,                // unused 26
	[27] = MobjType::Shotgun,      // shotgun
	[28] = MobjType::Chaingun,     // chaingun
	[29] = MobjType::Misc27,       // rocket launcher
	[30] = MobjType::Misc28,       // plasma rifle
	[31] = MobjType::Misc25,       // BFG
	[32] = MobjType::Misc26,       // chainsaw
	[33] = MobjType::Supershotgun, // ssg
	MobjType::Null,                // unused 34
	MobjType::Null,                // unused 35
	MobjType::Null,                // unused 36
	MobjType::Null,                // unused 37
	MobjType::Null,                // unused 38
	MobjType::Null,                // unused 39
	MobjType::Null,                // unused 40
	MobjType::Null,                // unused 41
	MobjType::Null,                // unused 42
	MobjType::Null,                // unused 43
	MobjType::Null,                // unused 44
	MobjType::Null,                // unused 45
	MobjType::Null,                // unused 46
	MobjType::Null,                // unused 47
	MobjType::Null,                // unused 48
	MobjType::Null,                // unused 49
	MobjType::Null,                // unused 50
	[51] = MobjType::Plasma,       // plasma
	MobjType::Null,                // unused 52
	[53] = MobjType::Tracer,       // rev rocket
	MobjType::Null,                // unused 54
	MobjType::Null,                // unused 55
	MobjType::Null,                // unused 56
	MobjType::Null,                // unused 57
	MobjType::Null,                // unused 58
	MobjType::Null,                // unused 59
	MobjType::Null,                // unused 60
	MobjType::Null,                // unused 61
	MobjType::Null,                // unused 62
	MobjType::Null,                // unused 63
	MobjType::Null,                // unused 64
	MobjType::Null,                // unused 65
	MobjType::Null,                // unused 66
	MobjType::Null,                // unused 67
	[68] = MobjType::Misc0,        // green armor
	[69] = MobjType::Misc1,        // blue armor
	MobjType::Null,                // unused 70
	MobjType::Null,                // unused 71
	MobjType::Null,                // unused 72
	MobjType::Null,                // unused 73
	MobjType::Null,                // unused 74
	[75] = MobjType::Misc20,       // cell
	MobjType::Null,                // unused 76
	MobjType::Null,                // unused 77
	MobjType::Null,                // unused 78
	MobjType::Null,                // unused 79
	MobjType::Null,                // unused 80
	MobjType::Null,                // unused 81
	MobjType::Null,                // unused 82
	MobjType::Null,                // unused 83
	MobjType::Null,                // unused 84
	[85] = MobjType::Misc4,        // blue keycard
	[86] = MobjType::Misc5,        // red keycard
	[87] = MobjType::Misc6,        // yellow keycard
	[88] = MobjType::Misc7,        // yellow skull key
	[89] = MobjType::Misc8,        // red skull key
	[90] = MobjType::Misc9,        // blue skull key
	MobjType::Null,                // unused 91
	MobjType::Null,                // unused 92
	MobjType::Null,                // unused 93
	MobjType::Null,                // unused 94
	MobjType::Null,                // unused 95
	MobjType::Null,                // unused 96
	MobjType::Null,                // unused 97
	[98] = MobjType::Fire,         // archvile fire
	MobjType::Null,                // unused 99
	[100] = MobjType::Null,        // stealth baron (x)
	[101] = MobjType::Null,        // stealth hell knight (x)
	[102] = MobjType::Null,        // stealth zombieman (x)
	[103] = MobjType::Null,        // stealth shutgun guy (x)
	MobjType::Null,                // unused 104
	MobjType::Null,                // unused 105
	MobjType::Null,                // unused 106
	MobjType::Null,                // unused 107
	MobjType::Null,                // unused 108
	MobjType::Null,                // unused 109
	[110] = MobjType::Skull,       // lost soul
	[111] = MobjType::Vile,        // archvile
	[112] = MobjType::Fatso,       // mancubus
	[113] = MobjType::Knight,      // hell knight
	[114] = MobjType::Cyborg,      // cyberdemon
	[115] = MobjType::Pain,        // pain elemental
	[116] = MobjType::Wolfss,      // wolf ss
	[117] = MobjType::Null,        // stealth arachnotron (x)
	[118] = MobjType::Null,        // stealth archvile (x)
	[119] = MobjType::Null,        // stealth cacodemon (x)
	[120] = MobjType::Null,        // stealth chaingun guy (x)
	[121] = MobjType::Null,        // stealth demon (x)
	[122] = MobjType::Null,        // stealth imp (x)
	[123] = MobjType::Null,        // stealth mancubus (x)
	[124] = MobjType::Null,        // stealth revenant (x)
	[125] = MobjType::Barrel,      // barrel
	[126] = MobjType::Headshot,    // cacodemon fireball
	[127] = MobjType::Rocket,      // rocket
	[128] = MobjType::Bfg,         // bfg shot
	[129] = MobjType::Arachplaz,   // arachnotron shot
	MobjType::Null,                // unused 130
	[131] = MobjType::Puff,        // bullet puff
	[132] = MobjType::Mega,        // megasphere
	[133] = MobjType::Inv,         // invulnerability
	[134] = MobjType::Misc13,      // berserk
	[135] = MobjType::Ins,         // partial invisibility
	[136] = MobjType::Misc14,      // radiation suit
	[137] = MobjType::Misc15,      // computer map
	[138] = MobjType::Misc16,      // light amp goggles
	[139] = MobjType::Misc17,      // ammo box
	[140] = MobjType::Misc18,      // rocket (ammo)
	[141] = MobjType::Misc19,      // box of rockets
	[142] = MobjType::Misc21,      // cell pack
	[143] = MobjType::Misc23,      // box of shells
	[144] = MobjType::Misc24,      // backpack
	[145] = MobjType::Misc68,      // guts
	[146] = MobjType::Misc71,      // pool of blood
	[147] = MobjType::Misc84,      // pool of blood 1
	[148] = MobjType::Misc85,      // pool of blood 2
	[149] = MobjType::Misc77,      // flaming barrel
	[150] = MobjType::Misc86,      // brain
	[151] = MobjType::Null,        // scripted marine (x)
	[152] = MobjType::Misc2,       // health bonus
	[153] = MobjType::Fatshot,     // mancubus shot
	[154] = MobjType::Bruisershot, // baron fireball
};

MobjType dsda_ThingTypeFromSpawnNumber(int spawn_number)
{
	if(spawn_number < 0 || spawn_number >= SPAWN_NUMBER_MAX)
		return MobjType::Null;

	return doom_spawn_numbers[spawn_number];
}
