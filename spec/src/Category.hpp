// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the speedrun category the game reports in `analysis.txt`.
//	The enum and its names are the game's, in `prboom2/src/dsda/analysis_category.hpp`.

#pragma once

#include <expected>
#include <string>
#include <string_view>

#include "dsda/analysis_category.hpp"

[[nodiscard]] std::expected<Category, std::string> ParseCategory(std::string_view text);
