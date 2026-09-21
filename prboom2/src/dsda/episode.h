// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Episode

#ifndef __DSDA_EPISODE__
#define __DSDA_EPISODE__

#include "doomtype.h"

typedef struct {
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

void dsda_AddOriginalEpisodes(void);
void dsda_ClearEpisodes(void);
void dsda_AddEpisode(const char* map_lump, const char* name,
                     const char* pic_name, char key, dboolean vanilla);

#endif
