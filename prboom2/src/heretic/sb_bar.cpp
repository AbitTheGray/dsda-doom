// SPDX-License-Identifier: GPL-2.0-or-later

// SB_bar.c

#include <utility>

#include "doomstat.hpp"
#include "m_cheat.hpp"
#include "m_menu.hpp"
#include "m_random.hpp"
#include "v_video.hpp"
#include "r_main.hpp"
#include "w_wad.hpp"
#include "st_stuff.hpp"
#include "r_draw.hpp"

#include "dsda/pause.hpp"
#include "dsda/settings.hpp"

#include "heretic/def.hpp"
#include "heretic/dstrings.hpp"

#include "sb_bar.hpp"

// Private Functions

static void ShadeLine(int x, int y, int height, int shade);
static void ShadeChain();
static void DrINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrawCommonBar();
static void DrawMainBar();
static void DrawInventoryBar();

static void Hexen_SB_Init();
static void Hexen_DrINumber(signed int val, int x, int y);
static void Hexen_DrSmallNumberVPT(int val, int x, int y, PatchTranslation vpt);
static void DrRedINumber(signed int val, int x, int y);
static void DrawKeyBar();
static void DrawWeaponPieces();
static void DrawAnimatedIcons();
static void Hexen_DrawMainBar();

// Public Data

dboolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;
int SB_state = -1;

// Private Data

static int HealthMarker;
static int ChainWiggle;
static player_t* CPlayer;

int LumpLTFACE;
int LumpRTFACE;
int LumpBARBACK;
int LumpCHAIN;
int LumpSTATBAR;
int LumpLIFEGEM;
int LumpLTFCTOP;
int LumpRTFCTOP;
int LumpSELECTBOX;
int LumpINVLFGEM1;
int LumpINVLFGEM2;
int LumpINVRTGEM1;
int LumpINVRTGEM2;
int LumpINumbers[10];
int LumpNEGATIVE;
int LumpSmNumbers[10];
int LumpBLACKSQ;
int LumpINVBAR;
int LumpARMCLEAR;
int LumpCHAINBACK;
int FontBNumBase;
int spinbooklump;
int spinflylump;

static char heretic_namearti[][10] = {
	{"ARTIBOX"},  // none
	{"ARTIINVU"}, // invulnerability
	{"ARTIINVS"}, // invisibility
	{"ARTIPTN2"}, // health
	{"ARTISPHL"}, // superhealth
	{"ARTIPWBK"}, // tomeofpower
	{"ARTITRCH"}, // torch
	{"ARTIFBMB"}, // firebomb
	{"ARTIEGGC"}, // egg
	{"ARTISOAR"}, // fly
	{"ARTIATLP"}  // teleport
};
static int heretic_lumparti[11];

static int SpinMinotaurLump;
static int SpinSpeedLump;
static int SpinDefenseLump;

static int LumpH2BAR;
static int LumpH2TOP;
static int LumpLFEDGE;
static int LumpRTEDGE;
static int LumpARTICLEAR;
static int LumpMANACLEAR;
static int LumpKILLS;
static int LumpMANAVIAL1;
static int LumpMANAVIAL2;
static int LumpMANAVIALDIM1;
static int LumpMANAVIALDIM2;
static int LumpMANADIM1;
static int LumpMANADIM2;
static int LumpMANABRIGHT1;
static int LumpMANABRIGHT2;
static int LumpKEYBAR;
static int LumpWEAPONSLOT;
static int LumpWEAPONFULL;
static int LumpPIECE1;
static int LumpPIECE2;
static int LumpPIECE3;

static char hexen_namearti[][10] = {
	{"ARTIBOX"},  // none
	{"ARTIINVU"}, // invulnerability
	{"ARTIPTN2"}, // health
	{"ARTISPHL"}, // superhealth
	{"ARTIHRAD"}, // healing radius
	{"ARTISUMN"}, // summon maulator
	{"ARTITRCH"}, // torch
	{"ARTIPORK"}, // egg
	{"ARTISOAR"}, // fly
	{"ARTIBLST"}, // blast radius
	{"ARTIPSBG"}, // poison bag
	{"ARTITELO"}, // teleport other
	{"ARTISPED"}, // speed
	{"ARTIBMAN"}, // boost mana
	{"ARTIBRAC"}, // boost armor
	{"ARTIATLP"}, // teleport
	{"ARTISKLL"}, // hexen_arti_puzzskull
	{"ARTIBGEM"}, // hexen_arti_puzzgembig
	{"ARTIGEMR"}, // hexen_arti_puzzgemred
	{"ARTIGEMG"}, // hexen_arti_puzzgemgreen1
	{"ARTIGMG2"}, // hexen_arti_puzzgemgreen2
	{"ARTIGEMB"}, // hexen_arti_puzzgemblue1
	{"ARTIGMB2"}, // hexen_arti_puzzgemblue2
	{"ARTIBOK1"}, // hexen_arti_puzzbook1
	{"ARTIBOK2"}, // hexen_arti_puzzbook2
	{"ARTISKL2"}, // hexen_arti_puzzskull2
	{"ARTIFWEP"}, // hexen_arti_puzzfweapon
	{"ARTICWEP"}, // hexen_arti_puzzcweapon
	{"ARTIMWEP"}, // hexen_arti_puzzmweapon
	{"ARTIGEAR"}, // hexen_arti_puzzgear1
	{"ARTIGER2"}, // hexen_arti_puzzgear2
	{"ARTIGER3"}, // hexen_arti_puzzgear3
	{"ARTIGER4"}, // hexen_arti_puzzgear4
};
static int hexen_lumparti[33];

static int* lumparti;

// game config
static int sb_ticker_delta_cap;
static int sb_icon_y;
static int sb_inv_bar_x;
static int sb_inv_bar_y;
static int sb_inv_arti_y;
static int sb_inv_arti_count_x;
static int sb_inv_arti_count_y;
static int sb_inv_select_y;
static int sb_inv_gem_x;
static int sb_inv_gem_y;
static int sb_full_arti_x;
static int sb_full_arti_y;
static int sb_full_arti_count_x;
static int sb_full_inv_arti_x;
static int sb_full_inv_arti_y;
static int sb_full_inv_arti_count_x;
static int sb_full_inv_arti_count_y;
static int sb_full_inv_select_y;
static int sb_full_inv_gem_xl;
static int sb_full_inv_gem_xr;

