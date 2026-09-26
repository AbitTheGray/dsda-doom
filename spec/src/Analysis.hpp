// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the `analysis.txt` report.
//	The struct itself is the game's, filled by `dsda_WriteAnalysis` in
//	`prboom2/src/dsda/analysis.cpp`.

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

#include "dsda/analysis_report.hpp"

[[nodiscard]] std::expected<Analysis, std::string> ParseAnalysis(std::string_view contents);
[[nodiscard]] std::expected<Analysis, std::string> ReadAnalysis(const std::filesystem::path& file);
