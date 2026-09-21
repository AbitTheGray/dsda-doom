// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Entry point of the demo regression suite.

#include <cstdint>

#include <gtest/gtest.h>

#include "HereticTest.hpp"
#include "SpecEnvironment.hpp"

int32_t main(int32_t argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	// Both happen before the tests run, so `--gtest_list_tests` sees the
	// generated Heretic cases and CMake can discover them.
	Spec::RegisterEnvironment();
	RegisterHereticDemos();

	return RUN_ALL_TESTS();
}
