// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Input

#pragma once

#include "doomdef.hpp"
#include "doomtype.hpp"
#include "d_event.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define DSDA_INPUT_PROFILE_COUNT 3
#define MAX_MOUSE_BUTTONS 8
#define MAX_JOY_BUTTONS 23

enum struct InputId : int32_t
{
	Null,
	Forward,
	Backward,
	Turnleft,
	Turnright,
	Speed,
	Strafeleft,
	Straferight,
	Strafe,
	Autorun,
	Reverse,
	Use, // note - joyb use doubles as join demo
	Flyup,
	Flydown,
	Flycenter,
	Mlook,
	Novert,
	Weapon1,
	Weapon2,
	Weapon3,
	Weapon4,
	Weapon5,
	Weapon6,
	Weapon7,
	Weapon8,
	Weapon9,
	Nextweapon,
	Prevweapon,
	Toggleweapon,
	Fire,
	Lookup,
	Lookdown,
	Lookcenter,
	UseArtifact,
	ArtiTome,
	ArtiQuartz,
	ArtiUrn,
	ArtiBomb,
	ArtiRing,
	ArtiChaosdevice,
	ArtiShadowsphere,
	ArtiWings,
	ArtiTorch,
	ArtiMorph,
	Invleft,
	Invright,
	Spy,
	JoinDemo,
	Pause,
	Map,
	Soundvolume,
	Hud,
	Messages,
	Gamma,
	Zoomin,
	Zoomout,
	Screenshot,
	Savegame,
	Loadgame,
	Quicksave,
	Quickload,
	LevelTable,
	Endgame,
	Quit,
	StoreQuickKeyFrame,
	RestoreQuickKeyFrame,
	Rewind,
	MapFollow,
	MapZoomin,
	MapZoomout,
	MapUp,
	MapDown,
	MapLeft,
	MapRight,
	MapMark,
	MapClear,
	MapGobig,
	MapGrid,
	MapRotate,
	MapOverlay,
	MapTextured,
	MapHighlightByTag,
	RepeatMessage,
	SpeedUp,
	SpeedDown,
	SpeedDefault,
	DemoSkip,
	DemoEndlevel,
	Walkcamera,
	Restart,
	Nextlevel,
	Prevlevel,
	Showalive,
	MenuDown,
	MenuUp,
	MenuLeft,
	MenuRight,
	MenuBackspace,
	MenuEnter,
	MenuEscape,
	MenuClear,
	Help,
	CycleProfile,
	Iddqd,
	Idkfa,
	Idfa,
	Idclip,
	Idbeholdh,
	Idbeholdm,
	Idbeholdv,
	Idbeholds,
	Idbeholdi,
	Idbeholdr,
	Idbeholda,
	Idbeholdl,
	Idmypos,
	Idrate,
	Iddt,
	CyclePalette,
	CommandDisplay,
	StrictMode,
	Ponce,
	Shazam,
	Chicken,
	Console,
	CoordinateDisplay,
	Jump,
	HexenArtiIncant,
	HexenArtiSummon,
	HexenArtiDisk,
	HexenArtiFlechette,
	HexenArtiBanishment,
	HexenArtiBoots,
	HexenArtiKrater,
	HexenArtiBracers,
	Avj,
	Exhud,
	MuteSfx,
	MuteMusic,
	CheatCodes,
	Notarget,
	Freeze,
	Build,
	BuildAdvanceFrame,
	BuildReverseFrame,
	BuildResetCommand,
	BuildForward,
	BuildBackward,
	BuildTurnLeft,
	BuildTurnRight,
	BuildStrafeLeft,
	BuildStrafeRight,
	BuildUse,
	BuildFire,
	BuildWeapon1,
	BuildWeapon2,
	BuildWeapon3,
	BuildWeapon4,
	BuildWeapon5,
	BuildWeapon6,
	BuildWeapon7,
	BuildWeapon8,
	BuildWeapon9,
	BuildSource,
	BuildFineForward,
	BuildFineBackward,
	BuildFineStrafeLeft,
	BuildFineStrafeRight,
	Script0,
	Script1,
	Script2,
	Script3,
	Script4,
	Script5,
	Script6,
	Script7,
	Script8,
	Script9,
	Fps,
	Count,

	End = -1 // terminates an input list
};

typedef struct
{
	KeyCode* key;
	int num_keys;
	int mouseb;
	int joyb;
} dsda_input_t;

typedef struct
{
	KeyCode key;
	int mouseb;
	int joyb;
} dsda_input_default_t;

void dsda_InputFlushTick();
void dsda_InputTrackEvent(event_t* ev);
void dsda_InputTrackGameEvent(event_t* ev);
dboolean dsda_InputActivated(InputId identifier);
dboolean dsda_InputTickActivated(InputId identifier);
dboolean dsda_InputDeactivated(InputId identifier);
dsda_input_t* dsda_Input(InputId identifier);
void dsda_InputFlush();
void dsda_InputCopy(InputId identifier, dsda_input_t* input[DSDA_INPUT_PROFILE_COUNT]);
int dsda_InputMatchKey(InputId identifier, KeyCode value);
int dsda_InputMatchMouseB(InputId identifier, int value);
int dsda_InputMatchJoyB(InputId identifier, int value);
void dsda_InputReset(InputId identifier);
void dsda_InputResetSpecific(int config_index, InputId identifier);
void dsda_InputSet(InputId identifier, dsda_input_default_t input);
void dsda_InputSetSpecific(int config_index, InputId identifier, dsda_input_default_t input);
void dsda_InputAddKey(InputId identifier, KeyCode value);
void dsda_InputAddSpecificKey(int config_index, InputId identifier, KeyCode value);
void dsda_InputAddMouseB(InputId identifier, int value);
void dsda_InputAddSpecificMouseB(int config_index, InputId identifier, int value);
void dsda_InputAddJoyB(InputId identifier, int value);
void dsda_InputAddSpecificJoyB(int config_index, InputId identifier, int value);
void dsda_InputRemoveKey(InputId identifier, KeyCode value);
void dsda_InputRemoveMouseB(InputId identifier, int value);
void dsda_InputRemoveJoyB(InputId identifier, int value);
dboolean dsda_InputActive(InputId identifer);
dboolean dsda_InputKeyActive(InputId identifier);
dboolean dsda_InputMouseBActive(InputId identifier);
dboolean dsda_InputJoyBActive(InputId identifier);

#ifdef __cplusplus
}
#endif
