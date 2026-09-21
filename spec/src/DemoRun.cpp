// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Replays one demo through the built game and keeps the reports it wrote.

#include <atomic>
#include <cstdlib>
#include <format>
#include <random>
#include <stdexcept>

#include "DemoRun.hpp"
#include "SpecEnvironment.hpp"
#include "Text.hpp"

namespace
{
	constexpr std::string_view k_logName = "dsda-doom.log";
	constexpr std::string_view k_levelstatName = "levelstat.txt";
	constexpr std::string_view k_analysisName = "analysis.txt";

	// Keep failure messages readable when the game is chatty.
	constexpr std::size_t k_maximumReportedOutput = 2048;

	[[nodiscard]] std::string UniqueDirectoryName()
	{
		static std::atomic<uint32_t> counter { 0 };
		static const uint32_t seed = std::random_device {}();

		return std::format("dsda-spec-{:08x}-{}", seed, counter.fetch_add(1));
	}

	/**
	 * Quote one argument for the command processor.
	 * @note `std::system` is the only portable way to start a process, and it
	 *       hands the string to `sh` or to `cmd.exe`. Double quotes are the one
	 *       form both agree on; our own paths never contain quotes themselves.
	 */
	[[nodiscard]] std::string Quote(const std::filesystem::path& path)
	{
		return std::format("\"{}\"", path.string());
	}

	[[nodiscard]] std::string_view Tail(const std::string& text)
	{
		if(text.size() <= k_maximumReportedOutput)
			return text;

		return std::string_view(text).substr(text.size() - k_maximumReportedOutput);
	}
}

TemporaryDirectory::TemporaryDirectory()
	: m_path(std::filesystem::temp_directory_path() / UniqueDirectoryName())
{
	std::filesystem::create_directories(m_path);
}

TemporaryDirectory::~TemporaryDirectory()
{
	std::error_code error;
	std::filesystem::remove_all(m_path, error);
}

ScopedWorkingDirectory::ScopedWorkingDirectory(const std::filesystem::path& directory)
	: m_previous(std::filesystem::current_path())
{
	std::filesystem::current_path(directory);
}

ScopedWorkingDirectory::~ScopedWorkingDirectory()
{
	std::error_code error;
	std::filesystem::current_path(m_previous, error);
}

DemoRun::DemoRun(const DemoOptions& options)
{
	m_command = Quote(Spec::Executable());
	m_command += std::format(" -iwad {}", Quote(Spec::WadPath(options.iwad)));

	if(!options.pwad.empty())
		m_command += std::format(" -file {}", Quote(Spec::WadPath(options.pwad)));

	m_command += std::format(" -fastdemo {}", Quote(Spec::LmpPath(options.lmp)));
	m_command += " -nosound -nomusic -nodraw -levelstat -analysis";

	// The autoload directories are the developer's own (`I_ConfigDir`), so
	// whatever is in them would be merged into every run and change the very
	// numbers these tests compare. Every demo here names the WADs it needs.
	m_command += " -noautoload";

	if(!options.extra.empty())
		m_command += std::format(" {}", options.extra);

	// Both shells understand this redirect, and it keeps the game's chatter out
	// of the test output while remaining available for failure messages.
	m_command += std::format(" > \"{}\" 2>&1", k_logName);

	{
		const ScopedWorkingDirectory working(m_directory.Path());
		m_exitCode = static_cast<int32_t>(std::system(m_command.c_str()));
	}

	m_output = Spec::ReadFile(m_directory.Path() / k_logName).value_or(std::string {});
	m_levelstat = Spec::ReadFile(m_directory.Path() / k_levelstatName).value_or(std::string {});
	m_analysis = Spec::ReadFile(m_directory.Path() / k_analysisName).value_or(std::string {});
}

void DemoRun::Fail(const std::string_view what) const
{
	throw std::runtime_error(std::format(
		"{}\ncommand: {}\nexit code: {}\ngame output:\n{}",
		what, m_command, m_exitCode, Tail(m_output)
	));
}

std::string DemoRun::TotalTime() const
{
	const auto total = Levelstat::Parse(m_levelstat).TotalTime();

	if(!total)
		Fail(std::format("could not read the total time: {}", total.error()));

	return *total;
}

Analysis DemoRun::GetAnalysis() const
{
	if(m_analysis.empty())
		Fail("the game wrote no analysis.txt");

	const auto analysis = Analysis::Parse(m_analysis);

	if(!analysis)
		Fail(std::format("could not read analysis.txt: {}", analysis.error()));

	return *analysis;
}

Category DemoRun::GetCategory() const
{
	const Analysis analysis = GetAnalysis();
	const auto category = ParseCategory(analysis.category);

	if(!category)
		Fail(category.error());

	return *category;
}
