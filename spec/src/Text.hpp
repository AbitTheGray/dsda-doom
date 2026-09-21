// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Small text helpers shared by the report parsers.

#pragma once

#include <filesystem>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace Spec
{
	/**
	 * Split into lines, accepting both `\n` and `\r\n`.
	 * @note The game opens `levelstat.txt` in binary mode and writes `\r\n` on every platform.
	 */
	[[nodiscard]] std::vector<std::string_view> SplitLines(std::string_view text);

	/**
	 * Split on runs of spaces and tabs, dropping empty fields.
	 * @note The report writers pad their columns, so empty fields are common.
	 */
	[[nodiscard]] std::vector<std::string_view> SplitWhitespace(std::string_view line);

	/**
	 * Split on every `separator`, dropping the trailing empty fields.
	 * @note Matches Ruby's `String#split`, which the demo list was written for.
	 */
	[[nodiscard]] std::vector<std::string_view> SplitFields(std::string_view line, char separator);

	[[nodiscard]] std::string_view Trim(std::string_view text);

	[[nodiscard]] std::string Join(const std::vector<std::string>& values, std::string_view separator);

	[[nodiscard]] std::expected<std::string, std::string> ReadFile(const std::filesystem::path& file);
}
