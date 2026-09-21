// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the `levelstat.txt` report.

#include <format>

#include "Levelstat.hpp"
#include "Text.hpp"

namespace
{
	// "MAP01 - 1:23.45 (2:03)  K: 10/10  I: 5/5  S: 1/2"
	//    0    1    2       3
	constexpr std::size_t k_totalTimeField = 3;
}

std::expected<std::string, std::string> Levelstat::TotalTime() const
{
	if(levels.empty())
		return std::string("00:00");

	const std::string& line = levels.back();
	const auto fields = Spec::SplitWhitespace(line);

	if(fields.size() <= k_totalTimeField)
		return std::unexpected(std::format("malformed levelstat line '{}'", line));

	// `e6y_WriteStats` pads this column to the width of the largest total time.
	// The last level always holds that largest value, so its own field is never
	// padded and the parentheses stay glued to the digits.
	std::string_view total = fields[k_totalTimeField];

	if(total.starts_with('('))
		total.remove_prefix(1);

	if(total.ends_with(')'))
		total.remove_suffix(1);

	return std::string(total);
}

Levelstat Levelstat::Parse(const std::string_view contents)
{
	Levelstat levelstat;

	for(const std::string_view line : Spec::SplitLines(contents))
		if(!Spec::Trim(line).empty())
			levelstat.levels.emplace_back(line);

	return levelstat;
}

std::expected<Levelstat, std::string> Levelstat::Read(const std::filesystem::path& file)
{
	const auto contents = Spec::ReadFile(file);

	if(!contents)
		return std::unexpected(contents.error());

	return Parse(*contents);
}
