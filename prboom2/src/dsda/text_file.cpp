// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text File

#include <format>
#include <print>
#include <string>
#include <utility>

#include "doomstat.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "e6y.hpp"

#include "dsda.hpp"
#include "dsda/analysis.hpp"
#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/playback.hpp"

#include "text_file.hpp"

extern int dsda_last_leveltime, dsda_last_gamemap, dsda_startmap;

static char* dsda_TextFileName()
{
	int name_length;
	char* name;
	char* playdemo;
	const char* playback_name;

	playback_name = dsda_PlaybackName();

	if(!playback_name)
		return nullptr;

	playdemo = Z_Strdup(playback_name);
	name_length = strlen(playdemo);

	if(name_length > 4 && !stricmp(playdemo + name_length - 4, ".lmp"))
	{
		name = Z_Strdup(playdemo);
		name[name_length - 4] = '\0';
	}
	else
	{
		// The name, ".txt" and the terminating zero.
		name = static_cast<char*>(Z_Calloc(name_length + 5, 1));
		strcat(name, playdemo);
	}

	strcat(name, ".txt");

	Z_Free(playdemo);

	return name;
}

static int dsda_IL()
{
	extern int dsda_startmap;

	return dsda_startmap == dsda_last_gamemap;
}

static const char* dsda_Movie()
{
	if(gamemode == GameMode::Commercial)
	{
		if(dsda_startmap == 1 && dsda_last_gamemap == 10)
			return "Episode 1";
		if(dsda_startmap == 11 && dsda_last_gamemap == 20)
			return "Episode 2";
		if(dsda_startmap == 21 && dsda_last_gamemap == 30)
			return "Episode 3";
		if(dsda_startmap == 1 && dsda_last_gamemap == 30)
			return "D2All";
	}
	else
	{
		if(dsda_startmap == 1 && dsda_last_gamemap == 8)
		{
			if(gameepisode == 1)
				return "Episode 1";
			if(gameepisode == 2)
				return "Episode 2";
			if(gameepisode == 3)
				return "Episode 3";
			if(gameepisode == 4)
				return "Episode 4";
		}
	}

	return nullptr;
}

static std::string dsda_TextFileTime()
{
	if(dsda_IL())
		return std::format(
			"{}:{:05.2f}",
			dsda_last_leveltime / TICRATE / 60,
			static_cast<float>(dsda_last_leveltime % (60 * TICRATE)) / TICRATE
		);

	return std::format(
		"{}:{:02}",
		totalleveltimes / TICRATE / 60,
		(totalleveltimes / TICRATE) % 60
	);
}

void dsda_ExportTextFile()
{
	dsda_arg_t* arg;
	char* name;
	const char* iwad = nullptr;
	const char* pwad = nullptr;
	const char* dsda_player_name;
	FILE* file;

	if(!dsda_Flag(ArgId::ExportTextFile))
		return;

	name = dsda_TextFileName();

	if(!name)
		return;

	file = M_OpenFile(name, "wb");
	Z_Free(name);

	if(!file)
		I_Error("Unable to export text file!");

	arg = dsda_Arg(ArgId::Iwad);
	if(arg->found)
		iwad = PathFindFileName(arg->value.v_string);

	arg = dsda_Arg(ArgId::File);
	if(arg->found)
		pwad = PathFindFileName(arg->value.v_string_array[0]);

	std::println(file, "Doom Speed Demo Archive");
	std::println(file, "https://dsdarchive.com/");
	std::println(file, "");
	if(iwad)
		std::println(file, "Iwad:      {}", iwad);
	if(pwad)
		std::println(file, "Pwad:      {}", pwad);

	if(dsda_IL())
		std::println(file, "Map:       {}", dsda_MapLumpName(gameepisode, dsda_startmap));
	else
	{
		const char* movie;

		movie = dsda_Movie();

		if(movie)
			std::println(file, "Movie:     {}", movie);
		else
		{
			std::print(file, "Movie:     {}", dsda_MapLumpName(gameepisode, dsda_startmap));
			std::println(file, " - {}", dsda_MapLumpName(gameepisode, dsda_last_gamemap));
		}
	}

	std::println(file, "Skill:     {}", gameskill + 1);
	std::println(file, "Category:  {}", dsda_DetectCategory());
	std::println(file, "Exe:       {} -complevel {}", PROJECT_STRING, std::to_underlying(compatibility_level));
	std::println(file, "");

	std::println(file, "Time:      {}", dsda_TextFileTime());

	dsda_player_name = dsda_StringConfig(ConfigId::PlayerName);

	std::println(file, "");
	std::println(file, "Author:    {}", dsda_player_name);
	std::println(file, "");
	std::println(file, "Comments:");

	fclose(file);
}