void SB_Start()
{
	SB_state = -1;
}

//---------------------------------------------------------------------------
//
// PROC SB_Init
//
//---------------------------------------------------------------------------

extern patchnum_t stbarbg;
extern patchnum_t brdr_b;

void SB_Init()
{
	int i;
	int startLump;

	if(hexen) return Hexen_SB_Init();

	lumparti = heretic_lumparti;
	sb_ticker_delta_cap = 8;
	sb_icon_y = 17;
	sb_inv_bar_x = 34;
	sb_inv_bar_y = 160;
	sb_inv_arti_y = 160;
	sb_inv_arti_count_x = 69;
	sb_inv_arti_count_y = 182;
	sb_inv_select_y = 189;
	sb_inv_gem_x = 38;
	sb_inv_gem_y = 159;
	sb_full_arti_x = 286;
	sb_full_arti_y = 170;
	sb_full_arti_count_x = 307;
	sb_full_inv_arti_x = 50;
	sb_full_inv_arti_y = 168;
	sb_full_inv_arti_count_x = 69;
	sb_full_inv_arti_count_y = 190;
	sb_full_inv_select_y = 197;
	sb_full_inv_gem_xl = 38;
	sb_full_inv_gem_xr = 269;

	// magic globals that ends up in the background
	R_SetPatchNum(&brdr_b, "bordb");
	R_SetPatchNum(&stbarbg, "BARBACK");

	for(i = 0; i < 11; ++i)
	{
		lumparti[i] = W_CheckNumForName2(heretic_namearti[i], LumpNamespace::Sprites);
	}

	LumpLTFACE = W_GetNumForName("LTFACE");
	LumpRTFACE = W_GetNumForName("RTFACE");
	LumpBARBACK = W_GetNumForName("BARBACK");
	LumpINVBAR = W_GetNumForName("INVBAR");
	LumpCHAIN = W_GetNumForName("CHAIN");
	if(deathmatch)
	{
		LumpSTATBAR = W_GetNumForName("STATBAR");
	}
	else
	{
		LumpSTATBAR = W_GetNumForName("LIFEBAR");
	}
	if(!netgame)
	{
		// single player game uses red life gem
		LumpLIFEGEM = W_GetNumForName("LIFEGEM2");
	}
	else
	{
		LumpLIFEGEM = W_GetNumForName("LIFEGEM0") + consoleplayer;
	}
	LumpLTFCTOP = W_GetNumForName("LTFCTOP");
	LumpRTFCTOP = W_GetNumForName("RTFCTOP");
	LumpSELECTBOX = W_GetNumForName("SELECTBOX");
	LumpINVLFGEM1 = W_GetNumForName("INVGEML1");
	LumpINVLFGEM2 = W_GetNumForName("INVGEML2");
	LumpINVRTGEM1 = W_GetNumForName("INVGEMR1");
	LumpINVRTGEM2 = W_GetNumForName("INVGEMR2");
	LumpBLACKSQ = W_GetNumForName("BLACKSQ");
	LumpARMCLEAR = W_GetNumForName("ARMCLEAR");
	LumpCHAINBACK = W_GetNumForName("CHAINBACK");
	startLump = W_GetNumForName("IN0");
	for(i = 0; i < 10; i++)
	{
		LumpINumbers[i] = startLump + i;
	}
	LumpNEGATIVE = W_GetNumForName("NEGNUM");
	FontBNumBase = W_GetNumForName("FONTB16");
	startLump = W_GetNumForName("SMALLIN0");
	for(i = 0; i < 10; i++)
	{
		LumpSmNumbers[i] = startLump + i;
	}
	spinbooklump = W_GetNumForName("SPINBK0");
	spinflylump = W_GetNumForName("SPFLY0");

	// [FG] support widescreen status bar backgrounds
	ST_SetScaledWidth();

	ST_LoadTextColors();
}

//---------------------------------------------------------------------------
//
// PROC SB_Ticker
//
//---------------------------------------------------------------------------

void SB_Ticker()
{
	int delta;
	int curHealth;

	if(heretic && leveltime & 1 && !dsda_PausedOutsideDemo())
	{
		ChainWiggle = P_Random(RandomClass::Heretic) & 1;
	}
	curHealth = players[consoleplayer].mo->health;
	if(curHealth < 0)
	{
		curHealth = 0;
	}
	if(curHealth < HealthMarker)
	{
		delta = (HealthMarker - curHealth) >> 2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > sb_ticker_delta_cap)
		{
			delta = sb_ticker_delta_cap;
		}
		HealthMarker -= delta;
	}
	else if(curHealth > HealthMarker)
	{
		delta = (curHealth - HealthMarker) >> 2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > sb_ticker_delta_cap)
		{
			delta = sb_ticker_delta_cap;
		}
		HealthMarker += delta;
	}
}

//---------------------------------------------------------------------------
//
// PROC DrINumber
//
// Draws a three digit number.
//
//---------------------------------------------------------------------------

