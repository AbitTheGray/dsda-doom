// SPDX-License-Identifier: LGPL-2.0-or-later

//-----------------------------------------------------------------------------
//
// Copyright 2017 Christoph Oelckers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//-----------------------------------------------------------------------------

#include <utility>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "umapinfo.hpp"
#include "scanner.hpp"

#include "m_misc.hpp"
#include "g_game.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp"
#include "dsda/episode.hpp"
#include "dsda/name.hpp"

extern "C"
{
	MapList Maps;
}

// -----------------------------------------------
//
//
// -----------------------------------------------

static void FreeMap(MapEntry* mape)
{
	if(mape->lumpname) Z_Free(mape->lumpname);
	if(mape->levelname) Z_Free(mape->levelname);
	if(mape->label) Z_Free(mape->label);
	if(mape->author) Z_Free(mape->author);
	if(mape->intertext) Z_Free(mape->intertext);
	if(mape->intertextsecret) Z_Free(mape->intertextsecret);
	if(mape->bossactions) Z_Free(mape->bossactions);
	mape->lumpname = nullptr;
}

void FreeMapList()
{
	unsigned i;

	for(i = 0; i < Maps.mapcount; i++)
	{
		FreeMap(&Maps.maps[i]);
	}
	Z_Free(Maps.maps);
	Maps.maps = nullptr;
	Maps.mapcount = 0;
}

void ReplaceString(char** pptr, const char* newstring)
{
	if(*pptr != nullptr) Z_Free(*pptr);
	*pptr = Z_Strdup(newstring);
}

// -----------------------------------------------
//
// Parses a set of string and concatenates them
//
// -----------------------------------------------

static char* ParseMultiString(Scanner& scanner, int error)
{
	char* build = nullptr;

	if(scanner.CheckToken(TokenType::Identifier))
	{
		if(scanner.StringMatch("clear"))
		{
			return Z_Strdup("-"); // this was explicitly deleted to override the default.
		}
		else
		{
			scanner.ErrorF("Either 'clear' or string constant expected");
		}
	}

	do
	{
		scanner.MustGetToken(TokenType::StringConst);
		if(build == nullptr) build = Z_Strdup(scanner.string);
		else
		{
			size_t newlen = strlen(build) + strlen(scanner.string) + 2; // strlen for both the existing text and the new line, plus room for one \n and one \0
			build = (char*)Z_Realloc(build, newlen);                    // Prepare the destination memory for the below strcats
			strcat(build, "\n");                                        // Replace the existing text's \0 terminator with a \n
			strcat(build, scanner.string);                              // Concatenate the new line onto the existing text
		}
	}
	while(scanner.CheckToken(static_cast<TokenType>(',')));
	return build;
}

// -----------------------------------------------
//
// Parses a lump name. The buffer must be at least 9 characters.
//
// -----------------------------------------------

static int ParseLumpName(Scanner& scanner, char* buffer)
{
	scanner.MustGetToken(TokenType::StringConst);
	if(strlen(scanner.string) > 8)
	{
		scanner.ErrorF("String too long. Maximum size is 8 characters.");
		return 0;
	}
	strncpy(buffer, scanner.string, 8);
	buffer[8] = 0;
	M_Strupr(buffer);
	return 1;
}

// -----------------------------------------------
//
// Parses a standard property that is already known
// These do not get stored in the property list
// but in dedicated struct member variables.
//
// -----------------------------------------------

