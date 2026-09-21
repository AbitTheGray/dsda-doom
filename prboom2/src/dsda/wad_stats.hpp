// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Wad Stats

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	char lump[9];
	int episode;
	int map;
	int best_skill;
	int best_time;
	int best_max_time;
	int best_nm_time;
	int total_exits;
	int total_kills;
	int best_kills;
	int best_items;
	int best_secrets;
	int max_kills;
	int max_items;
	int max_secrets;
} map_stats_t;

typedef struct
{
	int total_kills;
	map_stats_t* maps;
	int maps_size;
	int map_count;
} wad_stats_t;

extern wad_stats_t wad_stats;

void dsda_WadStatsEnterMap();
void dsda_WadStatsExitMap(int missed_monsters);
void dsda_WadStatsKill();
void dsda_SaveWadStats();
void dsda_InitWadStats();

#ifdef __cplusplus
}
#endif
