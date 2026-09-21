// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Small text helpers shared by the report parsers.

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>
#include <sstream>

#include "Text.hpp"

namespace
{
	[[nodiscard]] constexpr bool IsSpace(const char c) noexcept
	{
		return c == ' ' || c == '\t';
	}

	[[nodiscard]] std::string_view AsView(const auto& subrange)
	{
		return std::string_view(subrange.begin(), subrange.end());
	}
}

namespace Spec
{
	std::vector<std::string_view> SplitLines(const std::string_view text)
	{
		std::vector<std::string_view> lines;

		for(const auto& part : text | std::views::split('\n'))
		{
			std::string_view line = AsView(part);

			if(line.ends_with('\r'))
				line.remove_suffix(1);

			lines.push_back(line);
		}

		// A trailing newline produces one empty line that carries no record.
		if(!lines.empty() && lines.back().empty())
			lines.pop_back();

		return lines;
	}

	std::vector<std::string_view> SplitWhitespace(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0;

		while(start < line.size())
		{
			while(start < line.size() && IsSpace(line[start]))
				++start;

			if(start >= line.size())
				break;

			std::size_t end = start;
			while(end < line.size() && !IsSpace(line[end]))
				++end;

			fields.push_back(line.substr(start, end - start));
			start = end;
		}

		return fields;
	}

	std::vector<std::string_view> SplitFields(const std::string_view line, const char separator)
	{
		std::vector<std::string_view> fields;

		for(const auto& part : line | std::views::split(separator))
			fields.push_back(AsView(part));

		while(!fields.empty() && fields.back().empty())
			fields.pop_back();

		return fields;
	}

	std::string_view Trim(std::string_view text)
	{
		while(!text.empty() && IsSpace(text.front()))
			text.remove_prefix(1);

		while(!text.empty() && IsSpace(text.back()))
			text.remove_suffix(1);

		return text;
	}

	std::string Join(const std::vector<std::string>& values, const std::string_view separator)
	{
		std::string joined;

		for(const auto& [index, value] : std::views::enumerate(values))
		{
			if(index != 0)
				joined += separator;

			joined += value;
		}

		return joined;
	}

	std::expected<std::string, std::string> ReadFile(const std::filesystem::path& file)
	{
		std::ifstream stream(file, std::ios::binary);

		if(!stream)
			return std::unexpected(std::format("could not open '{}'", file.string()));

		std::ostringstream contents;
		contents << stream.rdbuf();

		return contents.str();
	}
}
