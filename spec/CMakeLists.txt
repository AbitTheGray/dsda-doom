# SPDX-License-Identifier: GPL-2.0-or-later

# Demo regression suite: replays recorded demos through the built game and
# checks the reports it writes. See README.md for how to run it.

include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

add_executable(dsda-spec
    "src/Analysis.cpp"
    "src/AnalysisTest.cpp"
    "src/Category.cpp"
    "src/CategoryTest.cpp"
    "src/DemoRun.cpp"
    "src/HereticTest.cpp"
    "src/Levelstat.cpp"
    "src/Main.cpp"
    "src/SpecEnvironment.cpp"
    "src/SyncTest.cpp"
    "src/Text.cpp"
)

set_target_properties(dsda-spec PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    # Beside the game binary, the same as `dsda-doom` itself.
    RUNTIME_OUTPUT_DIRECTORY "${DSDA_OUTPUT_PATH}"
)

target_include_directories(dsda-spec PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    # For `cpp/Util.hpp`, shared with the game.
    "${CMAKE_CURRENT_SOURCE_DIR}/../prboom2/src"
)

# Absolute paths, so the suite does not care which directory it is started from
# and no test has to guess where the binary ended up.
target_compile_definitions(dsda-spec PRIVATE
    "DSDA_EXECUTABLE=\"$<TARGET_FILE:dsda-doom>\""
    "SPEC_DATA_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

target_link_libraries(dsda-spec PRIVATE GTest::gtest)

# The suite runs the game, so it has to be built first.
add_dependencies(dsda-spec dsda-doom)

# One CTest entry per case, so `ctest -j` can spread them over cores. Each run
# gets its own working directory, so they do not fight over the report files.
#
# The Heretic archive is more than a thousand demos and was excluded by default
# in the previous suite; here it is a separate label instead.
gtest_discover_tests(dsda-spec
    TEST_FILTER "-HereticDemo.*"
    PROPERTIES LABELS "spec"
    DISCOVERY_MODE PRE_TEST
)

gtest_discover_tests(dsda-spec
    TEST_FILTER "HereticDemo.*"
    PROPERTIES LABELS "heretic"
    DISCOVERY_MODE PRE_TEST
)
