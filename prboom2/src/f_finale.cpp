// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Game completion, final screen animation.
 */

#include "umapinfo.hpp"

#include <utility>

#include "doomstat.hpp"
#include "d_event.hpp"
#include "g_game.hpp"
#include "lprintf.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "d_deh.hpp"  // Ty 03/22/98 - externalizations

#include "heretic/f_finale.hpp"
#include "hexen/f_finale.hpp"

#include "dsda/font.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/palette.hpp"

#include "f_finale.hpp" // CPhipps - hmm...

// defines for the end mission display text                     // phares

#define TEXTSPEED    3     // original value                    // phares
#define TEXTWAIT     250   // original value                    // phares
#define NEWTEXTSPEED 0.01f // new value                         // phares
#define NEWTEXTWAIT  1000  // new value                         // phares

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast

FinaleScreen finalestage;
int finalecount;
const char* finaletext;
const char* finaleflat;
const char* finalepatch;
const char* endpic;
const char* endpalette;
UMapinfoFlags endgameflags;

// defines for the end mission display text                     // phares

// CPhipps - removed the old finale screen text message strings;
// they were commented out for ages already
// Ty 03/22/98 - ... the new s_WHATEVER extern variables are used
// in the code below instead.

extern "C" void F_CastTicker();
extern "C" dboolean F_CastResponder(event_t* ev);
extern "C" void F_CastDrawer();

extern "C" void WI_checkForAccelerate(); // killough 3/28/98: used to
extern int acceleratestage;       // accelerate intermission screens
int midstage;                     // whether we're in "mid-stage"

//
// F_StartFinale
//
void F_StartFinale()
{
	int mnum;
	int muslump;

	finaletext = nullptr;
	finaleflat = nullptr;
	finalepatch = nullptr;
	endpic = nullptr;
	endpalette = nullptr;
	endgameflags = static_cast<UMapinfoFlags>(0);

	if(heretic) return Heretic_F_StartFinale();
	if(hexen) return Hexen_F_StartFinale();

	gameaction = GameAction::Nothing;
	gamestate = GameState::Finale;
	automap_full = false;

	// killough 3/28/98: clear accelerative text flags
	acceleratestage = midstage = 0;

	dsda_InterMusic(&mnum, &muslump);

	if(muslump >= 0)
	{
		S_ChangeMusInfoMusic(muslump, true);
	}
	else
	{
		S_ChangeMusic(static_cast<MusicId>(mnum), true);
	}

	// Okay - IWAD dependend stuff.
	// This has been changed severly, and
	//  some stuff might have changed in the process.
	switch(gamemode)
	{
		// DOOM 1 - E1, E3 or E4, but each nine missions
		case GameMode::Shareware:
		case GameMode::Registered:
		case GameMode::Retail:
		{
			switch(gameepisode)
			{
				case 1:
					finaleflat = bgflatE1; // Ty 03/30/98 - new externalized bg flats
					finaletext = s_E1TEXT; // Ty 03/23/98 - Was e1text variable.
					break;
				case 2:
					finaleflat = bgflatE2;
					finaletext = s_E2TEXT; // Ty 03/23/98 - Same stuff for each
					break;
				case 3:
					finaleflat = bgflatE3;
					finaletext = s_E3TEXT;
					break;
				case 4:
					finaleflat = bgflatE4;
					finaletext = s_E4TEXT;
					break;
				default:
					// Ouch.
					break;
			}
			break;
		}

		// DOOM II and missions packs with E1, M34
		case GameMode::Commercial:
		{
			// Ty 08/27/98 - added the gamemission logic
			switch(gamemap)
			{
				case 6:
					finaleflat = bgflat06;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T1TEXT : (gamemission == GameMission::PackPlut) ? s_P1TEXT : s_C1TEXT;
					break;
				case 11:
					finaleflat = bgflat11;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T2TEXT : (gamemission == GameMission::PackPlut) ? s_P2TEXT : s_C2TEXT;
					break;
				case 20:
					finaleflat = bgflat20;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T3TEXT : (gamemission == GameMission::PackPlut) ? s_P3TEXT : s_C3TEXT;
					break;
				case 30:
					finaleflat = bgflat30;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T4TEXT : (gamemission == GameMission::PackPlut) ? s_P4TEXT : s_C4TEXT;
					break;
				case 15:
					finaleflat = bgflat15;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T5TEXT : (gamemission == GameMission::PackPlut) ? s_P5TEXT : s_C5TEXT;
					break;
				case 31:
					finaleflat = bgflat31;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T6TEXT : (gamemission == GameMission::PackPlut) ? s_P6TEXT : s_C6TEXT;
					break;
				default:
					// Ouch.
					break;
			}
			if(gamemission == GameMission::PackNerve && gamemap == 8)
			{
				finaleflat = bgflat06;
				finaletext = s_C6TEXT;
			}
			break;
			// Ty 08/27/98 - end gamemission logic
		}

		// Indeterminate.
		default:                   // Ty 03/30/98 - not externalized
			finaleflat = "F_SKY1"; // Not used anywhere else.
			finaletext = s_C1TEXT; // FIXME - other text, music?
			break;
	}

	if(dsda_FinaleShortcut())
	{
		switch(gamemode)
		{
			case GameMode::Shareware:
			case GameMode::Registered:
			case GameMode::Retail:
				switch(gameepisode)
				{
					case 1:
						finaleflat = bgflatE1;
						finaletext = s_E1TEXT;
						break;
					case 2:
						finaleflat = bgflatE2;
						finaletext = s_E2TEXT;
						break;
					case 3:
						finaleflat = bgflatE3;
						finaletext = s_E3TEXT;
						break;
					case 4:
					default:
						finaleflat = bgflatE4;
						finaletext = s_E4TEXT;
						break;
				}
				break;
			case GameMode::Commercial:
				if(gamemission == GameMission::PackNerve)
				{
					finaleflat = bgflat06;
					finaletext = s_C6TEXT;
				}
				else
				{
					finaleflat = bgflat30;
					finaletext = (gamemission == GameMission::PackTnt) ? s_T4TEXT : (gamemission == GameMission::PackPlut) ? s_P4TEXT : s_C4TEXT;
				}
				break;
			case GameMode::Indetermined:
				break;
		}
	}

	dsda_StartFinale();

	finalestage = FinaleScreen::Text;
	finalecount = 0;
}



