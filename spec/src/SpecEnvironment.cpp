// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Where the suite finds its inputs, and what to do when they are absent.

#include <array>
#include <format>
#include <iostream>

#include "SpecEnvironment.hpp"

namespace
{
	// Both are absolute paths supplied by CMake, so the suite does not care
	// which directory it is started from.
	const std::filesystem::path k_dataDirectory { SPEC_DATA_DIRECTORY };
	const std::filesystem::path k_executable { DSDA_EXECUTABLE };

	// Every IWAD the suite can use, for the one-off report at start-up.
	constexpr std::array k_reportedWads {
		std::string_view("DOOM.WAD"),
		std::string_view("DOOM2.WAD"),
		std::string_view("HERETIC.WAD"),
		std::string_view("HEXEN.WAD"),
		std::string_view("Valiant.wad"),
		std::string_view("rush.wad"),
	};

	class SpecEnvironment final : public ::testing::Environment
	{
	public:
		void SetUp() override
		{
			if(!std::filesystem::exists(k_executable))
			{
				std::cout << std::format(
					"[ spec     ] game binary not found at '{}' - every test will be skipped\n",
					k_executable.string()
				);
				return;
			}

			std::vector<std::string> missing;

			for(const std::string_view wad : k_reportedWads)
				if(!std::filesystem::exists(Spec::WadPath(wad)))
					missing.emplace_back(wad);

			if(missing.empty())
				return;

			std::cout << std::format(
				"[ spec     ] {} of {} WADs are missing from '{}': {}\n"
				"[ spec     ] tests needing them will be reported as skipped\n",
				missing.size(), k_reportedWads.size(),
				(k_dataDirectory / "support" / "wads").string(),
				Spec::Join(missing, ", ")
			);
		}
	};
}

namespace Spec
{
	const std::filesystem::path& DataDirectory()
	{
		return k_dataDirectory;
	}

	const std::filesystem::path& Executable()
	{
		return k_executable;
	}

	std::filesystem::path WadPath(const std::string_view wad)
	{
		return k_dataDirectory / "support" / "wads" / wad;
	}

	std::filesystem::path LmpPath(const std::string_view lmp)
	{
		return k_dataDirectory / "support" / "lmps" / lmp;
	}

	std::vector<std::string> MissingInputs(const DemoOptions& options)
	{
		std::vector<std::string> missing;

		if(!std::filesystem::exists(k_executable))
			missing.push_back(std::format("game binary '{}'", k_executable.string()));

		if(!options.iwad.empty() && !std::filesystem::exists(WadPath(options.iwad)))
			missing.emplace_back(options.iwad);

		if(!options.pwad.empty() && !std::filesystem::exists(WadPath(options.pwad)))
			missing.emplace_back(options.pwad);

		if(!options.lmp.empty() && !std::filesystem::exists(LmpPath(options.lmp)))
			missing.emplace_back(options.lmp);

		return missing;
	}

	void RegisterEnvironment()
	{
		::testing::AddGlobalTestEnvironment(new SpecEnvironment);
	}
}
