// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Misc system stuff needed by Doom, implemented for Linux.
 *  Mainly timer handling.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "SDL.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#ifdef HAVE_GETPWUID
#include <sys/types.h>
#include <pwd.h>
#endif
#endif

#ifdef _MSC_VER
#include <io.h>
#endif

#include "lprintf.hpp"
#include "m_file.hpp"
#include "doomtype.hpp"
#include "doomdef.hpp"
#include "d_player.hpp"
#include "m_fixed.hpp"
#include "r_fps.hpp"
#include "e6y.hpp"
#include "i_system.hpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.hpp"

#include "dsda/settings.hpp"
#include "dsda/signal_context.hpp"
#include "dsda/time.hpp"
#include "dsda/utility.hpp"

void I_uSleep(unsigned long usecs)
{
	SDL_Delay(usecs / 1000);
}

static dboolean InDisplay = false;
static int saved_gametic = -1;
dboolean realframe = false;

dboolean I_StartDisplay()
{
	if(InDisplay)
		return false;

	realframe = (!movement_smooth) || (gametic > saved_gametic);

	if(realframe)
		saved_gametic = gametic;

	InDisplay = true;
	DSDA_ADD_CONTEXT(SignalContext::Display);
	return true;
}

void I_EndDisplay()
{
	InDisplay = false;
	DSDA_REMOVE_CONTEXT(SignalContext::Display);
}

int interpolation_method;
fixed_t I_GetTimeFrac()
{
	fixed_t frac;

	if(!movement_smooth)
	{
		frac = FRACUNIT;
	}
	else
	{
		static fixed_t last_frac;
		static int last_gametic;
		unsigned long long tic_time;
		const double tics_per_usec = TICRATE / 1000000.0f;

		tic_time = dsda_TickElapsedTime();

		frac = (fixed_t)(tic_time * FRACUNIT * tics_per_usec);
		frac = BETWEEN(0, FRACUNIT, frac);

		if(frac < last_frac && last_gametic == gametic)
		{
			frac = FRACUNIT;
		}

		last_frac = frac;
		last_gametic = gametic;
	}

	return frac;
}

/*
 * I_GetRandomTimeSeed
 *
 * CPhipps - extracted from G_ReloadDefaults because it is O/S based
 */
unsigned long I_GetRandomTimeSeed()
{
	return (unsigned long)time(nullptr);
}

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char* buf, size_t sz)
{
	snprintf(buf, sz, "%s v%s (https://github.com/kraflab/dsda-doom/)", PROJECT_NAME, PROJECT_VERSION);
	return buf;
}

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum)
{
#ifdef HAVE_STRSIGNAL
	if(strsignal(signum) && strlen(strsignal(signum)) < sz)
		strcpy(buf, strsignal(signum));
	else
#endif
		snprintf(buf, sz, "signal %d", signum);
	return buf;
}

/*
 * I_Read
 *
 * cph 2001/11/18 - wrapper for read(2) which handles partial reads and aborts
 * on error.
 */
void I_Read(int fd, void* vbuf, size_t sz)
{
	unsigned char* buf = (unsigned char*)vbuf;

	while(sz)
	{
		int rc = read(fd, buf, sz);
		if(rc <= 0)
		{
			Log::Fatal("I_Read: read failed: {}", rc ? strerror(errno) : "EOF");
		}
		sz -= rc;
		buf += rc;
	}
}

/*
 * I_Filelength
 *
 * Return length of an open file.
 */

int I_Filelength(int handle)
{
	struct stat fileinfo;
	if(fstat(handle, &fileinfo) == -1)
		Log::Fatal("I_Filelength: {}", strerror(errno));
	return fileinfo.st_size;
}

// Return the path where the executable lies -- Lee Killough
// proff_fs 2002-07-04 - moved to i_system
#ifdef _WIN32

void I_SwitchToWindow(HWND hwnd)
{
	typedef BOOL (WINAPI
	*TSwitchToThisWindow) (HWND
	wnd, BOOL
	restore);
	static TSwitchToThisWindow SwitchToThisWindow = nullptr;

	if(!SwitchToThisWindow)
		SwitchToThisWindow = (TSwitchToThisWindow)GetProcAddress(GetModuleHandle("user32.dll"), "SwitchToThisWindow");

	if(SwitchToThisWindow)
	{
		HWND hwndLastActive = GetLastActivePopup(hwnd);

		if(IsWindowVisible(hwndLastActive))
			hwnd = hwndLastActive;

		SetForegroundWindow(hwnd);
		Sleep(100);
		SwitchToThisWindow(hwnd, TRUE);
	}
}

