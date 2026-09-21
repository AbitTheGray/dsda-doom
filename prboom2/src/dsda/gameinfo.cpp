// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//  DSDA GAMEINFO

#include <string.h>

extern "C"
{
#include "d_main.h"
#include "w_wad.h"
#include "lprintf.h"
#include "z_zone.h"
}

#include "scanner.h"

#include "gameinfo.h"

void dsda_ParseGameInfoLine(Scanner& scanner)
{
	if(!scanner.CheckString())
	{
		scanner.GetNextToken();
		scanner.SkipLine();
		return;
	}

	if(!stricmp(scanner.string, "IWAD"))
	{
		scanner.MustGetToken('=');
		scanner.MustGetString();

		if(iwadlump)
			Z_Free(iwadlump);

		iwadlump = Z_Strdup(scanner.string);
	}
}

void dsda_LoadGameInfo(void)
{
	int lump;

	lump = W_CheckNumForName("GAMEINFO");

	if(lump == LUMP_NOT_FOUND)
		return;

	Scanner scanner((const char*)W_LumpByNum(lump), W_LumpLength(lump));

	scanner.SetErrorCallback(I_Error);

	while(scanner.TokensLeft())
		dsda_ParseGameInfoLine(scanner);
}
