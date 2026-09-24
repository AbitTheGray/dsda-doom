// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  Support MUSINFO lump (dynamic music changing)
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include "doomstat.hpp"
#include "doomtype.hpp"
#include "d_main.hpp"
#include "p_mobj.hpp"
#include "m_misc.hpp"
#include "sounds.hpp"
#include "s_sound.hpp"
#include "i_sound.hpp"
#include "r_defs.hpp"
#include "sc_man.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"

#include "s_advsound.hpp"

#define TIDNUM(x) ((int)(x->iden_nums & 0xFFFF))		// thing identifier

musinfo_t musinfo;

//
// S_ParseMusInfo
// Parses MUSINFO lump.
//
void S_ParseMusInfo(const char* mapid)
{
	if(gamemode != GameMode::Shareware && W_LumpNameExists("MUSINFO"))
	{
		int num, lumpnum;
		int inMap = false;
		int load_muslump = -1;

		/* don't restart music that is already playing */
		if(mus_playing &&
			mus_playing->lumpnum == S_music[mus_musinfo].lumpnum)
		{
			load_muslump = S_music[mus_musinfo].lumpnum;
		}

		memset(&musinfo, 0, sizeof(musinfo));
		musinfo.items[0] = -1;
		musinfo.current_item = load_muslump;
		S_music[mus_musinfo].lumpnum = load_muslump;

		SC_OpenLump("MUSINFO");

		while(SC_GetString())
		{
			if(inMap || SC_Compare(mapid))
			{
				if(!inMap)
				{
					SC_GetString();
					inMap = true;
				}

				if(sc_String[0] == 'E' || sc_String[0] == 'e' ||
					sc_String[0] == 'M' || sc_String[0] == 'm')
				{
					break;
				}

				// Check number in range
				if(M_StrToInt(sc_String, &num) && num >= 0 && num < MAX_MUS_ENTRIES)
				{
					if(SC_GetString())
					{
						lumpnum = W_CheckNumForName(sc_String);

						if(lumpnum != LUMP_NOT_FOUND)
						{
							musinfo.items[num] = lumpnum;
						}
						else
						{
							lprintf(OutputLevels::Error, "S_ParseMusInfo: Unknown MUS lump %s", sc_String);
						}
					}
				}
				else
				{
					lprintf(OutputLevels::Error, "S_ParseMusInfo: Number not in range 0 to %d", MAX_MUS_ENTRIES - 1);
				}
			}
		}

		SC_Close();
	}
	else // No MUSINFO lump -> clear MUSINFO music state (without restarting music)
	{
		memset(musinfo.items, 0, sizeof(musinfo.items));
		musinfo.items[0] = -1;
		musinfo.mapthing = nullptr;
		musinfo.lastmapthing = nullptr;
		musinfo.tics = 0;
	}
}

void MusInfoThinker(mobj_t* thing)
{
	if(musinfo.mapthing != thing &&
		thing->subsector->sector == players[displayplayer].mo->subsector->sector)
	{
		musinfo.lastmapthing = musinfo.mapthing;
		musinfo.mapthing = thing;
		musinfo.tics = 30;
	}
}

void T_MAPMusic()
{
	if(musinfo.tics < 0 || !musinfo.mapthing)
	{
		return;
	}

	if(musinfo.tics > 0)
	{
		musinfo.tics--;
	}
	else
	{
		if(!musinfo.tics && musinfo.lastmapthing != musinfo.mapthing)
		{
			int arraypt = TIDNUM(musinfo.mapthing);

			if(arraypt >= 0 && arraypt < MAX_MUS_ENTRIES)
			{
				int lumpnum = musinfo.items[arraypt];

				if(lumpnum > 0 && lumpnum < numlumps)
				{
					S_ChangeMusInfoMusic(lumpnum, true);
				}
				else // missing musinfo entry -> silence
				{
					lprintf(OutputLevels::Warn, "T_MAPMusic: MUSINFO entry %d not defined\n", arraypt);
					S_StopMusic();
					musinfo.current_item = -1;
				}
			}

			musinfo.tics = -1;
		}
	}
}
