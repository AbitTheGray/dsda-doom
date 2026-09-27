// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Provides a logical console output routine that allows what is
 *  output to console normally and when output is redirected to
 *  be controlled..
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif
#ifdef _MSC_VER
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

#include "doomtype.hpp"
#include "lprintf.hpp"
#include "i_main.hpp"
#include "i_system.hpp"
#include "e6y.hpp"//e6y
#include "i_capture.hpp"

#include "dsda/args.hpp"

static dboolean disable_message_box;

int cons_stdout_mask = std::to_underlying(OutputLevels::Info);
int cons_stderr_mask = std::to_underlying(OutputLevels::Warn) | std::to_underlying(OutputLevels::Error);

/* cphipps - enlarged message buffer and made non-static
 * We still have to be careful here, this function can be called after exit
 */
#define MAX_MESSAGE_SIZE 2048

// Write `text` as it is to the console streams that show `level`.
// Returns what the last write returned, as `lprintf` always has.
static int32_t WriteText(const OutputLevels level, const std::string_view text)
{
	int32_t r = 0;
	const int32_t lvl = std::to_underlying(level);

#ifdef _WIN32
	// do not crash with unicode dirs
	if(fileno(stdout) != -1)
#endif
	if(lvl & cons_stdout_mask)
		r = static_cast<int32_t>(std::fwrite(text.data(), 1, text.size(), stdout));

#ifdef _WIN32
	// do not crash with unicode dirs
	if(fileno(stderr) != -1)
#endif
	if(lvl & cons_stderr_mask)
		r = static_cast<int32_t>(std::fwrite(text.data(), 1, text.size(), stderr));

	return r;
}

int lprintf(OutputLevels pri, const char* s, ...)
{
	char msg[MAX_MESSAGE_SIZE];

	va_list v;
	va_start(v, s);
	vsnprintf(msg, sizeof(msg), s, v); /* print message in buffer  */
	va_end(v);

	return WriteText(pri, msg);
}

void I_EnableVerboseLogging()
{
	cons_stdout_mask = std::to_underlying(OutputLevels::Info) | std::to_underlying(OutputLevels::Debug);
}

void I_DisableAllLogging()
{
	cons_stdout_mask = 0;
	cons_stderr_mask = 0;
}

void I_DisableMessageBoxes()
{
	disable_message_box = true;
}

// Show a fatal error in a message box on Windows, unless message boxes are off or nothing is drawn.
static void ShowErrorBox([[maybe_unused]] const char* text)
{
#ifdef _WIN32
	if(!disable_message_box && !dsda_Flag(ArgId::Nodraw) && !capturing_video)
	{
		I_MessageBox(text, PRB_MB_OK);
	}
#endif
}

/*
 * I_Error
 *
 * cphipps - moved out of i_* headers, to minimise source files that depend on
 * the low-level headers. All this does is print the error, then call the
 * low-level safe exit function.
 * killough 3/20/98: add const
 */

void I_Error(const char* error, ...)
{
	char errmsg[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, error);
	vsnprintf(errmsg, sizeof(errmsg), error, argptr);
	va_end(argptr);
	lprintf(OutputLevels::Error, "%s\n", errmsg);
	ShowErrorBox(errmsg);
	I_SafeExit(-1);
}

void I_Warn(const char* error, ...)
{
	char errmsg[MAX_MESSAGE_SIZE];
	va_list argptr;
	va_start(argptr, error);
	vsnprintf(errmsg, sizeof(errmsg), error, argptr);
	va_end(argptr);
	lprintf(OutputLevels::Warn, "%s\n", errmsg);
#ifdef _WIN32
	if(!dsda_Flag(ArgId::Nodraw) && !capturing_video)
	{
		I_MessageBox(errmsg, PRB_MB_OK);
	}
#endif
}

// A `Log` message as it is printed: cut where `lprintf` cuts if the build enables `LIMIT_LOG_MESSAGES`, otherwise whole.
static std::string_view LimitMessage(const std::string_view text)
{
#ifdef LIMIT_LOG_MESSAGES
	return text.substr(0, MAX_MESSAGE_SIZE - 1);
#else
	return text;
#endif
}

void Log::Detail::Print(const OutputLevels level, const std::string_view text)
{
	WriteText(level, LimitMessage(text));
}

void Log::Detail::Fatal(const std::string_view text)
{
	const std::string message(LimitMessage(text));

	WriteText(OutputLevels::Error, message + "\n");
	ShowErrorBox(message.c_str());
	I_SafeExit(-1);
}
