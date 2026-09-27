// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *    Declarations etc. for logical console output
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>

#include <format>
#include <string_view>
#include <utility>

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct OutputLevels : int32_t
{
	Info  = 1,
	Warn  = 2,
	Error = 4,
	Debug = 8,
};

#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

extern int lprintf(OutputLevels pri, const char* fmt, ...) __attribute__((format(printf,2,3)));

void I_EnableVerboseLogging();
void I_DisableAllLogging();
void I_DisableMessageBoxes();

/* killough 3/20/98: add const
 * killough 4/25/98: add gcc attributes
 * cphipps 01/11- moved from i_system.h */
NORETURNC11 void I_Error(const char* error, ...) __attribute__((format(printf,1,2))) NORETURN;
void I_Warn(const char* error, ...) __attribute__((format(printf,1,2)));

#ifdef __cplusplus
}
#endif

/**
 * Console output formatted with `std::format`, e.g. `Log::Info("FINISHED: {}\n", map)`.
 * Like `lprintf`, no newline is added, so a line can be printed in parts.
 * A message is never cut, unless the build enables `LIMIT_LOG_MESSAGES`, which cuts it where `lprintf` does.
 */
namespace Log
{
	namespace Detail
	{
		void Print(OutputLevels level, std::string_view text);
		[[noreturn]] void Fatal(std::string_view text);
	}

	template<typename... Args>
	void Print(const OutputLevels level, const std::format_string<Args...> format, Args&&... args)
	{
		Detail::Print(level, std::format(format, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Info(const std::format_string<Args...> format, Args&&... args)
	{
		Print(OutputLevels::Info, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void Warn(const std::format_string<Args...> format, Args&&... args)
	{
		Print(OutputLevels::Warn, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void Error(const std::format_string<Args...> format, Args&&... args)
	{
		Print(OutputLevels::Error, format, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void Debug(const std::format_string<Args...> format, Args&&... args)
	{
		Print(OutputLevels::Debug, format, std::forward<Args>(args)...);
	}

	/**
	 * Print an error, show it in a message box on Windows, and exit the game, as `I_Error` does.
	 * A newline is added.
	 */
	template<typename... Args>
	[[noreturn]] void Fatal(const std::format_string<Args...> format, Args&&... args)
	{
		Detail::Fatal(std::format(format, std::forward<Args>(args)...));
	}
}
