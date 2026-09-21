// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	The speedrun category the game derives from a run.
//	Ported from the `category` group of the former `spec/category_spec.rb`.

#include <gtest/gtest.h>

#include "Category.hpp"
#include "DemoRun.hpp"
#include "SpecEnvironment.hpp"

// "doom2 map 1 uv speed by Thomas Pilger"
TEST(DetectedCategory, UvSpeedByThomasPilger)
{
	constexpr DemoOptions options { .lmp = "lv01-005.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::UvSpeed);
}

// "doom2 map 1 uv max by Xit Vono"
TEST(DetectedCategory, UvMaxByXitVono)
{
	constexpr DemoOptions options { .lmp = "lv01-039.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::UvMax);
}

// "doom2 map 1 tyson by j4rio"
TEST(DetectedCategory, UvTysonByJ4rio)
{
	constexpr DemoOptions options { .lmp = "lv01t040.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::UvTyson);
}

// "doom2 map 8 pacifist by 4shockblast"
TEST(DetectedCategory, PacifistBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "pa08-020.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::Pacifist);
}

// "doom2 map 8 stroller by 4shockblast"
TEST(DetectedCategory, StrollerBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "lv08str037.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::Stroller);
}

// "doom2 map 4 nm speed by Vile"
TEST(DetectedCategory, NmSpeedByVile)
{
	constexpr DemoOptions options { .lmp = "nm04-036.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::NmSpeed);
}

// "doom2 episode 1 nm100s in 11:56 by JCD"
TEST(DetectedCategory, Nm100SecretsByJcd)
{
	constexpr DemoOptions options { .lmp = "1156ns01.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::Nm100S);
}

// "doom2 map 1 nomonsters by depr4vity"
TEST(DetectedCategory, NoMoByDepr4vity)
{
	constexpr DemoOptions options { .lmp = "lv01o497.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::NoMo);
}

// "doom2 map 1 nomo100s by 4shockblast"
TEST(DetectedCategory, NoMo100SecretsBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "os01-2394.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::NoMo100S);
}

// "doom2 map 2 uv respawn by Looper"
TEST(DetectedCategory, UvRespawnByLooper)
{
	constexpr DemoOptions options { .lmp = "re02-107.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::UvRespawn);
}

// "doom2 map 4 uv fast by Radek Pecka"
TEST(DetectedCategory, UvFastByRadekPecka)
{
	constexpr DemoOptions options { .lmp = "fa04-109.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::UvFast);
}

// "doom2 done turbo quicker by 4shockblast"
TEST(DetectedCategory, OtherForDoneTurboQuickerBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "d2dtqr.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetCategory(), Category::Other);
}
