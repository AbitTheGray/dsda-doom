// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text File

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <ostream>
#include <print>
#include <string>
#include <string_view>
#include <utility>

#include "doomstat.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"

#include "dsda.hpp"
#include "dsda/analysis.hpp"
#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/playback.hpp"

#include "text_file.hpp"

extern int dsda_last_leveltime, dsda_last_gamemap, dsda_startmap;

// The demo's name with a `.lmp` extension (in any case) swapped for `.txt`, or `.txt` appended to any other name.
// Empty when no demo is played back.
static std::string dsda_TextFileName()
{
	const char* const playback_name = dsda_PlaybackName();

	if(!playback_name)
		return {};

	constexpr std::string_view k_demoExtension = ".lmp";
	std::string_view name = playback_name;

	const auto lower = [](const char c) { return std::tolower(static_cast<unsigned char>(c)); };

	if(
		name.size() > k_demoExtension.size()
		&& std::ranges::equal(name.substr(name.size() - k_demoExtension.size()), k_demoExtension, {}, lower, lower)
	)
		name.remove_suffix(k_demoExtension.size());

	return std::format("{}.txt", name);
}

static int dsda_IL()
{
	extern int dsda_startmap;

	return dsda_startmap == dsda_last_gamemap;
}

// The name of a whole-episode or whole-game movie; empty for any other run.
static std::string_view dsda_Movie()
{
	if(gamemode == GameMode::Commercial)
	{
		if(dsda_startmap == 1 && dsda_last_gamemap == 10)
			return "Episode 1";
		if(dsda_startmap == 11 && dsda_last_gamemap == 20)
			return "Episode 2";
		if(dsda_startmap == 21 && dsda_last_gamemap == 30)
			return "Episode 3";
		if(dsda_startmap == 1 && dsda_last_gamemap == 30)
			return "D2All";
	}
	else
	{
		if(dsda_startmap == 1 && dsda_last_gamemap == 8)
		{
			if(gameepisode == 1)
				return "Episode 1";
			if(gameepisode == 2)
				return "Episode 2";
			if(gameepisode == 3)
				return "Episode 3";
			if(gameepisode == 4)
				return "Episode 4";
		}
	}

	return {};
}

static std::string dsda_TextFileTime()
{
	if(dsda_IL())
		return std::format(
			"{}:{:05.2f}",
			dsda_last_leveltime / TICRATE / 60,
			static_cast<float>(dsda_last_leveltime % (60 * TICRATE)) / TICRATE
		);

	return std::format(
		"{}:{:02}",
		totalleveltimes / TICRATE / 60,
		(totalleveltimes / TICRATE) % 60
	);
}

void dsda_ExportTextFile()
{
	if(!dsda_Flag(ArgId::ExportTextFile))
		return;

	const std::string name = dsda_TextFileName();
	if(name.empty())
		return;

	// The name is UTF-8; a path from `std::u8string` keeps non-ASCII names working on Windows, as `M_OpenFile` did.
	const std::filesystem::path path = std::u8string(name.begin(), name.end());
	// Binary, as upstream's "wb": `\n` stays `\n` on every platform.
	std::ofstream file(path, std::ios::binary);
	if(!file)
		I_Error("Unable to export text file!");

	std::println(file, "Doom Speed Demo Archive");
	std::println(file, "https://dsdarchive.com/");
	std::println(file, "");
	if(const dsda_arg_t* const iwad = dsda_Arg(ArgId::Iwad); iwad->found)
		std::println(file, "Iwad:      {}", PathFindFileName(iwad->value.v_string));
	if(const dsda_arg_t* const pwad = dsda_Arg(ArgId::File); pwad->found)
		std::println(file, "Pwad:      {}", PathFindFileName(pwad->value.v_string_array[0]));

	if(dsda_IL())
		std::println(file, "Map:       {}", dsda_MapLumpName(gameepisode, dsda_startmap));
	else
	{
		if(const std::string_view movie = dsda_Movie(); !movie.empty())
			std::println(file, "Movie:     {}", movie);
		else
		{
			std::print(file, "Movie:     {}", dsda_MapLumpName(gameepisode, dsda_startmap));
			std::println(file, " - {}", dsda_MapLumpName(gameepisode, dsda_last_gamemap));
		}
	}

	std::println(file, "Skill:     {}", gameskill + 1);
	std::println(file, "Category:  {}", dsda_DetectCategory());
	std::println(file, "Exe:       {} -complevel {}", PROJECT_STRING, std::to_underlying(compatibility_level));
	std::println(file, "");

	std::println(file, "Time:      {}", dsda_TextFileTime());

	std::println(file, "");
	std::println(file, "Author:    {}", dsda_StringConfig(ConfigId::PlayerName));
	std::println(file, "");
	std::println(file, "Comments:");
}
