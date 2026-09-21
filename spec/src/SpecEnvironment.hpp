// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Where the suite finds its inputs, and what to do when they are absent.
//	The IWADs are commercial data that cannot be checked in, so a test whose
//	inputs are missing is reported as skipped rather than failed.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "DemoOptions.hpp"
#include "Text.hpp"

namespace Spec
{
	/** The `spec` directory, as an absolute path baked in at configure time. */
	[[nodiscard]] const std::filesystem::path& DataDirectory();

	/** The `dsda-doom` binary this suite was built against. */
	[[nodiscard]] const std::filesystem::path& Executable();

	[[nodiscard]] std::filesystem::path WadPath(std::string_view wad);

	[[nodiscard]] std::filesystem::path LmpPath(std::string_view lmp);

	/** The inputs `options` needs that are not on disk, described for a human. */
	[[nodiscard]] std::vector<std::string> MissingInputs(const DemoOptions& options);

	/** Registers the listener that reports missing inputs once per run. */
	void RegisterEnvironment();
}

/**
 * Skip the current test when its demo, IWAD or PWAD is unavailable.
 * @note Must be used inside a test body - `GTEST_SKIP` returns from it.
 */
#define SPEC_REQUIRE_INPUTS(a_options) \
	do \
	{ \
		const std::vector<std::string> spec_missing = ::Spec::MissingInputs(a_options); \
		if(!spec_missing.empty()) \
			GTEST_SKIP() << "missing input: " << ::Spec::Join(spec_missing, ", "); \
	} \
	while(false)
