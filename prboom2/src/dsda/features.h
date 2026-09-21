// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Features

#pragma once

#include "doomtype.h"

typedef enum
{
	uf_menu,
	uf_exhud,
	uf_advhud,
	uf_crosshair,
	uf_quickstartcache,
	uf_100k,
	uf_console,
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

	uf_unknown = 30,
	uf_invalid = 31,

	uf_iddt = 32,
	uf_automap,
	uf_liteamp,
	uf_build,
	uf_buildzero,
	uf_bruteforce,
	uf_tracker,
	uf_keyframe,
	uf_skip,
	uf_wipescreen,
	uf_speedup,
	uf_slowdown,
	uf_coordinates,
	uf_mouselook,
	uf_weaponalignment,
	uf_commanddisplay,
	uf_crosshaircolor,
	uf_crosshairlock,
	uf_shadows,
	uf_painpalette,
	uf_bonuspalette,
	uf_powerpalette,
	uf_healthbar,
	uf_alwayssr50,
	uf_maxplayercorpse,
	uf_hideweapon,
	uf_showalive,
	uf_join,
	uf_mouse_and_controller,
	uf_ghost,
	uf_advanced_map,
	// uf_blink_keys = 63
	// uf_fuzz = 64
	uf_vanillatrans = 65,
	uf_ghosttrans,
	uf_levelbrightness,
	// 68

	// 127
} dsda_feature_flag_t;

#define BITMASK(b) (1 << ((b) % 8))
#define BITSLOT(b) ((b) / 8)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + 8 - 1) / 8)

#define FEATURE_SIZE 128
#define FEATURE_SLOTS BITNSLOTS(FEATURE_SIZE)


void dsda_TrackFeature(int feature);
void dsda_ResetFeatures(void);
byte* dsda_UsedFeatures(void);
void dsda_MergeFeatures(byte* source);
void dsda_CopyFeatures(byte* result);
char* dsda_DescribeFeatures(void);
