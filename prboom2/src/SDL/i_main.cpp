// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Startup and quit functions. Handles signals, inits the
 *      memory management, then calls D_DoomMain. Also contains
 *      I_Init which does other system-related startup stuff.
 */

#ifdef HAVE_CONFIG_H
#include <array>
#include <utility>

#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <errno.h>

#include "doomdef.hpp"
#include "d_main.hpp"
#include "m_fixed.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "z_zone.hpp"
#include "lprintf.hpp"
#include "m_random.hpp"
#include "doomstat.hpp"
#include "g_game.hpp"
#include "m_misc.hpp"
#include "i_sound.hpp"
#include "i_main.hpp"
#include "r_fps.hpp"
#include "lprintf.hpp"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "e6y.hpp"

#include "dsda.hpp"
#include "dsda/args.hpp"
#include "dsda/analysis.hpp"
#include "dsda/args.hpp"
#include "dsda/endoom.hpp"
#include "dsda/settings.hpp"
#include "dsda/signal_context.hpp"
#include "dsda/split_tracker.hpp"
#include "dsda/text_file.hpp"
#include "dsda/time.hpp"
#include "dsda/wad_stats.hpp"
#include "dsda/zipfile.hpp"

/* Most of the following has been rewritten by Lee Killough
 *
 * killough 4/13/98: Make clock rate adjustable by scale factor
 * cphipps - much made static
 */

void I_Init()
{
	dsda_ResetTimeFunctions(fastdemo);
	I_InitSound();
}

//e6y
void I_Init2()
{
	dsda_ResetTimeFunctions(fastdemo);

	force_singletics_to = gametic + BACKUPTICS;
}

SignalContext signal_context;

static volatile sig_atomic_t interrupted = 0;

dboolean I_Interrupted()
{
	return interrupted;
}

static void I_SignalHandler(int s)
{
	char buf[2048];

	signal(s, SIG_IGN); /* Ignore future instances of this signal.*/

	// Terminal Interrupt
	if(s == 2)
		I_DisableMessageBoxes();

	I_SigString(buf, sizeof(buf), s);

	Log::Fatal("The game has crashed!\n"
		"Please report the following information: {} (0x{:04x})",
		std::string_view(buf), std::to_underlying(signal_context));
}

static void I_IntHandler(int s)
{
	interrupted = 1;
}

static void PrintVer()
{
	char vbuf[200];
	Log::Info("{}\n", I_GetVersionString(vbuf, 200));
}

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)
// Copyright(C) 2005-2014 Simon Howard

typedef struct atexit_listentry_s atexit_listentry_t;

struct atexit_listentry_s
{
	atexit_func_t func;
	dboolean run_on_error;
	atexit_listentry_t* next;
	const char* name;
};

static std::array<atexit_listentry_t*, std::to_underlying(ExitPriority::Max)> exit_funcs;
static int exit_priority;

void I_AtExit(atexit_func_t func, dboolean run_on_error,
	const char* name, ExitPriority priority)
{
	atexit_listentry_t* entry;

	entry = static_cast<atexit_listentry_t*>(Z_Malloc(sizeof(*entry)));

	entry->func = func;
	entry->run_on_error = run_on_error;
	entry->next = exit_funcs[std::to_underlying(priority)];
	entry->name = name;
	exit_funcs[std::to_underlying(priority)] = entry;
}

void I_SafeExit(int rc)
{
	atexit_listentry_t* entry;

	Log::Debug("\n"); // Separator after game loop

	// Run through all exit functions
	for(; exit_priority < std::to_underlying(ExitPriority::Max); ++exit_priority)
	{
		while((entry = exit_funcs[exit_priority]))
		{
			exit_funcs[exit_priority] = exit_funcs[exit_priority]->next;

			if(rc == 0 || entry->run_on_error)
			{
				Log::Debug("Exit Sequence[{}]: {} ({})\n", exit_priority, entry->name, rc);
				entry->func();
			}
		}
	}

	exit(rc);
}

