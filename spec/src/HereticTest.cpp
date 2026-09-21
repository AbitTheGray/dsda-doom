// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	The full Heretic demo archive, one test per recorded run.
//	Ported from the former `spec/heretic_spec.rb`, which generated its examples
//	from the same `spec/support/lmps/heretic/list.txt`.
//
//	These are registered at start-up rather than written out, so the list file
//	stays the single place where demos are added.

#include <format>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include "DemoRun.hpp"
#include "HereticTest.hpp"
#include "SpecEnvironment.hpp"
#include "Text.hpp"

namespace
{
	// "description|expected time|path below spec/support/lmps|extra arguments"
	constexpr std::size_t k_descriptionField = 0;
	constexpr std::size_t k_timeField = 1;
	constexpr std::size_t k_lmpField = 2;
	constexpr std::size_t k_extraField = 3;

	// The trailing field is omitted for most demos, so a line may be short.
	constexpr std::size_t k_requiredFields = 3;

	/** Turn a human description into something usable as a test name. */
	[[nodiscard]] std::string SanitiseName(const std::string_view description)
	{
		std::string name;
		bool pendingSeparator = false;

		for(const char c : description)
		{
			const bool isWord = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');

			if(!isWord)
			{
				pendingSeparator = !name.empty();
				continue;
			}

			if(pendingSeparator)
			{
				name += '_';
				pendingSeparator = false;
			}

			name += c;
		}

		return name;
	}

	class HereticDemoTest final : public ::testing::Test
	{
	public:
		HereticDemoTest(std::string lmp, std::string expectedTime, std::string extra)
			: m_lmp(std::move(lmp))
			, m_expectedTime(std::move(expectedTime))
			, m_extra(std::move(extra))
		{
		}

		void TestBody() override
		{
			const DemoOptions options {
				.lmp = m_lmp,
				.iwad = "HERETIC.WAD",
				.pwad = {},
				.extra = m_extra,
			};
			SPEC_REQUIRE_INPUTS(options);

			EXPECT_EQ(DemoRun(options).TotalTime(), m_expectedTime);
		}

	private:
		std::string m_lmp;
		std::string m_expectedTime;
		std::string m_extra;
	};
}

void RegisterHereticDemos()
{
	const std::filesystem::path list = Spec::LmpPath("heretic/list.txt");
	const auto contents = Spec::ReadFile(list);

	if(!contents)
	{
		// Notices go to stderr so they never reach `--gtest_list_tests` output,
		// which CTest parses to discover these generated cases.
		std::cerr << std::format(
			"[ spec     ] no Heretic demo list at '{}' - that suite is empty\n", list.string()
		);
		return;
	}

	int32_t index = 0;

	for(const std::string_view line : Spec::SplitLines(*contents))
	{
		if(Spec::Trim(line).empty())
			continue;

		const auto fields = Spec::SplitFields(line, '|');

		if(fields.size() < k_requiredFields)
		{
			std::cerr << std::format("[ spec     ] ignoring malformed demo list line '{}'\n", line);
			continue;
		}

		++index;

		std::string lmp(fields[k_lmpField]);
		std::string expectedTime(fields[k_timeField]);
		std::string extra(fields.size() > k_extraField ? fields[k_extraField] : std::string_view {});

		// The index keeps the names unique and in file order.
		const std::string name = std::format(
			"Demo{:04}_{}", index, SanitiseName(fields[k_descriptionField])
		);

		::testing::RegisterTest(
			"HereticDemo", name.c_str(), nullptr, nullptr, __FILE__, __LINE__,
			[lmp = std::move(lmp), expectedTime = std::move(expectedTime), extra = std::move(extra)]
			() -> ::testing::Test*
			{
				return new HereticDemoTest(lmp, expectedTime, extra);
			}
		);
	}
}
