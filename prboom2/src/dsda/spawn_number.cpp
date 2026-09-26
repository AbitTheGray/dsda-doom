// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Spawn Numbers

#include "cpp/Designated.hpp"

#include "info.hpp"

#include "spawn_number.hpp"

#define SPAWN_NUMBER_MAX 155

constinit DesignatedArray<MobjType, SPAWN_NUMBER_MAX> doom_spawn_numbers = {
	{At(0), MobjType::Null},
	{At(1), MobjType::Shotguy},       // shotgun guy
	{At(2), MobjType::Chainguy},      // chaingun guy
	{At(3), MobjType::Bruiser},       // baron
	{At(4), MobjType::Possessed},     // zombieman
	{At(5), MobjType::Troop},         // imp
	{At(6), MobjType::Baby},          // arachnotron
	{At(7), MobjType::Spider},        // spider mastermind
	{At(8), MobjType::Sergeant},      // demon
	{At(9), MobjType::Shadows},       // spectre
	{At(10), MobjType::Troopshot},    // imp fireball
	{At(11), MobjType::Clip},         // ammo clip
	{At(12), MobjType::Misc22},       // shotgun shells
	MobjType::Null,                // unused 13
	MobjType::Null,                // unused 14
	MobjType::Null,                // unused 15
	MobjType::Null,                // unused 16
	MobjType::Null,                // unused 17
	MobjType::Null,                // unused 18
	{At(19), MobjType::Head},         // cacodemon
	{At(20), MobjType::Undead},       // revenant
	{At(21), MobjType::Null},         // bridge (x)
	{At(22), MobjType::Misc3},        // armor bonus
	{At(23), MobjType::Misc10},       // stimpack
	{At(24), MobjType::Misc11},       // medkit
	{At(25), MobjType::Misc12},       // soul sphere
	MobjType::Null,                // unused 26
	{At(27), MobjType::Shotgun},      // shotgun
	{At(28), MobjType::Chaingun},     // chaingun
	{At(29), MobjType::Misc27},       // rocket launcher
	{At(30), MobjType::Misc28},       // plasma rifle
	{At(31), MobjType::Misc25},       // BFG
	{At(32), MobjType::Misc26},       // chainsaw
	{At(33), MobjType::Supershotgun}, // ssg
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
	{At(51), MobjType::Plasma},       // plasma
	MobjType::Null,                // unused 52
	{At(53), MobjType::Tracer},       // rev rocket
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
	{At(68), MobjType::Misc0},        // green armor
	{At(69), MobjType::Misc1},        // blue armor
	MobjType::Null,                // unused 70
	MobjType::Null,                // unused 71
	MobjType::Null,                // unused 72
	MobjType::Null,                // unused 73
	MobjType::Null,                // unused 74
	{At(75), MobjType::Misc20},       // cell
	MobjType::Null,                // unused 76
	MobjType::Null,                // unused 77
	MobjType::Null,                // unused 78
	MobjType::Null,                // unused 79
	MobjType::Null,                // unused 80
	MobjType::Null,                // unused 81
	MobjType::Null,                // unused 82
	MobjType::Null,                // unused 83
	MobjType::Null,                // unused 84
	{At(85), MobjType::Misc4},        // blue keycard
	{At(86), MobjType::Misc5},        // red keycard
	{At(87), MobjType::Misc6},        // yellow keycard
	{At(88), MobjType::Misc7},        // yellow skull key
	{At(89), MobjType::Misc8},        // red skull key
	{At(90), MobjType::Misc9},        // blue skull key
	MobjType::Null,                // unused 91
	MobjType::Null,                // unused 92
	MobjType::Null,                // unused 93
	MobjType::Null,                // unused 94
	MobjType::Null,                // unused 95
	MobjType::Null,                // unused 96
	MobjType::Null,                // unused 97
	{At(98), MobjType::Fire},         // archvile fire
	MobjType::Null,                // unused 99
	{At(100), MobjType::Null},        // stealth baron (x)
	{At(101), MobjType::Null},        // stealth hell knight (x)
	{At(102), MobjType::Null},        // stealth zombieman (x)
	{At(103), MobjType::Null},        // stealth shutgun guy (x)
	MobjType::Null,                // unused 104
	MobjType::Null,                // unused 105
	MobjType::Null,                // unused 106
	MobjType::Null,                // unused 107
	MobjType::Null,                // unused 108
	MobjType::Null,                // unused 109
	{At(110), MobjType::Skull},       // lost soul
	{At(111), MobjType::Vile},        // archvile
	{At(112), MobjType::Fatso},       // mancubus
	{At(113), MobjType::Knight},      // hell knight
	{At(114), MobjType::Cyborg},      // cyberdemon
	{At(115), MobjType::Pain},        // pain elemental
	{At(116), MobjType::Wolfss},      // wolf ss
	{At(117), MobjType::Null},        // stealth arachnotron (x)
	{At(118), MobjType::Null},        // stealth archvile (x)
	{At(119), MobjType::Null},        // stealth cacodemon (x)
	{At(120), MobjType::Null},        // stealth chaingun guy (x)
	{At(121), MobjType::Null},        // stealth demon (x)
	{At(122), MobjType::Null},        // stealth imp (x)
	{At(123), MobjType::Null},        // stealth mancubus (x)
	{At(124), MobjType::Null},        // stealth revenant (x)
	{At(125), MobjType::Barrel},      // barrel
	{At(126), MobjType::Headshot},    // cacodemon fireball
	{At(127), MobjType::Rocket},      // rocket
	{At(128), MobjType::Bfg},         // bfg shot
	{At(129), MobjType::Arachplaz},   // arachnotron shot
	MobjType::Null,                // unused 130
	{At(131), MobjType::Puff},        // bullet puff
	{At(132), MobjType::Mega},        // megasphere
	{At(133), MobjType::Inv},         // invulnerability
	{At(134), MobjType::Misc13},      // berserk
	{At(135), MobjType::Ins},         // partial invisibility
	{At(136), MobjType::Misc14},      // radiation suit
	{At(137), MobjType::Misc15},      // computer map
	{At(138), MobjType::Misc16},      // light amp goggles
	{At(139), MobjType::Misc17},      // ammo box
	{At(140), MobjType::Misc18},      // rocket (ammo)
	{At(141), MobjType::Misc19},      // box of rockets
	{At(142), MobjType::Misc21},      // cell pack
	{At(143), MobjType::Misc23},      // box of shells
	{At(144), MobjType::Misc24},      // backpack
	{At(145), MobjType::Misc68},      // guts
	{At(146), MobjType::Misc71},      // pool of blood
	{At(147), MobjType::Misc84},      // pool of blood 1
	{At(148), MobjType::Misc85},      // pool of blood 2
	{At(149), MobjType::Misc77},      // flaming barrel
	{At(150), MobjType::Misc86},      // brain
	{At(151), MobjType::Null},        // scripted marine (x)
	{At(152), MobjType::Misc2},       // health bonus
	{At(153), MobjType::Fatshot},     // mancubus shot
	{At(154), MobjType::Bruisershot}, // baron fireball
};

MobjType dsda_ThingTypeFromSpawnNumber(int spawn_number)
{
	if(spawn_number < 0 || spawn_number >= SPAWN_NUMBER_MAX)
		return MobjType::Null;

	return doom_spawn_numbers[spawn_number];
}
