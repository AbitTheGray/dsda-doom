// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  definitions, declarations and prototypes for specials
 */

#pragma once

#include "r_defs.hpp"
#include "d_player.hpp"

#include "cpp/Util.hpp"

// How EV_BuildZDoomStairs builds its stairs.
enum struct StairFlag : uint8_t
{
	UseSpecials = Bit<uint8_t>(0u),
	Sync = Bit<uint8_t>(1u),
	Crush = Bit<uint8_t>(2u),
};
ENUM_FLAGS_FUNC(StairFlag)

// How a teleport treats the thing it moves.
enum struct TeleportFlag : uint8_t
{
	DestFog = Bit<uint8_t>(0u),
	SourceFog = Bit<uint8_t>(1u),
	KeepOrientation = Bit<uint8_t>(2u),
	KeepVelocity = Bit<uint8_t>(3u),
	KeepHeight = Bit<uint8_t>(4u),
	RotateBoom = Bit<uint8_t>(5u),
	RotateBoomInverse = Bit<uint8_t>(6u),
	Vanilla = SourceFog | DestFog,
	Silent = KeepOrientation | RotateBoom | KeepHeight,
};
ENUM_FLAGS_FUNC(TeleportFlag)

#ifdef __cplusplus
extern "C"
{
#endif

//      Define values for map objects
#define MO_TELEPORTMAN  14

// p_floor

#define ELEVATORSPEED (FRACUNIT*4)
#define FLOORSPEED     FRACUNIT

// p_ceilng

#define CEILSPEED   FRACUNIT
#define CEILWAIT    150

// p_doors

#define VDOORSPEED  (FRACUNIT*2)
#define VDOORWAIT   150

// p_plats

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT

// p_switch

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS  16

// 1 second, in ticks.
#define BUTTONTIME  TICRATE

// p_lights

// relative to TICRATE
#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80
#define SECRET_SHIFT    7
#define FRICTION_MASK   0x100
#define FRICTION_SHIFT  8
#define PUSH_MASK       0x200
#define PUSH_SHIFT      9

// reserved by boom spec - not implemented?
// bit 10: suppress all sounds within the sector
// bit 11: disable any sounds due to floor or ceiling motion by the sector

// mbf21
#define DEATH_MASK 0x1000 // bit 12
#define KILL_MONSTERS_MASK 0x2000 // bit 13

//jff 02/04/98 Define masks, shifts, for fields in
// generalized linedef types

#define GenEnd                0x8000
#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3c00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift      0

// define masks and shifts for the floor type fields

#define FloorCrush            0x1000
#define FloorChange           0x0c00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3

// define masks and shifts for the ceiling type fields

#define CeilingCrush          0x1000
#define CeilingChange         0x0c00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields

#define LiftTarget            0x0300
#define LiftDelay             0x00c0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftMonsterShift           5
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields

#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00c0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairMonsterShift          5
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields

#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherMonsterShift        5
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields

#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorMonsterShift           7
#define DoorKindShift              5
#define DoorSpeedShift             3

// define masks and shifts for the locked door type fields

#define LockedNKeys           0x0200
#define LockedKey             0x01c0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
	dboolean istexture;
	int picnum;
	int basepic;
	int numpics;
	int speed;
} anim_t;

//e6y
typedef struct
{
	int index;
	anim_t* anim;
} TAnimItemParam;

extern TAnimItemParam* anim_flats;
extern TAnimItemParam* anim_textures;

// define names for the TriggerType field of the general linedefs

enum struct GenTriggerType : int32_t
{
	WalkOnce,
	WalkMany,
	SwitchOnce,
	SwitchMany,
	GunOnce,
	GunMany,
	PushOnce,
	PushMany,
};

// define names for the Speed field of the general linedefs

enum struct MotionSpeed : int32_t
{
	Slow,
	Normal,
	Fast,
	Turbo,
};

// define names for the Target field of the general floor

enum struct GenFloorTarget : int32_t
{
	ToHnF,
	ToLnF,
	ToNnF,
	ToLnC,
	ToC,
	ByST,
	By24,
	By32,
};

// define names for the Changer Type field of the general floor

enum struct GenFloorChange : int32_t
{
	NoChg,
	ChgZero,
	ChgTxt,
	ChgTyp,
};

// define names for the Change Model field of the general floor

enum struct GenFloorModel : int32_t
{
	TriggerModel,
	NumericModel,
};

// define names for the Target field of the general ceiling

enum struct GenCeilingTarget : int32_t
{
	ToHnC,
	ToLnC,
	ToNnC,
	ToHnF,
	ToF,
	ByST,
	By24,
	By32,
};

// define names for the Changer Type field of the general ceiling

enum struct GenCeilingChange : int32_t
{
	NoChg,
	ChgZero,
	ChgTxt,
	ChgTyp,
};

// define names for the Change Model field of the general ceiling

