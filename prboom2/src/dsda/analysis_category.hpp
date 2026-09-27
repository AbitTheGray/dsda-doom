// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Speedrun categories `dsda_DetectCategory` detects, and the names
//	`analysis.txt` and the text file write for them.
//	The spec suite parses the names back into the enum.

#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "cpp/EnumArray.hpp"
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
	Count,
};

inline constexpr EnumArray<std::string_view, Category> k_categoryNames = {
	"Other",
	"NoMo",
	"NoMo 100S",
	"UV Respawn",
	"UV Fast",
	"UV Max",
	"UV Tyson",
	"Stroller",
	"Pacifist",
	"UV Speed",
	"NM 100S",
	"NM Speed",
};

[[nodiscard]] inline std::string to_string(const Category value)
{
	if(value >= Category::Count)
		return std::format("<unknown category {}>", std::to_underlying(value));

	return std::string(k_categoryNames[value]);
}

DOOM_STD_FORMATTER_TOSTRING(Category)
