// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the `analysis.txt` report.

#include <charconv>
#include <format>
#include <utility>

#include "Analysis.hpp"
#include "Text.hpp"

namespace
{
	[[nodiscard]] std::expected<int32_t, std::string> ParseInteger(const std::string_view text)
	{
		int32_t value = 0;
		const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);

		if(error != std::errc {} || end != text.data() + text.size())
			return std::unexpected(std::format("'{}' is not an integer", text));

		return value;
	}

	// The writer prints every flag with `%d`, so only "1" counts as set.
	[[nodiscard]] std::expected<bool, std::string> ParseFlag(const std::string_view text)
	{
		const auto value = ParseInteger(text);

		if(!value)
			return std::unexpected(value.error());

		return *value != 0;
	}

	[[nodiscard]] std::expected<Signature, std::string> ParseSignature(const std::string_view text)
	{
		const auto value = ParseInteger(text);

		if(!value)
			return std::unexpected(value.error());

		for(const Signature signature : {Signature::Invalid, Signature::Unsigned, Signature::Signed})
		{
			if(*value == std::to_underlying(signature))
				return signature;
		}

		return std::unexpected(std::format("'{}' is not a signature state", text));
	}
}

std::expected<Analysis, std::string> ParseAnalysis(const std::string_view contents)
{
	Analysis analysis;

	for(const std::string_view line : Spec::SplitLines(contents))
	{
		if(Spec::Trim(line).empty())
			continue;

		const std::size_t separator = line.find(' ');

		if(separator == std::string_view::npos)
			return std::unexpected(std::format("malformed line '{}'", line));

		const std::string_view key = line.substr(0, separator);
		// The category is the one value that contains spaces, e.g. "UV Max".
		const std::string_view value = line.substr(separator + 1);

		const auto assignFlag = [&](bool& target) -> std::expected<void, std::string>
		{
			const auto parsed = ParseFlag(value);

			if(!parsed)
				return std::unexpected(std::format("{}: {}", key, parsed.error()));

			target = *parsed;
			return {};
		};

		const auto assignInteger = [&](int32_t& target) -> std::expected<void, std::string>
		{
			const auto parsed = ParseInteger(value);

			if(!parsed)
				return std::unexpected(std::format("{}: {}", key, parsed.error()));

			target = *parsed;
			return {};
		};

		const auto assignSignature = [&](Signature& target) -> std::expected<void, std::string>
		{
			const auto parsed = ParseSignature(value);

			if(!parsed)
				return std::unexpected(std::format("{}: {}", key, parsed.error()));

			target = *parsed;
			return {};
		};

		std::expected<void, std::string> result;

		if(key == "skill")                 result = assignInteger(analysis.skill);
		else if(key == "nomonsters")       result = assignFlag(analysis.noMonsters);
		else if(key == "respawn")          result = assignFlag(analysis.respawn);
		else if(key == "fast")             result = assignFlag(analysis.fast);
		else if(key == "pacifist")         result = assignFlag(analysis.pacifist);
		else if(key == "stroller")         result = assignFlag(analysis.stroller);
		else if(key == "reality")          result = assignFlag(analysis.reality);
		else if(key == "almost_reality")   result = assignFlag(analysis.almostReality);
		else if(key == "reborn")           result = assignFlag(analysis.reborn);
		else if(key == "100k")             result = assignFlag(analysis.hundredKills);
		else if(key == "100s")             result = assignFlag(analysis.hundredSecrets);
		else if(key == "missed_monsters")  result = assignInteger(analysis.missedMonsters);
		else if(key == "missed_secrets")   result = assignInteger(analysis.missedSecrets);
		else if(key == "weapon_collector") result = assignFlag(analysis.weaponCollector);
		else if(key == "tyson_weapons")    result = assignFlag(analysis.tysonWeapons);
		else if(key == "turbo")            result = assignFlag(analysis.turbo);
		else if(key == "solo_net")         result = assignFlag(analysis.soloNet);
		else if(key == "coop_spawns")      result = assignFlag(analysis.coopSpawns);
		else if(key == "category")         analysis.category = value;
		else if(key == "signature")        result = assignSignature(analysis.signature);
		// An unknown key means the game grew a field we do not track yet, which
		// is not a reason to fail the run that produced it.

		if(!result)
			return std::unexpected(result.error());
	}

	return analysis;
}

std::expected<Analysis, std::string> ReadAnalysis(const std::filesystem::path& file)
{
	const auto contents = Spec::ReadFile(file);

	if(!contents)
		return std::unexpected(contents.error());

	return ParseAnalysis(*contents);
}
