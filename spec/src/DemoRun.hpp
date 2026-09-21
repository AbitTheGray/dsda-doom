// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Replays one demo through the built game and keeps the reports it wrote.
//
//	The game writes `levelstat.txt` and `analysis.txt` into the working
//	directory under fixed names, so every run gets a private temporary
//	directory. Its configuration, save games and autoload WADs live elsewhere,
//	in the developer's home directory (`I_ConfigDir` in
//	`prboom2/src/SDL/i_system.c`), which is why the run passes `-noautoload`.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "cpp/Util.hpp"

#include "Analysis.hpp"
#include "Category.hpp"
#include "DemoOptions.hpp"
#include "Levelstat.hpp"

/** Creates a private directory and removes it again. */
class TemporaryDirectory
{
public:
	TemporaryDirectory();
	~TemporaryDirectory();

	DOOM_NO_COPY(TemporaryDirectory)
	DOOM_NO_MOVE(TemporaryDirectory)

	[[nodiscard]] const std::filesystem::path& Path() const noexcept { return m_path; }

private:
	std::filesystem::path m_path;
};

/** Switches the working directory and switches it back. */
class ScopedWorkingDirectory
{
public:
	explicit ScopedWorkingDirectory(const std::filesystem::path& directory);
	~ScopedWorkingDirectory();

	DOOM_NO_COPY(ScopedWorkingDirectory)
	DOOM_NO_MOVE(ScopedWorkingDirectory)

private:
	std::filesystem::path m_previous;
};

class DemoRun
{
public:
	explicit DemoRun(const DemoOptions& options);

	DOOM_NO_COPY(DemoRun)
	DOOM_NO_MOVE(DemoRun)

	/** The cumulative time of the last finished level, e.g. "17:55". */
	[[nodiscard]] std::string TotalTime() const;

	[[nodiscard]] Analysis GetAnalysis() const;

	[[nodiscard]] Category GetCategory() const;

	/** Everything the game printed, kept so failures can show it. */
	[[nodiscard]] const std::string& Output() const noexcept { return m_output; }

	[[nodiscard]] int32_t ExitCode() const noexcept { return m_exitCode; }

private:
	[[noreturn]] void Fail(std::string_view what) const;

	TemporaryDirectory m_directory;
	std::string m_command;
	std::string m_output;
	std::string m_levelstat;
	std::string m_analysis;
	int32_t m_exitCode = 0;
};
