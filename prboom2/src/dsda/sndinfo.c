// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA SndInfo

#include <stddef.h>

#include "m_misc.h"
#include "s_sound.h"
#include "sc_man.h"
#include "sounds.h"

#include "dsda/ambient.h"
#include "dsda/map_format.h"

#include "sndinfo.h"

static char song_lump[99][10];

static void ReadMapSongLumpName(int map, char* lump_name)
{
	if(map < 1 || map > 98)
		return;

	M_StringCopy(song_lump[map], lump_name, sizeof(song_lump[map]));
}

const char* dsda_SndInfoMapSongLumpName(int map)
{
	if(map < 1 || map > 98)
		return song_lump[0];

	return song_lump[map];
}

void dsda_LoadSndInfo(void)
{
	int i;

	if(!hexen)
	{
		dsda_LoadAmbientSndInfo();
		return;
	}

	SC_OpenLump("sndinfo");

	while(SC_GetString())
	{
		if(*sc_String == '$')
		{
			if(!strcasecmp(sc_String, "$ARCHIVEPATH"))
				SC_MustGetString();
			else if(!strcasecmp(sc_String, "$MAP"))
			{
				SC_MustGetNumber();
				SC_MustGetString();

				if(sc_Number)
					ReadMapSongLumpName(sc_Number, sc_String);
			}

			continue;
		}
		else
		{
			for(i = 0; i < num_sfx; i++)
			{
				if(!strcmp(S_sfx[i].tagname, sc_String))
				{
					SC_MustGetString();

					if(*sc_String != '?')
						S_sfx[i].name = Z_Strdup(sc_String);
					else
						S_sfx[i].name = Z_Strdup("default");

					break;
				}
			}
			if(i == num_sfx)
				SC_MustGetString();
		}
	}

	SC_Close();

	for(i = 0; i < num_sfx; i++)
		if(!strcmp(S_sfx[i].name, ""))
			S_sfx[i].name = Z_Strdup("default");
}
