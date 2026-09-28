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

void I_EnableVerboseLogging();
void I_DisableAllLogging();
void I_DisableMessageBoxes();


#ifdef __cplusplus
}
#endif

/**
 * Console output formatted with `std::format`, e.g. `Log::Info("FINISHED: {}\n", map)`.
 * No newline is added (as upstream's `lprintf` did not add one), so a line can be printed in parts.
 * A message is never cut, unless the build enables `LIMIT_LOG_MESSAGES`, which cuts it at 2047 characters as upstream's `lprintf` did.
 */
namespace Log
{
	namespace Detail
	{
		void Print(OutputLevels level, std::string_view text);
		[[noreturn]] void Fatal(std::string_view text);
		void Alert(std::string_view text);
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
	 * Print an error, show it in a message box on Windows, and exit the game, as upstream's `I_Error` did.
	 * A newline is added.
	 */
	template<typename... Args>
	[[noreturn]] void Fatal(const std::format_string<Args...> format, Args&&... args)
	{
		Detail::Fatal(std::format(format, std::forward<Args>(args)...));
	}

	/**
	 * Print a warning and show it in a message box on Windows, as upstream's `I_Warn` did.
	 * A newline is added.
	 */
	template<typename... Args>
	void Alert(const std::format_string<Args...> format, Args&&... args)
	{
		Detail::Alert(std::format(format, std::forward<Args>(args)...));
	}
}