enum struct GenCeilingModel : int32_t
{
	TriggerModel,
	NumericModel,
};

// define names for the Target field of the general lift

enum struct GenLiftTarget : int32_t
{
	F2LnF,
	F2NnF,
	F2LnC,
	LnF2HnF,
};

// define names for the door Kind field of the general ceiling

enum struct GenDoorKind : int32_t
{
	OpenDelayClose,
	Open,
	CloseDelayOpen,
	Close,
};

// define names for the locked door Kind field of the general ceiling

enum struct KeyKind : int32_t
{
	Any,
	RedCard,
	BlueCard,
	YellowCard,
	RedSkull,
	BlueSkull,
	YellowSkull,
	All,
};

//////////////////////////////////////////////////////////////////
//
// enums for classes of linedef triggers
//
//////////////////////////////////////////////////////////////////

//jff 2/23/98 identify the special classes that can share sectors

enum struct SpecialKind : int32_t
{
	Floor,
	Ceiling,
	Lighting,
};

//jff 3/15/98 pure texture/type change for better generalized support
enum struct ChangeKind : int32_t
{
	TriggerOnly,
	NumericOnly,
};

// p_plats

enum struct PlatState : int32_t
{
	Up,
	Down,
	Waiting,
	InStasis
};

enum struct PlatType : int32_t
{
	PerpetualRaise,
	DownWaitUpStay,
	RaiseAndChange,
	RaiseToNearestAndChange,
	BlazeDWUS,
	GenLift, //jff added to support generalized Plat types
	GenPerpetual,
	ToggleUpDn, //jff 3/14/98 added to support instant toggle type

	// hexen - can probably be merged
	PlatPerpetualraise,
	PlatDownwaitupstay,
	PlatDownbyvaluewaitupstay,
	PlatUpwaitdownstay,
	PlatUpbyvaluewaitdownstay,

	// zdoom
	PlatPerpetualRaise,
	PlatDownWaitUpStay,
	PlatDownWaitUpStayStone,
	PlatDownByValue,
	PlatUpByValue,
	PlatUpWaitDownStay,
	PlatUpNearestWaitDownStay,
	PlatRaiseAndStay,
	PlatRaiseAndStayLockout,
	PlatUpByValueStay,
	PlatToggle,
	PlatDownToNearestFloor,
	PlatDownToLowestCeiling,
};

// p_doors

enum struct VerticalDoorType : int32_t
{
	Normal,
	Close30ThenOpen,
	CloseDoor,
	OpenDoor,
	WaitRaiseDoor,
	WaitCloseDoor,
	BlazeRaise,
	BlazeOpen,
	BlazeClose,

	//jff 02/05/98 add generalize door types
	GenRaise,
	GenBlazeRaise,
	GenOpen,
	GenBlazeOpen,
	GenClose,
	GenBlazeClose,
	GenCdO,
	GenBlazeCdO,

	// heretic
	VldNormal,
	VldNormalTurbo,
	VldClose30ThenOpen,
	VldClose,
	VldOpen,
	VldRaiseIn5Mins,

	// hexen - can probably be merged
	DrevNormal,
	DrevClose30thenopen,
	DrevClose,
	DrevOpen,
	DrevRaisein5mins,
};

// p_ceilng

enum struct CeilingKind : int32_t
{
	LowerToFloor,
	RaiseToHighest,
	LowerToLowest,
	LowerToMaxFloor,
	LowerAndCrush,
	CrushAndRaise,
	FastCrushAndRaise,
	SilentCrushAndRaise,

	//jff 02/04/98 add types for generalized ceiling mover
	GenCeiling,
	GenCeilingChg,
	GenCeilingChg0,
	GenCeilingChgT,

	//jff 02/05/98 add types for generalized ceiling mover
	GenCrusher,
	GenSilentCrusher,

	// hexen - can probably be merged
	ClevLowertofloor,
	ClevRaisetohighest,
	ClevLowerandcrush,
	ClevCrushandraise,
	ClevLowerbyvalue,
	ClevRaisebyvalue,
	ClevCrushraiseandstay,
	ClevMovetovaluetimes8,

	// zdoom
	CeilLowerByValue,
	CeilRaiseByValue,
	CeilMoveToValue,
	CeilLowerToHighestFloor,
	CeilLowerInstant,
	CeilRaiseInstant,
	CeilCrushAndRaise,
	CeilLowerAndCrush,
	CeilPlaceholder,
	CeilCrushRaiseAndStay,
	CeilRaiseToNearest,
	CeilLowerToLowest,
	CeilLowerToFloor,
	CeilRaiseToHighest,
	CeilLowerToHighest,
	CeilRaiseToLowest,
	CeilLowerToNearest,
	CeilRaiseToHighestFloor,
	CeilRaiseToFloor,
	CeilRaiseByTexture,
	CeilLowerByTexture,
};

