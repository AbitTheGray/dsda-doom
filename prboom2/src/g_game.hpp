// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION: Main game control interface.
 */

#pragma once

#include "doomdef.hpp"
#include "d_event.hpp"
#include "d_ticcmd.hpp"
#include "tables.hpp"

enum struct PlaybackBehaviour : uint8_t;

#ifdef __cplusplus
extern "C"
{
#endif

//
// GAME
//

#define DEMOMARKER    0x80

dboolean G_Responder(event_t* ev);
dboolean G_CheckDemoStatus();
void G_DeathMatchSpawnPlayer(int playernum);
void G_InitNew(int skill, int episode, int map, dboolean prepare);
void G_DeferedInitNew(int skill, int episode, int map);
void G_DeferedPlayDemo(const char* demo);            // CPhipps - const
void G_LoadGame(int slot, dboolean via_commandline); // killough 5/15/98
void G_ForcedLoadGame();                         // killough 5/15/98: forced loadgames
void G_DoLoadGame();
void G_SaveGame(int slot, const char* description); // Called by M_Responder.
void G_BeginRecording();
void G_ExitLevel(int position);
void G_SecretExitLevel(int position);
void G_WorldDone();
void G_EndGame(); /* cph - make m_menu.c call a G_* function for this */
void G_Ticker();
void G_ReloadDefaults();      // killough 3/1/98: loads game defaults
void G_RefreshFastMonsters(); // killough 4/10/98: sets -fast parameters
void G_DoNewGame();
void G_DoReborn(int playernum);
void G_StartDemoPlayback(const byte* buffer, int length, PlaybackBehaviour behaviour);
void G_DoPlayDemo();
void G_DoCompleted();
void G_WriteDemoTiccmd(ticcmd_t* cmd);
void G_DoWorldDone();
void G_Compatibility();
const byte* G_ReadOptions(const byte* demo_p); /* killough 3/1/98 - cph: const byte* */
byte* G_WriteOptions(byte* demo_p);            // killough 3/1/98
void G_PlayerReborn(int player);
void G_DoVictory();
void G_BuildTiccmd(ticcmd_t* cmd); // CPhipps - move decl to header
void G_ReadOneTick(ticcmd_t* cmd, const byte** data_p);
void G_ChangedPlayerColour(int pn, int cl);    // CPhipps - On-the-fly player colour changing
void G_MakeSpecialEvent(ButtonCode bc, ...); /* cph - new event stuff */
int G_ValidateMapName(const char* mapname, int* pEpi, int* pMap);

//e6y
void G_ContinueDemo(const char* playback_name);
void G_SetSpeed(dboolean force);

//e6y
#define RDH_SAFE 0x00000001
#define RDH_SKIP_HEADER 0x00000002
const byte* G_ReadDemoHeaderEx(const byte* demo_p, size_t size, unsigned int params);
void G_CalculateDemoParams(const byte* demo_p);

// killough 5/2/98: moved from m_misc.c:

extern int key_forward;
extern int key_backward;

extern dboolean haswolflevels; //jff 4/18/98 wolf levels present
extern dboolean secretexit;

// killough 5/2/98: moved from d_deh.c:
// Par times (new item with BOOM) - from g_game.c
extern int pars[5][10]; // hardcoded array size
extern int cpars[];     // hardcoded array size
// CPhipps - Make savedesciption visible in wider scope
#define SAVEDESCLEN 32
extern char savedescription[SAVEDESCLEN]; // Description to save in savegame

/* cph - compatibility level strings */
extern const char* comp_lev_str[];

// Hexen Skill Strings
extern const char* hexen_skill_fighter[5];
extern const char* hexen_skill_cleric[5];
extern const char* hexen_skill_mage[5];

// e6y
// There is a new command-line switch "-shorttics".
// This makes it possible to practice routes and tricks
// (e.g. glides, where this makes a significant difference)
// with the same mouse behaviour as when recording,
// but without having to be recording every time.
extern int shorttics;
extern int longtics;

// Allows use of HELP2 screen for PWADs under DOOM 1
extern int pwad_help2_check;

// hexen

void G_Completed(int map, int position, int flags, angle_t angle);

#ifdef __cplusplus
}
#endif