const char* I_ConfigDir()
{
	return I_ExeDir();
}

const char* I_ExeDir()
{
	extern char** dsda_argv;

	static char* base;
	if(!base) // cache multiple requests
	{
		size_t len = strlen(*dsda_argv);
		char* p = (base = (char*)Z_Malloc(len + 1)) + len - 1;
		strcpy(base, *dsda_argv);
		while(p > base && *p != '/' && *p != '\\')
			*p-- = 0;
		if(*p == '/' || *p == '\\')
			*p-- = 0;
		if(strlen(base) < 2 || !M_WriteAccess(base))
		{
			Z_Free(base);
			base = (char*)Z_Malloc(1024);
			if(!M_getcwd(base, 1024) || !M_WriteAccess(base))
				strcpy(base, ".");
		}
	}
	return base;
}

const char* I_GetTempDir()
{
	static const char* tmp_path;

	if(!tmp_path)
	{
		wchar_t wpath[PATH_MAX];
		DWORD result;

		result = GetTempPathW(PATH_MAX, wpath);

		if(result == 0 || result > MAX_PATH)
			Log::Fatal("I_GetTempDir: GetTempPathW failed");
		else
			tmp_path = ConvertWideToUtf8(wpath);
	}

	return tmp_path;
}

#elif defined(AMIGA)

const char* I_ConfigDir()
{
	return "PROGDIR:";
}

const char* I_ExeDir()
{
	return "PROGDIR:";
}

const char* I_GetTempDir()
{
	return "PROGDIR:";
}

#else /* not Windows, not Amiga */

static const char* I_GetHomeDir()
{
	const char* home = M_getenv("HOME");

	if(!home)
	{
#ifdef HAVE_GETPWUID
		struct passwd* user_info = getpwuid(getuid());
		if(user_info != nullptr)
			home = user_info->pw_dir;
		else
#endif
			home = "/";
	}

	return home;
}

// Reference for XDG directories:
// <https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static const char* I_GetXDGDataHome()
{
	static char* datahome = nullptr;

	if(!datahome)
	{
		const char* xdgdatahome = M_getenv("XDG_DATA_HOME");

		if(!xdgdatahome || !*xdgdatahome)
		{
			size_t datahome_size;
			const char* home = I_GetHomeDir();

			datahome_size = strlen(home) + 1 + sizeof(".local/share");
			datahome = static_cast<char*>(Z_Malloc(datahome_size));
			snprintf(datahome, datahome_size, "%s%s%s", home, !HasTrailingSlash(home) ? "/" : "", ".local/share");
		}
		else
		{
			datahome = Z_Strdup(xdgdatahome);
		}
	}
	return datahome;
}

static const char* I_GetXDGDataDirs()
{
	const char* datadirs = M_getenv("XDG_DATA_DIRS");

	if(!datadirs || !*datadirs)
		return "/usr/local/share/:/usr/share/";
	return datadirs;
}

const char* I_ConfigDir()
{
	static char* base;

	if(!base)
	{
		const char* home = I_GetHomeDir();

		// First, try legacy directory.
		base = dsda_ConcatDir(home, ".dsda-doom");
		if(access(base, F_OK) != 0)
		{
			// Legacy directory is not accessible. Use XDG directory.
			Z_Free(base);

#ifdef __APPLE__
			base = dsda_ConcatDir(home, "Library/Application Support/dsda-doom");
#else
			base = dsda_ConcatDir(I_GetXDGDataHome(), "dsda-doom");
#endif
		}

		M_MakeDir(base, false);
	}

	return base;
}

const char* I_ExeDir()
{
	extern char** dsda_argv;

	static char* base;
	if(!base) // cache multiple requests
	{
		size_t len = strlen(*dsda_argv);
		char* p = (base = (char*)Z_Malloc(len + 1)) + len - 1;
		strcpy(base, *dsda_argv);
		while(p > base && *p != '/' && *p != '\\')
			*p-- = 0;
		if(*p == '/' || *p == '\\')
			*p-- = 0;
		if(strlen(base) < 2 || !M_WriteAccess(base))
		{
			Z_Free(base);
			base = (char*)Z_Malloc(1024);
			if(!M_getcwd(base, 1024) || !M_WriteAccess(base))
				strcpy(base, ".");
		}
	}
	return base;
}