enum struct CrushMode : int32_t
{
	Doom     = 0,
	Hexen    = 1,
	Slowdown = 2,
};

// p_floor

enum struct FloorKind : int32_t
{
	// lower floor to highest surrounding floor
	LowerFloor,

	// lower floor to lowest surrounding floor
	LowerFloorToLowest,

	// lower floor to highest surrounding floor VERY FAST
	TurboLower,

	// raise floor to lowest surrounding CEILING
	RaiseFloor,

	// raise floor to next highest surrounding floor
	RaiseFloorToNearest,

	//jff 02/03/98 lower floor to next lowest neighbor
	LowerFloorToNearest,

	//jff 02/03/98 lower floor 24 absolute
	LowerFloor24,

	//jff 02/03/98 lower floor 32 absolute
	LowerFloor32Turbo,

	// raise floor to shortest height texture around it
	RaiseToTexture,

	// lower floor to lowest surrounding floor
	//  and change floorpic
	LowerAndChange,

	RaiseFloor24,

	//jff 02/03/98 raise floor 32 absolute
	RaiseFloor32Turbo,

	RaiseFloor24AndChange,
	RaiseFloorCrush,

	// raise to next highest floor, turbo-speed
	RaiseFloorTurbo,
	DonutRaise,
	RaiseFloor512,

	//jff 02/04/98  add types for generalized floor mover
	GenFloor,
	GenFloorChg,
	GenFloorChg0,
	GenFloorChgT,

	//new types for stair builders
	BuildStair,
	GenBuildStair,

	// hexen
	FlevLowerfloor,         // lower floor to highest surrounding floor
	FlevLowerfloortolowest, // lower floor to lowest surrounding floor
	FlevLowerfloorbyvalue,
	FlevRaisefloor,          // raise floor to lowest surrounding CEILING
	FlevRaisefloortonearest, // raise floor to next highest surrounding floor
	FlevRaisefloorbyvalue,
	FlevRaisefloorcrush,
	FlevRaisebuildstep, // One step of a staircase
	FlevRaisebyvaluetimes8,
	FlevLowerbyvaluetimes8,
	FlevLowertimes8instant,
	FlevRaisetimes8instant,
	FlevMovetovaluetimes8,

	// zdoom
	FloorLowerByValue,
	FloorLowerToLowest,
	FloorLowerToHighest,
	FloorLowerToNearest,
	FloorRaiseByValue,
	FloorRaiseToHighest,
	FloorRaiseToNearest,
	FloorRaiseToLowest,
	FloorRaiseAndCrush,
	FloorRaiseAndCrushDoom,
	FloorLowerInstant,
	FloorRaiseInstant,
	FloorLowerToCeiling,
	FloorMoveToValue,
	FloorRaiseToLowestCeiling,
	FloorLowerToLowestCeiling,
	FloorRaiseByTexture,
	FloorLowerByTexture,
	FloorRaiseToCeiling,
	FloorRaiseAndChange,
	FloorLowerAndChange,

	FloorBuildStair,
	FloorWaitStair,
	FloorResetStair,
};

enum struct StairType : int32_t
{
	Build8,  // slowly build by 8
	Turbo16, // quickly build by 16

	// heretic
	HereticBuild8,
	HereticTurbo16,

	// zdoom
	BuildDown,
	BuildUp,
};


enum struct ElevatorType : int32_t
{
	Up,
	Down,
	Current,

	// zdoom
	Lower,
	Raise,
};

//////////////////////////////////////////////////////////////////
//
// general enums
//
//////////////////////////////////////////////////////////////////

// texture type enum
enum struct ButtonWhere : int32_t
{
	Top,
	Middle,
	Bottom
};

// crush check returns
enum struct MoveResult : int32_t
{
	Ok,
	Crushed,
	PastDest
};

//////////////////////////////////////////////////////////////////
//
// linedef and sector special data types
//
//////////////////////////////////////////////////////////////////

// p_switch

// switch animation structure type

#if defined(__MWERKS__)
#pragma options align=packed
#endif

typedef struct
{
	char name1[9];
	char name2[9];
	short episode;
}
	PACKEDATTR switchlist_t; //jff 3/23/98 pack to read from memory

#if defined(__MWERKS__)
#pragma options align=reset
#endif

typedef struct
{
	line_t* line;
	ButtonWhere where;
	int btexture;
	int btimer;
	degenmobj_t* soundorg;
} button_t;

// p_lights

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int maxlight;
	int minlight;
} fireflicker_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int maxlight;
	int minlight;
	int maxtime;
	int mintime;
} lightflash_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int minlight;
	int maxlight;
	int darktime;
	int brighttime;
} strobe_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int minlight;
	int maxlight;
	int direction;
} glow_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	short startlevel;
	short endlevel;
	short tics;
	short maxtics;
	dboolean oneshot;
} zdoom_glow_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	short upper;
	short lower;
	short count;
} zdoom_flicker_t;

