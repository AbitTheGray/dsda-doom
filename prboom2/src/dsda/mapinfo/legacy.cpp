// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MapInfo Legacy

#include <utility>

#include "doomstat.hpp"
#include "g_game.hpp"
#include "m_misc.hpp"
#include "r_data.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "w_wad.hpp"

#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"

#include "legacy.hpp"

int dsda_LegacyNameToMap(int* found, const char* name, int* episode, int* map)
{
	char name_upper[9];
	int episode_from_name = -1;
	int map_from_name = -1;

	if(strlen(name) > 8)
	{
		*found = false;

		return true;
	}

	strncpy(name_upper, name, 8);
	name_upper[8] = 0;
	M_Strupr(name_upper);

	if(gamemode != GameMode::Commercial)
	{
		if(sscanf(name_upper, "E%dM%d", &episode_from_name, &map_from_name) != 2)
		{
			*found = false;

			return true;
		}
	}
	else
	{
		if(sscanf(name_upper, "MAP%d", &map_from_name) != 1)
		{
			*found = false;

			return true;
		}

		episode_from_name = 1;
	}

	*found = true;

	*episode = episode_from_name;
	*map = map_from_name;

	return true;
}

int dsda_LegacyFirstMap(int* episode, int* map)
{
	int i, j, lump;

	*episode = 1;
	*map = 1;

	if(gamemode == GameMode::Commercial)
	{
		for(i = 1; i < 33; i++)
		{
			lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(1, i));

			if(lump != LUMP_NOT_FOUND && lumpinfo[lump].source == WadSource::Pwad)
			{
				*map = i;

				return true;
			}
		}
	}
	else
		for(i = 1; i < 5; i++)
			for(j = 1; j < 10; j++)
			{
				lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(i, j));

				if(lump != LUMP_NOT_FOUND && lumpinfo[lump].source == WadSource::Pwad)
				{
					*episode = i;
					*map = j;

					return true;
				}
			}

	return true;
}

int dsda_LegacyNewGameMap(int* episode, int* map)
{
	return true;
}

int dsda_LegacyResolveWarp(int* args, int arg_count, int* episode, int* map)
{
	if(gamemode == GameMode::Commercial)
	{
		if(arg_count)
		{
			*episode = 1;
			*map = args[0];
		}
	}
	else if(arg_count)
	{
		*episode = args[0];

		if(arg_count > 1)
			*map = args[1];
		else
			*map = 1;
	}

	return true;
}

