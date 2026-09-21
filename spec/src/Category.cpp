// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Speedrun categories the game reports in `analysis.txt`.

#include <algorithm>
#include <array>
#include <format>
#include <utility>

#include "Category.hpp"

namespace
{
	struct CategoryName
	{
		Category category;
		std::string_view name;
	};

	// Every string `dsda_DetectCategory` can return.
	constexpr std::array k_categoryNames {
		CategoryName { Category::Other,     "Other" },
		CategoryName { Category::NoMo,      "NoMo" },
		CategoryName { Category::NoMo100S,  "NoMo 100S" },
		CategoryName { Category::UvRespawn, "UV Respawn" },
		CategoryName { Category::UvFast,    "UV Fast" },
		CategoryName { Category::UvMax,     "UV Max" },
		CategoryName { Category::UvTyson,   "UV Tyson" },
		CategoryName { Category::Stroller,  "Stroller" },
		CategoryName { Category::Pacifist,  "Pacifist" },
		CategoryName { Category::UvSpeed,   "UV Speed" },
		CategoryName { Category::Nm100S,    "NM 100S" },
		CategoryName { Category::NmSpeed,   "NM Speed" },
	};
}

std::string to_string(const Category value)
{
	const auto match = std::ranges::find(k_categoryNames, value, &CategoryName::category);

	if(match == k_categoryNames.end())
		return std::format("<unknown category {}>", std::to_underlying(value));

	return std::string(match->name);
}

std::expected<Category, std::string> ParseCategory(const std::string_view text)
{
	const auto match = std::ranges::find(k_categoryNames, text, &CategoryName::name);

	if(match == k_categoryNames.end())
		return std::unexpected(std::format("unknown category '{}'", text));

	return match->category;
}
