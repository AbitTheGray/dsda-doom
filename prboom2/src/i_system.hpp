// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System specific interface stuff.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#include "m_fixed.hpp"

#ifdef _MSC_VER
#define F_OK    0    /* Check for file existence */
#define W_OK    2    /* Check for write permission */
#define R_OK    4    /* Check for read permission */
#endif

extern int interpolation_method;
extern int ms_to_next_tick;
dboolean I_StartDisplay();
void I_EndDisplay();
fixed_t I_GetTimeFrac();

unsigned long I_GetRandomTimeSeed(); /* cphipps */

void I_uSleep(unsigned long usecs);

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char* buf, size_t sz);

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum);

#ifdef _WIN32
void I_SwitchToWindow(HWND hwnd);
#endif

// e6y
const char* I_GetTempDir();

const char* I_ExeDir();    // killough 2/16/98: path to executable's dir
const char* I_ConfigDir(); // path to config and autoload dir

dboolean HasTrailingSlash(const char* dn);
char* I_RequireFile(const char* wfname, const char* ext);
char* I_FindFile(const char* wfname, const char* ext);
const char* I_FindFile2(const char* wfname, const char* ext);
char* I_RequireAnyFile(const char* wfname, const char** ext);

char* I_RequireWad(const char* wfname);
char* I_FindWad(const char* wfname);

char* I_RequireDeh(const char* wfname);
char* I_FindDeh(const char* wfname);

char* I_RequireZip(const char* wfname);
char* I_FindZip(const char* wfname);

/* cph 2001/11/18 - wrapper for read(2) which deals with partial reads */
void I_Read(int fd, void* buf, size_t sz);

/* cph 2001/11/18 - Move W_Filelength to i_system.c */
int I_Filelength(int handle);

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)

typedef enum
{
	exit_priority_first,
	exit_priority_normal,
	exit_priority_last,
	exit_priority_max,
} exit_priority_t;

typedef void (*atexit_func_t)();
void I_AtExit(atexit_func_t func, dboolean run_if_error,
	const char* name, exit_priority_t priority);

#ifdef __cplusplus
}
#endif