// p_plats

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	fixed_t speed;
	fixed_t low;
	fixed_t high;
	int wait;
	int count;
	PlatState status;
	PlatState oldstatus;
	int crush;
	int tag;
	PlatType type;

	struct platlist* list; // killough
} plat_t;

// New limit-free plat structure -- killough

typedef struct platlist
{
	plat_t* plat;
	struct platlist *next, **prev;
} platlist_t;

// p_ceilng

typedef struct
{
	thinker_t thinker;
	VerticalDoorType type;
	sector_t* sector;
	fixed_t topheight;
	fixed_t speed;

	// 1 = up, 0 = waiting at top, -1 = down
	int direction;

	// tics to wait at the top
	int topwait;
	// (keep in case a door going down is reset)
	// when it reaches 0, start going down
	int topcountdown;

	//jff 1/31/98 keep track of line door is triggered by
	line_t* line;

	/* killough 10/98: sector tag for gradual lighting effects */
	int lighttag;
} vldoor_t;

typedef struct
{
	short special;
	unsigned int flags;
	damage_t damage;
} newspecial_t;

// p_doors

typedef struct
{
	thinker_t thinker;
	CeilingKind type;
	sector_t* sector;
	fixed_t bottomheight;
	fixed_t topheight;
	fixed_t speed;
	fixed_t oldspeed;
	int crush;

	//jff 02/04/98 add these to support ceiling changers
	newspecial_t newspecial;
	short texture;

	// 1 = up, 0 = waiting, -1 = down
	int direction;

	// ID
	int tag;
	int olddirection;
	struct ceilinglist* list; // jff 2/22/98 copied from killough's plats

	// zdoom
	fixed_t speed2;
	CrushMode crushmode;
	byte silent;
} ceiling_t;

typedef struct ceilinglist
{
	ceiling_t* ceiling;
	struct ceilinglist *next, **prev;
} ceilinglist_t;

// p_floor

typedef struct
{
	thinker_t thinker;
	FloorKind type;
	int crush;
	sector_t* sector;
	int direction;
	newspecial_t newspecial;
	short texture;
	fixed_t floordestheight;
	fixed_t speed;

	// hexen
	int delayCount;
	int delayTotal;
	fixed_t stairsDelayHeight;
	fixed_t stairsDelayHeightDelta;
	fixed_t resetHeight;
	short resetDelay;
	short resetDelayCount;
	byte textureChange;

	// zdoom
	dboolean hexencrush;
} floormove_t;

typedef struct
{
	thinker_t thinker;
	ElevatorType type;
	sector_t* sector;
	int direction;
	fixed_t floordestheight;
	fixed_t ceilingdestheight;
	fixed_t speed;
} elevator_t;

// p_spec

// phares 3/12/98: added new model of friction for ice/sludge effects

typedef struct
{
	thinker_t thinker; // Thinker structure for friction
	int friction;      // friction value (E800 = normal)
	int movefactor;    // inertia factor when adding to momentum
	int affectee;      // Number of affected sector
} friction_t;

// phares 3/20/98: added new model of Pushers for push/pull effects

// Declared outside the struct: in C++ an enum nested in a struct scopes its
// enumerators to that struct, while the code uses them unqualified as C did.
enum struct PusherType : int32_t
{
	Push,
	Pull,
	Wind,
	Current,
};

typedef struct
{
	thinker_t thinker; // Thinker structure for Pusher
	PusherType type;

	mobj_t* source; // Point source if point pusher
	int x_mag;      // X Strength
	int y_mag;      // Y Strength
	int magnitude;  // Vector strength for point pusher
	int radius;     // Effective radius for point pusher
	int x;          // X of point source if point pusher
	int y;          // Y of point source if point pusher
	int affectee;   // Number of affected sector
} pusher_t;

//////////////////////////////////////////////////////////////////
//
// external data declarations
//
//////////////////////////////////////////////////////////////////

// list of retriggerable buttons active
extern button_t buttonlist[MAXBUTTONS];

extern platlist_t* activeplats; // killough 2/14/98

extern ceilinglist_t* activeceilings; // jff 2/22/98

////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

int twoSided
(int sector,
	int line);

sector_t* getSector
(int currentSector,
	int line,
	int side);

side_t* getSide
(int currentSector,
	int line,
	int side);

fixed_t P_FindLowestFloorSurrounding
(sector_t* sec);

fixed_t P_FindHighestFloorSurrounding
(sector_t* sec);

fixed_t P_FindNextHighestFloor
(sector_t* sec,
	int currentheight);

fixed_t P_FindNextLowestFloor
(sector_t* sec,
	int currentheight);

fixed_t P_FindLowestCeilingSurrounding
(sector_t* sec); // jff 2/04/98

fixed_t P_FindHighestCeilingSurrounding
(sector_t* sec); // jff 2/04/98