const char* I_GetTempDir()
{
	return "/tmp";
}

#endif

/*
 * HasTrailingSlash
 *
 * cphipps - simple test for trailing slash on dir names
 */

dboolean HasTrailingSlash(const char* dn)
{
	return ((dn[strlen(dn) - 1] == '/')
#if defined(_WIN32)
        || (dn[strlen(dn) - 1] == '\\')
#endif
#if defined(AMIGA)
        || (dn[strlen(dn) - 1] == ':')
#endif
	);
}

static const char* I_GetBasePath()
{
	static char* executable_dir;
	/* SDL_GetBasePath is an expensive call */
	if(!executable_dir)
		executable_dir = SDL_GetBasePath();
	return executable_dir;
}

/*
 * I_FindFile
 *
 * proff_fs 2002-07-04 - moved to i_system
 *
 * cphipps 19/1999 - writen to unify the logic in FindIWADFile and the WAD
 *      autoloading code.
 * Searches the standard dirs for a named WAD file
 * The dirs are listed at the start of the function
 */

#ifdef _WIN32
#define PATH_SEPARATOR ";"
#else
#define PATH_SEPARATOR ":"
#endif

char* I_FindFileInternal(const char* wfname, const char* ext, dboolean isStatic)
{
	// lookup table of directories to search
	static struct
	{
		const char* dir;          // directory
		const char* sub;          // subdirectory
		const char* env;          // environment variable
		const char*(*func)(); // for functions that return the directory
	} search0[] = {
	{nullptr, nullptr, nullptr, I_ExeDir}, // executable directory
#if !defined(_WIN32) && !defined(AMIGA)
	{nullptr, nullptr, nullptr, I_ConfigDir}, // config and autoload directory. on windows/amiga, this is the same as I_ExeDir
#endif
	{nullptr},                                                // current working directory
	{nullptr, nullptr, "DOOMWADDIR"},                         // run-time $DOOMWADDIR
	{DOOMWADDIR},                                             // build-time configured DOOMWADDIR
	{DSDA_ABSOLUTE_PWAD_PATH},                                // build-time configured absolute path to dsda-doom.wad
	{nullptr, nullptr, nullptr, I_GetBasePath},               // search the base path provided by SDL
	{nullptr, "../share/games/doom", nullptr, I_GetBasePath}, // AppImage
	{nullptr, "doom", "HOME"},                                // ~/doom
	{nullptr, nullptr, "HOME"},                                      // ~
#if !defined(_WIN32) && !defined(AMIGA)
	{nullptr, "games/doom", nullptr, I_GetXDGDataHome}, // $HOME/.local/share/games/doom
#endif
}, *search;

	static size_t num_search;
	size_t i;
	size_t pl;

	static char static_p[PATH_MAX];
	char* dinamic_p = nullptr;
	char* p = (isStatic ? static_p : dinamic_p);

	if(!wfname)
		return nullptr;

	if(!num_search)
	{
		int extra = 0;
#if !defined(_WIN32) && !defined(AMIGA)
		int datadirs = 0;
#endif
		const char* dwp;

		// calculate how many extra entries we need to add to the table
#if !defined(_WIN32) && !defined(AMIGA)
		dwp = I_GetXDGDataDirs();
		datadirs++;
		while((dwp = strchr(dwp, *PATH_SEPARATOR)))
			dwp++, datadirs++;
		extra += datadirs * 2; // two entries for each datadir
#endif
		if((dwp = M_getenv("DOOMWADPATH")))
		{
			extra++;
			while((dwp = strchr(dwp, *PATH_SEPARATOR)))
				dwp++, extra++;
		}

		// initialize with the static lookup table
		num_search = sizeof(search0) / sizeof(*search0);
		search = static_cast<decltype(search)>(Z_Malloc((num_search + extra) * sizeof(*search)));
		memcpy(search, search0, num_search * sizeof(*search));
		memset(&search[num_search], 0, extra * sizeof(*search));

#if !defined(_WIN32) && !defined(AMIGA)
		// add $XDG_DATA_DIRS/games/doom and $XDG_DATA_DIRS/doom
		// by default this includes:
		// - /usr/local/share/games/doom
		// - /usr/share/games/doom
		// - /usr/local/share/doom
		// - /usr/share/doom
		{
			char *ptr, *dup_dwp;

			dup_dwp = Z_Strdup(I_GetXDGDataDirs());
			ptr = strtok(dup_dwp, PATH_SEPARATOR);
			while(ptr)
			{
				search[num_search].dir = Z_Strdup(ptr);
				search[num_search].sub = "games/doom";
				search[num_search + datadirs].dir = Z_Strdup(ptr);
				search[num_search + datadirs].sub = "doom";
				num_search++;
				ptr = strtok(nullptr, PATH_SEPARATOR);
			}
			Z_Free(dup_dwp);
			num_search += datadirs;
		}
#endif

		// add each directory from the $DOOMWADPATH environment variable
		if((dwp = M_getenv("DOOMWADPATH")))
		{
			char *ptr, *dup_dwp;

			dup_dwp = Z_Strdup(dwp);
			ptr = strtok(dup_dwp, PATH_SEPARATOR);
			while(ptr)
			{
				search[num_search].dir = Z_Strdup(ptr);
				num_search++;
				ptr = strtok(nullptr, PATH_SEPARATOR);
			}
			Z_Free(dup_dwp);
		}
	}

	/* Precalculate a length we will need in the loop */
	pl = strlen(wfname) + (ext ? strlen(ext) : 0) + 4;

	for(i = 0; i < num_search; i++)
	{
		const char* d = nullptr;
		const char* s = nullptr;
		size_t p_size = PATH_MAX;
		/* Each entry in the switch sets d to the directory to look in,
		* and optionally s to a subdirectory of d */
		// switch replaced with lookup table
		if(search[i].env)
		{
			if(!(d = M_getenv(search[i].env)))
				continue;
		}
		else if(search[i].func)
			d = search[i].func();
		else
			d = search[i].dir;
		s = search[i].sub;

		if(!isStatic)
		{
			p_size = (d ? strlen(d) : 0) + (s ? strlen(s) : 0) + pl;
			p = (char*)Z_Malloc(p_size);
		}

		snprintf(p, p_size, "%s%s%s%s%s", d ? d : "", (d && !HasTrailingSlash(d)) ? "/" : "",
			s ? s : "", (s && !HasTrailingSlash(s)) ? "/" : "",
			wfname);

		if(ext && !M_FileExists(p))
			strcat(p, ext);
		if(M_FileExists(p))
		{
			if(!isStatic)
				Log::Debug(" found {}\n", p);
			return p;
		}
		if(!isStatic)
			Z_Free(p);
	}
	return nullptr;
}