static int ParseStandardProperty(Scanner& scanner, MapEntry* mape)
{
	// find the next line with content.
	// this line is no property.

	scanner.MustGetToken(TokenType::Identifier);
	char* pname = Z_Strdup(scanner.string);
	scanner.MustGetToken(static_cast<TokenType>('='));

	if(!stricmp(pname, "levelname"))
	{
		scanner.MustGetToken(TokenType::StringConst);
		ReplaceString(&mape->levelname, scanner.string);
	}
	else if(!stricmp(pname, "label"))
	{
		if(scanner.CheckToken(TokenType::Identifier))
		{
			if(scanner.StringMatch("clear"))
			{
				mape->flags |= UMapinfoFlags::LabelClear;
			}
			else
			{
				scanner.ErrorF("Either 'clear' or string constant expected");
				return 0;
			}
		}
		else
		{
			scanner.MustGetToken(TokenType::StringConst);
			mape->flags -= UMapinfoFlags::LabelClear;
			ReplaceString(&mape->label, scanner.string);
		}
	}
	else if(!stricmp(pname, "author"))
	{
		scanner.MustGetToken(TokenType::StringConst);
		ReplaceString(&mape->author, scanner.string);
	}
	else if(!stricmp(pname, "next"))
	{
		ParseLumpName(scanner, mape->nextmap);
		if(!G_ValidateMapName(mape->nextmap, nullptr, nullptr))
		{
			scanner.ErrorF("Invalid map name %s.", mape->nextmap);
			return 0;
		}
	}
	else if(!stricmp(pname, "nextsecret"))
	{
		ParseLumpName(scanner, mape->nextsecret);
		if(!G_ValidateMapName(mape->nextsecret, nullptr, nullptr))
		{
			scanner.ErrorF("Invalid map name %s", mape->nextsecret);
			return 0;
		}
	}
	else if(!stricmp(pname, "levelpic"))
	{
		ParseLumpName(scanner, mape->levelpic);
	}
	else if(!stricmp(pname, "skytexture"))
	{
		ParseLumpName(scanner, mape->skytexture);
	}
	else if(!stricmp(pname, "music"))
	{
		ParseLumpName(scanner, mape->music);
	}
	else if(!stricmp(pname, "endpic"))
	{
		mape->flags -= UMapinfoFlags::EndGameAny;
		ParseLumpName(scanner, mape->endpic);
		mape->flags |= UMapinfoFlags::EndGameArt;
	}
	else if(!stricmp(pname, "endcast") && !raven)
	{
		scanner.MustGetToken(TokenType::BoolConst);
		mape->flags -= UMapinfoFlags::EndGameAny;
		mape->flags |= (scanner.boolean)
			? UMapinfoFlags::EndGameCast
			: UMapinfoFlags::EndGameClear;
	}
	else if((!stricmp(pname, "endbunny") && !raven) ||
		(!stricmp(pname, "enddemon") && heretic))
	{
		scanner.MustGetToken(TokenType::BoolConst);
		mape->flags -= UMapinfoFlags::EndGameAny;
		mape->flags |= (scanner.boolean)
			? UMapinfoFlags::EndGameScroll
			: UMapinfoFlags::EndGameClear;
	}
	else if(!stricmp(pname, "endgame"))
	{
		scanner.MustGetToken(TokenType::BoolConst);
		mape->flags -= UMapinfoFlags::EndGameAny;
		mape->flags |= (scanner.boolean)
			? UMapinfoFlags::EndGameStandard
			: UMapinfoFlags::EndGameClear;
	}
	else if(!stricmp(pname, "endpalette"))
	{
		ParseLumpName(scanner, mape->endpalette);
	}
	else if(!stricmp(pname, "exitpic"))
	{
		ParseLumpName(scanner, mape->exitpic);
	}
	else if(!stricmp(pname, "enterpic"))
	{
		ParseLumpName(scanner, mape->enterpic);
	}
	else if(!stricmp(pname, "nointermission"))
	{
		scanner.MustGetToken(TokenType::BoolConst);
		if(scanner.boolean)
			mape->flags |= UMapinfoFlags::NoIntermission;
		else
			mape->flags -= UMapinfoFlags::NoIntermission;
	}
	else if(!stricmp(pname, "partime"))
	{
		scanner.MustGetInteger();
		mape->partime = TICRATE * scanner.number;
	}
	else if(!stricmp(pname, "intertext"))
	{
		if(scanner.CheckToken(TokenType::Identifier))
		{
			if(!stricmp(scanner.string, "clear"))
				mape->flags |= UMapinfoFlags::InterTextClear;
			else
				scanner.ErrorF("Either 'clear' or string constant expected");
		}
		else
		{
			mape->flags -= UMapinfoFlags::InterTextClear;
			if(mape->intertext) Z_Free(mape->intertext);
			mape->intertext = ParseMultiString(scanner, 1);
		}
	}
	else if(!stricmp(pname, "intertextsecret"))
	{
		if(scanner.CheckToken(TokenType::Identifier))
		{
			if(!stricmp(scanner.string, "clear"))
				mape->flags |= UMapinfoFlags::InterTextSecretClear;
			else
				scanner.ErrorF("Either 'clear' or string constant expected");
		}
		else
		{
			mape->flags -= UMapinfoFlags::InterTextSecretClear;
			if(mape->intertextsecret) Z_Free(mape->intertextsecret);
			mape->intertextsecret = ParseMultiString(scanner, 1);
		}
	}
	else if(!stricmp(pname, "interbackdrop"))
	{
		ParseLumpName(scanner, mape->interbackdrop);
	}
	else if(!stricmp(pname, "intermusic"))
	{
		ParseLumpName(scanner, mape->intermusic);
	}
	else if(!stricmp(pname, "episode"))
	{
		if(scanner.CheckToken(TokenType::Identifier))
		{
			if(scanner.StringMatch("clear")) dsda_ClearEpisodes();
			else
			{
				scanner.ErrorF("Either 'clear' or string constant expected");
				return 0;
			}
		}
		else
		{
			char lumpname[9] = {0};
			char* alttext = nullptr;
			char key = 0;

			ParseLumpName(scanner, lumpname);
			if(scanner.CheckToken(static_cast<TokenType>(',')))
			{
				scanner.MustGetToken(TokenType::StringConst);
				alttext = Z_Strdup(scanner.string);
				if(scanner.CheckToken(static_cast<TokenType>(',')))
				{
					scanner.MustGetToken(TokenType::StringConst);
					key = tolower(scanner.string[0]);
				}
			}

			dsda_AddEpisode(mape->lumpname, alttext, lumpname, key, false);

			if(alttext) Z_Free(alttext);
		}
	}
	else if(!stricmp(pname, "bossaction"))
	{
		scanner.MustGetToken(TokenType::Identifier);
		if(scanner.StringMatch("clear"))
		{
			// mark level free of boss actions
			if(mape->bossactions) Z_Free(mape->bossactions);
			mape->bossactions = nullptr;
			mape->numbossactions = 0;
			mape->flags |= UMapinfoFlags::BossActionClear;
		}
		else
		{
			int special, tag, type;
			mape->flags -= UMapinfoFlags::BossActionClear;

			type = dsda_ActorNameToType(scanner.string);

			if(type == NAME_NOT_FOUND)
			{
				scanner.ErrorF("Unknown thing type %s", scanner.string);
				return 0;
			}

			scanner.MustGetToken(static_cast<TokenType>(','));
			scanner.MustGetInteger();
			special = scanner.number;
			scanner.MustGetToken(static_cast<TokenType>(','));
			scanner.MustGetInteger();
			tag = scanner.number;
			// allow no 0-tag specials here, unless a level exit or massacre.
			if(tag != 0 || special == 11 || special == 51 || special == 52 ||
				(heretic ? special == 105 : special == 124) ||
				(heretic && special == 515))
			{
				mape->numbossactions++;
				mape->bossactions = (struct BossAction*)Z_Realloc(mape->bossactions, sizeof(struct BossAction) * mape->numbossactions);
				mape->bossactions[mape->numbossactions - 1].type = static_cast<MobjType>(type);
				mape->bossactions[mape->numbossactions - 1].special = special;
				mape->bossactions[mape->numbossactions - 1].tag = tag;
			}
		}
	}
	else
	{
		do
		{
			if(!scanner.CheckFloat()) scanner.GetNextToken();
			if(scanner.token > TokenType::BoolConst)
			{
				scanner.Error(TokenType::Identifier);
			}
		}
		while(scanner.CheckToken(static_cast<TokenType>(',')));
	}
	Z_Free(pname);
	return 1;
}

