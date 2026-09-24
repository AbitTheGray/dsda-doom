// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tools

#pragma once

#include "doomdef.hpp"
#include "p_mobj.hpp"
#include "d_player.hpp"
#include "r_defs.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	int m, s, t;
} dsda_level_time_t;

typedef struct
{
	int h, m, s;
} dsda_movie_time_t;

#define LINE_ACTIVATION_INDEX_MAX 8

// TODO: Probably want a command history object split from display
void dsda_ResetCommandHistory();
void dsda_InitCommandHistory();
void dsda_AddCommandToCommandDisplay(ticcmd_t* cmd);

// TODO: Might want a split object separate from display
enum struct SplitClass : int32_t
{
	BlueKey,
	YellowKey,
	RedKey,
	Use,
	Secret,
	Count
};

void dsda_AddSplit(SplitClass split_class, int lifetime);

void dsda_ReadCommandLine();
int dsda_SessionAttempts();
void dsda_DisplayNotifications();
void dsda_WatchReborn(int playernum);
void dsda_WatchCard(Card card);
void dsda_WatchCrush(mobj_t* thing, int damage);
void dsda_WatchDamage(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage);
void dsda_WatchDeath(mobj_t* thing);
void dsda_WatchKill(player_t* player, mobj_t* target);
void dsda_WatchResurrection(mobj_t* target, mobj_t* raiser);
void dsda_WatchFailedSpawn(mobj_t* spawned);
void dsda_WatchMorph(mobj_t* morphed);
void dsda_WatchUnMorph(mobj_t* morphed);
void dsda_WatchSpawn(mobj_t* spawned);
void dsda_WatchIconSpawn(mobj_t* spawned);
void dsda_WatchCommand();
void dsda_WatchLedgeImpact(mobj_t* thing, int target_z);
void dsda_WatchBeforeLevelSetup();
void dsda_WatchAfterLevelSetup();
void dsda_WatchNewLevel();
void dsda_WatchLevelCompletion();
void dsda_WatchWeaponFire(WeaponType weapon);
void dsda_WatchSecret();
void dsda_WatchDeferredInitNew(int skill, int episode, int map);
void dsda_WatchNewGame();
void dsda_WatchLevelReload(int* reloaded);
void dsda_WatchLineActivation(line_t* line, mobj_t* mo);
void dsda_WatchPTickCompleted();

dboolean dsda_ILComplete();
dboolean dsda_MovieComplete();
void dsda_DecomposeILTime(dsda_level_time_t* level_time);
void dsda_DecomposeMovieTime(dsda_movie_time_t* total_time);
int dsda_MaxKillRequirement();
int* dsda_PlayerActivatedLines();

int dsda_TurboScale();
int dsda_StartInBuildMode();

dboolean dsda_FrozenMode();
void dsda_ToggleFrozenMode();

#ifdef __cplusplus
}
#endif
