// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Save

#include <utility>

#include <stdlib.h>

#include "doomstat.hpp"
#include "g_game.hpp"
#include "lprintf.hpp"
#include "z_zone.hpp"
#include "p_saveg.hpp"
#include "p_map.hpp"
#include "s_sound.hpp"

#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/data_organizer.hpp"
#include "dsda/excmd.hpp"
#include "dsda/features.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/music.hpp"
#include "dsda/options.hpp"
#include "dsda/settings.hpp"

#include "save.hpp"

static char* dsda_base_save_dir;
static char* dsda_wad_save_dir;

extern int dsda_max_kill_requirement;
extern int player_damage_last_tic;

static void dsda_ArchiveInternal()
{
	P_SAVE_X(dsda_max_kill_requirement);
	P_SAVE_X(player_damage_last_tic);

	for(int f = 0; f < FEATURE_SLOTS; f++)
	{
		P_SAVE_X(dsda_UsedFeatures()[f]);
	}
}

static void dsda_UnArchiveInternal()
{
	byte features[FEATURE_SLOTS];

	P_LOAD_X(dsda_max_kill_requirement);
	P_LOAD_X(player_damage_last_tic);

	P_LOAD_X(features);
	dsda_MergeFeatures(features);
}

static void dsda_ArchiveContext()
{
	int i;
	int boom_logictic_value;
	int true_logictic_value;

	P_SAVE_BYTE(std::to_underlying(compatibility_level));
	P_SAVE_BYTE(gameskill);
	P_SAVE_BYTE(gameepisode);
	P_SAVE_BYTE(gamemap);

	for(i = 0; i < g_maxplayers; ++i)
		P_SAVE_BYTE(playeringame[i]);

	for(; i < FUTURE_MAXPLAYERS; ++i)
		P_SAVE_BYTE(0);

	P_SAVE_BYTE(idmusnum);

	dsda_ArchiveMusic();

	CheckSaveGame(dsda_GameOptionSize());
	save_p = G_WriteOptions(save_p);

	P_SAVE_X(leave_data);

	P_SAVE_X(map_colormap);

	P_SAVE_X(leveltime);
	P_SAVE_X(totalleveltimes);
	P_SAVE_X(levels_completed);

	boom_logictic_value = boom_logictic;
	P_SAVE_X(boom_logictic_value);

	true_logictic_value = true_logictic;
	P_SAVE_X(true_logictic_value);
}

static void dsda_UnArchiveContext()
{
	int i;
	int epi, map;
	int boom_logictic_value;
	int true_logictic_value;

	{ byte level; P_LOAD_BYTE(level); compatibility_level = static_cast<CompLevel>(level); }
	P_LOAD_BYTE(gameskill);

	P_LOAD_BYTE(epi);
	P_LOAD_BYTE(map);
	dsda_UpdateGameMap(epi, map);

	for(i = 0; i < g_maxplayers; ++i)
		P_LOAD_BYTE(playeringame[i]);
	save_p += FUTURE_MAXPLAYERS - g_maxplayers;

	P_LOAD_BYTE(idmusnum);
	if(idmusnum == 255)
		idmusnum = -1;

	dsda_UnArchiveMusic();

	save_p += (G_ReadOptions(save_p) - save_p);

	P_LOAD_X(leave_data);

	G_InitNew(gameskill, gameepisode, gamemap, false);

	P_LOAD_X(map_colormap);

	P_LOAD_X(leveltime);
	P_LOAD_X(totalleveltimes);
	P_LOAD_X(levels_completed);

	P_LOAD_X(boom_logictic_value);
	boom_basetic = gametic - boom_logictic_value;

	P_LOAD_X(true_logictic_value);
	true_basetic = gametic - true_logictic_value;
}

static int saved_pistolstart;
static int saved_respawnparm;
static int saved_fastparm;
static int saved_nomonsters;
static int saved_coop_spawns;

static void dsda_GetGameModifiers()
{
	saved_pistolstart = pistolstart;
	saved_respawnparm = respawnparm;
	saved_fastparm = fastparm;
	saved_nomonsters = nomonsters;
	saved_coop_spawns = coop_spawns;
}

static void dsda_SetGameModifiers()
{
	// [AR] when playing / recording demos, do NOT touch configs
	if(!allow_incompatibility)
		return;

	// If "Always Pistol Start" is enabled, skip resetting "Pistol Start"
	if(!dsda_IntConfig(ConfigId::AlwaysPistolStart))
		dsda_UpdateIntConfig(ConfigId::PistolStart, saved_pistolstart,true);

	dsda_UpdateIntConfig(ConfigId::RespawnMonsters, saved_respawnparm,true);
	dsda_UpdateIntConfig(ConfigId::FastMonsters, saved_fastparm,true);
	dsda_UpdateIntConfig(ConfigId::NoMonsters, saved_nomonsters,true);
	dsda_UpdateIntConfig(ConfigId::CoopSpawns, saved_coop_spawns,true);
}

static void dsda_ArchiveGameModifiers()
{
	dsda_GetGameModifiers();

	P_SAVE_X(saved_pistolstart);
	P_SAVE_X(saved_respawnparm);
	P_SAVE_X(saved_fastparm);
	P_SAVE_X(saved_nomonsters);
	P_SAVE_X(saved_coop_spawns);
}