// -----------------------------------------------
//
// Parses a complete map entry
//
// -----------------------------------------------

static int ParseMapEntry(Scanner& scanner, MapEntry* val)
{
	val->lumpname = nullptr;

	scanner.MustGetIdentifier("map");
	scanner.MustGetToken(TokenType::Identifier);
	if(!G_ValidateMapName(scanner.string, nullptr, nullptr))
	{
		scanner.ErrorF("Invalid map name %s", scanner.string);
		return 0;
	}

	ReplaceString(&val->lumpname, scanner.string);
	scanner.MustGetToken(static_cast<TokenType>('{'));
	while(!scanner.CheckToken(static_cast<TokenType>('}')))
	{
		ParseStandardProperty(scanner, val);
	}
	return 1;
}

// -----------------------------------------------
//
// Parses a complete UMAPINFO lump
//
// -----------------------------------------------

int ParseUMapInfo(const unsigned char* buffer, size_t length, umapinfo_errorfunc err)
{
	Scanner scanner((const char*)buffer, length);
	unsigned int i;

	scanner.SetErrorCallback(err);

	while(scanner.TokensLeft())
	{
		MapEntry parsed = {nullptr};
		ParseMapEntry(scanner, &parsed);

		// Set default level progression here to simplify the checks elsewhere.
		// Doing this lets us skip all normal code for this if nothing has been defined.
		if(!parsed.nextmap[0] && (parsed.flags & (UMapinfoFlags::EndGameAny | UMapinfoFlags::EndGameClear)) == UMapinfoFlags{})
		{
			if(!raven)
			{
				if(!stricmp(parsed.lumpname, "MAP30"))
				{
					parsed.flags |= UMapinfoFlags::EndGameCast;
				}
				else if(!stricmp(parsed.lumpname, "E1M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, gamemode == GameMode::Retail && !pwad_help2_check ? "CREDIT" : "HELP2");
				}
				else if(!stricmp(parsed.lumpname, "E2M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, "VICTORY2");
				}
				else if(!stricmp(parsed.lumpname, "E3M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameScroll;
				}
				else if(!stricmp(parsed.lumpname, "E4M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, "ENDPIC");
				}
				else if(gamemission == GameMission::TcChex && !stricmp(parsed.lumpname, "E1M5"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, "CREDIT");
				}
			}
			else if(heretic)
			{
				if(!stricmp(parsed.lumpname, "E1M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, gamemode == GameMode::Shareware ? "ORDER" : "CREDIT");
				}
				else if(!stricmp(parsed.lumpname, "E2M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, "E2END");
					strcpy(parsed.endpalette, "E2PAL");
				}
				else if(!stricmp(parsed.lumpname, "E3M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameScroll;
				}
				else if(!stricmp(parsed.lumpname, "E4M8") || !stricmp(parsed.lumpname, "E5M8"))
				{
					parsed.flags |= UMapinfoFlags::EndGameArt;
					strcpy(parsed.endpic, "CREDIT");
				}
			}

			// If no default attribute, just go to the next map
			if((parsed.flags & (UMapinfoFlags::EndGameAny | UMapinfoFlags::EndGameClear)) == UMapinfoFlags{})
			{
				int ep, map;
				if(G_ValidateMapName(parsed.lumpname, &ep, &map))
				{
					strcpy(parsed.nextmap, VANILLA_MAP_LUMP_NAME(ep, map + 1));
				}
			}
		}

		// Does this property already exist? If yes, replace it.
		for(i = 0; i < Maps.mapcount; i++)
		{
			if(!strcmp(parsed.lumpname, Maps.maps[i].lumpname))
			{
				FreeMap(&Maps.maps[i]);
				Maps.maps[i] = parsed;
				break;
			}
		}
		// Not found so create a new one.
		if(i == Maps.mapcount)
		{
			Maps.mapcount++;
			Maps.maps = (MapEntry*)Z_Realloc(Maps.maps, sizeof(MapEntry) * Maps.mapcount);
			Maps.maps[Maps.mapcount - 1] = parsed;
		}
	}
	return 1;
}

MapProperty* FindProperty(MapEntry* map, const char* name)
{
	return nullptr;
}
