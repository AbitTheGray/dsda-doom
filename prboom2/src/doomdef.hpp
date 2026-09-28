// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Internally used data structures for virtually everything,
 *   key definitions, lots of other stuff.
 */

#pragma once

#include "z_zone.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "m_swap.hpp"
#include "doomtype.hpp"

// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
//
// A key code is what a key press puts in event_t::data1: the key's unshifted character for a printable key,
// or one of the codes below. Keys without a name here (other layouts' characters, SDL's own codes)
// pass through unnamed, so any int32_t is a valid KeyCode.
enum struct KeyCode : int32_t
{
	None = 0,

	Tab = 9,
	Enter = 13,
	Escape = 27,
	Space = ' ',

	// The printable characters (see IsPrintable). SDL reports letters in lowercase,
	// so A to Z hold 'a' to 'z' and the uppercase codes have no name.
	Exclamation = '!',
	Quote = '"',
	Hash = '#',
	Dollar = '$',
	Percent = '%',
	Ampersand = '&',
	Apostrophe = '\'',
	LeftParen = '(',
	RightParen = ')',
	Asterisk = '*',
	Plus = '+',
	Comma = ',',
	Minus = '-',
	Period = '.',
	Slash = '/',
	Digit0 = '0',
	Digit1 = '1',
	Digit2 = '2',
	Digit3 = '3',
	Digit4 = '4',
	Digit5 = '5',
	Digit6 = '6',
	Digit7 = '7',
	Digit8 = '8',
	Digit9 = '9',
	Colon = ':',
	Semicolon = ';',
	Less = '<',
	Equals = '=',
	Greater = '>',
	Question = '?',
	At = '@',
	LeftBracket = '[',
	Backslash = '\\',
	RightBracket = ']',
	Caret = '^',
	Underscore = '_',
	Grave = '`',
	A = 'a',
	B = 'b',
	C = 'c',
	D = 'd',
	E = 'e',
	F = 'f',
	G = 'g',
	H = 'h',
	I = 'i',
	J = 'j',
	K = 'k',
	L = 'l',
	M = 'm',
	N = 'n',
	O = 'o',
	P = 'p',
	Q = 'q',
	R = 'r',
	S = 's',
	T = 't',
	U = 'u',
	V = 'v',
	W = 'w',
	X = 'x',
	Y = 'y',
	Z = 'z',
	LeftBrace = '{',
	Pipe = '|',
	RightBrace = '}',
	Tilde = '~',

	Backspace = 127,

	// Not a key: one past the ASCII range, the size of a table indexed by ASCII key codes.
	AsciiCount = 0x80,

	F1 = 0x80 + 0x3b,
	F2 = 0x80 + 0x3c,
	F3 = 0x80 + 0x3d,
	F4 = 0x80 + 0x3e,
	F5 = 0x80 + 0x3f,
	F6 = 0x80 + 0x40,
	F7 = 0x80 + 0x41,
	F8 = 0x80 + 0x42,
	F9 = 0x80 + 0x43,
	F10 = 0x80 + 0x44,
	F11 = 0x80 + 0x57,
	F12 = 0x80 + 0x58,
	// One code per modifier: left and right keys share it (vanilla's PC scancodes did not tell them apart).
	Shift = 0x80 + 0x36,
	Ctrl = 0x80 + 0x1d,
	Alt = 0x80 + 0x38,
	RightArrow = 0xae,
	LeftArrow = 0xac,
	UpArrow = 0xad,
	DownArrow = 0xaf,
	CapsLock = 0xba, // phares
	PrintScreen = 0xfe,
	Pause = 0xff,

	// phares 3/2/98:
	Insert = 0xd2,
	Home = 0xc7,
	PageUp = 0xc9,
	PageDown = 0xd1,
	Delete = 0xc8,
	End = 0xcf,
	ScrollLock = 0xc6,
	// phares 3/2/98

