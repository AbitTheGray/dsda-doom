// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <utility>

#include <stdarg.h>
#include "hu_lib.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define GL_COMBINE_ARB                    0x8570
#define GL_RGB_SCALE_ARB                  0x8573

#define FOV_CORRECTION_FACTOR (1.13776f)
#define FOV90 (90)

typedef struct camera_s
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	fixed_t PrevX;
	fixed_t PrevY;
	fixed_t PrevZ;
	angle_t angle;
	angle_t pitch;
	angle_t PrevAngle;
	angle_t PrevPitch;
	int type;
} camera_t;

extern dboolean wasWiped;

extern int secretfound;
extern int demo_tics_count;
extern int demo_playerscount;
extern char demo_len_st[80];

extern int mouse_handler;

extern int gl_render_fov;
extern float gl_render_ratio;
extern float gl_render_fovratio;
extern float gl_render_fovy;
extern float gl_render_multiplier;
void M_ChangeAspectRatio();
void M_ChangeStretch();

extern camera_t walkcamera;

extern int PitchSign;
extern angle_t viewpitch;
extern float skyscale;
extern float screen_skybox_zplane;
extern float maxNoPitch[];
extern float tan_pitch;
extern float skyUpAngle;
extern float skyUpShift;
extern float skyXShift;
extern float skyYShift;

void ParamsMatchingCheck();
void e6y_HandleSkip();
void e6y_InitCommandLine();

void P_WalkTicker();
void P_SyncWalkcam(dboolean sync_coords, dboolean sync_sight);
void P_ResetWalkcam();

extern dboolean sound_inited_once;
void e6y_I_uSleep(unsigned long usecs);
void G_SkipDemoStop();
void G_SkipDemoStartCheck();
void G_SkipDemoCheck();
int G_ReloadLevel();
int G_GotoNextLevel();
int G_GotoPrevLevel();

void M_ChangeSkyMode();

void M_ChangeFOV();

void M_ChangeSpeed();
void M_ChangeScreenMultipleFactor();
void M_ChangeInterlacedScanning();
void M_MouseMLook(int choice);
void M_MouseAccel(int choice);
void CheckPitch(signed int* pitch);

dboolean HaveMouseLook();

extern float viewPitch;

typedef struct prboom_comp_s
{
	unsigned int minver;
	unsigned int maxver;
	dboolean state;
	int arg_id;
} prboom_comp_t;

enum struct PrboomComp : int32_t
{
	MonsterAvoidHazards,
	RemoveSlimeTrails,
	NoDropoff,
	TruncatedSectorSpecials,
	BoomBrainAwake,
	PrboomFriction,
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
	Max
};

extern prboom_comp_t prboom_comp[];

int StepwiseSum(int value, int direction, int minval, int maxval, int defval);

enum struct TotalsDisplay : int32_t
{
	AllKill,
	AllItem,
	AllSecret,

	Time,
	TotalTime,
	TotalKill,
	TotalItem,
	TotalSecret,

	Max
};

typedef struct timetable_s
{
	char map[16];

	int kill[MAX_MAXPLAYERS];
	int item[MAX_MAXPLAYERS];
	int secret[MAX_MAXPLAYERS];

	int stat[std::to_underlying(TotalsDisplay::Max)];
} timetable_t;

#ifdef _WIN32
const char* WINError();
#endif

extern int stats_level;
extern int stroller;

void e6y_G_DoCompleted();
void e6y_WriteStats();

void e6y_G_DoTeleportNewMap();
void e6y_G_DoWorldDone();

void I_ProcessWin32Mouse();
void I_StartWin32Mouse();
void I_EndWin32Mouse();
int AccelerateMouse(int val);
int AccelerateAnalog(float val);
void AccelChanging();

extern int mlooky;

void e6y_G_Compatibility();

const char* PathFindFileName(const char* pPath);

//extern int viewMaxY;

extern dboolean isskytexture;

extern int levelstarttic;

extern int force_singletics_to;

int HU_DrawDemoProgress(int force);
dboolean HU_MouseOnDemoProgressBar(int* x);

#ifdef _WIN32
int GetFullPath(const char* FileName, const char* ext, char* Buffer, size_t BufferLength);
#endif

void I_vWarning(const char* message, va_list argList);

#define PRB_MB_OK                       0x00000000
#define PRB_MB_OKCANCEL                 0x00000001
#define PRB_MB_ABORTRETRYIGNORE         0x00000002
#define PRB_MB_YESNOCANCEL              0x00000003
#define PRB_MB_YESNO                    0x00000004
#define PRB_MB_RETRYCANCEL              0x00000005
#define PRB_MB_DEFBUTTON1               0x00000000
#define PRB_MB_DEFBUTTON2               0x00000100
#define PRB_MB_DEFBUTTON3               0x00000200
#define PRB_IDOK                1
#define PRB_IDCANCEL            2
#define PRB_IDABORT             3
#define PRB_IDRETRY             4
#define PRB_IDIGNORE            5
#define PRB_IDYES               6
#define PRB_IDNO                7
int I_MessageBox(const char* text, unsigned int type);

#ifdef __cplusplus
}
#endif