static dboolean F_BlockingInput()
{
	return finalestage == FinaleScreen::Art && endpalette && endpalette[0];
}

dboolean F_Responder(event_t* event)
{
	if(heretic) return Heretic_F_Responder(event);
	if(hexen) return Hexen_F_Responder(event);

	if(finalestage == FinaleScreen::Cast)
		return F_CastResponder(event);
	else if(finalestage == FinaleScreen::Art)
	{
		// If the palette is changed, kick to title instead of opening the menu
		if(F_BlockingInput() && event->type == EventType::KeyDown)
		{
			finalestage = FinaleScreen::Title;
			S_StartVoidSound(g_sfx_swtchx);
			V_SetPlayPal(PlaypalIndex::Default);
			V_DrawRawScreen("TITLEPIC");
			return true;
		}
	}

	return false;
}

// Get_TextSpeed() returns the value of the text display speed  // phares
// Rewritten to allow user-directed acceleration -- killough 3/28/98

extern "C" float Get_TextSpeed()
{
	return midstage ? NEWTEXTSPEED : (midstage = acceleratestage) ? acceleratestage = 0, NEWTEXTSPEED : TEXTSPEED;
}


//
// F_Ticker
//
// killough 3/28/98: almost totally rewritten, to use
// player-directed acceleration instead of constant delays.
// Now the player can accelerate the text display by using
// the fire/use keys while it is being printed. The delay
// automatically responds to the user, and gives enough
// time to read.
//
// killough 5/10/98: add back v1.9 demo compatibility
//

dboolean F_ShowCast()
{
	return gamemap == 30 ||
		(gamemission == GameMission::PackNerve && allow_incompatibility && gamemap == 8) ||
		dsda_FinaleShortcut();
}