	NumLock = 0xc5, // killough 3/6/98

	// cph - Add the numeric keypad keys, as suggested by krose 4/22/99:
	// The way numbers are assigned to keys is a mess, but it's too late to
	// change that easily. At least these additions are don neatly.
	// Codes 0x100-0x200 are reserved for number pad
	Keypad0 = 0x100 + '0',
	Keypad1 = 0x100 + '1',
	Keypad2 = 0x100 + '2',
	Keypad3 = 0x100 + '3',
	Keypad4 = 0x100 + '4',
	Keypad5 = 0x100 + '5',
	Keypad6 = 0x100 + '6',
	Keypad7 = 0x100 + '7',
	Keypad8 = 0x100 + '8',
	Keypad9 = 0x100 + '9',
	KeypadEnter = 0x100 + Enter,
	KeypadDivide = 0x100 + '/',
	KeypadMultiply = 0x100 + '*',
	KeypadMinus = 0x100 + '-',
	KeypadPlus = 0x100 + '+',
	KeypadPeriod = 0x100 + '.',

	// haleyjd: virtual keys
	Mouse1 = 0x80 + 0x60,
	Mouse2 = 0x80 + 0x61,
	Mouse3 = 0x80 + 0x62,
	MouseWheelUp = 0x80 + 0x6b,
	MouseWheelDown = 0x80 + 0x6c,
	MouseWheelLeft = 0x80 + 0x6d,
	MouseWheelRight = 0x80 + 0x6e,

	// Not a key: the number of key states tracked (dsda/input.cpp). Higher codes are ignored there.
	Count = 512,
};

// Whether the key is a printable ASCII character, Space to Tilde.
[[nodiscard]]
inline constexpr bool IsPrintable(const KeyCode key)
{
	return key >= KeyCode::Space && key <= KeyCode::Tilde;
}

// The character of a printable key.
// @pre IsPrintable(key)
[[nodiscard]]
inline constexpr char ToChar(const KeyCode key)
{
	assert(IsPrintable(key));
	return static_cast<char>(std::to_underlying(key));
}