char* I_RequireFile(const char* wfname, const char* ext)
{
	char* result = I_FindFileInternal(wfname, ext, false);

	if(!result)
		Log::Fatal("Unable to find required file \"{}\"", wfname);

	return result;
}

char* I_FindFile(const char* wfname, const char* ext)
{
	return I_FindFileInternal(wfname, ext, false);
}

const char* I_FindFile2(const char* wfname, const char* ext)
{
	return (const char*)I_FindFileInternal(wfname, ext, true);
}

char* I_RequireAnyFile(const char* wfname, const char** ext)
{
	char* result = nullptr;

	for(; *ext; ext++)
	{
		result = I_FindFile(wfname, *ext);
		if(result)
			return result;
	}

	Log::Fatal("Unable to find required file \"{}\"", wfname);
}

char* I_RequireWad(const char* wfname)
{
	return I_RequireFile(wfname, ".wad");
}

char* I_FindWad(const char* wfname)
{
	return I_FindFile(wfname, ".wad");
}

char* I_RequireDeh(const char* wfname)
{
	char* result;

	result = I_FindFile(wfname, ".bex");
	if(result)
		return result;

	return I_RequireFile(wfname, ".deh");
}

char* I_FindDeh(const char* wfname)
{
	char* result;

	result = I_FindFile(wfname, ".bex");
	if(result)
		return result;

	return I_FindFile(wfname, ".deh");
}

char* I_RequireZip(const char* wfname)
{
	return I_RequireFile(wfname, ".zip");
}

char* I_FindZip(const char* wfname)
{
	return I_FindFile(wfname, ".zip");
}