fixed_t P_FindNextLowestCeiling
(sector_t* sec,
	int currentheight); // jff 2/04/98

fixed_t P_FindNextHighestCeiling
(sector_t* sec,
	int currentheight); // jff 2/04/98

fixed_t P_FindShortestTextureAround
(int secnum); // jff 2/04/98

fixed_t P_FindShortestUpperAround
(int secnum); // jff 2/04/98

sector_t* P_FindModelFloorSector
(fixed_t floordestheight,
	int secnum); //jff 02/04/98

sector_t* P_FindModelCeilingSector
(fixed_t ceildestheight,
	int secnum); //jff 02/04/98

int P_FindMinSurroundingLight
(sector_t* sector,
	int max);

sector_t* getNextSector
(line_t* line,
	sector_t* sec);

int P_CheckTag
(line_t* line); // jff 2/27/98

dboolean P_CanUnlockGenDoor
(line_t* line,
	player_t* player);

dboolean PUREFUNC P_PlaneActive(const sector_t* sec);
dboolean PUREFUNC P_CeilingActive(const sector_t* sec);
dboolean PUREFUNC P_FloorActive(const sector_t* sec);
dboolean PUREFUNC P_LightingActive(const sector_t* sec);

short P_FloorLightLevel(const sector_t* sec);
short P_CeilingLightLevel(const sector_t* sec);
dboolean P_FloorPlanesDiffer(const sector_t* sec, const sector_t* other);
dboolean P_CeilingPlanesDiffer(const sector_t* sec, const sector_t* other);

dboolean PUREFUNC P_IsSecret
(const sector_t* sec);

dboolean PUREFUNC P_IsDeathExit
(const sector_t* sec);

dboolean PUREFUNC P_WasSecret
(const sector_t* sec);

dboolean PUREFUNC P_RevealedSecret
(const sector_t* sec);

void P_ChangeSwitchTexture
(line_t* line,
	int useAgain);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special action function prototypes
//
////////////////////////////////////////////////////////////////

// p_lights

void T_LightFlash
(lightflash_t* flash);

void T_StrobeFlash
(strobe_t* flash);

// jff 8/8/98 add missing thinker for flicker
void T_FireFlicker
(fireflicker_t* flick);

void T_Glow
(glow_t* g);

// p_plats

void T_PlatRaise
(plat_t* plat);

// p_doors

void T_VerticalDoor
(vldoor_t* door);

// p_ceilng

void T_MoveCeiling
(ceiling_t* ceiling);

// p_floor

MoveResult T_MoveFloorPlane
(sector_t* sector,
	fixed_t speed,
	fixed_t dest,
	int crush,
	int direction,
	dboolean hexencrush);

void T_MoveFloor
(floormove_t* floor);

void T_MoveElevator
(elevator_t* elevator);

// p_spec

void T_Friction
(friction_t*); // phares 3/12/98: friction thinker

void T_Pusher
(pusher_t*); // phares 3/20/98: Push thinker

////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

// killough 1/31/98: Add silent line teleporter
int EV_SilentLineTeleport
(line_t* line,
	int side,
	mobj_t* thing,
	int tag,
	dboolean reverse);

// p_floor

int
EV_DoElevator
(line_t* line,
	ElevatorType type);

int EV_BuildStairs
(line_t* line,
	StairType type);

int EV_DoFloor
(line_t* line,
	FloorKind floortype);

// p_ceilng

MoveResult T_MoveCeilingPlane
(sector_t* sector,
	fixed_t speed,
	fixed_t dest,
	int crush,
	int direction,
	dboolean hexencrush);

int EV_DoCeiling
(line_t* line,
	CeilingKind type);

int EV_CeilingCrushStop
(line_t* line);

// p_doors

int EV_VerticalDoor
(line_t* line,
	mobj_t* thing);

int EV_DoDoor
(line_t* line,
	VerticalDoorType type);

int EV_DoLockedDoor
(line_t* line,
	VerticalDoorType type,
	mobj_t* thing);

// p_lights

int EV_StartLightStrobing
(line_t* line);

int EV_TurnTagLightsOff
(line_t* line);

int EV_LightTurnOn
(line_t* line,
	int bright);

int EV_LightTurnOnPartway(line_t* line, fixed_t level); // killough 10/10/98

// p_floor

int EV_DoChange
(line_t* line,
	ChangeKind changetype,
	int tag);

int EV_DoDonut
(line_t* line);

// p_plats

int EV_DoPlat
(line_t* line,
	PlatType type,
	int amount);

int EV_StopPlat
(line_t* line);

// p_genlin

int EV_DoGenFloor
(line_t* line);

int EV_DoGenCeiling
(line_t* line);

int EV_DoGenLift
(line_t* line);

int EV_DoGenStairs
(line_t* line);

int EV_DoGenCrusher
(line_t* line);

