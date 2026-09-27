// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text File

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
		name = static_cast<char*>(Z_Calloc(name_length + 4, 1));
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

static char* dsda_TextFileTime()
{
	char* text_file_time;

	text_file_time = static_cast<char*>(Z_Malloc(16));

	if(dsda_IL())
		snprintf(
			text_file_time,
			16,
			"%d:%05.2f",
			dsda_last_leveltime / TICRATE / 60,
			(float)(dsda_last_leveltime % (60 * TICRATE)) / TICRATE
		);
	else
		snprintf(
			text_file_time,
			16,
			"%d:%02d",
			totalleveltimes / TICRATE / 60,
			(totalleveltimes / TICRATE) % 60
		);

	return text_file_time;
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

	fprintf(file, "Doom Speed Demo Archive\n");
	fprintf(file, "https://dsdarchive.com/\n");
	fprintf(file, "\n");
	if(iwad)
		fprintf(file, "Iwad:      %s\n", iwad);
	if(pwad)
		fprintf(file, "Pwad:      %s\n", pwad);

	if(dsda_IL())
		fprintf(file, "Map:       %s\n", dsda_MapLumpName(gameepisode, dsda_startmap));
	else
	{
		const char* movie;

		movie = dsda_Movie();

		if(movie)
			fprintf(file, "Movie:     %s\n", movie);
		else
		{
			fprintf(file, "Movie:     %s", dsda_MapLumpName(gameepisode, dsda_startmap));
			fprintf(file, " - %s\n", dsda_MapLumpName(gameepisode, dsda_last_gamemap));
		}
	}

	fprintf(file, "Skill:     %i\n", gameskill + 1);
	fprintf(file, "Category:  %s\n", to_string(dsda_DetectCategory()).c_str());
	fprintf(file, "Exe:       %s -complevel %i\n", PROJECT_STRING, compatibility_level);
	fprintf(file, "\n");

	name = dsda_TextFileTime();
	fprintf(file, "Time:      %s\n", name);
	Z_Free(name);

	dsda_player_name = dsda_StringConfig(ConfigId::PlayerName);

	fprintf(file, "\n");
	fprintf(file, "Author:    %s\n", dsda_player_name);
	fprintf(file, "\n");
	fprintf(file, "Comments:\n");

	fclose(file);
}
