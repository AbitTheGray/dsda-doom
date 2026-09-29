// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to determine game mode (shareware, registered),
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include "SDL_timer.h"

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>

#include "doomdef.hpp"
#include "doomtype.hpp"
#include "doomstat.hpp"
#include "d_net.hpp"
#include "dstrings.hpp"
#include "sounds.hpp"
#include "z_zone.hpp"
#include "w_wad.hpp"
#include "s_sound.hpp"
#include "v_video.hpp"
#include "f_finale.hpp"
#include "f_wipe.hpp"
#include "m_file.hpp"
#include "m_misc.hpp"
#include "m_menu.hpp"
#include "i_main.hpp"
#include "i_system.hpp"
#include "i_sound.hpp"
#include "i_video.hpp"
#include "g_game.hpp"
#include "hu_stuff.hpp"
#include "wi_stuff.hpp"
#include "st_stuff.hpp"
#include "am_map.hpp"
#include "p_setup.hpp"
#include "r_draw.hpp"
#include "r_main.hpp"
#include "r_fps.hpp"
#include "d_main.hpp"
#include "d_deh.hpp"  // Ty 04/08/98 - Externalizations
#include "lprintf.hpp"  // jff 08/03/98 - declaration of lprintf
#include "am_map.hpp"
#include "e6y.hpp"

#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/demo.hpp"
#include "dsda/exdemo.hpp"
#include "dsda/features.hpp"
#include "dsda/global.hpp"
#include "dsda/save.hpp"
#include "dsda/data_organizer.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/gameinfo.hpp"
#include "dsda/mobjinfo.hpp"
#include "dsda/options.hpp"
#include "dsda/pause.hpp"
#include "dsda/playback.hpp"
#include "dsda/preferences.hpp"
#include "dsda/render_stats.hpp"
#include "dsda/settings.hpp"
#include "dsda/signal_context.hpp"
#include "dsda/skill_info.hpp"
#include "dsda/skip.hpp"
#include "dsda/sndinfo.hpp"
#include "dsda/time.hpp"
#include "dsda/utility.hpp"
#include "dsda/wad_stats.hpp"
#include "dsda/zipfile.hpp"
#include "dsda/gl/render_scale.hpp"

#include "heretic/mn_menu.hpp"
#include "heretic/sb_bar.hpp"

#include "hexen/sn_sonix.hpp"

// NSM
#include "i_capture.hpp"

#include "i_glob.hpp"

static void D_PageDrawer();

char* iwadlump;
char* iwadver;
int EpisodeStructure = false;

// jff 1/24/98 add new versions of these variables to remember command line
dboolean clnomonsters;  // checkparm of -nomonsters
dboolean clrespawnparm; // checkparm of -respawn
dboolean clfastparm;    // checkparm of -fast
// jff 1/24/98 end definition of command line version of play mode switches

dboolean nomonsters;  // working -nomonsters
dboolean respawnparm; // working -respawn
dboolean fastparm;    // working -fast

dboolean pistolstart;

dboolean randomclass;

int map_colormap;
fixed_t map_gravity;
fixed_t map_aircontrol;
fixed_t map_airfriction;

dboolean singletics = false; // debug flag to cancel adaptiveness

//jff 1/22/98 parms for disabling music and sound
dboolean nosfxparm;
dboolean nomusicparm;

//jff 4/18/98
extern dboolean inhelpscreens;
extern dboolean BorderNeedRefresh;

int startskill;
int startepisode;
int startmap;
dboolean autostart;
FILE* debugfile;

dboolean advancedemo;

//jff 4/19/98 list of standard IWAD names
const char* const standard_iwads[] =
{
	"doom2f.wad",
	"doom2.wad",
	"plutonia.wad",
	"tnt.wad",

	"doom.wad",
	"doom1.wad",
	"doomu.wad", /* CPhipps - alow doomu.wad */

	"freedoom2.wad", /* wart@kobold.org:  added freedoom for Fedora Extras */
	"freedoom1.wad",
	"freedm.wad",

	"chex.wad",
	"chex3v.wad",
	"chex3d2.wad",

	"hacx.wad",
	"rekkrsa.wad",

	"bfgdoom2.wad",
	"bfgdoom.wad",

	"heretic.wad",
	"hexen.wad",

	"heretic1.wad"
};

// list of episode-formatted IWAD names
const char* const episode_iwads[] =
{
	"doom.wad",
	"doom1.wad",
	"doomu.wad", /* CPhipps - alow doomu.wad */

	"freedoom1.wad",

	"chex.wad",
	"rekkrsa.wad",

	"heretic.wad",

	"heretic1.wad"
};

//e6y static
const int nstandard_iwads = sizeof standard_iwads / sizeof *standard_iwads;
const int nepisode_iwads = sizeof episode_iwads / sizeof *episode_iwads;

/*
 * D_PostEvent - Event handling
 *
 * Called by I/O functions when an event is received.
 * Try event handlers for each code area in turn.
 * cph - in the true spirit of the Boom source, let the
 *  short ciruit operator madness begin!
 */

