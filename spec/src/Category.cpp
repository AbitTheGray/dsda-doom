// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the speedrun category the game reports in `analysis.txt`.

#include <algorithm>
#include <format>

#include "Category.hpp"

std::expected<Category, std::string> ParseCategory(const std::string_view text)
{
	const auto& keys = k_categoryNames.Keys();
	const auto match = std::ranges::find(keys, text, [](const Category key) { return k_categoryNames[key]; });

	if(match == keys.end())
		return std::unexpected(std::format("unknown category '{}'", text));

	return *match;
}
