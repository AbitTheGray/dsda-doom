// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Speedrun categories the game reports in `analysis.txt`.
//	Mirrors `dsda_DetectCategory` in `prboom2/src/dsda/analysis.c` - the strings
//	must stay identical to the ones returned there.

#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "cpp/Util.hpp"

enum struct Category : uint8_t
{
	Other,
	NoMo,
	NoMo100S,
	UvRespawn,
	UvFast,
	UvMax,
	UvTyson,
	Stroller,
	Pacifist,
	UvSpeed,
	Nm100S,
	NmSpeed,
};

[[nodiscard]] std::string to_string(Category value);

[[nodiscard]] std::expected<Category, std::string> ParseCategory(std::string_view text);

DOOM_STD_FORMATTER_TOSTRING(Category)