#ifdef __cplusplus
extern "C"
{
#endif

/* use config.h if autoconf made one -- josh */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// killough 4/25/98: Make gcc extensions mean nothing on other compilers
#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

// This must come first, since it redefines malloc(), free(), etc. -- killough:

extern dboolean bfgedition;

// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
enum struct GameMode : int32_t
{
	Shareware,   // DOOM 1 shareware, E1, M9
	Registered,  // DOOM 1 registered, E3, M27
	Commercial,  // DOOM 2 retail, E1 M34  (DOOM 2 german edition not handled)
	Retail,      // DOOM 1 retail, E4, M36
	Indetermined // Well, no IWAD found.
};

// Mission packs - might be useful for TC stuff?
enum struct GameMission : int32_t
{
	Doom,        // DOOM 1
	Doom2,       // DOOM 2
	PackTnt,    // TNT mission pack
	PackPlut,   // Plutonia pack
	PackNerve,  // No Rest For The Living
	TcHacx,     // HACX - Twitch 'n Kill
	TcChex,     // Chex Quest
	TcChex3v,   // Chex Quest 3: Vanilla/Modding Edition
	TcRekkr,    // REKKR
	TcFreedoom, // Freedoom
	None
};

extern dboolean tc_game;

// Identify language to use, software localization.
enum struct Language : int32_t
{
	English,
	French,
	German,
	Unknown
};

//
// For resize of screen, at start of game.
//

#define BASE_WIDTH 320

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.

#define INV_ASPECT_RATIO   0.625 /* 0.75, ideally */

// killough 2/8/98: MAX versions for maximum screen sizes
// allows us to avoid the overhead of dynamic allocation
// when multiple screen sizes are supported

// SCREENWIDTH and SCREENHEIGHT define the visible size
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
// ACTUALHEIGHT is the actual height of the resolution
// If the resolution is 200p or 400p, aspect ratio correction
// should be applied, making this value 240 or 480
extern int ACTUALHEIGHT;
// SCREENPITCH is the size of one line in the buffer and
// can be bigger than the SCREENWIDTH depending on the size
// of one pixel (8, 16 or 32 bit) and the padding at the
// end of the line caused by hardware considerations
extern int SCREENPITCH;

// e6y: wide-res
extern int WIDE_SCREENWIDTH;
extern int WIDE_SCREENHEIGHT;
extern int SCREEN_320x200;

// The maximum number of players, multiplayer/networking.
#define MAX_MAXPLAYERS   8

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.
#define FUTURE_MAXPLAYERS 32

// phares 5/14/98:
// DOOM Editor Numbers (aka doomednum in mobj_t)

#define DEN_PLAYER5 4001
#define DEN_PLAYER6 4002
#define DEN_PLAYER7 4003
#define DEN_PLAYER8 4004

// State updates, number of tics / second.
#define TICRATE          35

// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

enum struct GameState : int32_t
{
	Default = -1,
	Level,
	Intermission,
	Finale,
	Demoscreen
};

//
// Key cards.
//

enum struct Card : int32_t
{
	BlueCard,
	YellowCard,
	RedCard,
	BlueSkull,
	YellowSkull,
	RedSkull,
	DoomCount,

	// heretic
	KeyBlue = 0,
	KeyYellow,
	KeyGreen,

	// hexen
	Key1 = 0,
	Key2,
	Key3,
	Key4,
	Key5,
	Key6,
	Key7,
	Key8,
	Key9,
	KeyA,
	KeyB,
	Count
};

// The defined weapons, including a marker
// indicating user has not changed weapon.
enum struct WeaponType : int32_t
{
	Fist,
	Pistol,
	Shotgun,
	Chaingun,
	Missile,
	Plasma,
	Bfg,
	Chainsaw,
	Supershotgun,

	// heretic
	Staff = 0,
	GoldWand,
	Crossbow,
	Blaster,
	SkullRod,
	PhoenixRod,
	Mace,
	Gauntlets,
	Beak,

	Count,
	Nochange, // No pending weapon change.

	// hexen
	First = 0,
	Second,
	Third,
	Fourth,
	HexenCount
};

// Ammunition types defined.
enum struct AmmoType : int32_t
{
	Clip,  // Pistol / chaingun ammo.
	Shell, // Shotgun / double barreled shotgun.
	Cell,  // Plasma rifle, BFG.
	Misl,  // Missile launcher.
	DoomCount,

	// heretic
	GoldWand = 0,
	Crossbow,
	Blaster,
	SkullRod,
	PhoenixRod,
	Mace,
	HereticCount,

	Count = HereticCount,
	NoAmmo, // fist, chainsaw, staff, gauntlets

	// hexen
	Mana1 = 0,
	Mana2,
	ManaCount,
	ManaBoth,
	ManaNone = NoAmmo
};

// Power up artifacts.
enum struct PowerType : int32_t
{
	Invulnerability,
	Strength,
	Invisibility,
	IronFeet,
	AllMap,
	Infrared,

	// heretic
	WeaponLevel2,
	Flight,
	Shield,
	Health2,

	// hexen
	Speed,
	Minotaur,

	Count
};

// Power up durations (how many seconds till expiration).
enum struct PowerDuration : int32_t
{
	Invulntics   = (30 * TICRATE),
	Invistics    = (60 * TICRATE),
	Infratics    = (120 * TICRATE),
	Irontics     = (60 * TICRATE),
	Wpnlev2tics  = (40 * TICRATE),
	Flighttics   = (60 * TICRATE),
	Speedtics    = (45 * TICRATE),
	Morphtics    = (40 * TICRATE),
	Maulatortics = (25 * TICRATE)
};

// phares 3/20/98:
//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.

#define MORE_FRICTION_MOMENTUM 15000       // mud factor based on momentum
#define ORIG_FRICTION          0xE800      // original value
#define ORIG_FRICTION_FACTOR   2048        // original value
#define FRICTION_FLY           0xeb00

extern dboolean started_demo;

extern dboolean raven;

// heretic

#define FRICTION_LOW 0xf900
#define TELEFOGHEIGHT (32*FRACUNIT)
#define ANG1_X          0x01000000

#define FOOTCLIPSIZE 10*FRACUNIT

// Any floor type >= FLOOR_LIQUID will floorclip sprites (hexen)
enum struct FloorType : int32_t
{
	Solid,
	Ice,
	Liquid,
	Water,
	Lava,
	Sludge
};

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

#define TOCENTER -8

#define BLINKTHRESHOLD (4*32)

// TODO_HEXEN: BLINKTHRESHOLD is (4*35)

extern dboolean heretic;

//hexen

// The top 3 bits of the artifact field in the ticcmd_t struct are used
//              as additional flags
#define AFLAG_MASK    0x3F
#define AFLAG_SUICIDE 0x40
#define AFLAG_JUMP    0x80

enum struct ArmorType : int32_t
{
	Armor,
	Shield,
	Helmet,
	Amulet,
	Count
};

enum struct PClass : int32_t
{
	Null,
	Fighter,
	Cleric,
	Mage,
	Pig,
	Count
};

typedef AmmoType manatype_t;

#define MAX_MANA 200

#define WPIECE1 1
#define WPIECE2 2
#define WPIECE3 4

enum struct SoundSequence : int32_t
{
	Platform,
	PlatformHeavy, // same script as a normal platform
	PlatformMetal,
	PlatformCreak, // same script as a normal platform
	PlatformSilence,
	PlatformLava,
	PlatformWater,
	PlatformIce,
	PlatformEarth,
	PlatformMetal2,
	DoorStone,
	DoorHeavy,
	DoorMetal,
	DoorCreak,
	DoorSilence,
	DoorLava,
	DoorWater,
	DoorIce,
	DoorEarth,
	DoorMetal2,
	EsoundWind,
	Numseq
};

enum struct SeqType : int32_t
{
	Stone,
	Heavy,
	Metal,
	Creak,
	Silence,
	Lava,
	Water,
	Ice,
	Earth,
	Metal2,
	Numseq
};

#define MAX_INTRMSN_MESSAGE_SIZE 1024

// Puzzle artifacts

#define TXT_ARTIPUZZSKULL      "YORICK'S SKULL"
#define TXT_ARTIPUZZGEMBIG     "HEART OF D'SPARIL"
#define TXT_ARTIPUZZGEMRED     "RUBY PLANET"
#define TXT_ARTIPUZZGEMGREEN1  "EMERALD PLANET"
#define TXT_ARTIPUZZGEMGREEN2  "EMERALD PLANET"
#define TXT_ARTIPUZZGEMBLUE1   "SAPPHIRE PLANET"
#define TXT_ARTIPUZZGEMBLUE2   "SAPPHIRE PLANET"
#define TXT_ARTIPUZZBOOK1      "DAEMON CODEX"
#define TXT_ARTIPUZZBOOK2      "LIBER OSCURA"
#define TXT_ARTIPUZZSKULL2     "FLAME MASK"
#define TXT_ARTIPUZZFWEAPON    "GLAIVE SEAL"
#define TXT_ARTIPUZZCWEAPON    "HOLY RELIC"
#define TXT_ARTIPUZZMWEAPON    "SIGIL OF THE MAGUS"
#define TXT_ARTIPUZZGEAR       "CLOCK GEAR"
#define TXT_USEPUZZLEFAILED    "YOU CANNOT USE THIS HERE"

extern dboolean hexen;

#ifdef __cplusplus
}
#endif