int EV_DoGenDoor
(line_t* line);

int EV_DoGenLockedDoor
(line_t* line);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special thinker spawning
//
////////////////////////////////////////////////////////////////

// at game start
void P_InitPicAnims
();

void P_InitSwitchList
();

void P_StartButton
(line_t* line,
	ButtonWhere w,
	int texture,
	int time);

// at map load
void P_SpawnSpecials
();

// every tic
void P_UpdateSpecials
();

// when needed
dboolean P_UseSpecialLine
(mobj_t* thing,
	line_t* line,
	int side,
	dboolean noplayercheck);

void P_PlayerInSpecialSector
(player_t* player);

// p_lights

void P_SpawnFireFlicker
(sector_t* sector);

void P_SpawnLightFlash
(sector_t* sector);

void P_SpawnStrobeFlash
(sector_t* sector,
	int fastOrSlow,
	int inSync);

void P_SpawnGlowingLight
(sector_t* sector);

// p_plats

void P_AddActivePlat
(plat_t* plat);

void P_RemoveActivePlat
(plat_t* plat);

void P_RemoveAllActivePlats
(); // killough

void P_ActivateInStasis
(int tag);

// p_doors

void P_SpawnDoorCloseIn30
(sector_t* sec);

void P_SpawnDoorRaiseIn5Mins
(sector_t* sec,
	int secnum);

// p_ceilng

void P_RemoveActiveCeiling
(ceiling_t* ceiling); //jff 2/22/98

void P_RemoveAllActiveCeilings
(); //jff 2/22/98

void P_AddActiveCeiling
(ceiling_t* c);

int P_ActivateInStasisCeiling
(int tag);

mobj_t* P_GetPushThing(int); // phares 3/23/98

// heretic

void P_InitTerrainTypes();
void P_InitLava();
void P_SpawnLineSpecials();

#define MAX_AMBIENT_SFX 8

extern int AmbSfxTics;
extern int AmbSfxVolume;
extern int AmbSfxPtrIndex;
extern int* AmbSfxPtr;
extern int* LevelAmbientSfx[MAX_AMBIENT_SFX];
extern int* TerrainTypes;

void P_InitAmbientSound();
void P_AmbientSound();
void P_AddAmbientSfx(int sequence);
dboolean P_Teleport(mobj_t* thing, fixed_t x, fixed_t y, angle_t angle, dboolean useFog);
dboolean Heretic_P_UseSpecialLine(mobj_t* thing, line_t* line, int side, dboolean bossaction);
void Heretic_EV_VerticalDoor(line_t* line, mobj_t* thing);

// hexen

// p_lights

enum struct LightType : int32_t
{
	RaiseByValue,
	LowerByValue,
	ChangeToValue,
	Fade,
	Glow,
	Flicker,
	Strobe
};

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	LightType type;
	int value1;
	int value2;
	int tics1;
	int tics2;
	int count;
} light_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int index;
	int base;
} phase_t;

void T_Phase(phase_t* phase);
void T_Light(light_t* light);
void P_SpawnPhasedLight(sector_t* sector, int base, int index);
void P_SpawnLightSequence(sector_t* sector, int indexStep);
dboolean EV_SpawnLight(line_t* line, byte* arg, LightType type);

// p_ceilng

int Hexen_EV_CeilingCrushStop(line_t* line, byte* args);
int Hexen_EV_DoCeiling(line_t* line, byte* arg, CeilingKind type);

// p_telept

dboolean EV_HexenTeleport(int tid, mobj_t* thing, dboolean fog);

// p_doors

int Hexen_EV_DoDoor(line_t* line, byte* args, VerticalDoorType type);
dboolean Hexen_EV_VerticalDoor(line_t* line, mobj_t* thing);

// p_floor

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int ceilingSpeed;
	int floorSpeed;
	int floordest;
	int ceilingdest;
	int direction;
	int crush;

	// zdoom
	dboolean hexencrush;
} pillar_t;

enum struct PillarType : int32_t
{
	Build,
	Open,
};

// Phase of a Hexen floor or ceiling waggle.
enum struct WaggleState : int32_t
{
	Expand = 1,
	Stable = 2,
	Reduce = 3,
};

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	fixed_t originalHeight;
	fixed_t accumulator;
	fixed_t accDelta;
	fixed_t targetScale;
	fixed_t scale;
	fixed_t scaleDelta;
	int ticker;
	WaggleState state;
} planeWaggle_t;

enum struct StairsMode : int32_t
{
	Normal,
	Sync,
	Phased
};

