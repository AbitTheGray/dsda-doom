// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Player state structure.
 */

#pragma once

#include <utility>

#include "dsda/pclass.hpp"
#include "d_items.hpp"
#include "p_pspr.hpp"
#include "p_mobj.hpp"
#include "d_ticcmd.hpp"
#include "cpp/Util.hpp"

// Cheats a player has on (`player_t::cheats`).
enum struct CheatFlag : uint32_t
{
	NoClip = Bit<uint32_t>(0u),       // no clipping
	GodMode = Bit<uint32_t>(1u),      // immune to damage
	InfiniteAmmo = Bit<uint32_t>(2u), // infinite ammo
	NoTarget = Bit<uint32_t>(3u),     // monsters don't target
	Fly = Bit<uint32_t>(4u),          // flying player
};
ENUM_FLAGS_FUNC(CheatFlag)

#ifdef __cplusplus
extern "C"
{
#endif

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).

// In addition, the player is just a special
// case of the generic moving object/actor.

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.

//
// Player states.
//
enum struct PlayerState : int32_t
{
	// Playing or camping.
	Live,
	// Dead on the ground, view follows killer.
	Dead,
	// Ready to restart/respawn???
	Reborn
};

// heretic
typedef struct
{
	int type;
	int count;
} inventory_t;

// heretic
enum struct ArtiType : int32_t
{
	None,
	Invulnerability,
	Invisibility,
	Health,
	SuperHealth,
	TomeOfPower,
	Torch,
	Firebomb,
	Egg,
	Fly,
	Teleport,
	Count,

	// hexen
	HexenNone = None,
	HexenInvulnerability,
	HexenHealth,
	HexenSuperhealth,
	HexenHealingradius,
	HexenSummon,
	HexenTorch,
	HexenEgg,
	HexenFly,
	HexenBlastradius,
	HexenPoisonbag,
	HexenTeleportother,
	HexenSpeed,
	HexenBoostmana,
	HexenBoostarmor,
	HexenTeleport,
	// Puzzle artifacts
	HexenFirstpuzzitem,
	HexenPuzzskull = HexenFirstpuzzitem,
	HexenPuzzgembig,
	HexenPuzzgemred,
	HexenPuzzgemgreen1,
	HexenPuzzgemgreen2,
	HexenPuzzgemblue1,
	HexenPuzzgemblue2,
	HexenPuzzbook1,
	HexenPuzzbook2,
	HexenPuzzskull2,
	HexenPuzzfweapon,
	HexenPuzzcweapon,
	HexenPuzzmweapon,
	HexenPuzzgear1,
	HexenPuzzgear2,
	HexenPuzzgear3,
	HexenPuzzgear4,
	HexenCount
};

#define NUMINVENTORYSLOTS	std::to_underlying(ArtiType::HexenCount)

//
// Extended player object info: player_t
//
typedef struct player_s
{
	mobj_t* mo;
	PlayerState playerstate;
	ticcmd_t cmd;

	// Determine POV,
	//  including viewpoint bobbing during movement.
	// Focal origin above r.z
	fixed_t viewz;
	// Base height above floor for viewz.
	fixed_t viewheight;
	// Bob/squat speed.
	fixed_t deltaviewheight;
	// bounded/scaled total momentum.
	fixed_t bob;

	// This is only used between levels,
	// mo->health is used during levels.
	int health;
	int armorpoints[std::to_underlying(ArmorType::Count)];
	// Armor type is 0-2.
	int armortype;

	// Power ups. invinc and invis are tic counters.
	int powers[std::to_underlying(PowerType::Count)];
	dboolean cards[std::to_underlying(Card::Count)];
	dboolean backpack;

	// Frags, kills of other players.
	int frags[MAX_MAXPLAYERS];
	WeaponType readyweapon;

	// Is wp_nochange if not changing.
	WeaponType pendingweapon;

	dboolean weaponowned[std::to_underlying(WeaponType::Count)];
	int ammo[std::to_underlying(AmmoType::Count)];
	int maxammo[std::to_underlying(AmmoType::Count)];

	// True if button down last tic.
	int attackdown;
	int usedown;

	// See CF flags above.
	CheatFlag cheats;

	// Refired shots are less accurate.
	int refire;

	// For intermission stats.
	int killcount;
	int itemcount;
	int secretcount;

	// For screen flashing (red or bright).
	int damagecount;
	int bonuscount;

	// Who did damage (NULL for floors/ceilings).
	mobj_t* attacker;

	// So gun flashes light up areas.
	int extralight;

	// Current PLAYPAL, ???
	//  can be set to REDCOLORMAP for pain, etc.
	int fixedcolormap;

	// Player skin colorshift,
	//  0-3 for which color to draw player.
	int colormap;

	// Overlay view sprites (gun, etc).
	pspdef_t psprites[std::to_underlying(PspNum::Count)];

	// True if secret level has been done.
	dboolean didsecret;

	// e6y
	// All non original (new) fields of player_t struct are moved to bottom
	// for compatibility with overflow (from a deh) of player_t::ammo[NUMAMMO]

	/* killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
	* mo->momx and mo->momy represent true momenta experienced by player.
	* This only represents the thrust that the player applies himself.
	* This avoids anomolies with such things as Boom ice and conveyors.
	*/
	fixed_t momx, momy; // killough 10/98

	//e6y
	int maxkilldiscount;

	fixed_t prev_viewz;
	angle_t prev_viewangle;
	angle_t prev_viewpitch;

	// heretic
	int flyheight;
	int lookdir;
	dboolean centering;
	inventory_t inventory[NUMINVENTORYSLOTS];
	ArtiType readyArtifact;
	int artifactCount;
	int inventorySlotNum;
	int flamecount;  // for flame thrower duration
	int chickenTics; // player is a chicken if > 0
	int chickenPeck; // chicken peck countdown
	mobj_t* rain1;   // active rain maker 1
	mobj_t* rain2;   // active rain maker 2

	// hexen
	PClass pclass; // player class type
	int morphTics;   // player is a pig if > 0
	int pieces;      // Fourth Weapon pieces
	int ravenkeys;   // Track statusbar keys
	short yellowMessage;
	int poisoncount;         // screen flash for poison damage
	mobj_t* poisoner;        // NULL for non-player mobjs
	unsigned int jumpTics;   // delay the next jump for a moment
	unsigned int worldTimer; // total time the player's been playing

	// zdoom
	int hazardcount;
	byte hazardinterval;
} player_t;

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
	dboolean in; // whether the player is in game

	// Player stats, kills, collected items etc.
	int skills;
	int sitems;
	int ssecret;
	int stime;
	int frags[4];
	int score; // current score on entry, modified on return
} wbplayerstruct_t;

typedef struct
{
	int epsd; // episode # (0-2)

	// if true, splash the secret level
	dboolean didsecret;

	// previous and next levels, origin 0
	int last;
	int next;
	int nextep; // for when MAPINFO progression crosses into another episode.

	int maxkills;
	int maxitems;
	int maxsecret;
	int maxfrags;

	// the par time
	int partime;
	int fake_partime;
	dboolean modified_partime;

	// index of this player in game
	int pnum;

	wbplayerstruct_t plyr[MAX_MAXPLAYERS];

	// CPhipps - total game time for completed levels so far
	int totaltimes;
} wbstartstruct_t;

fixed_t P_PlayerSpeed(player_t* player);

#ifdef __cplusplus
}
#endif