void F_Ticker()
{
	int i;

	if(heretic) return Heretic_F_Ticker();
	if(hexen) return Hexen_F_Ticker();

	if(dsda_FTicker())
	{
		return;
	}

	if(!demo_compatibility)
		WI_checkForAccelerate();                        // killough 3/28/98: check for acceleration
	else if(gamemode == GameMode::Commercial && finalecount > 50) // check for skipping
		for(i = 0; i < g_maxplayers; i++)
			if(players[i].cmd.buttons != static_cast<ButtonCode>(0))
				goto next_level; // go on to the next level

	// advance animation
	finalecount++;

	if(finalestage == FinaleScreen::Cast)
		F_CastTicker();

	if(finalestage == FinaleScreen::Text)
	{
		/* killough 2/28/98: changed to allow acceleration */
		if(finalecount > strlen(finaletext) * (demo_compatibility ? TEXTSPEED : Get_TextSpeed()) +
			(midstage ? NEWTEXTWAIT : TEXTWAIT) ||
			(midstage && acceleratestage))
		{
			if(gamemode != GameMode::Commercial) // Doom 1 / Ultimate Doom episode end
			{
				// with enough time, it's automatic
				if(gameepisode == 3)
					F_StartScroll(nullptr, nullptr, nullptr, true);
				else
					F_StartPostFinale();
			}
			else // you must press a button to continue in Doom 2
				if(!demo_compatibility && midstage)
				{
				next_level:
					if(F_ShowCast())
						F_StartCast(nullptr, nullptr, true); // cast of Doom 2 characters
					else
						gameaction = GameAction::WorldDone; // next level, e.g. MAP07
				}
		}
	}
}

//
// F_TextWrite
//
// This program displays the background and text at end-mission     // phares
// text time. It draws both repeatedly so that other displays,      //   |
// like the main menu, can be drawn over it dynamically and         //   V
// erased dynamically. The TEXTSPEED constant is changed into
// the Get_TextSpeed function so that the speed of writing the      //   ^
// text can be increased, and there's still time to read what's     //   |
// written.                                                         // phares
// CPhipps - reformatted

#include "hu_stuff.hpp"

extern "C" void F_TextWrite()
{
	if(finalepatch)
	{
		V_ClearBorder();
		V_DrawNamePatchFS(0, 0, 0, finalepatch, ColorRange::Default, PatchTranslation::Stretch);
	}
	else
		V_DrawBackground(finaleflat, 0);

	{
		// draw some of the text onto the screen
		int cx = 10;
		int cy = 10;
		const char* ch = finaletext;                                    // CPhipps - const
		int count = (int)((float)(finalecount - 10) / Get_TextSpeed()); // phares
		int w;

		if(count < 0)
			count = 0;

		for(; count; count--)
		{
			int c = *ch++;

			if(!c)
				break;

			if(c == '\n')
			{
				cx = 10;
				cy += 11;
				continue;
			}

			c = toupper(c) - HU_FONTSTART;
			if(c < 0 || c > HU_FONTSIZE)
			{
				cx += 4;
				continue;
			}

			w = hud_font.font[c].width;
			if(cx + w > SCREENWIDTH)
				break;

			// CPhipps - patch drawing updated
			V_DrawNumPatch(cx, cy, 0, hud_font.font[c].lumpnum, ColorRange::Default, PatchTranslation::Stretch);
			cx += w;
		}
	}
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
	const char** name; // CPhipps - const**
	MobjType type;
} castinfo_t;

static const castinfo_t castorder_d2[] = {
	{&s_CC_ZOMBIE, MobjType::Possessed},
	{&s_CC_SHOTGUN, MobjType::Shotguy},
	{&s_CC_HEAVY, MobjType::Chainguy},
	{&s_CC_IMP, MobjType::Troop},
	{&s_CC_DEMON, MobjType::Sergeant},
	{&s_CC_LOST, MobjType::Skull},
	{&s_CC_CACO, MobjType::Head},
	{&s_CC_HELL, MobjType::Knight},
	{&s_CC_BARON, MobjType::Bruiser},
	{&s_CC_ARACH, MobjType::Baby},
	{&s_CC_PAIN, MobjType::Pain},
	{&s_CC_REVEN, MobjType::Undead},
	{&s_CC_MANCU, MobjType::Fatso},
	{&s_CC_ARCH, MobjType::Vile},
	{&s_CC_SPIDER, MobjType::Spider},
	{&s_CC_CYBER, MobjType::Cyborg},
	{&s_CC_HERO, MobjType::Player},
	{nullptr, static_cast<MobjType>(0)}
};

static const castinfo_t castorder_d1[] = {
	{&s_CC_ZOMBIE, MobjType::Possessed},
	{&s_CC_SHOTGUN, MobjType::Shotguy},
	{&s_CC_IMP, MobjType::Troop},
	{&s_CC_DEMON, MobjType::Sergeant},
	{&s_CC_LOST, MobjType::Skull},
	{&s_CC_CACO, MobjType::Head},
	{&s_CC_BARON, MobjType::Bruiser},
	{&s_CC_SPIDER, MobjType::Spider},
	{&s_CC_CYBER, MobjType::Cyborg},
	{&s_CC_HERO, MobjType::Player},
	{nullptr, static_cast<MobjType>(0)}
};