int Hexen_EV_DoFloor(line_t* line, byte* args, FloorKind floortype);
int EV_DoFloorAndCeiling(line_t* line, byte* args, dboolean raise);
int Hexen_EV_BuildStairs(line_t* line, byte* args, int direction, StairsMode stairsType);
void T_BuildPillar(pillar_t* pillar);
int EV_BuildPillar(line_t* line, byte* args, int crush);
int EV_OpenPillar(line_t* line, byte* args);
int EV_FloorCrushStop(line_t* line, byte* args);
void T_FloorWaggle(planeWaggle_t* waggle);
void T_CeilingWaggle(planeWaggle_t* waggle);
dboolean EV_StartFloorWaggle(int tag, int height, int speed, int offset, int timer);

// p_plats

int EV_DoHexenPlat(line_t* line, byte* args, PlatType type, int amount);
void Hexen_EV_StopPlat(line_t* line, byte* args);

// quake

typedef struct
{
	thinker_t thinker;
	mobj_t* location;
	int duration;
	int intensity;
	int damage_radius;
	int tremor_radius;
} quake_t;

void dsda_ResetQuakes();
void dsda_UpdateQuakeIntensity(int player_num, int intensity);
void dsda_UpdateQuake(quake_t* quake);
void dsda_SpawnQuake(mobj_t* location, int intensity, int duration,
	int damage_radius, int tremor_radius);

//

dboolean P_ActivateLine(line_t* line, mobj_t* mo, int side, line_activation_t activationType);
void P_PlayerOnSpecialFlat(player_t* player, FloorType floorType);
line_t* P_FindLine(int lineTag, int* searchPosition);

dboolean P_IsSpecialSector(sector_t* sector);
void P_CopySectorSpecial(sector_t* dest, sector_t* source);
void P_TransferSpecial(sector_t* sector, newspecial_t* newspecial);
void P_CopyTransferSpecial(newspecial_t* newspecial, sector_t* sector);
void P_ResetTransferSpecial(newspecial_t* newspecial);
void P_ResetSectorSpecial(sector_t* sector);
void P_ClearNonGeneralizedSectorSpecial(sector_t* sector);

enum struct ZDoomSectorSpecial : int32_t
{
	LightPhased            = 1,
	LightSequenceStart    = 2,
	LightSequenceSpecial1 = 3,
	LightSequenceSpecial2 = 4,

	StairsSpecial1 = 26,
	StairsSpecial2 = 27,

	WindEastWeak    = 40,
	WindEastMedium  = 41,
	WindEastStrong  = 42,
	WindNorthWeak   = 43,
	WindNorthMedium = 44,
	WindNorthStrong = 45,
	WindSouthWeak   = 46,
	WindSouthMedium = 47,
	WindSouthStrong = 48,
	WindWestWeak    = 49,
	WindWestMedium  = 50,
	WindWestStrong  = 51,

	DLightFlicker     = 65,
	DLightStrobeFast = 66,
	DLightStrobeSlow = 67,
	DLightStrobeHurt = 68,
	DDamageHellslime  = 69,

	DDamageNukage = 71,
	DLightGlow    = 72,

	DSectorDoorCloseIn30     = 74,
	DDamageEnd                  = 75,
	DLightStrobeSlowSync      = 76,
	DLightStrobeFastSync      = 77,
	DSectorDoorRaiseIn5Mins = 78,
	DFrictionLow                = 79,
	DDamageSuperHellslime      = 80,
	DLightFireFlicker          = 81,
	DDamageLavaWimpy           = 82,
	DDamageLavaHefty           = 83,
	DScrollEastLavaDamage     = 84,
	HDamageSludge               = 85,

	SectorOutside = 87,

	SLightStrobeHurt = 104,
	SDamageHellslime  = 105,

	DamageInstantDeath     = 115,
	SDamageSuperHellslime = 116,

	ScrollStrifeCurrent = 118,

	SectorHidden           = 195,
	SectorHeal             = 196,
	LightOutdoorLightning = 197,
	LightIndoorLightning1 = 198,
	LightIndoorLightning2 = 199,
	Sky2                    = 200,

	// hexen-type scrollers
	ScrollNorthSlow       = 201,
	ScrollNorthMedium     = 202,
	ScrollNorthFast       = 203,
	ScrollEastSlow        = 204,
	ScrollEastMedium      = 205,
	ScrollEastFast        = 206,
	ScrollSouthSlow       = 207,
	ScrollSouthMedium     = 208,
	ScrollSouthFast       = 209,
	ScrollWestSlow        = 210,
	ScrollWestMedium      = 211,
	ScrollWestFast        = 212,
	ScrollNorthwestSlow   = 213,
	ScrollNorthwestMedium = 214,
	ScrollNorthwestFast   = 215,
	ScrollNortheastSlow   = 216,
	ScrollNortheastMedium = 217,
	ScrollNortheastFast   = 218,
	ScrollSoutheastSlow   = 219,
	ScrollSoutheastMedium = 220,
	ScrollSoutheastFast   = 221,
	ScrollSouthwestSlow   = 222,
	ScrollSouthwestMedium = 223,
	ScrollSouthwestFast   = 224,