void D_PostEvent(event_t* ev)
{
	dsda_InputTrackEvent(ev);

	// Allow only sensible keys during skipping
	if(dsda_SkipMode())
	{
		if(dsda_InputActivated(InputId::Quit))
		{
			// Immediate exit if quit key is pressed in skip mode
			I_SafeExit(0);
		}
		else if(dsda_InputActivated(InputId::MenuEscape))
		{
			dsda_ExitSkipMode();
		}
		// use key is used for seeing the current frame
		else if(!dsda_InputActivated(InputId::Use) && !dsda_InputActivated(InputId::DemoSkip))
		{
			return;
		}
	}

	if(gamestate == GameState::Finale && !F_ShowCast() && F_Responder(ev))
		dsda_InputFlushTick(); // custom palette screen ate the event
	else if(M_Responder(ev))
		dsda_InputFlushTick(); // If the menu used the event, make it invisible
	else if(gamestate == GameState::Finale && F_Responder(ev))
		dsda_InputFlushTick(); // finale ate the event
	else
		G_Responder(ev);
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

static void D_Wipe()
{
	dboolean done;
	int wipestart;
	int old_game_speed = 0;

	//e6y
	if(!dsda_RenderWipeScreen() || dsda_SkipWipe())
	{
		if(!raven)
			dsda_TrackFeature(FeatureFlag::Wipescreen);

		// If there's no screen wipe, we still need to refresh the status bar
		SB_Start();
		return;
	}

	if(dsda_GameSpeed() != 100 && dsda_WipeAtFullSpeed())
	{
		old_game_speed = dsda_GameSpeed();
		dsda_UpdateGameSpeed(100);
	}

	wipestart = dsda_GetTick() - 1;

	do
	{
		int nowtime, tics;
		do
		{
			I_uSleep(5000); // CPhipps - don't thrash cpu in this loop
			nowtime = dsda_GetTick();
			tics = nowtime - wipestart;
		}
		while(!tics);

		// elim - Enable render-to-texture for GL so "melt" is rendered at same resolution as the game scene
		if(V_IsOpenGLMode())
		{
			dsda_GLLetterboxClear();
			dsda_GLStartMeltRenderTexture();
		}

		wipestart = nowtime;
		done = wipe_ScreenWipe(tics);

		// elim - Render texture to screen
		if(V_IsOpenGLMode())
		{
			dsda_GLEndMeltRenderTexture();
		}

		M_Drawer(); // menu is drawn even on top of wipes

		if(capturing_video && !dsda_SkipMode() && cap_wipescreen)
		{
			I_QueueFrameCapture();
		}

		I_FinishUpdate(); // page flip or blit buffer
	}
	while(!done);

	if(old_game_speed)
	{
		dsda_UpdateGameSpeed(old_game_speed);
	}

	force_singletics_to = gametic + BACKUPTICS;
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
GameState wipegamestate = GameState::Demoscreen;
extern dboolean setsizeneeded;

static void D_DrawPause()
{
	if(dsda_PauseMode(PauseMode::BuildMode))
		return;

	V_BeginUIDraw();

	if(hexen)
	{
		if(!netgame)
		{
			V_DrawNamePatch(160, 5, 0, "PAUSED", ColorRange::Default, PatchTranslation::Stretch);
		}
		else
		{
			V_DrawNamePatch(160, 70, 0, "PAUSED", ColorRange::Default, PatchTranslation::Stretch);
		}
	}
	else if(heretic)
		MN_DrawPause();
	else
		V_DrawNamePatch((320 - V_NamePatchWidth("M_PAUSE")) / 2, 4, 0, "M_PAUSE", ColorRange::Default, PatchTranslation::Stretch);

	V_EndUIDraw();
}

static dboolean must_fill_back_screen;

extern "C" void D_MustFillBackScreen()
{
	must_fill_back_screen = true;
}

void D_Display(fixed_t frac)
{
	static dboolean isborderstate = false;
	static dboolean borderwillneedredraw = false;
	static GameState oldgamestate = GameState::Default;
	dboolean wipe;
	dboolean viewactive = false, isborder = false;

	// e6y
	if(dsda_SkipMode())
	{
		if(HU_DrawDemoProgress(false))
			I_FinishUpdate();
		if(!dsda_InputActive(InputId::Use))
			return;

		if(V_IsOpenGLMode())
		{
			gld_PreprocessLevel();
		}
	}

	if(!dsda_SkipMode() || !dsda_InputActive(InputId::Use))
		if(nodrawers) // for comparative timing / profiling
			return;

	if(!I_StartDisplay())
		return;

	if(setsizeneeded)
	{
		// change the view size if needed
		R_ExecuteSetViewSize();
		oldgamestate = GameState::Default; // force background redraw
	}

	if(V_IsOpenGLMode() && !exclusive_fullscreen && !nodrawers)
		dsda_GLLetterboxClear();

	// save the current screen if about to wipe
	if((wipe = (gamestate != wipegamestate)))
	{
		wipe_StartScreen();
		R_ResetViewInterpolation();
	}

	if(gamestate != GameState::Level)
	{
		// Not a level
		switch(oldgamestate)
		{
			case GameState::Default:
			case GameState::Level:
				V_SetPalette(0); // cph - use default (basic) palette
			default:
				break;
		}

		V_BeginUIDraw();
		switch(gamestate)
		{
			case GameState::Intermission:
				WI_Drawer();
				break;
			case GameState::Finale:
				F_Drawer();
				break;
			case GameState::Demoscreen:
				D_PageDrawer();
				break;
			default:
				break;
		}
		V_EndUIDraw();
	}
	else
	{
		// In a level
		dboolean redrawborderstuff;

		// Work out if the player view is visible, and if there is a border
		viewactive = !inhelpscreens && !automap_solid;
		isborder = viewactive ? R_PartialView() : (!inhelpscreens && automap_full);

		if(oldgamestate != GameState::Level || must_fill_back_screen)
		{
			must_fill_back_screen = false;
			R_FillBackScreen(); // draw the pattern into the back screen
			redrawborderstuff = isborder;
		}
		else
		{
			// CPhipps -
			// If there is a border, and either there was no border last time,
			// or the border might need refreshing, then redraw it.
			redrawborderstuff = isborder && (!isborderstate || borderwillneedredraw);
			// The border may need redrawing next time if the border surrounds the screen,
			// and there is a menu being displayed
			borderwillneedredraw = menuactive != MenuActive::Inactive && isborder && viewactive;
			// e6y
			// I should do it because I call R_RenderPlayerView in all cases,
			// not only if viewactive is true
			borderwillneedredraw = borderwillneedredraw || automap_on;
		}

		if(redrawborderstuff || V_IsOpenGLMode())
		{
			// elim - Update viewport and scene offsets whenever the view is changed (user hits "-" or "+")
			if(redrawborderstuff && V_IsOpenGLMode())
			{
				dsda_GLSetRenderViewportParams();
			}

			R_DrawViewBorder();
		}

		// elim - If we go from visible status bar to invisible status bar, update affected viewport params
		if(!isborder && isborderstate)
		{
			dsda_GLUpdateStatusBarVisible();
		}

		// e6y
		// Boom colormaps should be applied for everything in R_RenderPlayerView
		use_boom_cm = true;

		if(frac < 0)
			frac = I_GetTimeFrac();

		R_InterpolateView(&players[displayplayer], frac);

		DSDA_ADD_CONTEXT(SignalContext::PlayerView);
		R_RenderPlayerView(&players[displayplayer]);
		DSDA_REMOVE_CONTEXT(SignalContext::PlayerView);

		dsda_UpdateRenderStats();

		// e6y
		// but should NOT be applied for automap, statusbar and HUD
		use_boom_cm = false;
		frame_fixedcolormap = 0;

		if(automap_full)
		{
			AM_Drawer(false);
		}

		R_RestoreInterpolations();

		DSDA_ADD_CONTEXT(SignalContext::StatusBar);
		ST_Drawer(redrawborderstuff || BorderNeedRefresh);
		DSDA_REMOVE_CONTEXT(SignalContext::StatusBar);

		BorderNeedRefresh = false;
		if(V_IsSoftwareMode())
			R_DrawViewBorder();

		DSDA_ADD_CONTEXT(SignalContext::Hud);
		HU_Drawer();
		DSDA_REMOVE_CONTEXT(SignalContext::Hud);
	}

	isborderstate = isborder;
	oldgamestate = wipegamestate = gamestate;

	// draw pause pic
	if(dsda_Paused() && (menuactive != MenuActive::Full))
	{
		D_DrawPause();
	}

	V_BeginMenuDraw();
	if(M_MenuIsShaded())
		M_ShadedScreen(0);
	V_EndMenuDraw();

	// menus go directly to the screen
	M_Drawer(); // menu is drawn even on top of everything

	FakeNetUpdate(); // send out any new accumulation

	HU_DrawDemoProgress(true); //e6y

	// normal update
	if(!wipe)
		I_FinishUpdate(); // page flip or blit buffer
	else
	{
		// wipe update
		wipe_EndScreen();
		D_Wipe();
	}

	// e6y
	// Don't thrash cpu during pausing or if the window doesnt have focus
	if(dsda_CameraPaused())
	{
		I_uSleep(5000);
	}

	dsda_LimitFPS();

	I_EndDisplay();
}

//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//

static void D_DoomLoop()
{
	if(dsda_IntConfig(ConfigId::StartupDelayMs) > 0)
		I_uSleep(dsda_IntConfig(ConfigId::StartupDelayMs) * 1000);

	for(;;)
	{
		if(I_Interrupted())
			I_SafeExit(0);

		WasRenderedInTryRunTics = false;
		// frame syncronous IO operations
		I_StartFrame();

		// process one or more tics
		if(singletics)
		{
			I_StartTic();
			G_BuildTiccmd(&local_cmds[consoleplayer][maketic % BACKUPTICS]);
			if(advancedemo)
				D_DoAdvanceDemo();
			M_Ticker();
			G_Ticker();
			gametic++;
			maketic++;
		}
		else
			TryRunTics(); // will run at least one tic

		// killough 3/16/98: change consoleplayer to displayplayer
		if(players[displayplayer].mo) // cph 2002/08/10
			S_UpdateSounds();         // move positional sounds

		// Update display, next frame, with current state.
		if(!movement_smooth || !WasRenderedInTryRunTics || gamestate != wipegamestate)
		{
			// NSM
			if(capturing_video && !dsda_SkipMode())
			{
				dboolean first = true;
				int cap_step = TICRATE * FRACUNIT / cap_fps;
				cap_frac += cap_step;
				while(cap_frac <= FRACUNIT)
				{
					isExtraDDisplay = !first;
					first = false;

					if(gamestate == wipegamestate || cap_wipescreen)
					{
						I_QueueFrameCapture();
					}

					D_Display(cap_frac);

					isExtraDDisplay = false;
					cap_frac += cap_step;
				}
				cap_frac -= FRACUNIT + cap_step;
			}
			else
			{
				D_Display(-1);
			}
		}
	}
}

//
//  DEMO LOOP
//

static int demosequence; // killough 5/2/98: made static
static int pagetic;
static const char* pagename; // CPhipps - const
dboolean bfgedition = 0;
dboolean freedm = 0;

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker()
{
	if(--pagetic < 0)
		D_AdvanceDemo();
}

static dboolean dsda_IsBlankPWADLump(const char* lumpname)
{
	if(!W_PWADLumpNameExists(lumpname))
		return false;

	return lumpinfo[W_CheckNumForName(lumpname)].size == 0;
}

// Check whether to skip IWAD Demos
static dboolean dsda_SimpleDemoLoop()
{
	int pwaddemos = W_PWADLumpNameExists2("DEMO1");
	int pwadmaps = W_PWADMapsExist();

	if((pwadmaps && !pwaddemos) || dsda_IsBlankPWADLump("DEMO1"))
		return true;

	return false;
}

static dboolean dsda_ForcePWADCredit()
{
	if(W_PWADLumpNameExists("CREDIT"))
	{
		// Simple demo loop and PWAD CREDIT
		if(dsda_SimpleDemoLoop())
			return true;

		// Normal demo loop
		// Check if CREDIT is shown twice in demoloop, else skip dynamic credits
		// Fixes condition with PWAD CREDIT + REAL DEMO1 + BLANK DEMO2
		if(W_PWADLumpNameExists2("DEMO1") && dsda_IsBlankPWADLump("DEMO2"))
			return true;
	}

	return false;
}

//
// D_PageDrawer
//
static void D_PageDrawer()
{
	if(raven)
	{
		V_DrawRawScreen(pagename);
		if(demosequence == 1)
		{
			V_DrawNamePatch(4, 160, 0, "ADVISOR", ColorRange::Default, PatchTranslation::Stretch);
		}
		return;
	}

	// Allows use of PWAD HELP2 screen in demosequence
	if(demosequence == 4 && pwad_help2_check)
		pagename = "HELP2";

	// proff/nicolas 09/14/98 -- now stretchs bitmaps to fullscreen!
	// CPhipps - updated for new patch drawing
	// proff - added M_DrawCredits
	if(pagename)
	{
		// e6y: wide-res
		V_ClearBorder();
		V_DrawNamePatchFS(0, 0, 0, pagename, ColorRange::Default, PatchTranslation::Stretch);
	}
	else if(dsda_ForcePWADCredit())
		M_DrawCredits();
	else
		M_DrawCreditsDynamic();
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo()
{
	advancedemo = true;
}

/* killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char* name)
{
	if((bfgedition) && name && !strncmp(name, "TITLEPIC", 8))
		pagename = "DMENUPIC";
	else
		pagename = name;
}

void D_SetPage(const char* name, int tics, MusicId music)
{
	if(music != MusicId::None)
		S_StartMusic(music);

	if(tics)
		pagetic = tics;

	D_SetPageName(name);
}

static void D_DrawTitle1(const char* name)
{
	D_SetPage(name, TICRATE * 170 / 35, MusicId::Intro);
}

static void D_DrawTitle2(const char* name)
{
	D_SetPage(name, 0, MusicId::Dm2ttl);
}

/* killough 11/98: tabulate demo sequences
 */

extern const demostate_t (*demostates)[4];

extern const demostate_t doom_demostates[][4] =
{
	{
		{D_DrawTitle1, "TITLEPIC"},
		{D_DrawTitle1, "TITLEPIC"},
		{D_DrawTitle2, "TITLEPIC"},
		{D_DrawTitle1, "TITLEPIC"},
	},

	{
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
	},

	{
		{D_SetPageName, nullptr},
		{D_SetPageName, nullptr},
		{D_SetPageName, nullptr},
		{D_SetPageName, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
	},

	{
		{D_SetPageName, "HELP2"},
		{D_SetPageName, "HELP2"},
		{D_SetPageName, "CREDIT"},
		{D_DrawTitle1, "TITLEPIC"},
	},

	{
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
	},

	{
		{nullptr},
		{nullptr},
		// e6y
		// Both Plutonia and TNT are commercial like Doom2,
		// but in difference from  Doom2, they have demo4 in demo cycle.
		{G_DeferedPlayDemo, "demo4"},
		{D_SetPageName, "CREDIT"},
	},

	{
		{nullptr},
		{nullptr},
		{nullptr},
		{G_DeferedPlayDemo, "demo4"},
	},

	{
		{nullptr},
		{nullptr},
		{nullptr},
		{nullptr},
	}
};

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo()
{
	players[consoleplayer].playerstate = PlayerState::Live; /* not reborn */
	advancedemo = false;
	dsda_ResetPauseMode();
	gameaction = GameAction::Nothing;

	pagetic = TICRATE * 11; /* killough 11/98: default behavior */
	gamestate = GameState::Demoscreen;

	if(netgame && !demoplayback)
		demosequence = 0;
	else if(!demostates[++demosequence][std::to_underlying(gamemode)].func)
		demosequence = 0;

	// do not even attempt to play DEMO4 if it is not available
	if(demosequence == 6 && gamemode == GameMode::Commercial && !W_LumpNameExists("demo4"))
		demosequence = 0;

	if(dsda_SimpleDemoLoop())
	{
		// Skip blank / IWAD demos in PWADs
		if(demostates[demosequence][std::to_underlying(gamemode)].func == G_DeferedPlayDemo)
			demosequence++;

		// Limit to just TITLEPIC / CREDIT
		if(demosequence > (raven ? 3 : 2))
			demosequence = 0;
	}

	demostates[demosequence][std::to_underlying(gamemode)].func(demostates[demosequence][std::to_underlying(gamemode)].name);
}

//
// D_StartTitle
//
void D_StartTitle()
{
	gameaction = GameAction::Nothing;
	in_game = false;
	demosequence = -1;
	D_AdvanceDemo();
}

//
// D_AddFile
//
// Rewritten by Lee Killough
//
// Ty 08/29/98 - add source parm to indicate where this came from
// CPhipps - static, const char* parameter
//         - source is an enum
//         - modified to allocate & use new wadfiles array
void D_AddFile(const char* file, WadSource source)
{
	int len;

	// There can only be one iwad source!
	if(source == WadSource::Iwad)
	{
		int i;

		for(i = 0; i < numwadfiles; ++i)
			if(wadfiles[i].src == WadSource::Iwad)
				wadfiles[i].src = WadSource::Skip;
	}

	wadfiles = static_cast<wadfile_info_t*>(Z_Realloc(wadfiles, sizeof(*wadfiles) * (numwadfiles + 1)));
	wadfiles[numwadfiles].name =
		AddDefaultExtension(strcpy(static_cast<char*>(Z_Malloc(strlen(file) + 5)), file), ".wad");
	wadfiles[numwadfiles].src = source; // Ty 08/29/98
	wadfiles[numwadfiles].handle = 0;

	// No Rest For The Living
	len = strlen(wadfiles[numwadfiles].name);
	if(len >= 9 && !strnicmp(wadfiles[numwadfiles].name + len - 9, "nerve.wad", 9))
		gamemission = GameMission::PackNerve;

	numwadfiles++;
}

// killough 10/98: support -dehout filename
// cph - made const, don't cache results
//e6y static
const char* D_dehout()
{
	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::Dehout);

	return arg->found ? arg->value.v_string : nullptr;
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static
//e6y static
void CheckIWAD(const char* iwadname, GameMode* gmode, dboolean* hassec)
{
	if(M_ReadAccess(iwadname))
	{
		int ud = 0, rg = 0, sw = 0, cm = 0, sc = 0, hx = 0;
		dboolean dmenupic = false;
		dboolean large_titlepic = false;
		dboolean freedm_lmp = false;
		FILE* fp;

		// Identify IWAD correctly
		if((fp = M_OpenFile(iwadname, "rb")))
		{
			wadinfo_t header;

			// read IWAD header
			if(fread(&header, sizeof(header), 1, fp) == 1)
			{
				size_t length;
				filelump_t* fileinfo;

				if(strncmp(header.identification, "IWAD", 4)) // missing IWAD tag in header
				{
					Log::Warn("CheckIWAD: IWAD tag {} not present\n", iwadname);
				}

				// read IWAD directory
				header.numlumps = LittleLong(header.numlumps);
				header.infotableofs = LittleLong(header.infotableofs);
				length = header.numlumps;
				fileinfo = static_cast<filelump_t*>(Z_Malloc(length * sizeof(filelump_t)));
				if(fseek(fp, header.infotableofs, SEEK_SET) ||
					fread(fileinfo, sizeof(filelump_t), length, fp) != length)
				{
					fclose(fp);
					Log::Fatal("CheckIWAD: failed to read directory {}", iwadname);
				}

				// scan directory for levelname lumps
				while(length--)
				{
					if(fileinfo[length].name[0] == 'E' &&
						fileinfo[length].name[2] == 'M' &&
						fileinfo[length].name[4] == 0)
					{
						if(fileinfo[length].name[1] == '4')
							++ud;
						else if(fileinfo[length].name[1] == '3')
							++rg;
						else if(fileinfo[length].name[1] == '2')
							++rg;
						else if(fileinfo[length].name[1] == '1')
							++sw;
					}
					else if(fileinfo[length].name[0] == 'M' &&
						fileinfo[length].name[1] == 'A' &&
						fileinfo[length].name[2] == 'P' &&
						fileinfo[length].name[5] == 0)
					{
						++cm;
						if(fileinfo[length].name[3] == '3')
							if(fileinfo[length].name[4] == '1' ||
								fileinfo[length].name[4] == '2')
								++sc;
					}

					if(!strncmp(fileinfo[length].name, "DMENUPIC", 8))
						dmenupic = true;
					if(!strncmp(fileinfo[length].name, "TITLEPIC", 8) && fileinfo[length].size > 68168)
						large_titlepic = true;
					if(!strncmp(fileinfo[length].name, "HACX", 4))
						hx++;
					if(!strncmp(fileinfo[length].name, "FREEDM", 6))
						freedm_lmp = true;
				}
				Z_Free(fileinfo);
			}

			fclose(fp);
		}
		else // error from open call
			Log::Fatal("CheckIWAD: Can't open IWAD {}", iwadname);

		// unity iwad has dmenupic and a large titlepic
		if(dmenupic && !large_titlepic)
			bfgedition++;
		if(freedm_lmp)
			freedm++;

		// Determine game mode from levels present
		// Must be a full set for whichever mode is present
		// Lack of wolf-3d levels also detected here

		*gmode = GameMode::Indetermined;
		*hassec = false;
		if(cm >= 30 || (cm >= 20 && hx))
		{
			*gmode = GameMode::Commercial;
			*hassec = sc >= 2;
		}
		else if(ud >= 9)
			*gmode = GameMode::Retail;
		else if(rg >= 18)
			*gmode = GameMode::Registered;
		else if(sw >= 9)
			*gmode = GameMode::Shareware;
	}
	else // error from access call
		Log::Fatal("CheckIWAD: IWAD {} not readable", iwadname);
}

//
// AddIWAD
//
void AddIWAD(const char* iwad)
{
	size_t i;

	if(!(iwad && *iwad))
		return;

	//jff 9/3/98 use logical output routine
	Log::Debug("IWAD found: {}\n", iwad); //jff 4/20/98 print only if found
	CheckIWAD(iwad, &gamemode, &haswolflevels);

	/* jff 8/23/98 set gamemission global appropriately in all cases
	* cphipps 12/1999 - no version output here, leave that to the caller
	*/
	i = strlen(iwad);

	if(i >= 11 && !strnicmp(iwad + i - 11, "heretic.wad", 11))
	{
		if(!dsda_Flag(ArgId::Heretic))
			dsda_UpdateFlag(ArgId::Heretic, true);
	}

	if(i >= 9 && !strnicmp(iwad + i - 9, "hexen.wad", 9))
	{
		if(!dsda_Flag(ArgId::Hexen))
			dsda_UpdateFlag(ArgId::Hexen, true);

		gamemode = GameMode::Commercial;
		haswolflevels = false;
	}

	if(i >= 12 && !strnicmp(iwad + i - 12, "heretic1.wad", 12))
	{
		if(!dsda_Flag(ArgId::Heretic))
			dsda_UpdateFlag(ArgId::Heretic, true);

		gamemode = GameMode::Shareware;
	}

	switch(gamemode)
	{
		case GameMode::Retail:
		case GameMode::Registered:
		case GameMode::Shareware:
			gamemission = GameMission::Doom;
			if(i >= 8 && !strnicmp(iwad + i - 8, "chex.wad", 8))
				gamemission = GameMission::TcChex;
			else if(i >= 10 && !strnicmp(iwad + i - 10, "chex3v.wad", 10))
				gamemission = GameMission::TcChex3v;
			else if(i >= 11 && !strnicmp(iwad + i - 11, "rekkrsa.wad", 11))
				gamemission = GameMission::TcRekkr;
			else if(i >= 13 && !strnicmp(iwad + i - 13, "freedoom1.wad", 13))
				gamemission = GameMission::TcFreedoom;
			break;
		case GameMode::Commercial:
			gamemission = GameMission::Doom2;
			if(i >= 10 && !strnicmp(iwad + i - 10, "doom2f.wad", 10))
				language = Language::French;
			else if(i >= 7 && !strnicmp(iwad + i - 7, "tnt.wad", 7))
				gamemission = GameMission::PackTnt;
			else if(i >= 12 && !strnicmp(iwad + i - 12, "plutonia.wad", 12))
				gamemission = GameMission::PackPlut;
			else if(i >= 11 && !strnicmp(iwad + i - 11, "chex3d2.wad", 11))
				gamemission = GameMission::TcChex3v;
			else if(i >= 8 && !strnicmp(iwad + i - 8, "hacx.wad", 8))
				gamemission = GameMission::TcHacx;
			else if((i >= 13 && !strnicmp(iwad + i - 13, "freedoom2.wad", 13))
				|| (i >= 10 && !strnicmp(iwad + i - 10, "freedm.wad", 10)))
				gamemission = GameMission::TcFreedoom;
			break;
		default:
			gamemission = GameMission::None;
			break;
	}
	if(gamemode == GameMode::Indetermined)
		//jff 9/3/98 use logical output routine
		Log::Warn("Unknown Game Version, may not work\n");

	// Set up TC game logic
	tc_game = (gamemission > GameMission::PackNerve);

	D_AddFile(iwad, WadSource::Iwad);
}

/*
 * FindIWADFIle
 *
 * Search for one of the standard IWADs
 * CPhipps  - static, proper prototype
 *    - 12/1999 - rewritten to use I_FindFile
 */
static inline dboolean CheckExeSuffix(const char* suffix)
{
	extern char** dsda_argv;

	char* dash;

	if((dash = strrchr(dsda_argv[0], '-')))
		if(!strnicmp(dash, suffix, strlen(suffix)))
			return true;

	return false;
}

static char* FindIWADFile()
{
	int i;
	dsda_arg_t* arg;
	char* iwad = nullptr;
	int iwadnum = EpisodeStructure ? nepisode_iwads : nstandard_iwads;

	if(CheckExeSuffix("-heretic"))
	{
		if(!dsda_Flag(ArgId::Heretic))
			dsda_UpdateFlag(ArgId::Heretic, true);
	}
	else if(CheckExeSuffix("-hexen"))
	{
		if(!dsda_Flag(ArgId::Hexen))
			dsda_UpdateFlag(ArgId::Hexen, true);
	}

	arg = dsda_Arg(ArgId::Iwad);
	if(arg->found)
	{
		iwad = I_FindWad(arg->value.v_string);
	}
	else
	{
		if(dsda_Flag(ArgId::Heretic))
			return I_FindWad("heretic.wad");
		else if(dsda_Flag(ArgId::Hexen))
			return I_FindWad("hexen.wad");

		if(iwadlump != nullptr)
			return I_FindWad(iwadlump);

		for(i = 0; !iwad && i < iwadnum; i++)
			iwad = EpisodeStructure ? I_FindWad(episode_iwads[i]) : I_FindWad(standard_iwads[i]);
	}
	return iwad;
}

static dboolean FileMatchesIWAD(const char* name)
{
	int i;
	const char* base_name = dsda_BaseName(name);

	for(i = 0; i < nstandard_iwads; ++i)
	{
		if(!stricmp(base_name, standard_iwads[i]))
			return true;
	}

	return false;
}

//
// DoLooseFiles
//
// Take any file names on the command line before the first switch parm
// and insert the appropriate -file, -deh or -playdemo switch in front
// of them.
//
// e6y
// Fixed crash if numbers of wads/lmps/dehs is greater than 100
// Fixed bug when length of argname is smaller than 3
// Refactoring of the code to avoid use the static arrays
// The logic of DoLooseFiles has been rewritten in more optimized style
// MAXARGVS has been removed.

static void DoLooseFiles()
{
	extern int dsda_argc;
	extern char** dsda_argv;

	int i, k;
	const int loose_wad_index = 0;

	struct
	{
		const char* ext;
		ArgId arg_id;
	} looses[] = {
		{".wad", ArgId::File},
		{".zip", ArgId::File},
		{".lmp", ArgId::Playdemo},
		{".deh", ArgId::Deh},
		{".bex", ArgId::Deh},
		// assume wad if no extension or length of the extention is not equal to 3
		// must be last entry
		{"", ArgId::File},
		{nullptr}
	};

	for(i = 1; i < dsda_argc; i++)
	{
		size_t arglen, extlen;

		if(*dsda_argv[i] == '-') break; // quit at first switch

		// so now we must have a loose file.  Find out what kind and store it.
		arglen = strlen(dsda_argv[i]);

		for(k = 0; looses[k].ext; ++k)
		{
			extlen = strlen(looses[k].ext);
			if(arglen >= extlen && !stricmp(&dsda_argv[i][arglen - extlen], looses[k].ext))
			{
				// If a wad is an iwad, we don't want to send it to -file
				if(k == loose_wad_index && FileMatchesIWAD(dsda_argv[i]))
				{
					dsda_UpdateStringArg(ArgId::Iwad, dsda_argv[i]);
					break;
				}

				dsda_AppendStringArg(looses[k].arg_id, dsda_argv[i]);
				break;
			}
		}
	}
}

const char* port_wad_file;

// CPhipps - misc screen stuff
int desired_screenwidth, desired_screenheight;

// Calculate the path to the directory for autoloaded WADs/DEHs.
// Creates the directory as necessary.

static char* autoload_path = nullptr;

static char* GetAutoloadDir(const char* iwadname, dboolean createdir)
{
	char* result;
	int len;

	if(autoload_path == nullptr)
	{
		const char* configdir = I_ConfigDir();
		len = snprintf(nullptr, 0, "%s/autoload", configdir);
		autoload_path = static_cast<char*>(Z_Malloc(len + 1));
		snprintf(autoload_path, len + 1, "%s/autoload", configdir);
	}

	M_MakeDir(autoload_path, false);

	len = snprintf(nullptr, 0, "%s/%s", autoload_path, iwadname);
	result = static_cast<char*>(Z_Malloc(len + 1));
	snprintf(result, len + 1, "%s/%s", autoload_path, iwadname);

	if(createdir)
	{
		M_MakeDir(result, false);
	}

	return result;
}

const char* IWADBaseName()
{
	int i;

	for(i = 0; i < numwadfiles; i++)
	{
		if(wadfiles[i].src == WadSource::Iwad)
			break;
	}

	if(i == numwadfiles)
		Log::Fatal("IWADBaseName: IWAD not found\n");

	return dsda_BaseName(wadfiles[i].name);
}

typedef struct
{
	char** list;
	int count;
	int allocated_count;
} deh_queue_t;

static deh_queue_t autoload_deh_all_queue;
static deh_queue_t autoload_deh_game_queue;
static deh_queue_t autoload_deh_iwad_queue;
static deh_queue_t* autoload_deh_pwad_queue;
static int autoload_deh_pwad_count;

static void D_QueueAutoloadDeh(deh_queue_t* queue, const char* name)
{
	int old_count;

	old_count = queue->count;
	++queue->count;

	if(queue->count > queue->allocated_count)
	{
		if(!queue->allocated_count)
			queue->allocated_count = 4;

		while(queue->count > queue->allocated_count)
			queue->allocated_count *= 2;

		queue->list = static_cast<decltype(queue->list)>(Z_Realloc(queue->list, queue->allocated_count * sizeof(*queue->list)));
		memset(&queue->list[old_count], 0, (queue->allocated_count - old_count) * sizeof(*queue->list));
	}

	queue->list[queue->count - 1] = Z_Strdup(name);
}

static void D_ProcessDehAutoloadQueue(deh_queue_t* queue)
{
	int i;

	for(i = 0; i < queue->count; ++i)
	{
		ProcessDehFile(queue->list[i], D_dehout(), 0);
		Z_Free(queue->list[i]);
	}

	Z_Free(queue->list);
}

// A dangling symlink or a file without read permission still matches a glob.
// Warn and skip it, rather than let it end the game (see Compatibility.md).
static bool IsReadableOrWarn(const char* filename)
{
	if(M_ReadAccess(filename))
		return true;

	Log::Warn("Skipping unreadable {}\n", filename);
	return false;
}

// Load all WAD files from the given directory.

static void LoadWADsAtPath(const char* path, WadSource source)
{
	glob_t* glob;
	const char* filename;

	glob = I_StartMultiGlob(path, GlobFlag::NoCase | GlobFlag::Sorted,
		"*.wad", "*.lmp", nullptr);
	for(;;)
	{
		filename = I_NextGlob(glob);
		if(filename == nullptr)
		{
			break;
		}

		if(!IsReadableOrWarn(filename))
			continue;

		D_AddFile(filename, source);
	}

	I_EndGlob(glob);
}

static void LoadDehackedFilesAtPath(const char* path, dboolean defer_loading, deh_queue_t* deh_queue)
{
	const char* filename;
	glob_t* glob;

	glob = I_StartMultiGlob(path, GlobFlag::NoCase | GlobFlag::Sorted,
		"*.deh", "*.bex", nullptr);
	for(;;)
	{
		filename = I_NextGlob(glob);
		if(filename == nullptr)
		{
			break;
		}

		if(deh_queue)
		{
			D_QueueAutoloadDeh(deh_queue, filename);
		}
		else if(defer_loading)
		{
			dsda_AppendStringArg(ArgId::Deh, filename);
		}
		else
		{
			ProcessDehFile(filename, D_dehout(), 0);
		}
	}

	I_EndGlob(glob);
}

static void D_AddZip(const char* zipped_file_name, WadSource source, deh_queue_t* deh_queue)
{
	char* full_zip_path;
	const char* temporary_directory;

	full_zip_path = I_RequireZip(zipped_file_name);
	temporary_directory = dsda_UnzipFile(full_zip_path);

	LoadWADsAtPath(temporary_directory, source);
	if(MainLumpCache)
		LoadDehackedFilesAtPath(temporary_directory, true, deh_queue);

	Z_Free(full_zip_path);
}

static void D_AddUnzippedFile(const char* zipped_file_name, WadSource source, deh_queue_t* deh_queue)
{
	char* full_zip_path;
	const char* temporary_directory;

	full_zip_path = I_RequireZip(zipped_file_name);
	temporary_directory = dsda_ReadUnzippedFile(full_zip_path);

	LoadWADsAtPath(temporary_directory, source);
	LoadDehackedFilesAtPath(temporary_directory, true, deh_queue);

	Z_Free(full_zip_path);
}

static void LoadZIPsAtPath(const char* path, WadSource source, deh_queue_t* deh_queue)
{
	glob_t* glob;
	const char* filename;

	glob = I_StartMultiGlob(path, GlobFlag::NoCase | GlobFlag::Sorted,
		"*.zip", nullptr);
	for(;;)
	{
		filename = I_NextGlob(glob);
		if(filename == nullptr)
		{
			break;
		}

		if(!IsReadableOrWarn(filename))
			continue;

		D_AddZip(filename, source, deh_queue);
	}

	I_EndGlob(glob);
}

static const char* D_AutoLoadGameBase()
{
	return hexen
		? "hexen-all"
		: heretic
		? "heretic-all"
		: (gamemission == GameMission::TcChex ||
			gamemission == GameMission::TcChex3v)
		? "chex-all"
		: (gamemission == GameMission::TcFreedoom)
		? "freedoom-all"
		: !tc_game
		? "doom-all"
		: nullptr;
}

#define ALL_AUTOLOAD "all-all"

// auto-loading of .wad files.

void D_AutoloadIWadDir()
{
	char* autoload_dir;

	// common auto-loaded files for all games
	autoload_dir = GetAutoloadDir(ALL_AUTOLOAD, true);
	LoadWADsAtPath(autoload_dir, WadSource::AutoLoad);
	LoadZIPsAtPath(autoload_dir, WadSource::AutoLoad, &autoload_deh_all_queue);
	Z_Free(autoload_dir);

	if(D_AutoLoadGameBase())
	{
		// common auto-loaded files for the game
		autoload_dir = GetAutoloadDir(D_AutoLoadGameBase(), true);
		LoadWADsAtPath(autoload_dir, WadSource::AutoLoad);
		LoadZIPsAtPath(autoload_dir, WadSource::AutoLoad, &autoload_deh_game_queue);
		Z_Free(autoload_dir);
	}

	// auto-loaded files per IWAD
	autoload_dir = GetAutoloadDir(IWADBaseName(), true);
	LoadWADsAtPath(autoload_dir, WadSource::AutoLoad);
	LoadZIPsAtPath(autoload_dir, WadSource::AutoLoad, &autoload_deh_iwad_queue);
	Z_Free(autoload_dir);
}

static void D_AutoloadPWadDir()
{
	int i;

	autoload_deh_pwad_count = numwadfiles;
	autoload_deh_pwad_queue = static_cast<deh_queue_t*>(Z_Calloc(autoload_deh_pwad_count, sizeof(*autoload_deh_pwad_queue)));

	for(i = 0; i < numwadfiles; ++i)
		if(wadfiles[i].src == WadSource::Pwad)
		{
			char* autoload_dir;
			autoload_dir = GetAutoloadDir(dsda_BaseName(wadfiles[i].name), false);
			LoadWADsAtPath(autoload_dir, WadSource::PwadAutoLoad);
			LoadZIPsAtPath(autoload_dir, WadSource::PwadAutoLoad, &autoload_deh_pwad_queue[i]);
			Z_Free(autoload_dir);
		}
}

// auto-loading of .deh files.

static void D_AutoloadDehIWadDir()
{
	char* autoload_dir;

	// common auto-loaded files for all games
	autoload_dir = GetAutoloadDir(ALL_AUTOLOAD, true);
	LoadDehackedFilesAtPath(autoload_dir, false, nullptr);
	D_ProcessDehAutoloadQueue(&autoload_deh_all_queue);
	Z_Free(autoload_dir);

	if(D_AutoLoadGameBase())
	{
		// common auto-loaded files for the game
		autoload_dir = GetAutoloadDir(D_AutoLoadGameBase(), true);
		LoadDehackedFilesAtPath(autoload_dir, false, nullptr);
		D_ProcessDehAutoloadQueue(&autoload_deh_game_queue);
		Z_Free(autoload_dir);
	}

	// auto-loaded files per IWAD
	autoload_dir = GetAutoloadDir(IWADBaseName(), true);
	LoadDehackedFilesAtPath(autoload_dir, false, nullptr);
	D_ProcessDehAutoloadQueue(&autoload_deh_iwad_queue);
	Z_Free(autoload_dir);
}

static void D_AutoloadDehPWadDir()
{
	int i;
	for(i = 0; i < numwadfiles; ++i)
		if(wadfiles[i].src == WadSource::Pwad)
		{
			char* autoload_dir;
			autoload_dir = GetAutoloadDir(dsda_BaseName(wadfiles[i].name), false);
			LoadDehackedFilesAtPath(autoload_dir, false, nullptr);
			if(i < autoload_deh_pwad_count)
				D_ProcessDehAutoloadQueue(&autoload_deh_pwad_queue[i]);
			Z_Free(autoload_dir);
		}

	Z_Free(autoload_deh_pwad_queue);
}

int warpepisode = -1;
int warpmap = -1;

static void HandleWarp()
{
	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::Warp);

	if(arg->found)
	{
		autostart = true; // Ty 08/29/98 - move outside the decision tree

		dsda_ResolveWarp(arg->value.v_int_array, arg->count, &warpepisode, &warpmap);

		if(warpmap == -1)
			dsda_FirstMap(&warpepisode, &warpmap);

		startmap = warpmap;
		startepisode = warpepisode;
	}
}

static void HandleClass()
{
	int p;
	dsda_arg_t* arg;
	int player_class = std::to_underlying(PClass::Fighter);

	if(!hexen) return;

	arg = dsda_Arg(ArgId::Class);
	if(arg->found)
		player_class = arg->value.v_int + std::to_underlying(PClass::Fighter);

	if(
		player_class != std::to_underlying(PClass::Fighter) &&
		player_class != std::to_underlying(PClass::Cleric) &&
		player_class != std::to_underlying(PClass::Mage)
	)
		player_class = std::to_underlying(PClass::Fighter);

	PlayerClass[0] = static_cast<PClass>(player_class);
	for(p = 1; p < MAX_MAXPLAYERS; p++)
		PlayerClass[p] = PClass::Fighter;

	randomclass = dsda_Flag(ArgId::Randclass);
}

static void HandlePlayback()
{
	const char* file;

	file = dsda_ParsePlaybackOptions();

	if(!file)
		return;

	dsda_LoadExDemo(file);
}

const char* doomverstr = "Unknown";

static void EvaluateDoomVerStr()
{
	if(heretic)
	{
		if(gamemode == GameMode::Retail)
			doomverstr = "Heretic: Shadow of the Serpent Riders";
		else if(gamemode == GameMode::Shareware)
			doomverstr = "Heretic Shareware";
		else
			doomverstr = "Heretic";
	}
	else if(hexen)
	{
		doomverstr = "Hexen";
	}
	else
	{
		switch(gamemode)
		{
			case GameMode::Retail:
				switch(gamemission)
				{
					case GameMission::TcChex:
						doomverstr = "Chex(R) Quest";
						break;
					case GameMission::TcChex3v:
						doomverstr = "Chex(R) Quest 3: Vanilla Edition";
						break;
					case GameMission::TcRekkr:
						doomverstr = "REKKR";
						break;
					case GameMission::TcFreedoom:
						doomverstr = "Freedoom Phase 1";
						break;
					default:
						doomverstr = "The Ultimate DOOM";
						break;
				}
				break;
			case GameMode::Shareware:
				doomverstr = "DOOM Shareware";
				break;
			case GameMode::Registered:
				doomverstr = "DOOM Registered";
				break;
			case GameMode::Commercial: // Ty 08/27/98 - fixed gamemode vs gamemission
				switch(gamemission)
				{
					case GameMission::PackPlut:
						doomverstr = "Final DOOM - The Plutonia Experiment";
						break;
					case GameMission::PackTnt:
						doomverstr = "Final DOOM - TNT: Evilution";
						break;
					case GameMission::TcChex3v:
						doomverstr = "Chex(R) Quest 3: Modding Edition";
						break;
					case GameMission::TcHacx:
						doomverstr = "HACX - Twitch 'n Kill";
						break;
					case GameMission::TcFreedoom:
						doomverstr = freedm ? "FreeDM" : "Freedoom Phase 2";
						break;
					default:
						doomverstr = "DOOM 2: Hell on Earth";
						break;
				}
				break;
			default:
				doomverstr = "Public DOOM";
				break;
		}
	}

	if(bfgedition)
	{
		char* tempverstr;
		const char bfgverstr[] = " (BFG Edition)";
		tempverstr = static_cast<char*>(Z_Malloc(sizeof(char) * (strlen(doomverstr) + strlen(bfgverstr) + 1)));
		strcpy(tempverstr, doomverstr);
		strcat(tempverstr, bfgverstr);
		doomverstr = Z_Strdup(tempverstr);
		Z_Free(tempverstr);
	}

	/* cphipps - the main display. This shows the copyright and game type */
	Log::Info("{} is released under the GNU General Public license v2.0.\n"
		"You are welcome to redistribute it under certain conditions.\n"
		"It comes with ABSOLUTELY NO WARRANTY. See the file COPYING for details.\n\n",
		PROJECT_NAME);

	Log::Info("Playing: {}\n", doomverstr);
}

static void dsda_Loadfiles()
{
	dsda_arg_t* arg;

	if((arg = dsda_Arg(ArgId::File))->found)
	{
		int file_i;
		// the parms after p are wadfile/lump names,
		// until end of parms or another - preceded parm
		modifiedgame = true; // homebrew levels

		for(file_i = 0; file_i < arg->count; ++file_i)
		{
			const char* file_name;
			char* file = nullptr;

			file_name = arg->value.v_string_array[file_i];

			if(!dsda_FileExtension(file_name))
			{
				const char* extensions[] = {".wad", ".lmp", ".zip", ".deh", ".bex", nullptr};

				file = I_RequireAnyFile(file_name, extensions);
				file_name = file;
			}

			if(dsda_HasFileExt(file_name, ".deh") || dsda_HasFileExt(file_name, ".bex"))
			{
				if(MainLumpCache)
					dsda_AppendStringArg(ArgId::Deh, file_name);
			}
			else if(dsda_HasFileExt(file_name, ".zip"))
			{
				if(dsda_Arg(ArgId::Iwad)->found)
					D_AddZip(file_name, WadSource::Pwad, nullptr);
				else
					MainLumpCache ? D_AddUnzippedFile(file_name, WadSource::Pwad, nullptr) : D_AddZip(file_name, WadSource::Pwad, nullptr);
			}
			else if(dsda_HasFileExt(file_name, ".wad") || dsda_HasFileExt(file_name, ".lmp"))
			{
				if(!file)
					file = I_RequireWad(file_name);

				D_AddFile(file, WadSource::Pwad);
			}
			else
			{
				Log::Fatal("File type \"{}\" is not supported", dsda_FileExtension(file_name));
			}

			Z_Free(file);
		}
	}
}

//
// DetectEpisodeStructure
//
// If GAMEINFO is not present but Doom 1 maps are found, autoload episode IWAD.
//

static void dsda_DetectEpisodeStructure()
{
	int i, ii;
	char lump[5];
	char* iwad;

	for(i = 0; i < 10; i++)
	{
		for(ii = 0; ii < 10; ii++)
		{
			snprintf(lump, sizeof(lump), "E%dM%d", i, ii);
			if(W_LumpNameExists(lump))
			{
				EpisodeStructure = true;
				iwadver = Z_Strdup(lump);
				iwad = FindIWADFile();

				if(iwad)
					iwadlump = Z_Strdup(iwad);

				return;
			}
		}
	}
}

//
// IdentifyVersion
//
// Set the location of the defaults file and the savegame root
// Locate and validate an IWAD file
// Determine gamemode from the IWAD
//
// supports IWADs with custom names. Also allows the -iwad parameter to
// specify which iwad is being searched for if several exist in one dir.
// The -iwad parm may specify:
//
// 1) a specific pathname, which must exist (.wad optional)
// 2) or a directory, which must contain a standard IWAD,
// 3) or a filename, which must be found in one of the standard places:
//   a) current dir,
//   b) exe dir
//   c) $DOOMWADDIR
//   d) or $HOME
//
// jff 4/19/98 rewritten to use a more advanced search algorithm

static void IdentifyVersion()
{
	char* iwad = nullptr;

	// why is this here?
	dsda_InitDataDir();
	dsda_InitSaveDir();

	if(!dsda_Arg(ArgId::Iwad)->found)
	{
		dsda_Loadfiles();                                        // Load files for GAMEINFO lump
		if(!dsda_Flag(ArgId::Noautoload)) D_AutoloadPWadDir(); // Load autoload PWAD files for GAMEINFO lump
		W_Init();                                                // Quick cache to search for GAMEINFO lump

		// Parse GAMEINFO lump
		dsda_LoadGameInfo();

		// Autodetect Doom 1 maps
		if(iwadlump == nullptr)
			dsda_DetectEpisodeStructure();

		// Reset lump cache
		dsda_ResetInitLumpCache();

		// If IWAD found, check if it exists
		if(iwadlump != nullptr)
		{
			iwad = FindIWADFile();

			// Clear data if IWAD not found
			if(!(iwad && *iwad))
			{
				Z_Free(iwadlump);
				Z_Free(iwadver);
				iwadlump = nullptr;
				iwadver = nullptr;
			}
		}
	}

	// If GAMEINFO / Episode IWAD not found,
	// locate IWAD the traditional way
	if(iwadlump == nullptr)
	{
		EpisodeStructure = false;
		iwad = FindIWADFile();
	}

	// It is now ok to load dehacked / unzip files
	MainLumpCache = true;

	if(iwad && *iwad)
	{
		AddIWAD(iwad);
		Z_Free(iwad);
	}
	else
	{
		Log::Fatal("IdentifyVersion: IWAD not found\n\n"
			"Make sure your IWADs are in a folder that dsda-doom searches on\n"
			"For example: {}", I_ConfigDir());
	}
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup()
{
	int p, slot = -1;
	dsda_arg_t* arg;
	dboolean autoload;

	setbuf(stdout,nullptr);

	if(dsda_Flag(ArgId::Help))
	{
		dsda_PrintArgHelp();
		I_SafeExit(0);
	}

	// CPhipps - autoloading of wads
	autoload = !dsda_Flag(ArgId::Noautoload);

	DoLooseFiles(); // Ty 08/29/98 - handle "loose" files on command line

	IdentifyVersion(); // Get IWAD

	dsda_InitGlobal();

	// e6y: DEH files preloaded in wrong order
	// http://sourceforge.net/tracker/index.php?func=detail&aid=1418158&group_id=148658&atid=772943
	// The dachaked stuff has been moved below an autoload

	// jff 1/24/98 set both working and command line value of play parms
	nomonsters = clnomonsters = dsda_Flag(ArgId::Nomonsters);
	respawnparm = clrespawnparm = dsda_Flag(ArgId::Respawn);
	fastparm = clfastparm = dsda_Flag(ArgId::Fast);
	// jff 1/24/98 end of set to both working and command line value

	if(dsda_Flag(ArgId::Altdeath))
		deathmatch = 2;
	else if(dsda_Flag(ArgId::Deathmatch))
		deathmatch = 1;

	modifiedgame = false;

	// get skill / episode / map from parms

	startskill = dsda_IntConfig(ConfigId::DefaultSkill) - 1;
	startepisode = 1;
	startmap = 1;
	autostart = false;

	arg = dsda_Arg(ArgId::Skill);
	if(arg->found)
	{
		startskill = arg->value.v_int - 1;
		autostart = true;
	}

	arg = dsda_Arg(ArgId::Episode);
	if(arg->found)
	{
		startepisode = arg->value.v_int;
		startmap = 1;
		autostart = true;
	}

	HandleClass();

	arg = dsda_Arg(ArgId::Timer);
	if(arg->found && deathmatch)
	{
		int time = arg->value.v_int;
		//jff 9/3/98 use logical output routine
		Log::Info("Levels will end after {} minute{}.\n", time, time > 1 ? "s" : "");
	}

	//jff 1/22/98 add command line parms to disable sound and music
	{
		int nosound = dsda_Flag(ArgId::Nosound);
		nomusicparm = nosound || dsda_Flag(ArgId::Nomusic);
		nosfxparm = nosound || dsda_Flag(ArgId::Nosfx);
	}
	//jff end of sound/music command line parms

	// killough 3/2/98: allow -nodraw generally
	nodrawers = dsda_Flag(ArgId::Nodraw);

	// init subsystems

	G_ReloadDefaults(); // killough 3/4/98: set defaults just loaded.
	// jff 3/24/98 this sets startskill if it was -1

	// proff 04/05/2000: for GL-specific switches
	gld_InitCommandLine();

	//jff 9/3/98 use logical output routine
	Log::Debug("V_Init: allocate screens.\n");
	V_Init();

	//e6y: Calculate the screen resolution and init all buffers
	I_InitScreenResolution();

	//e6y: some stuff from command-line should be initialised before ProcessDehFile()
	e6y_InitCommandLine();

	// Check arguments for demoplayback / demorecording
	started_demo = dsda_Flag(ArgId::Record) || dsda_Flag(ArgId::Recordfromto) ||
		dsda_Flag(ArgId::Playdemo) || dsda_Flag(ArgId::Timedemo) || dsda_Flag(ArgId::Fastdemo);

	D_AddFile(port_wad_file, WadSource::PortWad);

	HandlePlayback(); // must come before autoload: may detect iwad in footer

	EvaluateDoomVerStr(); // must come after HandlePlayback (may change iwad)

	// add wad files from autoload directory before wads from -file parameter
	if(autoload)
		D_AutoloadIWadDir();

	// add any files specified on the command line with -file wadfile
	// to the wad list

	dsda_Loadfiles();

	// add wad files from autoload PWAD directories
	if(autoload)
		D_AutoloadPWadDir();

	D_InitFakeNetGame();

	//jff 9/3/98 use logical output routine
	Log::Debug("W_Init: Init WADfiles.\n");
	W_Init(); // CPhipps - handling of wadfiles init changed

	if(hexen)
	{
		if(!W_LumpNameExists("MAP05"))
		{
			Log::Fatal("The Hexen IWAD shareware is not supported.");
			gamemode = GameMode::Shareware;
			g_maxplayers = 4;
		}
		else if(!W_LumpNameExists("CLUS1MSG"))
		{
			Log::Fatal("The Hexen v1.0 IWAD is not supported.");
		}
	}

	Log::Debug("G_ReloadDefaults: Checking OPTIONS.\n");
	dsda_ParseOptionsLump();

	if(iwadlump != nullptr)
	{
		Log::Info("Detected {} lump: {}\n", iwadver ? iwadver : "GAMEINFO", iwadlump);
		Z_Free(iwadlump);

		if(iwadver)
			Z_Free(iwadver);
	}

	G_ReloadDefaults();

	// e6y
	// option to disable automatic loading of dehacked-in-wad lump
	if(!dsda_Flag(ArgId::Nodeh))
	{
		// MBF-style DeHackEd in wad support: load all lumps, not just the last one
		for(p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0;)
			// Split loading DEHACKED lumps into IWAD/autoload and PWADs/others
			if(lumpinfo[p].source == WadSource::Iwad
				|| lumpinfo[p].source == WadSource::PortWad
				|| lumpinfo[p].source == WadSource::AutoLoad
				|| lumpinfo[p].source == WadSource::PwadAutoLoad)
				ProcessDehFile(nullptr, D_dehout(), p); // cph - add dehacked-in-a-wad support

		if(bfgedition)
		{
			int lump = W_CheckNumForName2("BFGBEX", LumpNamespace::Prboom);
			if(lump != LUMP_NOT_FOUND)
			{
				ProcessDehFile(nullptr, D_dehout(), lump);
			}
		}
		if(gamemission == GameMission::PackNerve)
		{
			int lump = W_CheckNumForName2("NERVEBEX", LumpNamespace::Prboom);
			if(lump != LUMP_NOT_FOUND)
			{
				ProcessDehFile(nullptr, D_dehout(), lump);
			}
		}
		if(gamemission == GameMission::TcChex)
		{
			int lump = W_CheckNumForName2("CHEXDEH", LumpNamespace::Prboom);
			if(lump != LUMP_NOT_FOUND)
			{
				ProcessDehFile(nullptr, D_dehout(), lump);
			}
		}
	}

	// process deh files from autoload directory before deh in wads from -file parameter
	if(autoload)
		D_AutoloadDehIWadDir();

	if(!dsda_Flag(ArgId::Nodeh))
		for(p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0;)
			if(!(lumpinfo[p].source == WadSource::Iwad
				|| lumpinfo[p].source == WadSource::PortWad
				|| lumpinfo[p].source == WadSource::AutoLoad
				|| lumpinfo[p].source == WadSource::PwadAutoLoad))
				ProcessDehFile(nullptr, D_dehout(), p);

	// process .deh files from PWADs autoload directories
	if(autoload)
		D_AutoloadDehPWadDir();

	// Load command line dehacked patches after WAD dehacked patches

	// e6y: DEH files preloaded in wrong order
	// http://sourceforge.net/tracker/index.php?func=detail&aid=1418158&group_id=148658&atid=772943

	// ty 03/09/98 do dehacked stuff
	// Using -deh in BOOM, others use -dehacked.
	// Ty 03/18/98 also allow .bex extension.  .bex overrides if both exist.

	arg = dsda_Arg(ArgId::Deh);
	if(arg->found)
	{
		int i;

		// e6y
		// reorganization of the code for looking for bex/deh patches
		// in all standard dirs (%DOOMWADDIR%, etc)
		for(i = 0; i < arg->count; ++i)
		{
			char* file = nullptr;

			file = I_RequireDeh(arg->value.v_string_array[i]);

			// during the beta we have debug output to dehout.txt
			ProcessDehFile(file, D_dehout(), 0);
			Z_Free(file);
		}
	}

	PostProcessDeh();
	dsda_AppendZDoomMobjInfo();
	dsda_ApplyBinaryMapFormat();

	Log::Debug("dsda_InitWadStats: Setting up wad stats.\n");
	dsda_InitWadStats();

	Log::Info("\n"); // Separator after file loading

	V_InitColorTranslation(); //jff 4/24/98 load color translation lumps

	//jff 9/3/98 use logical output routine
	Log::Debug("M_Init: Init miscellaneous info.\n");
	M_Init();

	dsda_LoadSndInfo();

	if(map_format.sndseq)
	{
		SN_InitSequenceScript();
	}

	//jff 9/3/98 use logical output routine
	Log::Debug("R_Init: Init DOOM refresh daemon - ");
	R_Init();

	dsda_LoadWadPreferences();
	dsda_LoadMapInfo();
	dsda_InitSkills();
	dsda_InitGameModifiers(); // Set game modifiers based off args / persistent cfgs

	//jff 9/3/98 use logical output routine
	Log::Debug("\nP_Init: Init Playloop state.\n");
	P_Init();

	// Must be after P_Init
	HandleWarp();

	// Must be after HandleWarp
	dsda_HandleSkip();

	//jff 9/3/98 use logical output routine
	Log::Debug("I_Init: Setting up machine state.\n");
	I_Init();

	//jff 9/3/98 use logical output routine
	Log::Debug("S_Init: Setting up sound.\n");
	S_Init();

	//jff 9/3/98 use logical output routine
	Log::Debug("dsda_InitFont: Loading the hud fonts.\n");
	dsda_InitFont();

	if(!(dsda_Flag(ArgId::Nodraw) && dsda_Flag(ArgId::Nosound)))
		I_InitGraphics();

	// NSM
	arg = dsda_Arg(ArgId::Viddump);
	if(arg->found)
	{
		I_CapturePrep(arg->value.v_string);
	}

	//jff 9/3/98 use logical output routine
	Log::Debug("ST_Init: Init status bar.\n");
	ST_Init();

	// start the appropriate game based on parms

	arg = dsda_Arg(ArgId::Record);
	if(arg->found)
	{
		autostart = true;
		dsda_SetDemoBaseName(arg->value.v_string);
		dsda_InitDemoRecording();
	}
	else
	{
		arg = dsda_Arg(ArgId::Loadgame);
		if(arg->found)
		{
			slot = arg->value.v_int;
			G_LoadGame(slot, true);
		}
	}

	dsda_ExecutePlaybackOptions();

	if(slot == -1 && !userdemo)
	{
		if(autostart || netgame)
		{
			G_InitNew(startskill, startepisode, startmap, true);
			if(demorecording)
				G_BeginRecording();
		}
		else
			D_StartTitle(); // start up intro loop
	}

	// do not try to interpolate during timedemo
	M_ChangeUncappedFrameRate();

	Log::Debug("\n"); // Separator after setup
}

//
// D_DoomMain
//

void D_DoomMain()
{
	D_DoomMainSetup(); // CPhipps - setup out of main execution stack

	D_DoomLoop(); // never returns
}