static void DrINumber(signed int val, int x, int y)
{
	int lump;
	int oldval;

	if(hexen) return Hexen_DrINumber(val, x, y);

	oldval = val;
	if(val < 0)
	{
		if(val < -9)
		{
			V_DrawNamePatch(x + 1, y + 1, 0, "LAME", ColorRange::Default, PatchTranslation::Stretch);
		}
		else
		{
			val = -val;
			V_DrawNumPatch(x + 18, y, 0, LumpINumbers[val], ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(x + 9, y, 0, LumpNEGATIVE, ColorRange::Default, PatchTranslation::Stretch);
		}
		return;
	}
	if(val > 99)
	{
		lump = LumpINumbers[val / 100];
		V_DrawNumPatch(x, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 100;
	if(val > 9 || oldval > 99)
	{
		lump = LumpINumbers[val / 10];
		V_DrawNumPatch(x + 9, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 10;
	lump = LumpINumbers[val];
	V_DrawNumPatch(x + 18, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
}

//---------------------------------------------------------------------------
//
// PROC DrBNumber
//
// Draws a three digit number using FontB
//
//---------------------------------------------------------------------------

static void DrBNumber(signed int val, int x, int y)
{
	int lump;
	int xpos;
	int oldval;

	oldval = val;
	xpos = x;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 99)
	{
		lump = FontBNumBase + val / 100;
		V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
	}
	val = val % 100;
	xpos += 12;
	if(val > 9 || oldval > 99)
	{
		lump = FontBNumBase + val / 10;
		V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
	}
	val = val % 10;
	xpos += 12;
	lump = FontBNumBase + val;
	V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumber
//
// Draws a small two digit number.
//
//---------------------------------------------------------------------------

static void DrSmallNumberVPT(int val, int x, int y, PatchTranslation vpt)
{
	int lump;

	if(hexen) return Hexen_DrSmallNumberVPT(val, x, y, vpt);

	if(val == 1)
	{
		return;
	}
	if(val > 9)
	{
		lump = LumpSmNumbers[val / 10];
		V_DrawNumPatch(x, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
	}
	val = val % 10;
	lump = LumpSmNumbers[val];
	V_DrawNumPatch(x + 4, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
}

static void DrSmallNumber(int val, int x, int y)
{
	DrSmallNumberVPT(val, x, y, PatchTranslation::Stretch);
}

//---------------------------------------------------------------------------
//
// PROC ShadeLine
//
//---------------------------------------------------------------------------

static void ShadeLine(int x, int y, int height, int shade)
{
	// HERETIC_TODO: ShadeLine
	// byte *dest;
	// byte *shades;
	//
	// x <<= crispy->hires;
	// y <<= crispy->hires;
	// height <<= crispy->hires;
	//
	// shades = colormaps + 9 * 256 + shade * 2 * 256;
	// dest = I_VideoBuffer + y * SCREENWIDTH + x;
	// while (height--)
	// {
	//     if (crispy->hires)
	//         *(dest + 1) = *(shades + *dest);
	//     *(dest) = *(shades + *dest);
	//     dest += SCREENWIDTH;
	// }
}

//---------------------------------------------------------------------------
//
// PROC ShadeChain
//
//---------------------------------------------------------------------------

static void ShadeChain()
{
	int i;

	for(i = 0; i < 16; i++)
	{
		ShadeLine(277 + i, 190, 10, i / 2);
		ShadeLine(19 + i, 190, 10, 7 - (i / 2));
	}
}

//---------------------------------------------------------------------------
//
// PROC SB_Drawer
//
//---------------------------------------------------------------------------

char ammopic[][10] = {
	{"INAMGLD"},
	{"INAMBOW"},
	{"INAMBST"},
	{"INAMRAM"},
	{"INAMPNX"},
	{"INAMLOB"}
};

static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldammo = -1;
static int oldarmor = -1;
static int oldweapon = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldkeys = -1;

static int oldmana1 = -1;
static int oldmana2 = -1;
static int oldpieces = -1;

void SB_Drawer(dboolean statusbaron, dboolean refresh)
{
	if(refresh || fadeBG() || V_IsOpenGLMode()) SB_state = -1;

	if(!statusbaron)
	{
		SB_PaletteFlash(false);
		if(R_FullView())
		{
			DrawAnimatedIcons();
		}
		return;
	}

	CPlayer = &players[consoleplayer];
	if(SB_state == -1)
	{
		if(heretic)
		{
			V_DrawNumPatchFS(0, 158, 0, LumpBARBACK, ColorRange::Default, PatchTranslation::Stretch);
			if(players[consoleplayer].cheats & CF_GODMODE)
			{
				V_DrawNamePatch(16, 167, 0, "GOD1", ColorRange::Default, PatchTranslation::Stretch);
				V_DrawNamePatch(287, 167, 0, "GOD2", ColorRange::Default, PatchTranslation::Stretch);
			}
		}
		else
		{
			V_DrawNumPatchFS(0, 134, 0, LumpH2BAR, ColorRange::Default, PatchTranslation::Stretch);
		}

		oldhealth = -1;
	}
	DrawCommonBar();
	if(!inventory)
	{
		if(SB_state != 0)
		{
			// Main interface
			if(heretic)
			{
				V_DrawNumPatch(34, 160, 0, LumpSTATBAR, ColorRange::Default, PatchTranslation::Stretch);
			}
			else
			{
				if(!automap_full)
				{
					V_DrawNumPatch(38, 162, 0, LumpSTATBAR, ColorRange::Default, PatchTranslation::Stretch);
				}
				else
				{
					V_DrawNumPatch(38, 162, 0, LumpKEYBAR, ColorRange::Default, PatchTranslation::Stretch);
				}
			}
			oldarti = 0;
			oldammo = -1;
			oldmana1 = -1;
			oldmana2 = -1;
			oldarmor = -1;
			oldpieces = -1;
			oldweapon = -1;
			oldfrags = -9999; //can't use -1, 'cuz of negative frags
			oldlife = -1;
			oldkeys = -1;
		}
		if(heretic || !automap_full)
		{
			DrawMainBar();
		}
		else if(hexen)
		{
			DrawKeyBar();
		}
		SB_state = 0;
	}
	else
	{
		if(heretic && SB_state != 1)
		{
			V_DrawNumPatch(34, 160, 0, LumpINVBAR, ColorRange::Default, PatchTranslation::Stretch);
		}
		DrawInventoryBar();
		SB_state = 1;
	}
	SB_PaletteFlash(false);
	DrawAnimatedIcons();
}

// sets the new palette based upon current values of player->damagecount
// and player->bonuscount
void SB_PaletteFlash(dboolean forceChange)
{
	static int sb_palette = 0;
	int palette;

	if(forceChange)
	{
		sb_palette = -1;
	}

	CPlayer = &players[consoleplayer];

	if(CPlayer->poisoncount)
	{
		palette = 0;
		palette = (CPlayer->poisoncount + 7) >> 3;
		if(palette >= NUMPOISONPALS)
		{
			palette = NUMPOISONPALS - 1;
		}
		palette += STARTPOISONPALS;
	}
	else if(CPlayer->damagecount)
	{
		palette = (CPlayer->damagecount + 7) >> 3;
		if(palette >= NUMREDPALS)
		{
			palette = NUMREDPALS - 1;
		}
		palette += STARTREDPALS;
	}
	else if(CPlayer->bonuscount)
	{
		palette = (CPlayer->bonuscount + 7) >> 3;
		if(palette >= NUMBONUSPALS)
		{
			palette = NUMBONUSPALS - 1;
		}
		palette += STARTBONUSPALS;
	}
	else if((CPlayer->mo->flags2 & MobjFlag2::IceDamage) != MobjFlag2{})
	{
		// Frozen player
		palette = STARTICEPAL;
	}
	else
	{
		palette = 0;
	}
	if(palette != sb_palette)
	{
		SB_state = -1;
		sb_palette = palette;
		V_SetPalette(sb_palette);
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawCommonBar
//
//---------------------------------------------------------------------------

void DrawCommonBar()
{
	int healthPos;

	if(!dsda_HideHorns())
	{
		if(heretic)
		{
			V_DrawNumPatchFS(0, 148, 0, LumpLTFCTOP, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatchFS(290, 148, 0, LumpRTFCTOP, ColorRange::Default, PatchTranslation::Stretch);
		}
		else
		{
			V_DrawNumPatchFS(0, 134, 0, LumpH2TOP, ColorRange::Default, PatchTranslation::Stretch);
		}
	}

	if(oldhealth != HealthMarker)
	{
		oldhealth = HealthMarker;
		healthPos = HealthMarker;
		if(healthPos < 0)
		{
			healthPos = 0;
		}
		if(healthPos > 100)
		{
			healthPos = 100;
		}

		if(heretic)
		{
			int chainY;

			healthPos = (healthPos * 256) / 100;
			chainY =
				(HealthMarker == CPlayer->mo->health) ? 191 : 191 + ChainWiggle;
			V_DrawNumPatchFS(0, 190, 0, LumpCHAINBACK, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(2 + (healthPos % 17), chainY, 0, LumpCHAIN, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(17 + healthPos, chainY, 0, LumpLIFEGEM, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(0, 190, 0, LumpLTFACE, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(276, 190, 0, LumpRTFACE, ColorRange::Default, PatchTranslation::Stretch);
			ShadeChain();
		}
		else
		{
			V_DrawNumPatch(28 + (((healthPos * 196) / 100) % 9), 193, 0, LumpCHAIN, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(7 + ((healthPos * 11) / 5), 193, 0, LumpLIFEGEM, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(0, 193, 0, LumpLFEDGE, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(277, 193, 0, LumpRTEDGE, ColorRange::Default, PatchTranslation::Stretch);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawMainBar
//
//---------------------------------------------------------------------------

void DrawMainBar()
{
	int i;
	int temp;

	if(hexen) return Hexen_DrawMainBar();

	// Ready artifact
	if(ArtifactFlash)
	{
		V_DrawNumPatch(180, 161, 0, LumpBLACKSQ, ColorRange::Default, PatchTranslation::Stretch);

		temp = W_GetNumForName("useartia") + ArtifactFlash - 1;

		V_DrawNumPatch(182, 161, 0, temp, ColorRange::Default, PatchTranslation::Stretch);
		ArtifactFlash--;
		oldarti = -1; // so that the correct artifact fills in after the flash
	}
	else if(oldarti != std::to_underlying(CPlayer->readyArtifact)
		|| oldartiCount != CPlayer->inventory[inv_ptr].count)
	{
		V_DrawNumPatch(180, 161, 0, LumpBLACKSQ, ColorRange::Default, PatchTranslation::Stretch);
		if(std::to_underlying(CPlayer->readyArtifact) > 0)
		{
			V_DrawNumPatch(
				179, 160, 0, lumparti[std::to_underlying(CPlayer->readyArtifact)], ColorRange::Default, PatchTranslation::Stretch
			);

			DrSmallNumber(CPlayer->inventory[inv_ptr].count, 201, 182);
		}
		oldarti = std::to_underlying(CPlayer->readyArtifact);
		oldartiCount = CPlayer->inventory[inv_ptr].count;
	}

	// Frags
	if(deathmatch)
	{
		temp = 0;
		for(i = 0; i < g_maxplayers; i++)
		{
			temp += CPlayer->frags[i];
		}
		if(temp != oldfrags)
		{
			V_DrawNumPatch(57, 171, 0, LumpARMCLEAR, ColorRange::Default, PatchTranslation::Stretch);
			DrINumber(temp, 61, 170);
			oldfrags = temp;
		}
	}
	else
	{
		temp = HealthMarker;
		if(temp < 0)
		{
			temp = 0;
		}
		else if(temp > 100)
		{
			temp = 100;
		}
		if(oldlife != temp)
		{
			oldlife = temp;
			V_DrawNumPatch(57, 171, 0, LumpARMCLEAR, ColorRange::Default, PatchTranslation::Stretch);
			DrINumber(temp, 61, 170);
		}
	}

	// Keys
	if(oldkeys != CPlayer->ravenkeys)
	{
		if(CPlayer->cards[std::to_underlying(Card::KeyYellow)])
		{
			V_DrawNamePatch(153, 164, 0, "ykeyicon", ColorRange::Default, PatchTranslation::Stretch);
		}
		if(CPlayer->cards[std::to_underlying(Card::KeyGreen)])
		{
			V_DrawNamePatch(153, 172, 0, "gkeyicon", ColorRange::Default, PatchTranslation::Stretch);
		}
		if(CPlayer->cards[std::to_underlying(Card::KeyBlue)])
		{
			V_DrawNamePatch(153, 180, 0, "bkeyicon", ColorRange::Default, PatchTranslation::Stretch);
		}
		oldkeys = CPlayer->ravenkeys;
	}
	// Ammo
	temp = CPlayer->ammo[std::to_underlying(wpnlev1info[std::to_underlying(CPlayer->readyweapon)].ammo)];
	if(oldammo != temp || oldweapon != std::to_underlying(CPlayer->readyweapon))
	{
		V_DrawNumPatch(108, 161, 0, LumpBLACKSQ, ColorRange::Default, PatchTranslation::Stretch);
		if(temp && std::to_underlying(CPlayer->readyweapon) > 0 && std::to_underlying(CPlayer->readyweapon) < 7)
		{
			DrINumber(temp, 109, 162);
			V_DrawNamePatch(
				111, 172, 0, ammopic[std::to_underlying(CPlayer->readyweapon) - 1], ColorRange::Default, PatchTranslation::Stretch
			);
		}
		oldammo = temp;
		oldweapon = std::to_underlying(CPlayer->readyweapon);
	}

	// Armor
	if(oldarmor != CPlayer->armorpoints[std::to_underlying(ArmorType::Armor)])
	{
		V_DrawNumPatch(224, 171, 0, LumpARMCLEAR, ColorRange::Default, PatchTranslation::Stretch);
		DrINumber(CPlayer->armorpoints[std::to_underlying(ArmorType::Armor)], 228, 170);
		oldarmor = CPlayer->armorpoints[std::to_underlying(ArmorType::Armor)];
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawInventoryBar
//
//---------------------------------------------------------------------------

void DrawInventoryBar()
{
	int i;
	int x;
	int lump;

	x = inv_ptr - curpos;
	V_DrawNumPatch(sb_inv_bar_x, sb_inv_bar_y, 0, LumpINVBAR, ColorRange::Default, PatchTranslation::Stretch);
	for(i = 0; i < 7; i++)
	{
		if(CPlayer->inventorySlotNum > x + i
			&& CPlayer->inventory[x + i].type != std::to_underlying(ArtiType::None))
		{
			V_DrawNumPatch(
				50 + i * 31, sb_inv_arti_y, 0,
				lumparti[CPlayer->inventory[x + i].type], ColorRange::Default, PatchTranslation::Stretch
			);
			DrSmallNumber(CPlayer->inventory[x + i].count,
				sb_inv_arti_count_x + i * 31, sb_inv_arti_count_y);
		}
	}
	V_DrawNumPatch(50 + curpos * 31, sb_inv_select_y, 0, LumpSELECTBOX, ColorRange::Default, PatchTranslation::Stretch);
	if(x != 0)
	{
		lump = !(leveltime & 4) ? LumpINVLFGEM1 : LumpINVLFGEM2;
		V_DrawNumPatch(sb_inv_gem_x, sb_inv_gem_y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	if(CPlayer->inventorySlotNum - x > 7)
	{
		lump = !(leveltime & 4) ? LumpINVRTGEM1 : LumpINVRTGEM2;
		V_DrawNumPatch(269, sb_inv_gem_y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
}

extern "C" void DrawArtifact(int x, int y, PatchTranslation vpt)
{
	inventory_t* inv;
	const int delta_x = heretic ? 22 : 19;
	const int delta_y = heretic ? 22 : 21;

	inv = &players[displayplayer].inventory[inv_ptr];

	if(inv->type > 0)
	{
		V_DrawNumPatch(x, y, 0, lumparti[inv->type], ColorRange::Default, static_cast<PatchTranslation>(vpt));
		DrSmallNumberVPT(inv->count, x + delta_x, y + delta_y, vpt);
	}
}

// hexen

static void Hexen_SB_Init()
{
	int i;
	int startLump;

	lumparti = hexen_lumparti;
	sb_ticker_delta_cap = 6;
	sb_icon_y = 19;
	sb_inv_bar_x = 38;
	sb_inv_bar_y = 162;
	sb_inv_arti_y = 163;
	sb_inv_arti_count_x = 68;
	sb_inv_arti_count_y = 185;
	sb_inv_select_y = 163;
	sb_inv_gem_x = 42;
	sb_inv_gem_y = 163;
	sb_full_arti_x = 284;
	sb_full_arti_y = 169;
	sb_full_arti_count_x = 302;
	sb_full_inv_arti_x = 49;
	sb_full_inv_arti_y = 167;
	sb_full_inv_arti_count_x = 66;
	sb_full_inv_arti_count_y = 188;
	sb_full_inv_select_y = 167;
	sb_full_inv_gem_xl = 40;
	sb_full_inv_gem_xr = 268;

	// magic globals that ends up in the background
	R_SetPatchNum(&brdr_b, "bordb");
	R_SetPatchNum(&stbarbg, "H2BAR");

	for(i = 0; i < 33; ++i)
	{
		lumparti[i] = W_CheckNumForName2(hexen_namearti[i], LumpNamespace::Sprites);
	}

	LumpH2BAR = W_GetNumForName("H2BAR");
	LumpH2TOP = W_GetNumForName("H2TOP");
	LumpINVBAR = W_GetNumForName("INVBAR");
	LumpLFEDGE = W_GetNumForName("LFEDGE");
	LumpRTEDGE = W_GetNumForName("RTEDGE");
	LumpSTATBAR = W_GetNumForName("STATBAR");
	LumpKEYBAR = W_GetNumForName("KEYBAR");
	LumpSELECTBOX = W_GetNumForName("SELECTBOX");
	LumpARTICLEAR = W_GetNumForName("ARTICLS");
	LumpARMCLEAR = W_GetNumForName("ARMCLS");
	LumpMANACLEAR = W_GetNumForName("MANACLS");
	LumpMANAVIAL1 = W_GetNumForName("MANAVL1");
	LumpMANAVIAL2 = W_GetNumForName("MANAVL2");
	LumpMANAVIALDIM1 = W_GetNumForName("MANAVL1D");
	LumpMANAVIALDIM2 = W_GetNumForName("MANAVL2D");
	LumpMANADIM1 = W_GetNumForName("MANADIM1");
	LumpMANADIM2 = W_GetNumForName("MANADIM2");
	LumpMANABRIGHT1 = W_GetNumForName("MANABRT1");
	LumpMANABRIGHT2 = W_GetNumForName("MANABRT2");
	LumpINVLFGEM1 = W_GetNumForName("invgeml1");
	LumpINVLFGEM2 = W_GetNumForName("invgeml2");
	LumpINVRTGEM1 = W_GetNumForName("invgemr1");
	LumpINVRTGEM2 = W_GetNumForName("invgemr2");

	startLump = W_GetNumForName("IN0");
	for(i = 0; i < 10; i++)
	{
		LumpINumbers[i] = startLump + i;
	}
	LumpNEGATIVE = W_GetNumForName("NEGNUM");
	FontBNumBase = W_GetNumForName("FONTB16");
	startLump = W_GetNumForName("SMALLIN0");
	for(i = 0; i < 10; i++)
	{
		LumpSmNumbers[i] = startLump + i;
	}
	spinflylump = W_GetNumForName("SPFLY0");
	SpinMinotaurLump = W_GetNumForName("SPMINO0");
	SpinSpeedLump = W_GetNumForName("SPBOOT0");
	SpinDefenseLump = W_GetNumForName("SPSHLD0");

	if(deathmatch)
	{
		LumpKILLS = W_GetNumForName("KILLS");
	}
	SB_SetClassData();

	// [FG] support widescreen status bar backgrounds
	ST_SetScaledWidth();

	ST_LoadTextColors();
}

void SB_SetClassData()
{
	int class_;

	class_ = std::to_underlying(PlayerClass[consoleplayer]) - 1; // original player class (not pig)
	LumpWEAPONSLOT = W_GetNumForName("wpslot0") + class_;
	LumpWEAPONFULL = W_GetNumForName("wpfull0") + class_;
	LumpPIECE1 = W_GetNumForName("wpiecef1") + class_;
	LumpPIECE2 = W_GetNumForName("wpiecef2") + class_;
	LumpPIECE3 = W_GetNumForName("wpiecef3") + class_;
	LumpCHAIN = W_GetNumForName("chain") + class_;
	if(!netgame)
	{
		// single player game uses red life gem (the second gem)
		LumpLIFEGEM = W_GetNumForName("lifegem") + g_maxplayers * class_ + 1;
	}
	else
	{
		LumpLIFEGEM = W_GetNumForName("lifegem") + g_maxplayers * class_ + consoleplayer;
	}
	SB_state = -1;
}

static void Hexen_DrINumber(signed int val, int x, int y)
{
	int lump;
	int oldval;

	oldval = val;
	if(val < 0)
	{
		val = -val;
		if(val > 99)
		{
			val = 99;
		}
		if(val > 9)
		{
			lump = LumpINumbers[val / 10];
			V_DrawNumPatch(x + 8, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNumPatch(x, y, 0, LumpNEGATIVE, ColorRange::Default, PatchTranslation::Stretch);
		}
		else
		{
			V_DrawNumPatch(x + 8, y, 0, LumpNEGATIVE, ColorRange::Default, PatchTranslation::Stretch);
		}
		val = val % 10;
		lump = LumpINumbers[val];
		V_DrawNumPatch(x + 16, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
		return;
	}
	if(val > 99)
	{
		lump = LumpINumbers[val / 100];
		V_DrawNumPatch(x, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 100;
	if(val > 9 || oldval > 99)
	{
		lump = LumpINumbers[val / 10];
		V_DrawNumPatch(x + 8, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 10;
	lump = LumpINumbers[val];
	V_DrawNumPatch(x + 16, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
}

static void Hexen_DrSmallNumberVPT(int val, int x, int y, PatchTranslation vpt)
{
	int lump;

	if(val <= 0)
	{
		return;
	}
	if(val > 999)
	{
		val %= 1000;
	}
	if(val > 99)
	{
		lump = LumpSmNumbers[val / 100];
		V_DrawNumPatch(x, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
		lump = LumpSmNumbers[(val % 100) / 10];
		V_DrawNumPatch(x + 4, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
	}
	else if(val > 9)
	{
		lump = LumpSmNumbers[val / 10];
		V_DrawNumPatch(x + 4, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
	}
	val %= 10;
	lump = LumpSmNumbers[val];
	V_DrawNumPatch(x + 8, y, 0, lump, ColorRange::Default, static_cast<PatchTranslation>(vpt));
}

static void DrRedINumber(signed int val, int x, int y)
{
	int lump;
	int oldval;

	oldval = val;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 99)
	{
		lump = W_GetNumForName("inred0") + val / 100;
		V_DrawNumPatch(x, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 100;
	if(val > 9 || oldval > 99)
	{
		lump = W_GetNumForName("inred0") + val / 10;
		V_DrawNumPatch(x + 8, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
	}
	val = val % 10;
	lump = W_GetNumForName("inred0") + val;
	V_DrawNumPatch(x + 16, y, 0, lump, ColorRange::Default, PatchTranslation::Stretch);
}

static void DrawAnimatedIcons()
{
	int frame;
	static dboolean hitCenterFrame;

	// Flight icons
	if(CPlayer->powers[std::to_underlying(PowerType::Flight)])
	{
		if(CPlayer->powers[std::to_underlying(PowerType::Flight)] > BLINKTHRESHOLD
			|| !(CPlayer->powers[std::to_underlying(PowerType::Flight)] & 16))
		{
			frame = (leveltime / 3) & 15;
			if((CPlayer->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{})
			{
				if(hitCenterFrame && (frame != 15 && frame != 0))
				{
					V_DrawNumPatch(20, sb_icon_y, 0, spinflylump + 15, ColorRange::Default, PatchTranslation::Stretch);
				}
				else
				{
					V_DrawNumPatch(20, sb_icon_y, 0, spinflylump + frame, ColorRange::Default, PatchTranslation::Stretch);
					hitCenterFrame = false;
				}
			}
			else
			{
				if(!hitCenterFrame && (frame != 15 && frame != 0))
				{
					V_DrawNumPatch(20, sb_icon_y, 0, spinflylump + frame, ColorRange::Default, PatchTranslation::Stretch);
					hitCenterFrame = false;
				}
				else
				{
					V_DrawNumPatch(20, sb_icon_y, 0, spinflylump + 15, ColorRange::Default, PatchTranslation::Stretch);
					hitCenterFrame = true;
				}
			}
		}
	}

	if(CPlayer->powers[std::to_underlying(PowerType::WeaponLevel2)] && !CPlayer->chickenTics)
	{
		if(CPlayer->powers[std::to_underlying(PowerType::WeaponLevel2)] > BLINKTHRESHOLD
			|| !(CPlayer->powers[std::to_underlying(PowerType::WeaponLevel2)] & 16))
		{
			frame = (leveltime / 3) & 15;
			V_DrawNumPatch(300, sb_icon_y, 0, spinbooklump + frame, ColorRange::Default, PatchTranslation::Stretch);
		}
	}

	// Speed Boots
	if(CPlayer->powers[std::to_underlying(PowerType::Speed)])
	{
		if(CPlayer->powers[std::to_underlying(PowerType::Speed)] > BLINKTHRESHOLD
			|| !(CPlayer->powers[std::to_underlying(PowerType::Speed)] & 16))
		{
			frame = (leveltime / 3) & 15;
			V_DrawNumPatch(60, sb_icon_y, 0, SpinSpeedLump + frame, ColorRange::Default, PatchTranslation::Stretch);
		}
	}

	// Defensive power
	if(hexen && CPlayer->powers[std::to_underlying(PowerType::Invulnerability)])
	{
		if(CPlayer->powers[std::to_underlying(PowerType::Invulnerability)] > BLINKTHRESHOLD
			|| !(CPlayer->powers[std::to_underlying(PowerType::Invulnerability)] & 16))
		{
			frame = (leveltime / 3) & 15;
			V_DrawNumPatch(260, sb_icon_y, 0, SpinDefenseLump + frame, ColorRange::Default, PatchTranslation::Stretch);
		}
	}

	// Minotaur Active
	if(CPlayer->powers[std::to_underlying(PowerType::Minotaur)])
	{
		if(CPlayer->powers[std::to_underlying(PowerType::Minotaur)] > BLINKTHRESHOLD
			|| !(CPlayer->powers[std::to_underlying(PowerType::Minotaur)] & 16))
		{
			frame = (leveltime / 3) & 15;
			V_DrawNumPatch(300, sb_icon_y, 0, SpinMinotaurLump + frame, ColorRange::Default, PatchTranslation::Stretch);
		}
	}
}

void DrawKeyBar()
{
	int i;
	int xPosition;
	int temp;

	if(oldkeys != CPlayer->ravenkeys)
	{
		xPosition = 46;
		for(i = 0; i < std::to_underlying(Card::Count) && xPosition <= 126; i++)
		{
			if(CPlayer->ravenkeys & (1 << i))
			{
				V_DrawNumPatch(xPosition, 164, 0,
					W_GetNumForName("keyslot1") + i, ColorRange::Default, PatchTranslation::Stretch);
				xPosition += 20;
			}
		}
		oldkeys = CPlayer->ravenkeys;
	}
	temp = pclass[std::to_underlying(CPlayer->pclass)].auto_armor_save +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Armor)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Shield)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Helmet)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Amulet)];
	if(oldarmor != temp)
	{
		for(i = 0; i < std::to_underlying(ArmorType::Count); i++)
		{
			if(!CPlayer->armorpoints[i])
			{
				continue;
			}
			if(CPlayer->armorpoints[i] <= (pclass[std::to_underlying(CPlayer->pclass)].armor_increment[i] >> 2))
			{
				V_DrawTLNumPatch(150 + 31 * i, 164, W_GetNumForName("armslot1") + i);
			}
			else if(CPlayer->armorpoints[i] <= (pclass[std::to_underlying(CPlayer->pclass)].armor_increment[i] >> 1))
			{
				V_DrawAltTLNumPatch(150 + 31 * i, 164, W_GetNumForName("armslot1") + i);
			}
			else
			{
				V_DrawNumPatch(150 + 31 * i, 164, 0,
					W_GetNumForName("armslot1") + i, ColorRange::Default, PatchTranslation::Stretch);
			}
		}
		oldarmor = temp;
	}
}

static int PieceX[std::to_underlying(PClass::Count)][3] = {
	[std::to_underlying(PClass::Fighter)] = {190, 225, 234},
	{190, 213, 226},
	{190, 205, 224},
	{0, 0, 0} // Pig is never used
};

static void DrawWeaponPieces()
{
	if(CPlayer->pieces == 7)
	{
		V_DrawNumPatch(190, 162, 0, LumpWEAPONFULL, ColorRange::Default, PatchTranslation::Stretch);
		return;
	}
	V_DrawNumPatch(190, 162, 0, LumpWEAPONSLOT, ColorRange::Default, PatchTranslation::Stretch);
	if(CPlayer->pieces & WPIECE1)
	{
		V_DrawNumPatch(PieceX[std::to_underlying(PlayerClass[consoleplayer])][0], 162, 0, LumpPIECE1, ColorRange::Default, PatchTranslation::Stretch);
	}
	if(CPlayer->pieces & WPIECE2)
	{
		V_DrawNumPatch(PieceX[std::to_underlying(PlayerClass[consoleplayer])][1], 162, 0, LumpPIECE2, ColorRange::Default, PatchTranslation::Stretch);
	}
	if(CPlayer->pieces & WPIECE3)
	{
		V_DrawNumPatch(PieceX[std::to_underlying(PlayerClass[consoleplayer])][2], 162, 0, LumpPIECE3, ColorRange::Default, PatchTranslation::Stretch);
	}
}

static void Hexen_DrawMainBar()
{
	int i;
	int temp;
	int manaLump1, manaLump2;
	int manaVialLump1, manaVialLump2;

	manaLump1 = 0;
	manaLump2 = 0;
	manaVialLump1 = 0;
	manaVialLump2 = 0;

	// Ready artifact
	if(ArtifactFlash)
	{
		V_DrawNumPatch(144, 160, 0, LumpARTICLEAR, ColorRange::Default, PatchTranslation::Stretch);
		V_DrawNumPatch(148, 164, 0,
			W_GetNumForName("useartia") + ArtifactFlash - 1, ColorRange::Default, PatchTranslation::Stretch);
		ArtifactFlash--;
		oldarti = -1; // so that the correct artifact fills in after the flash
	}
	else if(oldarti != std::to_underlying(CPlayer->readyArtifact)
		|| oldartiCount != CPlayer->inventory[inv_ptr].count)
	{
		V_DrawNumPatch(144, 160, 0, LumpARTICLEAR, ColorRange::Default, PatchTranslation::Stretch);
		if(std::to_underlying(CPlayer->readyArtifact) > 0)
		{
			V_DrawNumPatch(143, 163, 0,
				lumparti[std::to_underlying(CPlayer->readyArtifact)], ColorRange::Default, PatchTranslation::Stretch);
			if(CPlayer->inventory[inv_ptr].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[inv_ptr].count, 162, 184);
			}
		}
		oldarti = std::to_underlying(CPlayer->readyArtifact);
		oldartiCount = CPlayer->inventory[inv_ptr].count;
	}

	// Frags
	if(deathmatch)
	{
		temp = 0;
		for(i = 0; i < g_maxplayers; i++)
		{
			temp += CPlayer->frags[i];
		}
		if(temp != oldfrags)
		{
			V_DrawNumPatch(38, 162, 0, LumpKILLS, ColorRange::Default, PatchTranslation::Stretch);
			DrINumber(temp, 40, 176);
			oldfrags = temp;
		}
	}
	else
	{
		temp = HealthMarker;
		if(temp < 0)
		{
			temp = 0;
		}
		else if(temp > 100)
		{
			temp = 100;
		}
		if(oldlife != temp)
		{
			oldlife = temp;
			V_DrawNumPatch(41, 178, 0, LumpARMCLEAR, ColorRange::Default, PatchTranslation::Stretch);
			if(temp >= 25)
			{
				DrINumber(temp, 40, 176);
			}
			else
			{
				DrRedINumber(temp, 40, 176);
			}
		}
	}
	// Mana
	temp = CPlayer->ammo[0];
	if(oldmana1 != temp)
	{
		V_DrawNumPatch(77, 178, 0, LumpMANACLEAR, ColorRange::Default, PatchTranslation::Stretch);
		DrSmallNumber(temp, 79, 181);
		manaVialLump1 = -1; // force a vial update
		if(temp == 0)
		{
			// Draw Dim Mana icon
			manaLump1 = LumpMANADIM1;
		}
		else if(oldmana1 == 0)
		{
			manaLump1 = LumpMANABRIGHT1;
		}
		oldmana1 = temp;
	}
	temp = CPlayer->ammo[1];
	if(oldmana2 != temp)
	{
		V_DrawNumPatch(109, 178, 0, LumpMANACLEAR, ColorRange::Default, PatchTranslation::Stretch);
		DrSmallNumber(temp, 111, 181);
		manaVialLump1 = -1; // force a vial update
		if(temp == 0)
		{
			// Draw Dim Mana icon
			manaLump2 = LumpMANADIM2;
		}
		else if(oldmana2 == 0)
		{
			manaLump2 = LumpMANABRIGHT2;
		}
		oldmana2 = temp;
	}
	if(oldweapon != std::to_underlying(CPlayer->readyweapon) || manaLump1 || manaLump2
		|| manaVialLump1)
	{
		// Update mana graphics based upon mana count/weapon type
		if(CPlayer->readyweapon == WeaponType::First)
		{
			manaLump1 = LumpMANADIM1;
			manaLump2 = LumpMANADIM2;
			manaVialLump1 = LumpMANAVIALDIM1;
			manaVialLump2 = LumpMANAVIALDIM2;
		}
		else if(CPlayer->readyweapon == WeaponType::Second)
		{
			if(!manaLump1)
			{
				manaLump1 = LumpMANABRIGHT1;
			}
			manaVialLump1 = LumpMANAVIAL1;
			manaLump2 = LumpMANADIM2;
			manaVialLump2 = LumpMANAVIALDIM2;
		}
		else if(CPlayer->readyweapon == WeaponType::Third)
		{
			manaLump1 = LumpMANADIM1;
			manaVialLump1 = LumpMANAVIALDIM1;
			if(!manaLump2)
			{
				manaLump2 = LumpMANABRIGHT2;
			}
			manaVialLump2 = LumpMANAVIAL2;
		}
		else
		{
			manaVialLump1 = LumpMANAVIAL1;
			manaVialLump2 = LumpMANAVIAL2;
			if(!manaLump1)
			{
				manaLump1 = LumpMANABRIGHT1;
			}
			if(!manaLump2)
			{
				manaLump2 = LumpMANABRIGHT2;
			}
		}
		V_DrawNumPatch(77, 164, 0, manaLump1, ColorRange::Default, PatchTranslation::Stretch);
		V_DrawNumPatch(110, 164, 0, manaLump2, ColorRange::Default, PatchTranslation::Stretch);

		V_DrawNumPatch(94, 164, 0, manaVialLump1, ColorRange::Default, PatchTranslation::Stretch);
		V_FillRectVPT(0, 95, 165, 3, 22 - (22 * CPlayer->ammo[0]) / MAX_MANA, 0, PatchTranslation::Stretch);

		V_DrawNumPatch(102, 164, 0, manaVialLump2, ColorRange::Default, PatchTranslation::Stretch);
		V_FillRectVPT(0, 103, 165, 3, 22 - (22 * CPlayer->ammo[1]) / MAX_MANA, 0, PatchTranslation::Stretch);

		oldweapon = std::to_underlying(CPlayer->readyweapon);
	}
	// Armor
	temp = pclass[std::to_underlying(CPlayer->pclass)].auto_armor_save +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Armor)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Shield)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Helmet)] +
		CPlayer->armorpoints[std::to_underlying(ArmorType::Amulet)];
	if(oldarmor != temp)
	{
		oldarmor = temp;
		V_DrawNumPatch(255, 178, 0, LumpARMCLEAR, ColorRange::Default, PatchTranslation::Stretch);
		DrINumber(FixedDiv(temp, 5 * FRACUNIT) >> FRACBITS, 250, 176);
	}
	// Weapon Pieces
	if(oldpieces != CPlayer->pieces)
	{
		DrawWeaponPieces();
		oldpieces = CPlayer->pieces;
	}
}