static void dsda_UnArchiveGameModifiers()
{
	P_LOAD_X(saved_pistolstart);
	P_LOAD_X(saved_respawnparm);
	P_LOAD_X(saved_fastparm);
	P_LOAD_X(saved_nomonsters);
	P_LOAD_X(saved_coop_spawns);

	dsda_SetGameModifiers();
}

void dsda_ArchiveAll()
{
	dsda_ArchiveContext();

	P_ArchiveACS();
	P_ArchivePlayers();
	P_ThinkerToIndex();
	P_ArchiveWorld();
	P_ArchivePolyobjs();
	P_ArchiveThinkers();
	P_ArchiveScripts();
	P_ArchiveSounds();
	P_ArchiveAmbientSound();
	P_ArchiveMisc();
	P_IndexToThinker();
	P_ArchiveRNG();
	P_ArchiveMap();

	dsda_ArchiveGameModifiers();
	dsda_ArchiveInternal();
}

void dsda_UnArchiveAll()
{
	dsda_UnArchiveContext();

	P_MapStart();
	P_UnArchiveACS();
	P_UnArchivePlayers();
	P_UnArchiveWorld();
	P_UnArchivePolyobjs();
	P_UnArchiveThinkers();
	P_UnArchiveScripts();
	P_UnArchiveSounds();
	P_UnArchiveAmbientSound();
	P_UnArchiveMisc();
	P_UnArchiveRNG();
	P_UnArchiveMap();
	P_MapEnd();

	dsda_UnArchiveGameModifiers();
	dsda_UnArchiveInternal();
}

void dsda_InitSaveDir()
{
	dsda_base_save_dir = dsda_DetectDirectory("DOOMSAVEDIR", ArgId::Save);
}

char* dsda_SaveDir()
{
	dsda_arg_t* arg = dsda_Arg(ArgId::Save);

	if(!arg->found)
	{
		if(dsda_IntConfig(ConfigId::OrganizedSaves))
		{
			if(!dsda_wad_save_dir)
				dsda_wad_save_dir = dsda_DataDir();

			return dsda_wad_save_dir;
		}
	}

	return dsda_base_save_dir;
}

extern const char* savegamename;

char* dsda_SaveGameName(int slot, dboolean via_cmd)
{
	int length;
	char* name;
	const char* save_dir;
	const char* save_type;

	if(slot > 9999 || slot < 0)
		I_Error("dsda_SaveGameName: bad save slot %d", slot);

	save_dir = dsda_SaveDir();

	save_type = (via_cmd && demoplayback) ? "demosav" : savegamename;

	length = strlen(save_type) + strlen(save_dir) + 10; // "/" + "9999.dsg\0"

	name = static_cast<char*>(Z_Malloc(length));

	snprintf(name, length, "%s/%s%d.dsg", save_dir, save_type, slot);

	return name;
}

static int* demo_save_slots;
static int allocated_save_slot_count;
static int demo_save_slot_count;

void dsda_ResetDemoSaveSlots()
{
	demo_save_slot_count = 0;
}

static void dsda_MarkSaveSlotUsed(int slot)
{
	int i;

	if(!demorecording) return;

	for(i = 0; i < demo_save_slot_count; ++i)
		if(demo_save_slots[i] == slot)
			return;

	++demo_save_slot_count;
	if(demo_save_slot_count > allocated_save_slot_count)
	{
		allocated_save_slot_count = allocated_save_slot_count ? allocated_save_slot_count * 2 : 8;
		demo_save_slots = (int*)Z_Realloc(demo_save_slots, allocated_save_slot_count * sizeof(*demo_save_slots));
	}

	demo_save_slots[demo_save_slot_count - 1] = slot;
}

int dsda_AllowAnyMenuSave()
{
	return !dsda_StrictMode() || dsda_AllowCasualExCmdFeatures();
}

int dsda_AllowMenuLoad(int slot)
{
	int i;

	if(!demorecording) return true;
	if(!dsda_AllowCasualExCmdFeatures()) return false;

	for(i = 0; i < demo_save_slot_count; ++i)
		if(demo_save_slots[i] == slot)
			return true;

	return false;
}

int dsda_AllowAnyMenuLoad()
{
	return !demorecording || dsda_AllowCasualExCmdFeatures();
}

static int last_save_file_slot = -1;

void dsda_SetLastLoadSlot(int slot)
{
	last_save_file_slot = slot;
}

void dsda_SetLastSaveSlot(int slot)
{
	dsda_MarkSaveSlotUsed(slot);
	last_save_file_slot = slot;
}

int dsda_LastSaveSlot()
{
	return last_save_file_slot;
}

void dsda_ResetLastSaveSlot()
{
	last_save_file_slot = -1;
}

extern "C" void M_AutoSave();
void dsda_UpdateAutoSaves()
{
	static int automap = -1;
	static int autoepisode = -1;


	if(!dsda_IntConfig(ConfigId::AutoSave) ||
		gamestate != GameState::Level ||
		gameaction != GameAction::Nothing ||
		demoplayback ||
		demorecording)
		return;

	if(automap != gamemap || autoepisode != gameepisode)
	{
		automap = gamemap;
		autoepisode = gameepisode;

		if(!leveltime)
			M_AutoSave();
	}
}
