// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the `levelstat.txt` report.
//	Mirrors `e6y_WriteStats` in `prboom2/src/e6y.c`.

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

struct Levelstat
{
	// One line per finished level, in the order the game wrote them.
	std::vector<std::string> levels;

	/**
	 * The cumulative time of the final level, e.g. "17:55".
	 * @note Returns "00:00" when no level was finished. `DemoRun::TotalTime`
	 *       treats that as a failed run rather than a time, because a report
	 *       with no levels in it says nothing about why.
	 */
	[[nodiscard]] std::expected<std::string, std::string> TotalTime() const;

	[[nodiscard]] static Levelstat Parse(std::string_view contents);
	[[nodiscard]] static std::expected<Levelstat, std::string> Read(const std::filesystem::path& file);
};
