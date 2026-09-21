// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	The individual fields the game reports in `analysis.txt`.
//	Ported from the `analysis` group of the former `spec/analysis_spec.rb`.
//
//	The purpose-built cases use `analysis_test.wad`, which is checked in; the
//	rest replay published demos and need only the IWAD.

#include <gtest/gtest.h>

#include "DemoRun.hpp"
#include "SpecEnvironment.hpp"

namespace
{
	constexpr std::string_view k_analysisWad = "analysis_test.wad";
}

// skill

// "doom2 map 4 nm speed by Vile"
TEST(Analysis, SkillIsNightmareForNmSpeedByVile)
{
	constexpr DemoOptions options { .lmp = "nm04-036.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetAnalysis().skill, 5);
}

// "doom2 map 1 uv speed by Thomas Pilger"
TEST(Analysis, SkillIsUltraViolenceForUvSpeedByThomasPilger)
{
	constexpr DemoOptions options { .lmp = "lv01-005.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_EQ(DemoRun(options).GetAnalysis().skill, 4);
}

// nomonsters

// "doom2 map 1 nomonsters by depr4vity"
TEST(Analysis, NoMonstersIsSetForNoMonstersRunByDepr4vity)
{
	constexpr DemoOptions options { .lmp = "lv01o497.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().noMonsters);
}

// "doom2 map 1 uv speed by Thomas Pilger"
TEST(Analysis, NoMonstersIsClearForUvSpeedByThomasPilger)
{
	constexpr DemoOptions options { .lmp = "lv01-005.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().noMonsters);
}

// respawn

// "doom2 map 2 uv respawn by Looper"
TEST(Analysis, RespawnIsSetForUvRespawnByLooper)
{
	constexpr DemoOptions options { .lmp = "re02-107.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().respawn);
}

// "doom2 map 4 nm speed by Vile"
TEST(Analysis, RespawnIsClearForNmSpeedByVile)
{
	constexpr DemoOptions options { .lmp = "nm04-036.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().respawn);
}

// fast

// "doom2 map 4 uv fast by Radek Pecka"
TEST(Analysis, FastIsSetForUvFastByRadekPecka)
{
	constexpr DemoOptions options { .lmp = "fa04-109.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().fast);
}

// "doom2 map 4 nm speed by Vile"
TEST(Analysis, FastIsClearForNmSpeedByVile)
{
	constexpr DemoOptions options { .lmp = "nm04-036.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().fast);
}

// 100k

// "doom2 map 1 uv max by Xit Vono"
TEST(Analysis, HundredKillsIsSetForUvMaxByXitVono)
{
	constexpr DemoOptions options { .lmp = "lv01-039.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().hundredKills);
}

// "doom2 map 1 uv speed by Thomas Pilger"
TEST(Analysis, HundredKillsIsClearForUvSpeedByThomasPilger)
{
	constexpr DemoOptions options { .lmp = "lv01-005.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().hundredKills);
}

// 100s

// "doom2 episode 1 nm100s in 11:56 by JCD"
TEST(Analysis, HundredSecretsIsSetForNm100sByJcd)
{
	constexpr DemoOptions options { .lmp = "1156ns01.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().hundredSecrets);
}

// "doom2 map 1 uv speed by Thomas Pilger"
TEST(Analysis, HundredSecretsIsClearForUvSpeedByThomasPilger)
{
	constexpr DemoOptions options { .lmp = "lv01-005.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().hundredSecrets);
}

// missed things

// "doom2 ep 3 max in 26:54 by Vile" - misses one secret (map 27)
TEST(Analysis, MissedThingsForEpisode3MaxByVile)
{
	constexpr DemoOptions options { .lmp = "lve3-2654.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	const Analysis analysis = DemoRun(options).GetAnalysis();

	EXPECT_EQ(analysis.missedMonsters, 0);
	EXPECT_EQ(analysis.missedSecrets, 1);
}

// pacifist

TEST(Analysis, PacifistIsClearWhenThereIsABarrelChain)
{
	constexpr DemoOptions options { .lmp = "barrel_chain.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsClearWhenThereIsABarrelAssist)
{
	constexpr DemoOptions options { .lmp = "barrel_assist.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsClearWhenThePlayerShootsAKeen)
{
	constexpr DemoOptions options { .lmp = "keen.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsClearWhenThePlayerShootsARomero)
{
	constexpr DemoOptions options { .lmp = "romero.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsClearWhenThereIsSplashDamage)
{
	constexpr DemoOptions options { .lmp = "splash.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsSetWhenThereIsATelefrag)
{
	constexpr DemoOptions options { .lmp = "telefrag.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().pacifist);
}

TEST(Analysis, PacifistIsSetWhenThePlayerShootsAVoodooDoll)
{
	constexpr DemoOptions options { .lmp = "voodoo.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().pacifist);
}

// stroller

// "doom2 map 8 stroller by 4shockblast"
TEST(Analysis, StrollerIsSetForStrollerBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "lv08str037.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().stroller);
}

// "doom2 map 8 pacifist by 4shockblast"
TEST(Analysis, StrollerIsClearForPacifistBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "pa08-020.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().stroller);
}

// reality

TEST(Analysis, RealityIsClearWhenThePlayerTakesEnemyDamage)
{
	constexpr DemoOptions options { .lmp = "damage.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	const Analysis analysis = DemoRun(options).GetAnalysis();

	EXPECT_FALSE(analysis.reality);
	EXPECT_FALSE(analysis.almostReality);
}

TEST(Analysis, AlmostRealityIsSetWhenThePlayerTakesNukageDamage)
{
	constexpr DemoOptions options { .lmp = "nukage.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	const Analysis analysis = DemoRun(options).GetAnalysis();

	EXPECT_FALSE(analysis.reality);
	EXPECT_TRUE(analysis.almostReality);
}

TEST(Analysis, RealityIsClearWhenThePlayerTakesCrusherDamage)
{
	constexpr DemoOptions options { .lmp = "crusher.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	const Analysis analysis = DemoRun(options).GetAnalysis();

	EXPECT_FALSE(analysis.reality);
	EXPECT_FALSE(analysis.almostReality);
}

TEST(Analysis, RealityIsSetWhenThePlayerTakesNoDamage)
{
	constexpr DemoOptions options { .lmp = "reality.lmp", .pwad = k_analysisWad };
	SPEC_REQUIRE_INPUTS(options);

	const Analysis analysis = DemoRun(options).GetAnalysis();

	EXPECT_TRUE(analysis.reality);
	EXPECT_FALSE(analysis.almostReality);
}

// tyson weapons

// "doom2 map 1 tyson by j4rio"
TEST(Analysis, TysonWeaponsIsSetForTysonByJ4rio)
{
	constexpr DemoOptions options { .lmp = "lv01t040.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().tysonWeapons);
}

// "doom2 map 1 uv max by Xit Vono"
TEST(Analysis, TysonWeaponsIsClearForUvMaxByXitVono)
{
	constexpr DemoOptions options { .lmp = "lv01-039.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().tysonWeapons);
}

// turbo

// "doom2 done turbo quicker by 4shockblast"
TEST(Analysis, TurboIsSetForDoneTurboQuickerBy4shockblast)
{
	constexpr DemoOptions options { .lmp = "d2dtqr.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().turbo);
}

// "doom2 map 1 uv max by Xit Vono"
TEST(Analysis, TurboIsClearForUvMaxByXitVono)
{
	constexpr DemoOptions options { .lmp = "lv01-039.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().turbo);
}

// weapon_collector

// "doom2 map 1 collector by hokis"
TEST(Analysis, WeaponCollectorIsSetForCollectorByHokis)
{
	constexpr DemoOptions options { .lmp = "cl01-022.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_TRUE(DemoRun(options).GetAnalysis().weaponCollector);
}

// "doom2 map 1 tyson by j4rio"
TEST(Analysis, WeaponCollectorIsClearForTysonByJ4rio)
{
	constexpr DemoOptions options { .lmp = "lv01t040.lmp" };
	SPEC_REQUIRE_INPUTS(options);

	EXPECT_FALSE(DemoRun(options).GetAnalysis().weaponCollector);
}