static const castinfo_t* castorder = castorder_d2;

static int castnum;
static int casttics;
static state_t* caststate;
static dboolean castdeath;
static int castframes;
static int castonmelee;
static dboolean castattacking;
static const char* castbackground;

//
// F_StartCast
//

static void F_StartCastMusic(const char* music, dboolean loop_music)
{
	if(music)
	{
		if(!S_ChangeMusicByName(music, loop_music))
			lprintf(OutputLevels::Warn, "Finale cast music not found: %s\n", music);
	}
	else if(gamemode == GameMode::Commercial)
	{
		S_ChangeMusic(MusicId::Evil, loop_music);
	}
	else
	{
		lprintf(OutputLevels::Warn, "Finale cast music unspecified\n");
		S_StopMusic();
	}
}

void F_StartCast(const char* background, const char* music, dboolean loop_music)
{
	castorder = (gamemode == GameMode::Commercial ? castorder_d2 : castorder_d1);
	castbackground = (background ? background : bgcastcall);

	wipegamestate = static_cast<GameState>(-1); // force a screen wipe
	castnum = 0;
	caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].seestate)];
	casttics = caststate->tics;
	castdeath = false;
	finalestage = FinaleScreen::Cast;
	castframes = 0;
	castonmelee = 0;
	castattacking = false;

	F_StartCastMusic(music, loop_music);
}

//
// F_CastTicker
//
extern "C" void F_CastTicker()
{
	StateId st;
	SfxId sfx;

	if(--casttics > 0)
		return; // not time to change state yet

	if(caststate->tics == -1 || caststate->nextstate == StateId::Null)
	{
		// switch from deathstate to next monster
		castnum++;
		castdeath = false;
		if(castorder[castnum].name == nullptr)
			castnum = 0;
		if(mobjinfo[std::to_underlying(castorder[castnum].type)].seesound != SfxId::None)
			S_StartVoidSound(mobjinfo[std::to_underlying(castorder[castnum].type)].seesound);
		caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].seestate)];
		castframes = 0;
	}
	else
	{
		// just advance to next state in animation
		if(caststate == &states[std::to_underlying(StateId::PlayAtk1)])
			goto stopattack; // Oh, gross hack!
		st = caststate->nextstate;
		caststate = &states[std::to_underlying(st)];
		castframes++;

		// sound hacks....
		switch(st)
		{
			case StateId::PlayAtk1: sfx = SfxId::Dshtgn;
				break;
			case StateId::PossAtk2: sfx = SfxId::Pistol;
				break;
			case StateId::SposAtk2: sfx = SfxId::Shotgn;
				break;
			case StateId::VileAtk2: sfx = SfxId::Vilatk;
				break;
			case StateId::SkelFist2: sfx = SfxId::Skeswg;
				break;
			case StateId::SkelFist4: sfx = SfxId::Skepch;
				break;
			case StateId::SkelMiss2: sfx = SfxId::Skeatk;
				break;
			case StateId::FattAtk8:
			case StateId::FattAtk5:
			case StateId::FattAtk2: sfx = SfxId::Firsht;
				break;
			case StateId::CposAtk2:
			case StateId::CposAtk3:
			case StateId::CposAtk4: sfx = SfxId::Shotgn;
				break;
			case StateId::TrooAtk3: sfx = SfxId::Claw;
				break;
			case StateId::SargAtk2: sfx = SfxId::Sgtatk;
				break;
			case StateId::BossAtk2:
			case StateId::Bos2Atk2:
			case StateId::HeadAtk2: sfx = SfxId::Firsht;
				break;
			case StateId::SkullAtk2: sfx = SfxId::Sklatk;
				break;
			case StateId::SpidAtk2:
			case StateId::SpidAtk3: sfx = SfxId::Shotgn;
				break;
			case StateId::BspiAtk2: sfx = SfxId::Plasma;
				break;
			case StateId::CyberAtk2:
			case StateId::CyberAtk4:
			case StateId::CyberAtk6: sfx = SfxId::Rlaunc;
				break;
			case StateId::PainAtk3: sfx = SfxId::Sklatk;
				break;
			default: sfx = SfxId::None;
				break;
		}

		if(sfx != SfxId::None)
			S_StartVoidSound(sfx);
	}

	if(castframes == 12)
	{
		// go into attack frame
		castattacking = true;
		if(castonmelee)
			caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].meleestate)];
		else
			caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].missilestate)];
		castonmelee ^= 1;
		if(caststate == &states[std::to_underlying(StateId::Null)])
		{
			if(castonmelee)
				caststate =
					&states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].meleestate)];
			else
				caststate =
					&states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].missilestate)];
		}
	}

	if(castattacking)
	{
		if(castframes == 24
			|| caststate == &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].seestate)])
		{
		stopattack:
			castattacking = false;
			castframes = 0;
			caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].seestate)];
		}
	}

	casttics = caststate->tics;
	if(casttics == -1)
		casttics = 15;
}