static void I_EssentialQuit()
{
	if(demorecording)
	{
		G_CheckDemoStatus();
	}
	dsda_ExportTextFile();
	dsda_WriteAnalysis();
	dsda_WriteSplits();
	dsda_SaveWadStats();
	// We need to close out all wad handles/memory mappings before we can remove
	// temporary wads on Windows
	// Read Endoom before dumping the wads!
	dsda_CacheEndoom();
	W_Shutdown();
	dsda_CleanZipTempDirs();
}

static void I_Quit()
{
	M_SaveDefaults();
	dsda_DumpEndoom();
	dsda_Shutdown();
}

//
// Sets the priority class for the prboom-plus process
//

void I_SetProcessPriority()
{
	int process_priority = dsda_IntConfig(ConfigId::ProcessPriority);

	if(process_priority)
	{
		const char* errbuf = nullptr;

#ifdef _WIN32
		{
			DWORD dwPriorityClass = NORMAL_PRIORITY_CLASS;

			if(process_priority == 1)
				dwPriorityClass = HIGH_PRIORITY_CLASS;
			else if(process_priority == 2)
				dwPriorityClass = REALTIME_PRIORITY_CLASS;

			if(SetPriorityClass(GetCurrentProcess(), dwPriorityClass) == 0)
			{
				errbuf = WINError();
			}
		}
#else
		return;
#endif

		if(errbuf == nullptr)
		{
			Log::Info("I_SetProcessPriority: priority for the process is {}\n", process_priority);
		}
		else
		{
			Log::Error("I_SetProcessPriority: failed to set priority for the process ({})\n", errbuf);
		}
	}
}

//int main(int argc, const char * const * argv)
int main(int argc, char** argv)
{
	dsda_ParseCommandLineArgs(argc, argv);

	if(dsda_Flag(ArgId::Verbose))
		I_EnableVerboseLogging();

	if(dsda_Flag(ArgId::Quiet))
		I_DisableAllLogging();

	// Print the version and exit
	if(dsda_Flag(ArgId::V))
	{
		PrintVer();
		return 0;
	}

	// e6y: Check for conflicts.
	// Conflicting command-line parameters could cause the engine to be confused
	// in some cases. Added checks to prevent this.
	// Example: dsda-doom.exe -record mydemo -playdemo demoname
	ParamsMatchingCheck();

	// e6y: was moved from D_DoomMainSetup
	// init subsystems
	//jff 9/3/98 use logical output routine
	Log::Debug("M_LoadDefaults: Load system defaults.\n");
	M_LoadDefaults(); // load before initing other systems
	Log::Debug("\n");

	// Print date and time in the Load/Save Game menus in the current locale
	setlocale(LC_TIME, "");

	/* Version info */
	PrintVer();

	/*
		killough 1/98:

		This fixes some problems with exit handling
		during abnormal situations.

		The old code called I_Quit() to end program,
		while now I_Quit() is installed as an exit
		handler and exit() is called to exit, either
		normally or abnormally. Seg faults are caught
		and the error handler is used, to prevent
		being left in graphics mode or having very
		loud SFX noise because the sound card is
		left in an unstable state.
	*/

	I_AtExit(I_EssentialQuit, true, "I_EssentialQuit", ExitPriority::First);
	I_AtExit(I_Quit, false, "I_Quit", ExitPriority::Last);
#ifndef PRBOOM_DEBUG
	if(!dsda_Flag(ArgId::Sigsegv))
	{
		signal(SIGSEGV, I_SignalHandler);
	}
	signal(SIGFPE, I_SignalHandler);
	signal(SIGILL, I_SignalHandler);
	signal(SIGABRT, I_SignalHandler);

	signal(SIGTERM, I_IntHandler);
	signal(SIGINT, I_IntHandler);
#endif

	// Priority class for the prboom-plus process
	I_SetProcessPriority();

	/* cphipps - call to video specific startup code */
	I_PreInitGraphics();

	D_DoomMain();
	return 0;
}
