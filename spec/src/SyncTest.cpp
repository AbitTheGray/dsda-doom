// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Demo sync: replaying a recorded run must reproduce its original time.
//	Ported from the `sync` group of the former `spec/sync_spec.rb`.

#include <gtest/gtest.h>

#include "DemoRun.hpp"
#include "SpecEnvironment.hpp"

// complevel 2

// "doom2 30uv in 17:55 by Looper"
TEST(Sync, Doom2AllMapsUvSpeedByLooper)
{
	constexpr DemoOptions options { .lmp = "30uv1755.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "17:55");
}

// coop

// "doom2 20 uv max in 2:22 by termrork & kOeGy (a)"
// The two files are the two players' views of the same run.
TEST(Sync, Doom2Map20UvMaxByTermrorkAndKoegyFirstView)
{
	constexpr DemoOptions options { .lmp = "cm20k222.LMP" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "2:22");
}

// "doom2 20 uv max in 2:22 by termrork & kOeGy (b)"
TEST(Sync, Doom2Map20UvMaxByTermrorkAndKoegySecondView)
{
	constexpr DemoOptions options { .lmp = "cm20t222.LMP" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "2:22");
}

// complevel 9

// "rush 12 uv max in 21:14 by Ancalagon"
TEST(Sync, RushMap12UvMaxByAncalagon)
{
	constexpr DemoOptions options { .lmp = "ru12-2114.lmp", .pwad = "rush.wad" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "21:14");
}

// complevel 11

// "valiant e1 uv speed in 5:13 by Krankdud"
TEST(Sync, ValiantEpisode1UvSpeedByKrankdud)
{
	constexpr DemoOptions options { .lmp = "vae1-513.lmp", .pwad = "Valiant.wad" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "5:13");
}

// heretic
//
// These run the Heretic game code from the Doom IWAD plus HERETIC.WAD as a
// PWAD, exactly as the original suite did.

// "e1 sm max in 52:40 by JCD"
TEST(Sync, HereticEpisode1SkillMaxByJcd)
{
	constexpr DemoOptions options {
		.lmp = "h1m-5240.lmp", .iwad = "DOOM.WAD", .pwad = "HERETIC.WAD", .extra = "-heretic"
	};
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "52:40");
}

// "e2 sm max in 67:02 by JCD"
TEST(Sync, HereticEpisode2SkillMaxByJcd)
{
	constexpr DemoOptions options {
		.lmp = "h2ma6702.lmp", .iwad = "DOOM.WAD", .pwad = "HERETIC.WAD", .extra = "-heretic"
	};
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "67:02");
}

// "e3 sm max in 62:48 by JCD"
TEST(Sync, HereticEpisode3SkillMaxByJcd)
{
	constexpr DemoOptions options {
		.lmp = "h3ma6248.lmp", .iwad = "DOOM.WAD", .pwad = "HERETIC.WAD", .extra = "-heretic"
	};
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "62:48");
}

// "e4 sm speed in 10:55 by veovis"
TEST(Sync, HereticEpisode4SkillSpeedByVeovis)
{
	constexpr DemoOptions options {
		.lmp = "h4sp1055.lmp", .iwad = "DOOM.WAD", .pwad = "HERETIC.WAD", .extra = "-heretic"
	};
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "10:55");
}

// "e5 sm speed in 12:57 by veovis"
TEST(Sync, HereticEpisode5SkillSpeedByVeovis)
{
	constexpr DemoOptions options {
		.lmp = "h5sp1257.lmp", .iwad = "DOOM.WAD", .pwad = "HERETIC.WAD", .extra = "-heretic"
	};
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "12:57");
}

// hexen

// "e1 sk4 max in 45:37 by PVS"
TEST(Sync, HexenEpisode1Skill4MaxByPvs)
{
	constexpr DemoOptions options { .lmp = "me1c4537.lmp", .iwad = "HEXEN.WAD", .extra = "-hexen" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).TotalTime(), "45:37");
}