//
// F_CastResponder
//

extern "C" dboolean F_CastResponder(event_t* ev)
{
	if(ev->type != EventType::KeyDown)
		return false;

	if(castdeath)
		return true; // already in dying frames

	// go into death frame
	castdeath = true;
	caststate = &states[std::to_underlying(mobjinfo[std::to_underlying(castorder[castnum].type)].deathstate)];
	casttics = caststate->tics;
	castframes = 0;
	castattacking = false;
	if(mobjinfo[std::to_underlying(castorder[castnum].type)].deathsound != SfxId::None)
		S_StartVoidSound(mobjinfo[std::to_underlying(castorder[castnum].type)].deathsound);

	return true;
}


static void F_CastPrint(const char* text) // CPhipps - static, const char*
{
	const char* ch; // CPhipps - const
	int c;
	int cx;
	int w;
	int width;

	// find width
	ch = text;
	width = 0;

	while(ch)
	{
		c = *ch++;
		if(!c)
			break;
		c = toupper(c) - HU_FONTSTART;
		if(c < 0 || c > HU_FONTSIZE)
		{
			width += 4;
			continue;
		}

		w = hud_font.font[c].width;
		width += w;
	}

	// draw it
	cx = 160 - width / 2;
	ch = text;
	while(ch)
	{
		c = *ch++;
		if(!c)
			break;
		c = toupper(c) - HU_FONTSTART;
		if(c < 0 || c > HU_FONTSIZE)
		{
			cx += 4;
			continue;
		}

		w = hud_font.font[c].width;
		// CPhipps - patch drawing updated
		V_DrawNumPatch(cx, 180, 0, hud_font.font[c].lumpnum, ColorRange::Default, PatchTranslation::Stretch);
		cx += w;
	}
}


//
// F_CastDrawer
//

extern "C" void F_CastDrawer()
{
	spritedef_t* sprdef;
	spriteframe_t* sprframe;
	int lump;
	dboolean flip;

	// e6y: wide-res
	V_ClearBorder();
	// erase the entire screen to a background
	// CPhipps - patch drawing updated
	V_DrawNamePatchFS(0, 0, 0, castbackground, ColorRange::Default, PatchTranslation::Stretch); // Ty 03/30/98 bg texture extern

	F_CastPrint(*(castorder[castnum].name));

	// draw the current frame in the middle of the screen
	sprdef = &sprites[std::to_underlying(caststate->sprite)];
	sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
	lump = sprframe->lump[0];
	flip = (dboolean)(sprframe->flip & 1);

	// CPhipps - patch drawing updated
	V_DrawNumPatch(160, 170, 0, lump+firstspritelump, ColorRange::Default,
		(flip ? PatchTranslation::Stretch | PatchTranslation::Flip : PatchTranslation::Stretch));
}

//
// F_BunnyScroll
//
static const char* pfub1 = "PFUB1";
static const char* pfub2 = "PFUB2";

static const char* scrollpic1;
static const char* scrollpic2;

static void F_StartScrollMusic(const char* music, dboolean loop_music)
{
	if(music)
	{
		if(!S_ChangeMusicByName(music, loop_music))
			lprintf(OutputLevels::Warn, "Finale scroll music not found: %s\n", music);
	}
	else if(W_LumpNameExists("D_BUNNY"))
		S_ChangeMusic(MusicId::Bunny, loop_music);
	else
	{
		lprintf(OutputLevels::Warn, "Finale scroll music unspecified\n");
		S_StopMusic();
	}
}

static dboolean end_patches_exist;

