// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Functions to return random numbers.
 */

#pragma once

#include <utility>

#include "m_fixed.hpp"
#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// killough 1/19/98: rewritten to use to use a better random number generator
// in the new engine, although the old one is available for compatibility.

// killough 2/16/98:
//
// Make every random number generator local to each control-equivalent block.
// Critical for demo sync. Changing the order of this list breaks all previous
// versions' demos. The random number generators are made local to reduce the
// chances of sync problems. In Doom, if a single random number generator call
// was off, it would mess up all random number generators. This reduces the
// chances of it happening by making each RNG local to a control flow block.
//
// Notes to developers: if you want to reduce your demo sync hassles, follow
// this rule: for each call to P_Random you add, add a new class to the enum
// type below for each block of code which calls P_Random. If two calls to
// P_Random are not in "control-equivalent blocks", i.e. there are any cases
// where one is executed, and the other is not, put them in separate classes.
//
// Keep all current entries in this list the same, and in the order
// indicated by the #'s, because they're critical for preserving demo
// sync. Do not remove entries simply because they become unused later.

enum struct RandomClass : int32_t
{
	Skullfly,     // #1
	Damage,       // #2
	Crush,        // #3
	Genlift,      // #4
	Killtics,     // #5
	Damagemobj,   // #6
	Painchance,   // #7
	Lights,       // #8
	Explode,      // #9
	Respawn,      // #10
	Lastlook,     // #11
	Spawnthing,   // #12
	Spawnpuff,    // #13
	Spawnblood,   // #14
	Missile,      // #15
	Shadow,       // #16
	Plats,        // #17
	Punch,        // #18
	Punchangle,   // #19
	Saw,          // #20
	Plasma,       // #21
	Gunshot,      // #22
	Misfire,      // #23
	Shotgun,      // #24
	Bfg,          // #25
	Slimehurt,    // #26
	Dmspawn,      // #27
	Missrange,    // #28
	Trywalk,      // #29
	Newchase,     // #30
	Newchasedir,  // #31
	See,          // #32
	Facetarget,   // #33
	Posattack,    // #34
	Sposattack,   // #35
	Cposattack,   // #36
	Spidrefire,   // #37
	Troopattack,  // #38
	Sargattack,   // #39
	Headattack,   // #40
	Bruisattack,  // #41
	Tracer,       // #42
	Skelfist,     // #43
	Scream,       // #44
	Brainscream,  // #45
	Cposrefire,   // #46
	Brainexp,     // #47
	Spawnfly,     // #48
	Misc,         // #49
	AllInOne,   // #50
	/* CPhipps - new entries from MBF, mostly unused for now */
	Opendoor,     // #51
	Targetsearch, // #52
	Friends,      // #53
	Threshold,    // #54
	Skiptarget,   // #55
	Enemystrafe,  // #56
	Avoidcrush,   // #57
	Stayonlift,   // #58
	Helpfriend,   // #59
	Dropoff,      // #60
	Randomjump,   // #61
	Defect,       // #62  // Start new entries -- add new entries below
	Heretic,      // #63
	Mbf21,        // #64
	Hexen,        // #65

	// End of new entries
	Count // MUST be last item in list
};

// The random number generator's state.
typedef struct
{
	unsigned int seed[std::to_underlying(RandomClass::Count)]; // Each block's random seed
	int rndindex, prndindex;       // For compatibility support
} rng_t;

extern rng_t rng; // The rng's state

extern unsigned int rngseed; // The starting seed (not part of state)

// As M_Random, but used by the play simulation.
int P_Random(RandomClass);

// Returns a number from 0 to 255,
#define M_Random() P_Random(RandomClass::Misc)

// Fix randoms for demos.
void M_ClearRandom();

// [XA] Common random formulas used by codepointers
int P_RandomHitscanAngle(RandomClass pr_class, fixed_t spread);
int P_RandomHitscanSlope(RandomClass pr_class, fixed_t spread);

// heretic

#define HITDICE(a) ((1+(P_Random(RandomClass::Heretic)&7))*a)

int P_SubRandom();

#ifdef __cplusplus
}
#endif
