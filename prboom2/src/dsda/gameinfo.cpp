// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//  DSDA GAMEINFO

#include <string.h>

#include "d_main.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"
#include "z_zone.hpp"

#include "scanner.hpp"

#include "gameinfo.hpp"

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
		scanner.MustGetToken(static_cast<TokenType>('='));
		scanner.MustGetString();

		if(iwadlump)
			Z_Free(iwadlump);

		iwadlump = Z_Strdup(scanner.string);
	}
}

void dsda_LoadGameInfo()
{
	int lump;

	lump = W_CheckNumForName("GAMEINFO");

	if(lump == LUMP_NOT_FOUND)
		return;

	Scanner scanner((const char*)W_LumpByNum(lump), W_LumpLength(lump));

	scanner.SetErrorCallback([](const std::string_view message) { Log::Fatal("{}", message); });

	while(scanner.TokensLeft())
		dsda_ParseGameInfoLine(scanner);
}