int dsda_LegacyNextMap(int* episode, int* map)
{
	static byte doom2_next[33] = {
		2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
		22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
		32, 16, 3
	};
	static byte doom_next[4][9] = {
		{12, 13, 19, 15, 16, 17, 18, 21, 14},
		{22, 23, 24, 25, 29, 27, 28, 31, 26},
		{32, 33, 34, 35, 36, 39, 38, 41, 37},
		{42, 49, 44, 45, 46, 47, 48, 11, 43}
	};
	static byte heretic_next[6][9] = {
		{12, 13, 14, 15, 16, 19, 18, 21, 17},
		{22, 23, 24, 29, 26, 27, 28, 31, 25},
		{32, 33, 34, 39, 36, 37, 38, 41, 35},
		{42, 43, 44, 49, 46, 47, 48, 51, 45},
		{52, 53, 59, 55, 56, 57, 58, 61, 54},
		{62, 63, 11, 11, 11, 11, 11, 11, 11}, // E6M4-E6M9 shouldn't be accessible
	};

	// next arrays are 0-based, unlike gameepisode and gamemap
	*episode = gameepisode - 1;
	*map = gamemap - 1;

	if(heretic)
	{
		int next;

		if(gamemode == GameMode::Shareware)
			heretic_next[0][7] = 11;

		if(gamemode == GameMode::Registered)
			heretic_next[2][7] = 11;

		next = heretic_next[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
		*episode = next / 10;
		*map = next % 10;
	}
	else if(gamemode == GameMode::Commercial)
	{
		// secret level
		doom2_next[14] = (haswolflevels ? 31 : 16);

		if(bfgedition && allow_incompatibility)
		{
			if(gamemission == GameMission::PackNerve)
			{
				doom2_next[3] = 9;
				doom2_next[7] = 1;
				doom2_next[8] = 5;
			}
			else
				doom2_next[1] = 33;
		}

		*episode = 1;
		*map = doom2_next[BETWEEN(0, 32, *map)];
	}
	else
	{
		int next;

		// shareware doom has only episode 1
		doom_next[0][7] = (gamemode == GameMode::Shareware ? 11 : 21);

		doom_next[2][7] = // the fourth episode for pre-ultimate complevels is not allowed
			((gamemode == GameMode::Registered) || (compatibility_level < CompLevel::Ultdoom) ? 11 : 41);

		next = doom_next[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
		*episode = next / 10;
		*map = next % 10;
	}

	return true;
}

int dsda_LegacyPrevMap(int* episode, int* map)
{
	static byte doom2_prev[33] = {
		1, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 32, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		15, 31, 2
	};
	static byte doom_prev[4][9] = {
		{11, 11, 12, 19, 14, 15, 16, 17, 13},
		{18, 21, 22, 23, 24, 29, 26, 27, 25},
		{28, 31, 32, 33, 34, 35, 39, 37, 36},
		{38, 41, 49, 43, 44, 45, 46, 47, 42}
	};
	static byte heretic_prev[6][9] = {
		{11, 11, 12, 13, 14, 15, 19, 17, 16},
		{18, 21, 22, 23, 29, 25, 26, 27, 24},
		{28, 31, 32, 33, 39, 35, 36, 37, 34},
		{38, 41, 42, 43, 49, 45, 46, 47, 44},
		{48, 51, 52, 59, 54, 55, 56, 57, 53},
		{58, 61, 62, 63, 63, 63, 63, 63, 63}, // E6M4-E6M9 shouldn't be accessible
	};

	// next arrays are 0-based, unlike gameepisode and gamemap
	*episode = gameepisode - 1;
	*map = gamemap - 1;

	if(heretic)
	{
		int prev;

		prev = heretic_prev[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
		*episode = prev / 10;
		*map = prev % 10;
	}
	else if(gamemode == GameMode::Commercial)
	{
		// secret level
		doom2_prev[15] = (haswolflevels ? 32 : 15);

		if(bfgedition && allow_incompatibility)
		{
			if(gamemission == GameMission::PackNerve)
			{
				doom2_prev[4] = 9;
				doom2_prev[8] = 4;
			}
			else
				doom2_prev[2] = 33;
		}

		*episode = 1;
		*map = doom2_prev[BETWEEN(0, 32, *map)];
	}
	else
	{
		int prev;

		prev = doom_prev[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
		*episode = prev / 10;
		*map = prev % 10;
	}

	return true;
}

int dsda_LegacyShowNextLocBehaviour(int* behaviour)
{
	if(
		gamemode != GameMode::Commercial &&
		(gamemap == 8 || (gamemission == GameMission::TcChex && gamemap == 5))
	)
		*behaviour = WI_SHOW_NEXT_DONE;
	else
		*behaviour = WI_SHOW_NEXT_LOC;

	if(dsda_FinaleShortcut())
		*behaviour = WI_SHOW_NEXT_DONE;

	return true;
}

int dsda_LegacySkipDrawShowNextLoc(int* skip)
{
	*skip = false;

	if(dsda_FinaleShortcut())
		*skip = true;

	return true;
}

void dsda_LegacyUpdateMapInfo()
{
	// nothing to do right now
}

void dsda_LegacyUpdateLastMapInfo()
{
	// nothing to do right now
}

void dsda_LegacyUpdateNextMapInfo()
{
	// nothing to do right now
}

static int dsda_CannotCLEV(int episode, int map)
{
	char* next;

	if(
		episode < 1 ||
		map < 0 ||
		((gamemode == GameMode::Retail || gamemode == GameMode::Registered) && (episode > 9 || map > 9)) ||
		(gamemode == GameMode::Shareware && (episode > 1 || map > 9)) ||
		(gamemode == GameMode::Commercial && (episode > 1 || map > 99)) ||
		(gamemission == GameMission::PackNerve && map > 9)
	)
		return true;

	// Catch invalid maps
	next = VANILLA_MAP_LUMP_NAME(episode, map);
	if(!W_LumpNameExists(next))
	{
		Message::Add("IDCLEV target not found: {}", next);
		return true;
	}

	return false;
}

int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map)
{
	if(dsda_CannotCLEV(*episode, *map))
		*clev = false;
	else
	{
		if(gamemission == GameMission::TcChex)
			*episode = 1;

		*clev = true;
	}

	return true;
}

int dsda_LegacyResolveINIT(int* init)
{
	*init = false;

	return true;
}

int dsda_LegacyMusicIndexToLumpNum(int* lump, int music_index)
{
	char name[9];
	const char* format;

	format = raven ? "%s" : "d_%s";

	snprintf(name, sizeof(name), format, S_music[music_index].name);

	*lump = W_GetNumForName(name);

	return true;
}

static inline int WRAP(int i, int w)
{
	while(i < 0)
		i += w;

	return i % w;
}

int dsda_LegacyMapMusic(int* music_index, int* music_lump, int episode, int map)
{
	*music_lump = -1;

	if(idmusnum != -1)
		*music_index = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
	else
	{
		if(gamemode == GameMode::Commercial)
			*music_index = std::to_underlying(MusicId::Runnin) + WRAP(map - 1, std::to_underlying(MusicId::DoomMusinfo) - std::to_underlying(MusicId::Runnin));
		else
		{
			static const int spmus[] = {
				std::to_underlying(MusicId::E3m4),
				std::to_underlying(MusicId::E3m2),
				std::to_underlying(MusicId::E3m3),
				std::to_underlying(MusicId::E1m5),
				std::to_underlying(MusicId::E2m7),
				std::to_underlying(MusicId::E2m4),
				std::to_underlying(MusicId::E2m6),
				std::to_underlying(MusicId::E2m5),
				std::to_underlying(MusicId::E1m9)
			};

			if(heretic)
				*music_index = std::to_underlying(MusicId::HereticE1m1) +
					WRAP((episode - 1) * 9 + map - 1,
						std::to_underlying(MusicId::HereticCount) - std::to_underlying(MusicId::HereticE1m1));
			else if(episode < 4)
				*music_index = std::to_underlying(MusicId::E1m1) +
					WRAP((episode - 1) * 9 + map - 1, std::to_underlying(MusicId::Runnin) - std::to_underlying(MusicId::E1m1));
			else
				*music_index = spmus[WRAP(map - 1, 9)];
		}
	}

	return true;
}

int dsda_LegacyIntermissionMusic(int* music_index, int* music_lump)
{
	*music_lump = -1;

	if(gamemode == GameMode::Commercial)
		*music_index = std::to_underlying(MusicId::Dm2int);
	else
		*music_index = std::to_underlying(MusicId::Inter);

	return true;
}

int dsda_LegacyInterMusic(int* music_index, int* music_lump)
{
	*music_lump = -1;

	switch(gamemode)
	{
		case GameMode::Shareware:
		case GameMode::Registered:
		case GameMode::Retail:
			*music_index = std::to_underlying(MusicId::Victor);
			break;
		default:
			*music_index = std::to_underlying(MusicId::ReadM);
			break;
	}

	return true;
}

int dsda_LegacyStartFinale()
{
	return true;
}

int dsda_LegacyFTicker()
{
	return true;
}

void dsda_LegacyFDrawer()
{
	return;
}

int dsda_LegacyBossAction(mobj_t* mo)
{
	return false;
}

int dsda_LegacyMapLumpName(const char** name, int episode, int map)
{
	*name = VANILLA_MAP_LUMP_NAME(episode, map);

	return true;
}

int dsda_LegacyMapAuthor(const char** author)
{
	*author = nullptr;

	return true;
}

int dsda_LegacyHUTitle(dsda_string_t* str)
{
	extern char** mapnames[];
	extern char** mapnames2[];
	extern char** mapnamesp[];
	extern char** mapnamest[];
	extern const char* LevelNames[];

	dsda_InitString(str, nullptr);

	if(gamestate == GameState::Level && gamemap > 0 && gameepisode > 0)
	{
		if(heretic)
		{
			if(gameepisode < 6 && gamemap < 10)
				dsda_StringCat(str, LevelNames[(gameepisode - 1) * 9 + gamemap - 1]);
		}
		else
		{
			switch(gamemode)
			{
				case GameMode::Shareware:
				case GameMode::Registered:
				case GameMode::Retail:
					// Chex.exe always uses the episode 1 level title
					// eg. E2M1 gives the title for E1M1
					if(gamemission == GameMission::TcChex && gamemap < 10)
						dsda_StringCat(str, *mapnames[gamemap - 1]);
					else if(gameepisode < 6 && gamemap < 10)
						dsda_StringCat(str, *mapnames[(gameepisode - 1) * 9 + gamemap - 1]);
					break;

				default: // Ty 08/27/98 - modified to check mission for TNT/Plutonia
					if(gamemission == GameMission::PackTnt && gamemap < 33)
						dsda_StringCat(str, *mapnamest[gamemap - 1]);
					else if(gamemission == GameMission::PackPlut && gamemap < 33)
						dsda_StringCat(str, *mapnamesp[gamemap - 1]);
					else if(gamemap < 34)
						dsda_StringCat(str, *mapnames2[gamemap - 1]);
					break;
			}
		}
	}

	if(!str->string)
		dsda_StringCat(str, VANILLA_MAP_LUMP_NAME(gameepisode, gamemap));

	return true;
}

int dsda_LegacySkyTexture(int* sky)
{
	if(map_format.doublesky)
		*sky = Sky1Texture;
	else if(heretic)
	{
		static const char* sky_lump_names[5] = {
			"SKY1", "SKY2", "SKY3", "SKY1", "SKY3"
		};

		if(gameepisode < 6)
			*sky = R_TextureNumForName(sky_lump_names[gameepisode - 1]);
		else
			*sky = R_TextureNumForName("SKY1");
	}
	else if(gamemode == GameMode::Commercial)
	{
		*sky = R_TextureNumForName("SKY3");
		if(gamemap < 12)
			*sky = R_TextureNumForName("SKY1");
		else if(gamemap < 21)
			*sky = R_TextureNumForName("SKY2");
	}
	else
	{
		switch(gameepisode)
		{
			case 1:
				*sky = R_TextureNumForName("SKY1");
				break;
			case 2:
				*sky = R_TextureNumForName("SKY2");
				break;
			case 3:
				*sky = R_TextureNumForName("SKY3");
				break;
			case 4: // Special Edition sky
				*sky = R_TextureNumForName("SKY4");
				break;
			default:
				*sky = R_TextureNumForName("SKY1");
				break;
		}
	}

	return true;
}

int dsda_LegacyPrepareInitNew()
{
	return true;
}

extern int deh_pars;
extern "C" void dsda_LegacyParTime(int* partime, dboolean* modified)
{

	if(gamemode == GameMode::Commercial)
	{
		if(gamemap >= 1 && gamemap <= 34)
		{
			*partime = TICRATE * cpars[gamemap - 1];
			*modified = deh_pars;
		}
	}
	else
	{
		if(gameepisode >= 1 &&
			(gameepisode <= 3 || (allow_incompatibility && gameepisode <= 4)) &&
			gamemap >= 1 && gamemap <= 9)
		{
			*partime = TICRATE * pars[gameepisode][gamemap];
			*modified = deh_pars;
		}
		// Doom episode 4 doesn't have a par time, so this
		// overflows into the cpars array.
		else if(gameepisode == 4 && gamemap >= 1 && gamemap <= 9)
		{
			*partime = TICRATE * cpars[gamemap - 1];
			*modified = deh_pars;
		}
	}
}

int dsda_LegacyPrepareIntermission(int* result)
{
	if(gamemode != GameMode::Commercial)
		if(gamemap == 9)
		{
			int i;

			for(i = 0; i < g_maxplayers; i++)
				players[i].didsecret = true;
		}

	wminfo.didsecret = players[consoleplayer].didsecret;

	// wminfo.next is 0 biased, unlike gamemap
	if(gamemode == GameMode::Commercial)
	{
		if(secretexit)
			switch(gamemap)
			{
				case 15:
					wminfo.next = 30;
					break;
				case 31:
					wminfo.next = 31;
					break;
				case 2:
					if(bfgedition && allow_incompatibility)
						wminfo.next = 32;
					break;
				case 4:
					if(gamemission == GameMission::PackNerve && allow_incompatibility)
						wminfo.next = 8;
					break;
			}
		else
			switch(gamemap)
			{
				case 31:
				case 32:
					wminfo.next = 15;
					break;
				case 33:
					if(bfgedition && allow_incompatibility)
					{
						wminfo.next = 2;
						break;
					}
				// fallthrough
				default:
					wminfo.next = gamemap;
			}

		if(gamemission == GameMission::PackNerve && allow_incompatibility && gamemap == 9)
			wminfo.next = 4;
	}
	else
	{
		if(secretexit)
			wminfo.next = 8; // go to secret level
		else if(gamemap == 9)
		{
			// returning from secret level
			if(heretic)
			{
				static int after_secret[5] = {6, 4, 4, 4, 3};
				wminfo.next = after_secret[gameepisode - 1];
			}
			else
				switch(gameepisode)
				{
					case 1:
						wminfo.next = 3;
						break;
					case 2:
						wminfo.next = 5;
						break;
					case 3:
						wminfo.next = 6;
						break;
					case 4:
						wminfo.next = 2;
						break;
				}
		}
		else
			wminfo.next = gamemap; // go to next level
	}

	dsda_LegacyParTime(&wminfo.partime, &wminfo.modified_partime);

	if(map_format.zdoom)
		if(leave_data.map > 0)
			wminfo.next = leave_data.map - 1;

	*result = 0;

	return true;
}

int dsda_LegacyPrepareFinale(int* result)
{
	*result = 0;

	if(gamemode == GameMode::Commercial && gamemission != GameMission::PackNerve)
	{
		switch(gamemap)
		{
			case 15:
			case 31:
				if(!secretexit)
					break;
			// fallthrough
			case 6:
			case 11:
			case 20:
			case 30:
				*result = WD_START_FINALE;
				break;
		}
	}
	else if(gamemission == GameMission::PackNerve && allow_incompatibility && gamemap == 8)
		*result = WD_START_FINALE;
	else if(gamemap == 8)
		*result = WD_VICTORY;
	else if(gamemap == 5 && gamemission == GameMission::TcChex)
		*result = WD_VICTORY;

	if(dsda_FinaleShortcut())
		*result = WD_START_FINALE;

	return true;
}

void dsda_LegacyLoadMapInfo()
{
	return;
}

int dsda_LegacyExitPic(const char** exit_pic)
{
	*exit_pic = nullptr;

	return true;
}

int dsda_LegacyEnterPic(const char** enter_pic)
{
	*enter_pic = nullptr;

	return true;
}

int dsda_LegacyBorderTexture(const char** border_texture)
{
	*border_texture = heretic ? "FLOOR30" : gamemode == GameMode::Commercial ? "GRNROCK" : "FLOOR7_2";

	return true;
}

int dsda_LegacyPrepareEntering()
{
	extern const char* el_levelname;
	extern const char* el_levelpic;
	extern const char* el_author;

	el_levelname = nullptr;
	el_levelpic = nullptr;
	el_author = nullptr;

	return true;
}

int dsda_LegacyPrepareFinished()
{
	extern const char* lf_levelname;
	extern const char* lf_levelpic;
	extern const char* lf_author;

	lf_levelname = nullptr;
	lf_levelpic = nullptr;
	lf_author = nullptr;

	return true;
}

int dsda_LegacyMapLightning(int* lightning)
{
	*lightning = false;

	return true;
}

int dsda_LegacyApplyFadeTable()
{
	return true;
}

int dsda_LegacyMapCluster(int* cluster, int map)
{
	*cluster = -1;

	return true;
}

int dsda_LegacySky1Texture(short* texture)
{
	*texture = -1;

	return true;
}

int dsda_LegacySky2Texture(short* texture)
{
	*texture = -1;

	return true;
}

int dsda_LegacyGravity(fixed_t* gravity)
{
	*gravity = FRACUNIT;

	return true;
}

extern "C" dboolean dsda_AllowJumping();
int dsda_LegacyAirControl(fixed_t* air_control)
{

	*air_control = dsda_AllowJumping() ? (FRACUNIT >> 8) : 0;

	return true;
}

int dsda_LegacyInitSky()
{
	return true;
}

int dsda_LegacyMapColorMap(int* colormap)
{
	*colormap = 0;

	return true;
}
