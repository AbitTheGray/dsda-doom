// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Episode

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

typedef struct
{
	char* map_lump;
	char* name;
	char* pic_name;
	char key;
	dboolean vanilla;
	int start_map;
	int start_episode;
} dsda_episode_t;

extern dsda_episode_t* episodes;
extern size_t num_episodes;

void dsda_AddOriginalEpisodes();
void dsda_ClearEpisodes();
void dsda_AddEpisode(const char* map_lump, const char* name,
	const char* pic_name, char key, dboolean vanilla);

#ifdef __cplusplus
}
#endif
