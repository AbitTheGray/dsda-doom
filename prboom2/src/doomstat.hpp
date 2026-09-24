// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   All the global variables that store the internal state.
 *   Theoretically speaking, the internal state of the engine
 *    should be found by looking at the variables collected
 *    here, and every relevant module will have to include
 *    this header file.
 *   In practice, things are a bit messy.
 */

#pragma once

#include <utility>

#include "d_player.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// We need the playr data structure as well.

// ------------------------
// Command line parameters.
//

extern dboolean nomonsters;  // checkparm of -nomonsters
extern dboolean respawnparm; // checkparm of -respawn
extern dboolean fastparm;    // checkparm of -fast

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//

extern GameMode gamemode;
extern GameMission gamemission;
extern const char* doomverstr;

extern char* VANILLA_MAP_LUMP_NAME(int e, int m);

// Set if homebrew PWAD stuff has been added.
extern dboolean modifiedgame;

// CPhipps - new compatibility handling
extern complevel_t compatibility_level;

// CPhipps - old compatibility testing flags aliased to new handling
#define compatibility (compatibility_level<=CompLevel::BoomCompatibility)
#define demo_compatibility (compatibility_level < CompLevel::BoomCompatibility)
#define mbf_features (compatibility_level>=CompLevel::Mbf)
#define mbf21 (compatibility_level == CompLevel::Mbf21)

extern int demo_insurance; // killough 4/5/98

extern dboolean pistolstart;

// -------------------------------------------
// killough 10/98: compatibility vector

enum struct CompOption : int8_t
{
	Telefrag,
	DropOff,
	Vile,
	Pain,
	Skull,
	Blazing,
	DoorLight,
	Model,
	God,
	FallOff,
	Floors,
	SkyMap,
	Pursuit,
	DoorStuck,
	StayLift,
	Zombie,
	Stairs,
	InfCheat,
	ZeroTags,
	MoveBlock,
	Respawn, /* cph - alias of comp_respawnfix from eternity */
	Sound,
	Value666,
	Soul,
	MaskedAnim,

	//e6y
	OuchFace,
	MaxHealth,
	Translucency,

	// mbf21
	LedgeBlock,
	FriendlySpawn,
	VoodooScroller,
	ReservedLineFlag,

	MbfCompTotal = 32, // limit in MBF format

	End = -1 // terminates a compatibility option list
};

enum struct CompError : int32_t
{
	PassUse,
	HangSolid,
	BlockMap,

	Count
};

extern int comp[std::to_underlying(CompOption::MbfCompTotal)];
extern int default_comperr[std::to_underlying(CompError::Count)];

// -------------------------------------------
// Language.
extern Language language;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern int startskill;
extern int startepisode;

extern dboolean autostart;

// Selected by user.
extern int gameskill;
extern int gameepisode;
extern int gamemap;

typedef struct
{
	int map;
	int position;
	int flags;
	angle_t angle;
} leave_data_t;

extern leave_data_t leave_data;

#define LF_SET_ANGLE 0x01
#define LEAVE_VICTORY -1

// Netgame? Only true if >1 player.
extern dboolean netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern dboolean deathmatch;

extern int solo_net;
extern dboolean coop_spawns;

extern dboolean randomclass;

extern int map_colormap;
extern fixed_t map_gravity;
extern fixed_t map_aircontrol;
extern fixed_t map_airfriction;

// ------------------------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;   // maximum volume for sound
extern int snd_MusicVolume; // maximum volume for music

// CPhipps - screen parameters
extern int desired_screenwidth, desired_screenheight;

extern int automap_full;
extern int automap_overlay;
extern int automap_rotate;
extern int automap_follow;
extern int automap_grid;

#define automap_on    (automap_full)
#define automap_solid (automap_full && !automap_overlay)
#define automap_input (automap_full)
#define automap_stbar (automap_full && R_StatusBarVisible())

enum struct MenuActive : int32_t
{
	NoChange = -1,
	Inactive, // no menu
	Float,    // doom-style large font menu, doesn't overlap anything
	Full,     // boom-style small font menu, may overlap status bar
};

extern MenuActive menuactive; // Type of menu overlaid, if any

extern dboolean nodrawers;

// Player taking events, and displaying.
extern int consoleplayer;
extern int displayplayer;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int totalkills, totallive;
extern int totalitems;
extern int totalsecret;

extern int boom_basetic;
extern int true_basetic;
extern int leveltime;       // level time in tics
extern int totalleveltimes; // sum of intermission times in tics at second resolution
extern int levels_completed;

// --------------------------------------
// DEMO playback/recording related stuff.

extern dboolean demoplayback;
extern dboolean demorecording;
extern int demover;

#define allow_incompatibility (!demorecording && !demoplayback)
#define comperr(i) (default_comperr[std::to_underlying(i)] && allow_incompatibility)

extern dboolean userdemo;
#define userplayback (demoplayback && userdemo)
#define reelplayback (demoplayback && !userdemo)

// Print timing information after quitting.  killough
extern dboolean timingdemo;
// Run tick clock at fastest speed possible while playing demo.  killough
extern dboolean fastdemo;

extern GameState gamestate;
extern dboolean in_game;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int gametic;

#define boom_logictic (gametic - boom_basetic)
#define true_logictic (gametic - true_basetic)

//e6y
extern dboolean realframe;

// Bookkeeping on players - state.
extern player_t players[MAX_MAXPLAYERS];
extern int upmove;

// Alive? Disconnected?
extern dboolean playeringame[MAX_MAXPLAYERS];

extern PClass PlayerClass[MAX_MAXPLAYERS];

extern mapthing_t* deathmatchstarts; // killough
extern size_t num_deathmatchstarts;  // killough

extern mapthing_t* deathmatch_p;

// Player spawn spots.
#define MAX_PLAYER_STARTS 8
extern mapthing_t playerstarts[MAX_PLAYER_STARTS][MAX_MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern FILE* debugfile;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern GameState wipegamestate;

// debug flag to cancel adaptiveness
extern dboolean singletics;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.

extern int skyflatnum;

extern int maketic;

// Networking and tick handling related.
#define BACKUPTICS              12

extern ticcmd_t local_cmds[][BACKUPTICS];

//-----------------------------------------------------------------------------

extern int allow_pushers; // MT_PUSH Things    // phares 3/10/98

extern int variable_friction; // ice & mud            // phares 3/10/98

extern int monsters_remember; // killough 3/1/98

extern int weapon_recoil; // weapon recoil    // phares

extern int player_bobbing; // whether player bobs or not   // phares 2/25/98

extern int dogs;        // killough 7/19/98: Marine's best friend :)
extern int dog_jumping; // killough 10/98

/* killough 8/8/98: distance friendly monsters tend to stay from player */
extern int distfriend;

/* killough 9/8/98: whether monsters are allowed to strafe or retreat */
extern int monster_backing;

/* killough 9/9/98: whether monsters intelligently avoid hazards */
extern int monster_avoid_hazards;

/* killough 10/98: whether monsters are affected by friction */
extern int monster_friction;

/* killough 9/9/98: whether monsters help friends */
extern int help_friends;

/* killough 7/19/98: whether monsters should fight against each other */
extern int monster_infighting;

extern int monkeys;

extern int HelperThing; // type of thing to use for helper

#ifdef __cplusplus
}
#endif
