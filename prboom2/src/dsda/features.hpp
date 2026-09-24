// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Features

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct FeatureFlag : int32_t
{
	Menu,
	Exhud,
	Advhud,
	Crosshair,
	Quickstartcache,
	Track100k,
	Console,
	// 7
	// 8
	// 9
	// 10
	// 11
	// 12
	// 13
	// 14
	// 15
	// 16
	// 17
	// 18
	// 19
	// 20
	// 21
	// 22
	// 23
	// 24
	// 25
	// 26
	// 27
	// 28
	// 29

	Unknown = 30,
	Invalid = 31,

	Iddt = 32,
	Automap,
	Liteamp,
	Build,
	Buildzero,
	Bruteforce,
	Tracker,
	Keyframe,
	Skip,
	Wipescreen,
	Speedup,
	Slowdown,
	Coordinates,
	Mouselook,
	Weaponalignment,
	Commanddisplay,
	Crosshaircolor,
	Crosshairlock,
	Shadows,
	Painpalette,
	Bonuspalette,
	Powerpalette,
	Healthbar,
	Alwayssr50,
	Maxplayercorpse,
	Hideweapon,
	Showalive,
	Join,
	MouseAndController,
	Ghost,
	AdvancedMap,
	// uf_blink_keys = 63
	// uf_fuzz = 64
	Vanillatrans = 65,
	Ghosttrans,
	Levelbrightness,
	// 68

	// 127
};

#define BITMASK(b) (1 << ((b) % 8))
#define BITSLOT(b) ((b) / 8)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + 8 - 1) / 8)

#define FEATURE_SIZE 128
#define FEATURE_SLOTS BITNSLOTS(FEATURE_SIZE)

void dsda_TrackFeature(FeatureFlag feature);
void dsda_ResetFeatures();
byte* dsda_UsedFeatures();
void dsda_MergeFeatures(byte* source);
void dsda_CopyFeatures(byte* result);
char* dsda_DescribeFeatures();

#ifdef __cplusplus
}
#endif