void F_StartScroll(const char* right, const char* left, const char* music, dboolean loop_music)
{
	wipegamestate = static_cast<GameState>(-1); // force a wipe
	scrollpic1 = right ? right : pfub1;
	scrollpic2 = left ? left : pfub2;
	finalecount = 0;
	finalestage = FinaleScreen::Art;

	end_patches_exist = W_CheckNumForName("END0") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END1") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END2") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END3") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END4") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END5") != LUMP_NOT_FOUND &&
		W_CheckNumForName("END6") != LUMP_NOT_FOUND;

	F_StartScrollMusic(music, loop_music);
}

extern "C" void F_BunnyScroll()
{
	char name[10];
	int stage;
	static int laststage;
	static int p1offset, p2width;

	if(finalecount == 0)
	{
		const rpatch_t *p1, *p2;
		p1 = R_PatchByName(scrollpic1);
		p2 = R_PatchByName(scrollpic2);

		p2width = p2->width;
		if(p1->width == 320)
		{
			// Unity or original PFUBs.
			p1offset = (p2width - 320) / 2;
		}
		else
		{
			// Widescreen mod PFUBs.
			p1offset = 0;
		}
	}

	{
		int scrolled = 320 - (finalecount - 230) / 2;
		if(scrolled <= 0)
		{
			V_DrawNamePatchFS(0, 0, 0, scrollpic2, ColorRange::Default, PatchTranslation::Stretch);
		}
		else if(scrolled >= 320)
		{
			V_DrawNamePatchFS(p1offset, 0, 0, scrollpic1, ColorRange::Default, PatchTranslation::Stretch);
			if(p1offset > 0)
				V_DrawNamePatchFS(-320, 0, 0, scrollpic2, ColorRange::Default, PatchTranslation::Stretch);
		}
		else
		{
			V_DrawNamePatchFS(p1offset + 320 - scrolled, 0, 0, scrollpic1, ColorRange::Default, PatchTranslation::Stretch);
			V_DrawNamePatchFS(-scrolled, 0, 0, scrollpic2, ColorRange::Default, PatchTranslation::Stretch);
		}
		if(p2width == 320)
			V_ClearBorder();
	}

	if(!end_patches_exist)
		return;

	if(finalecount < 1130)
		return;
	if(finalecount < 1180)
	{
		// CPhipps - patch drawing updated
		V_DrawNamePatch((320-13*8)/2, (200-8*8)/2, 0, "END0", ColorRange::Default, PatchTranslation::Stretch);
		laststage = 0;
		return;
	}

	stage = (finalecount - 1180) / 5;
	if(stage > 6)
		stage = 6;
	if(stage > laststage)
	{
		S_StartVoidSound(SfxId::Pistol);
		laststage = stage;
	}

	snprintf(name, sizeof name, "END%i", stage);
	// CPhipps - patch drawing updated
	V_DrawNamePatch((320-13*8)/2, (200-8*8)/2, 0, name, ColorRange::Default, PatchTranslation::Stretch);
}

void F_StartPostFinale()
{
	finalecount = 0;
	finalestage = FinaleScreen::Art;
	wipegamestate = static_cast<GameState>(-1); // force a wipe
}

//
// F_Drawer
//
void F_Drawer()
{
	if(heretic) return Heretic_F_Drawer();
	if(hexen) return Hexen_F_Drawer();

	if(dsda_FDrawer())
	{
		return;
	}

	if(finalestage == FinaleScreen::Text)
		F_TextWrite();
	else if(finalestage == FinaleScreen::Art)
	{
		const char* finalelump = nullptr;

		// Allows use of HELP2 screen for PWADs under DOOM 1
		dboolean showhelp2 = (gamemode == GameMode::Retail && pwad_help2_check) || gamemode <= GameMode::Registered;

		switch(gameepisode)
		{
			// CPhipps - patch drawing updated
			case 1:
				finalelump = showhelp2 ? "HELP2" : "CREDIT";
				break;
			case 2:
				finalelump = "VICTORY2";
				break;
			case 3:
				F_BunnyScroll();
				break;
			case 4:
				finalelump = "ENDPIC";
				break;
		}

		if(finalelump)
		{
			V_ClearBorder(); // e6y: wide-res
			V_DrawNamePatchFS(0, 0, 0, finalelump, ColorRange::Default, PatchTranslation::Stretch);
		}
	}
	else if(finalestage == FinaleScreen::Cast)
		F_CastDrawer();
	else if(finalestage == FinaleScreen::Title)
		V_DrawRawScreen("TITLEPIC"); // Palette change has ended, just show the title
}