	// heretic-type scrollers
	CarryEast5   = 225,
	CarryEast10  = 226,
	CarryEast25  = 227,
	CarryEast30  = 228,
	CarryEast35  = 229,
	CarryNorth5  = 230,
	CarryNorth10 = 231,
	CarryNorth25 = 232,
	CarryNorth30 = 233,
	CarryNorth35 = 234,
	CarrySouth5  = 235,
	CarrySouth10 = 236,
	CarrySouth25 = 237,
	CarrySouth30 = 238,
	CarrySouth35 = 239,
	CarryWest5   = 240,
	CarryWest10  = 241,
	CarryWest25  = 242,
	CarryWest30  = 243,
	CarryWest35  = 244
};

#define ZDOOM_DAMAGE_MASK   0x0300
#define ZDOOM_SECRET_MASK   0x0400
#define ZDOOM_FRICTION_MASK 0x0800
#define ZDOOM_PUSH_MASK     0x1000

enum struct ZDoomLock : int32_t
{
	None         = 0,
	RedCard     = 1,
	BlueCard    = 2,
	YellowCard  = 3,
	RedSkull    = 4,
	BlueSkull   = 5,
	YellowSkull = 6,
	Any          = 100,
	All          = 101,
	Red          = 129,
	Blue         = 130,
	Yellow       = 131,
	Redx         = 132, // not sure why these redundant ones exist
	Bluex        = 133,
	Yellowx      = 134,
	EachColor   = 229,
};

void P_AddMobjSecret(mobj_t* mobj);
void P_PlayerCollectSecret(player_t* player);
dboolean P_CheckKeys(mobj_t* mo, ZDoomLock lock, dboolean legacy);
dboolean P_CheckSwitchRange(line_t* line, mobj_t* mo, int sideno);
int EV_DoZDoomDoor(VerticalDoorType type, line_t* line, mobj_t* mo, int tag, fixed_t speed, int topwait,
	ZDoomLock lock, int lightTag, dboolean boomgen, int topcountdown);
int EV_DoZDoomFloor(FloorKind floortype, line_t* line, int tag, fixed_t speed, fixed_t height,
	int crush, int change, dboolean hexencrush, dboolean hereticlower);
int EV_ZDoomFloorStop(int tag, line_t* line);
int EV_ZDoomFloorCrushStop(int tag);
int EV_DoZDoomDonut(int tag, line_t* line, fixed_t pillarspeed, fixed_t slimespeed);
int EV_DoZDoomCeiling(CeilingKind type, line_t* line, int tag, fixed_t speed, fixed_t speed2,
	fixed_t height, int crush, byte silent, int change, CrushMode crushmode);
int EV_ZDoomCeilingStop(int tag, line_t* line);
int EV_ZDoomCeilingCrushStop(int tag, dboolean remove);
int EV_DoZDoomPlat(int tag, line_t* line, PlatType type, fixed_t height,
	fixed_t speed, int delay, fixed_t lip, int change);
void EV_StopZDoomPlat(int tag, dboolean remove);
int EV_BuildZDoomStairs(int tag, StairType type, line_t* line, fixed_t stairsize,
	fixed_t speed, int delay, int reset, int igntxt, StairFlag usespecials);
dboolean EV_StartPlaneWaggle(int tag, line_t* line, int height,
	int speed, int offset, int timer, dboolean ceiling);
int EV_DoZDoomPillar(PillarType type, line_t* line, int tag, fixed_t speed,
	fixed_t height, fixed_t height2, int crush, dboolean hexencrush);
int EV_DoZDoomElevator(line_t* line, ElevatorType type, fixed_t speed, fixed_t height, int tag);
void EV_LightChange(int tag, short change);
void EV_LightSet(int tag, short level);
void EV_LightSetMinNeighbor(int tag);
void EV_LightSetMaxNeighbor(int tag);
void EV_StartLightFading(int tag, short level, short tics);
void EV_StartLightGlowing(int tag, short upper, short lower, short tics);
void EV_StartLightFlickering(int tag, short upper, short lower);
void EV_StartZDoomLightStrobing(int tag, int upper, int lower, int brighttime, int darktime);
void EV_StartZDoomLightStrobingDoom(int tag, int brighttime, int darktime);
void EV_StopLightEffect(int tag);
void T_ZDoom_Glow(zdoom_glow_t* g);
void T_ZDoom_Flicker(zdoom_flicker_t* g);
int P_ConvertHexenCrush(int crush);
void P_ResolveFrictionFactor(fixed_t friction_factor, sector_t* sec);

int EV_TeleportGroup(short group_tid, mobj_t* thing, short source_tid, short dest_tid,
	dboolean move_source, dboolean fog);
int EV_TeleportInSector(int tag, short source_tid, short dest_tid,
	dboolean fog, short group_tid);

#define NO_CRUSH -1
#define DOOM_CRUSH 10


#ifdef __cplusplus
}
#endif
