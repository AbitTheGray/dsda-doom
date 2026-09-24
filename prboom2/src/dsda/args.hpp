// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA (Command Line) Args

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct ArgId : int32_t
{
	Help,
	Iwad,
	File,
	Deh,
	Loadgame,
	Playdemo,
	Playlump,
	Timedemo,
	Fastdemo,
	Record,
	Recordfromto,
	FromKeyFrame,
	Warp,
	Skill,
	Uv,
	Nm,
	Episode,
	Complevel,
	Fast,
	Respawn,
	Nomonsters,
	Longtics,
	Shorttics,
	Heretic,
	Hexen,
	Class,
	Randclass,
	Dsdademo,
	SoloNet,
	CoopSpawns,
	PistolStart,
	ChainEpisodes,
	Stroller,
	Turbo,
	GameSpeed,
	Tas,
	Build,
	QuitAfterBruteForce,
	FirstInput,
	Command,
	Skipsec,
	Skiptic,
	TrackPacifist,
	Track100k,
	TrackReality,
	TimeKeys,
	TimeUse,
	TimeSecrets,
	TimeAll,
	TrackPlayer,
	TrackLine,
	TrackLineDistance,
	TrackSector,
	TrackMobj,
	Assign,
	Update,
	Analysis,
	Levelstat,
	ExportTextFile,
	TrackPlayback,
	ExportGhost,
	ImportGhost,
	Consoleplayer,
	Spechit,
	Setmem,
	Data,
	Save,
	Config,
	Hud,
	Shotdir,
	Movie,
	Viddump,
	Dehout,
	Verbose,
	Quiet,
	V,
	Resetgamma,
	ForceOldZdoomNodes,
	Sigsegv,
	Deathmatch,
	Altdeath,
	Timer,
	Frags,
	Nosound,
	Nomusic,
	Nosfx,
	Nodraw,
	Nodeh,
	Nomapinfo,
	Noautoload,
	Nocheats,
	Nojoy,
	Nomouse,
	NoMessageBox,
	Fullscreen,
	Window,
	Width,
	Height,
	Geometry,
	Vidmode,
	Aspect,
	Emulate,
	Doom95,
	Blockmap,
	ForceMonsterAvoidHazards,
	ForceRemoveSlimeTrails,
	ForceNoDropoff,
	ForceTruncatedSectorSpecials,
	ForceBoomBrainawake,
	ForcePrboomFriction,
	RejectPadWithFf,
	ForceLxdoomDemoCompatibility,
	AllowSsgDirect,
	TreatNoClippingThingsAsNotBlocking,
	ForceIncorrectProcessingOfRespawnFrameEntry,
	ForceCorrectCodeFor3KeysDoorsInMbf,
	UninitializeCrushFieldForStairs,
	ForceBoomFindnexthighestfloor,
	AllowSkyTransferInBoom,
	ApplyGreenArmorClassToArmorBonuses,
	ApplyBlueArmorClassToMegasphere,
	ForceIncorrectBobbingInBoom,
	BoomDehParser,
	MbfRemoveThinkerInKillmobj,
	DoNotInheritFriendlynessFlagOnSpawn,
	DoNotUseMisc12FrameParametersInAMushroom,
	ApplyMbfCodepointersToAnyComplevel,
	ResetMonsterspawnerParamsAfterLoading,
	Count,
};

typedef struct
{
	int count;
	dboolean found;

	union
	{
		int v_int;
		int* v_int_array;
		const char* v_string;
		const char** v_string_array;
	} value;
} dsda_arg_t;

void dsda_ParseCommandLineArgs(int argc, char** argv);
dsda_arg_t* dsda_Arg(ArgId id);
dboolean dsda_Flag(ArgId id);
int dsda_SimpleIntArg(ArgId id);
void dsda_UpdateIntArg(ArgId id, const char* param);
void dsda_UpdateStringArg(ArgId id, const char* param);
void dsda_AppendStringArg(ArgId id, const char* param);
void dsda_UpdateFlag(ArgId id, dboolean found);
void dsda_PrintArgHelp();

#ifdef __cplusplus
}
#endif
