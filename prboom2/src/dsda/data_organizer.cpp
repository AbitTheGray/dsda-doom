// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Data Organizer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "w_wad.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"
#include "e6y.hpp"

#include "dsda/args.hpp"
#include "dsda/utility.hpp"

#include "data_organizer.hpp"

#define DATA_DIR_LIMIT 9
static const char* dsda_data_root = "dsda_doom_data";
static char* dsda_data_dir_strings[DATA_DIR_LIMIT];
static char* dsda_base_data_dir;
static char* dsda_wad_data_dir;

char* dsda_DetectDirectory(const char* env_key, ArgId arg_id)
{
	dsda_arg_t* arg;
	char* result = nullptr;
	const char* default_directory;

	default_directory = M_getenv(env_key);

	if(!default_directory)
		default_directory = I_ConfigDir();

	arg = dsda_Arg(static_cast<ArgId>(arg_id));
	if(arg->found)
	{
		if(M_IsDir(arg->value.v_string))
		{
			if(result) Z_Free(result);
			result = Z_Strdup(arg->value.v_string);
		}
		else
			lprintf(OutputLevels::Error, "Error: path %s does not exist. Using %s\n",
				arg->value.v_string, default_directory);
	}

	if(!result)
		result = Z_Strdup(default_directory);

	return result;
}

void dsda_InitDataDir()
{
	char* parent_directory;
	dsda_string_t str;

	parent_directory = dsda_DetectDirectory("DOOMDATADIR", ArgId::Data);

	dsda_StringPrintF(&str, "%s/%s", parent_directory, dsda_data_root);

	dsda_base_data_dir = str.string;
	M_MakeDir(dsda_base_data_dir, false);

	Z_Free(parent_directory);
}

static void dsda_InitWadDataDir()
{
	int i;
	const int iwad_index = 0;
	int pwad_index = 1;
	dsda_string_t str;

	for(i = 0; i < numwadfiles; ++i)
	{
		const char* start;
		char* result;
		int length;

		start = PathFindFileName(wadfiles[i].name);

		length = strlen(start) - 4;

		if(length > 0 && !strcasecmp(start + length, ".wad"))
		{
			int dir_index;

			if(wadfiles[i].src == WadSource::Iwad)
				dir_index = iwad_index;
			else if(wadfiles[i].src == WadSource::Pwad)
				dir_index = pwad_index;
			else
				dir_index = -1;

			if(dir_index >= 0 && dir_index < DATA_DIR_LIMIT)
			{
				dsda_data_dir_strings[dir_index] = static_cast<char *>(Z_Malloc(length + 1));
				strncpy(dsda_data_dir_strings[dir_index], start, length);
				dsda_data_dir_strings[dir_index][length] = '\0';

				for(result = dsda_data_dir_strings[dir_index]; *result; ++result)
					*result = tolower(*result);

				if(dir_index == pwad_index)
					pwad_index++;
			}
		}
	}

	dsda_InitString(&str, dsda_base_data_dir);

	for(i = 0; i < DATA_DIR_LIMIT; ++i)
		if(dsda_data_dir_strings[i])
		{
			dsda_StringCatF(&str, "/%s", dsda_data_dir_strings[i]);
			M_MakeDir(str.string, false);
		}

	dsda_wad_data_dir = str.string;

	lprintf(OutputLevels::Info, "Using data file directory: %s\n", dsda_wad_data_dir);
}

char* dsda_DataDir()
{
	if(!dsda_wad_data_dir)
		dsda_InitWadDataDir();

	return dsda_wad_data_dir;
}

const char* dsda_DataRoot()
{
	return dsda_base_data_dir;
}
