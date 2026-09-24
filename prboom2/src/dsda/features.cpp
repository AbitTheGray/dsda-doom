// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Features

#include <utility>

#include <string.h>

#include "z_zone.hpp"

#include "dsda/utility.hpp"

#include "features.hpp"

static byte used_features[FEATURE_SLOTS];

static const char* feature_names[FEATURE_SIZE] = {
	[std::to_underlying(FeatureFlag::Menu)] = "Menu",
	[std::to_underlying(FeatureFlag::Exhud)] = "Extended HUD",
	[std::to_underlying(FeatureFlag::Advhud)] = "Advanced HUD",
	[std::to_underlying(FeatureFlag::Crosshair)] = "Crosshair",
	[std::to_underlying(FeatureFlag::Quickstartcache)] = "Quickstart Cache",
	[std::to_underlying(FeatureFlag::Track100k)] = "100K Tracker",
	[std::to_underlying(FeatureFlag::Console)] = "Console",

	[std::to_underlying(FeatureFlag::Iddt)] = "IDDT",
	[std::to_underlying(FeatureFlag::Automap)] = "IDBEHOLD Map",
	[std::to_underlying(FeatureFlag::Liteamp)] = "IDBEHOLD Light",
	[std::to_underlying(FeatureFlag::Build)] = "Build Mode",
	[std::to_underlying(FeatureFlag::Buildzero)] = "Build First Frame",
	[std::to_underlying(FeatureFlag::Bruteforce)] = "Brute Force",
	[std::to_underlying(FeatureFlag::Tracker)] = "TAS Tracker",
	[std::to_underlying(FeatureFlag::Keyframe)] = "Key Frame",
	[std::to_underlying(FeatureFlag::Skip)] = "Skip Forward",
	[std::to_underlying(FeatureFlag::Wipescreen)] = "Skip Wipe Screen",
	[std::to_underlying(FeatureFlag::Speedup)] = "Speed Up",
	[std::to_underlying(FeatureFlag::Slowdown)] = "Slow Down",
	[std::to_underlying(FeatureFlag::Coordinates)] = "Show Coordinates",
	[std::to_underlying(FeatureFlag::Mouselook)] = "Mouse Look",
	[std::to_underlying(FeatureFlag::Weaponalignment)] = "Weapon Alignment",
	[std::to_underlying(FeatureFlag::Commanddisplay)] = "Command Display",
	[std::to_underlying(FeatureFlag::Crosshaircolor)] = "Dynamic Crosshair Color",
	[std::to_underlying(FeatureFlag::Crosshairlock)] = "Crosshair Lock",
	[std::to_underlying(FeatureFlag::Shadows)] = "Shadows",
	[std::to_underlying(FeatureFlag::Painpalette)] = "Disable Pain Palette",
	[std::to_underlying(FeatureFlag::Bonuspalette)] = "Disable Bonus Palette",
	[std::to_underlying(FeatureFlag::Powerpalette)] = "Disable Power Palette",
	[std::to_underlying(FeatureFlag::Healthbar)] = "Show Health Bars",
	[std::to_underlying(FeatureFlag::Alwayssr50)] = "Always SR50",
	[std::to_underlying(FeatureFlag::Maxplayercorpse)] = "Edit Corpse Limit",
	[std::to_underlying(FeatureFlag::Hideweapon)] = "Hide Weapon",
	[std::to_underlying(FeatureFlag::Showalive)] = "Show Alive",
	[std::to_underlying(FeatureFlag::Join)] = "Join",
	[std::to_underlying(FeatureFlag::MouseAndController)] = "Mouse and Controller",
	[std::to_underlying(FeatureFlag::Ghost)] = "Ghost",
	[std::to_underlying(FeatureFlag::AdvancedMap)] = "Advanced Map",
	[std::to_underlying(FeatureFlag::Vanillatrans)] = "Vanilla Translucency",
	[std::to_underlying(FeatureFlag::Ghosttrans)] = "Ghost Translucency",
	[std::to_underlying(FeatureFlag::Levelbrightness)] = "Extra Lighting",
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
	int i;
	dboolean first = true;
	dsda_string_t description;

	dsda_InitString(&description, nullptr);

	for(i = 0; i < FEATURE_SIZE; i++)
	{
		if(BITTEST(used_features, i) && feature_names[i])
		{
			if(first)
				first = false;
			else
				dsda_StringCat(&description, ", ");

			dsda_StringCat(&description, feature_names[i]);
		}
	}

	if(!description.string)
		dsda_StringCat(&description, "Tachyeres pteneres");

	return description.string;
}
