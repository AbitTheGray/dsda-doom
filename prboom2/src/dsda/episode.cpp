// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Episode

#include <utility>

#include "doomstat.hpp"
#include "lprintf.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "dsda/mapinfo.hpp"

#include "episode.hpp"

dsda_episode_t* episodes;
size_t num_episodes;

static void dsda_DetermineEpisodeMap(dsda_episode_t* episode)
{
	if(!dsda_NameToMap(episode->map_lump, &episode->start_episode, &episode->start_map))
		Log::Fatal("Cannot evaluate start map for episode {}", episode->name ? episode->name : (episode->pic_name ? episode->pic_name : "UNKNOWN"));
}

void dsda_AddOriginalEpisodes()
{
	if(heretic)
	{
		dsda_AddEpisode("e1m1", "CITY OF THE DAMNED", nullptr, 'c', true);
		dsda_AddEpisode("e2m1", "HELL'S MAW", nullptr, 'h', true);
		dsda_AddEpisode("e3m1", "THE DOME OF D'SPARIL", nullptr, 't', true);

		if(gamemode == GameMode::Retail)
		{
			dsda_AddEpisode("e4m1", "THE OSSUARY", nullptr, 't', true);
			dsda_AddEpisode("e5m1", "THE STAGNANT DEMESNE", nullptr, 't', true);
		}
	}
	else if(hexen)
	{
		dsda_AddEpisode("map01", "FIGHTER", nullptr, 'f', true);
		dsda_AddEpisode("map01", "CLERIC", nullptr, 'c', true);
		dsda_AddEpisode("map01", "MAGE", nullptr, 'm', true);
	}
	else if(gamemode != GameMode::Commercial && gamemission != GameMission::TcChex)
	{
		dsda_AddEpisode("e1m1", nullptr, "M_EPI1", 'k', true);
		dsda_AddEpisode("e2m1", nullptr, "M_EPI2", 't', true);
		dsda_AddEpisode("e3m1", nullptr, "M_EPI3", 'i', true);

		if(gamemode == GameMode::Retail && (compatibility_level >= CompLevel::Ultdoom || W_PWADLumpNameExists2("E4M1")))
			dsda_AddEpisode("e4m1", nullptr, "M_EPI4", 't', true);
	}
}

void dsda_ClearEpisodes()
{
	int i;

	for(i = 0; i < num_episodes; ++i)
	{
		Z_Free(episodes[i].map_lump);
		Z_Free(episodes[i].name);
		Z_Free(episodes[i].pic_name);
	}

	Z_Free(episodes);
	episodes = nullptr;
	num_episodes = 0;
}

void dsda_AddEpisode(const char* map_lump, const char* name,
	const char* pic_name, char key, dboolean vanilla)
{
	++num_episodes;
	episodes = static_cast<dsda_episode_t*>(Z_Realloc(episodes, num_episodes * sizeof(*episodes)));

	episodes[num_episodes - 1].map_lump = map_lump ? Z_Strdup(map_lump) : nullptr;
	episodes[num_episodes - 1].name = name ? Z_Strdup(name) : nullptr;
	episodes[num_episodes - 1].pic_name = pic_name ? Z_Strdup(pic_name) : nullptr;
	episodes[num_episodes - 1].key = key;
	episodes[num_episodes - 1].vanilla = vanilla;

	dsda_DetermineEpisodeMap(&episodes[num_episodes - 1]);
}
