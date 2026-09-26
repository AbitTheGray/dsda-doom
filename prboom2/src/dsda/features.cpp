// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Features

#include <utility>

#include <string.h>

#include "cpp/EnumArray.hpp"

#include "z_zone.hpp"

#include "dsda/utility.hpp"

#include "features.hpp"

static byte used_features[FEATURE_SLOTS];

static constinit EnumArray<const char*, EnumCount<FeatureFlag>> feature_names = {
	{At(FeatureFlag::Menu), "Menu"},
	{At(FeatureFlag::Exhud), "Extended HUD"},
	{At(FeatureFlag::Advhud), "Advanced HUD"},
	{At(FeatureFlag::Crosshair), "Crosshair"},
	{At(FeatureFlag::Quickstartcache), "Quickstart Cache"},
	{At(FeatureFlag::Track100k), "100K Tracker"},
	{At(FeatureFlag::Console), "Console"},

	{At(FeatureFlag::Iddt), "IDDT"},
	{At(FeatureFlag::Automap), "IDBEHOLD Map"},
	{At(FeatureFlag::Liteamp), "IDBEHOLD Light"},
	{At(FeatureFlag::Build), "Build Mode"},
	{At(FeatureFlag::Buildzero), "Build First Frame"},
	{At(FeatureFlag::Bruteforce), "Brute Force"},
	{At(FeatureFlag::Tracker), "TAS Tracker"},
	{At(FeatureFlag::Keyframe), "Key Frame"},
	{At(FeatureFlag::Skip), "Skip Forward"},
	{At(FeatureFlag::Wipescreen), "Skip Wipe Screen"},
	{At(FeatureFlag::Speedup), "Speed Up"},
	{At(FeatureFlag::Slowdown), "Slow Down"},
	{At(FeatureFlag::Coordinates), "Show Coordinates"},
	{At(FeatureFlag::Mouselook), "Mouse Look"},
	{At(FeatureFlag::Weaponalignment), "Weapon Alignment"},
	{At(FeatureFlag::Commanddisplay), "Command Display"},
	{At(FeatureFlag::Crosshaircolor), "Dynamic Crosshair Color"},
	{At(FeatureFlag::Crosshairlock), "Crosshair Lock"},
	{At(FeatureFlag::Shadows), "Shadows"},
	{At(FeatureFlag::Painpalette), "Disable Pain Palette"},
	{At(FeatureFlag::Bonuspalette), "Disable Bonus Palette"},
	{At(FeatureFlag::Powerpalette), "Disable Power Palette"},
	{At(FeatureFlag::Healthbar), "Show Health Bars"},
	{At(FeatureFlag::Alwayssr50), "Always SR50"},
	{At(FeatureFlag::Maxplayercorpse), "Edit Corpse Limit"},
	{At(FeatureFlag::Hideweapon), "Hide Weapon"},
	{At(FeatureFlag::Showalive), "Show Alive"},
	{At(FeatureFlag::Join), "Join"},
	{At(FeatureFlag::MouseAndController), "Mouse and Controller"},
	{At(FeatureFlag::Ghost), "Ghost"},
	{At(FeatureFlag::AdvancedMap), "Advanced Map"},
	{At(FeatureFlag::Vanillatrans), "Vanilla Translucency"},
	{At(FeatureFlag::Ghosttrans), "Ghost Translucency"},
	{At(FeatureFlag::Levelbrightness), "Extra Lighting"},
};

void dsda_TrackFeature(FeatureFlag feature)
{
	BITSET(used_features, std::to_underlying(feature));
}

void dsda_ResetFeatures()
{
	memset(used_features, 0, FEATURE_SLOTS);
}

byte* dsda_UsedFeatures()
{
	return used_features;
}

void dsda_MergeFeatures(byte* source)
{
	for(int f = 0; f < FEATURE_SLOTS; f++)
	{
		used_features[f] |= source[f];
	}
}

void dsda_CopyFeatures(byte* result)
{
	for(int f = 0; f < FEATURE_SLOTS; f++)
	{
		result[f] = used_features[f];
	}
}

char* dsda_DescribeFeatures()
{
	dboolean first = true;
	dsda_string_t description;

	dsda_InitString(&description, nullptr);

	for(const FeatureFlag feature : feature_names.Keys())
	{
		if(BITTEST(used_features, std::to_underlying(feature)) && feature_names[feature])
		{
			if(first)
				first = false;
			else
				dsda_StringCat(&description, ", ");

			dsda_StringCat(&description, feature_names[feature]);
		}
	}

	if(!description.string)
		dsda_StringCat(&description, "Tachyeres pteneres");

	return description.string;
}
