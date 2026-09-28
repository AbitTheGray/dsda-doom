// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include "doomstat.hpp"
#include "w_wad.hpp"
#include "v_video.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "lprintf.hpp"

#include "dsda/palette.hpp"

#include "hexen/f_finale.hpp"

#define TEXTSPEED	3
#define TEXTWAIT	250

static void TextWrite();
static void DrawPic();
static void InitializeFade(dboolean fadeIn);
static void DeInitializeFade();
static void FadePic();
static char* GetFinaleText(int sequence);

static int FinaleStage;
static int FinaleCount;
static int FinaleEndCount;
static const char* FinaleLumpName;
static int FontABaseLump;
static char* FinaleText;

// static fixed_t *Palette;
// static fixed_t *PaletteDelta;
// static byte *RealPalette;

void Hexen_F_StartFinale()
{
	gameaction = GameAction::Nothing;
	gamestate = GameState::Finale;
	automap_full = false;

	FinaleStage = 0;
	FinaleCount = 0;
	FinaleText = GetFinaleText(0);
	FinaleEndCount = 70;
	FinaleLumpName = "FINALE1";
	FontABaseLump = W_GetNumForName("FONTA_S") + 1;
	InitializeFade(1);

	S_StartSongName("hall", false); // don't loop the song
}

dboolean Hexen_F_Responder(event_t* event)
{
	return false;
}

void Hexen_F_Ticker()
{
	FinaleCount++;
	if(FinaleStage < 5 && FinaleCount >= FinaleEndCount)
	{
		FinaleCount = 0;
		FinaleStage++;
		switch(FinaleStage)
		{
			case 1: // Text 1
				FinaleEndCount = strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
				break;
			case 2: // Pic 2, Text 2
				FinaleText = GetFinaleText(1);
				FinaleEndCount = strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
				FinaleLumpName = "FINALE2";
				S_StartSongName("orb", false);
				break;
			case 3: // Pic 2 -- Fade out
				FinaleEndCount = 70;
				DeInitializeFade();
				InitializeFade(0);
				break;
			case 4: // Pic 3 -- Fade in
				FinaleLumpName = "FINALE3";
				FinaleEndCount = 71;
				DeInitializeFade();
				InitializeFade(1);
				S_StartSongName("chess", true);
				break;
			case 5: // Pic 3 , Text 3
				FinaleText = GetFinaleText(2);
				DeInitializeFade();
				break;
			default:
				break;
		}
		return;
	}
	if(FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
	{
		FadePic();
	}
}

static void TextWrite()
{
	int count;
	char* ch;
	int c;
	int cx, cy;
	int lump;
	int width;

	V_DrawRawScreen(FinaleLumpName);
	if(FinaleStage == 5)
	{
		// Chess pic, draw the correct character graphic
		if(netgame)
		{
			V_DrawNamePatch(20, 0, 0, "chessall", ColorRange::Default, PatchTranslation::Stretch);
		}
		else if(std::to_underlying(PlayerClass[consoleplayer]) > 1)
		{
			V_DrawNumPatch(60, 0, 0,
				W_GetNumForName("chessc") + std::to_underlying(PlayerClass[consoleplayer]) - 2,
				ColorRange::Default, PatchTranslation::Stretch);
		}
	}
	// Draw the actual text
	if(FinaleStage == 5)
	{
		cy = 135;
	}
	else
	{
		cy = 5;
	}
	cx = 20;
	ch = FinaleText;
	count = (FinaleCount - 10) / TEXTSPEED;
	if(count < 0)
	{
		count = 0;
	}
	for(; count; count--)
	{
		c = *ch++;
		if(!c)
		{
			break;
		}
		if(c == '\n')
		{
			cx = 20;
			cy += 9;
			continue;
		}
		if(c < 32)
		{
			continue;
		}
		c = toupper(c);
		if(c == 32)
		{
			cx += 5;
			continue;
		}
		lump = FontABaseLump + c - 33;
		width = R_NumPatchWidth(lump);
		if(cx + width > SCREENWIDTH)
		{
			break;
		}
		V_DrawNumPatch(cx, cy, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
		cx += width;
	}
}

// hexen_note: dynamic palette editing
static void InitializeFade(dboolean fadeIn)
{
	// unsigned i;
	//
	// Palette = Z_Malloc(768 * sizeof(fixed_t));
	// PaletteDelta = Z_Malloc(768 * sizeof(fixed_t),);
	// RealPalette = Z_Malloc(768 * sizeof(byte));
	//
	// if (fadeIn)
	// {
	//     memset(RealPalette, 0, 768 * sizeof(byte));
	//     for (i = 0; i < 768; i++)
	//     {
	//         Palette[i] = 0;
	//         PaletteDelta[i] = FixedDiv((*((byte *) W_LumpByName("playpal") +
	//                                       i)) << FRACBITS, 70 * FRACUNIT);
	//     }
	// }
	// else
	// {
	//     for (i = 0; i < 768; i++)
	//     {
	//         RealPalette[i] =
	//             *((byte *) W_LumpByName("playpal") + i);
	//         Palette[i] = RealPalette[i] << FRACBITS;
	//         PaletteDelta[i] = FixedDiv(Palette[i], -70 * FRACUNIT);
	//     }
	// }
	// I_SetPalette(RealPalette);
}

static void DeInitializeFade()
{
	// Z_Free(Palette);
	// Z_Free(PaletteDelta);
	// Z_Free(RealPalette);
}

static void FadePic()
{
	// unsigned i;
	//
	// for (i = 0; i < 768; i++)
	// {
	//     Palette[i] += PaletteDelta[i];
	//     RealPalette[i] = Palette[i] >> FRACBITS;
	// }
	// I_SetPalette(RealPalette);
}

static void DrawPic()
{
	V_DrawRawScreen(FinaleLumpName);
	if(FinaleStage == 4 || FinaleStage == 5)
	{
		// Chess pic, draw the correct character graphic
		if(netgame)
		{
			V_DrawNamePatch(20, 0, 0, "chessall", ColorRange::Default, PatchTranslation::Stretch);
		}
		else if(std::to_underlying(PlayerClass[consoleplayer]) > 1)
		{
			V_DrawNumPatch(60, 0, 0,
				W_GetNumForName("chessc") + std::to_underlying(PlayerClass[consoleplayer]) - 2,
				ColorRange::Default, PatchTranslation::Stretch);
		}
	}
}

void Hexen_F_Drawer()
{
	switch(FinaleStage)
	{
		case 0: // Fade in initial finale screen
			DrawPic();
			break;
		case 1:
		case 2:
			TextWrite();
			break;
		case 3: // Fade screen out
			DrawPic();
			break;
		case 4: // Fade in chess screen
			DrawPic();
			break;
		case 5:
			TextWrite();
			break;
	}
}

extern char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE];

static char* GetFinaleText(int sequence)
{
	const char* msgLumpName;
	int msgSize;
	int msgLump;
	static const char* winMsgLumpNames[] = {
		"win1msg",
		"win2msg",
		"win3msg"
	};

	msgLumpName = winMsgLumpNames[sequence];
	msgLump = W_GetNumForName(msgLumpName);
	msgSize = W_LumpLength(msgLump);
	if(msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
	{
		Log::Fatal("Finale message too long ({})", msgLumpName);
	}

	memcpy(ClusterMessage, W_LumpByNum(msgLump), msgSize);
	ClusterMessage[msgSize] = '\0'; // Append terminator
	return ClusterMessage;
}
