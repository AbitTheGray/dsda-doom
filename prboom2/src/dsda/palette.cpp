// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Palette Management

#include <utility>

#include <math.h>

#include "r_main.hpp"
#include "w_wad.hpp"
#include "v_video.hpp"

#include "palette.hpp"

PlaypalIndex playpal_index = PlaypalIndex::Default;

static dsda_playpal_t playpal_data[std::to_underlying(PlaypalIndex::Count)] = {
	{PlaypalIndex::Default, "PLAYPAL"},
	{PlaypalIndex::Pal1, "PLAYPAL1"},
	{PlaypalIndex::Pal2, "PLAYPAL2"},
	{PlaypalIndex::Pal3, "PLAYPAL3"},
	{PlaypalIndex::Pal4, "PLAYPAL4"},
	{PlaypalIndex::Pal5, "PLAYPAL5"},
	{PlaypalIndex::Pal6, "PLAYPAL6"},
	{PlaypalIndex::Pal7, "PLAYPAL7"},
	{PlaypalIndex::Pal8, "PLAYPAL8"},
	{PlaypalIndex::Pal9, "PLAYPAL9"},
	{PlaypalIndex::HereticE2End, "E2PAL"},
	{PlaypalIndex::Custom, ""},
};

dsda_playpal_t* dsda_PlayPalData(PlaypalIndex playpal_i)
{
	return &playpal_data[std::to_underlying(playpal_i)];
}

void dsda_CyclePlayPal()
{
	int lump_num = -1;
	PlaypalIndex cycle_playpal_index;

	cycle_playpal_index = playpal_index;

	do
	{
		cycle_playpal_index = static_cast<PlaypalIndex>(std::to_underlying(cycle_playpal_index) + 1);

		if(cycle_playpal_index > PlaypalIndex::Pal9)
			cycle_playpal_index = PlaypalIndex::Default;

		// Looped around and found nothing
		if(cycle_playpal_index == playpal_index)
			return;

		lump_num = W_CheckNumForName(playpal_data[std::to_underlying(cycle_playpal_index)].lump_name);
	}
	while(lump_num == LUMP_NOT_FOUND);

	V_SetPlayPal(cycle_playpal_index);
}

void dsda_SetPlayPal(PlaypalIndex index)
{
	if(index < PlaypalIndex::Default || index >= PlaypalIndex::Count)
		index = PlaypalIndex::Default;

	playpal_index = index;
}

void dsda_FreePlayPal(PlaypalIndex playpal_i)
{
	if(playpal_data[std::to_underlying(playpal_i)].lump)
	{
		Z_Free(playpal_data[std::to_underlying(playpal_i)].lump);
		playpal_data[std::to_underlying(playpal_i)].lump = nullptr;
	}
	if(playpal_data[std::to_underlying(playpal_i)].colours)
	{
		Z_Free(playpal_data[std::to_underlying(playpal_i)].colours);
		playpal_data[std::to_underlying(playpal_i)].colours = nullptr;
	}
	playpal_data[std::to_underlying(playpal_i)].length = 0;
	playpal_data[std::to_underlying(playpal_i)].transparent = 0;
	playpal_data[std::to_underlying(playpal_i)].duplicate = 0;
	playpal_data[std::to_underlying(playpal_i)].darkest = 0;
	playpal_data[std::to_underlying(playpal_i)].lightest = 0;
}

void dsda_FreeAllPlayPals()
{
	for(int32_t i = 0; i < std::to_underlying(PlaypalIndex::Count); ++i)
		dsda_FreePlayPal(static_cast<PlaypalIndex>(i));
}

static dboolean dsda_DuplicatePaletteEntry(const byte* playpal, int i, int j)
{
	int colormap_i;

	if(
		playpal[3 * i + 0] != playpal[3 * j + 0] ||
		playpal[3 * i + 1] != playpal[3 * j + 1] ||
		playpal[3 * i + 2] != playpal[3 * j + 2]
	)
		return false;

	for(colormap_i = 0; colormap_i < NUMCOLORMAPS; ++colormap_i)
		if(colormaps[0][colormap_i * 256 + i] != colormaps[0][colormap_i * 256 + j])
			return false;

	return true;
}

double dsda_PaletteEntryLightness(const byte* playpal, int i)
{
	double L;
	byte pal_r, pal_g, pal_b;
	double r, g, b;
	double y1, y2;

	// this function basically does an RGB to L*a*b* (CIELAB) color
	// space conversion, but only for the L* component since we
	// just want the brightness. probably a bit overkill, but meh.

	// step 0: get colors from palette -- explicitly get it as a byte
	// (i.e. unsigned) so it doesn't get interpreted as a negative value.
	pal_r = playpal[3 * i + 0];
	pal_g = playpal[3 * i + 1];
	pal_b = playpal[3 * i + 2];

	// step 1: RGB to XYZ (...or just Y, I guess :P)
	r = pal_r / 255.0;
	g = pal_g / 255.0;
	b = pal_b / 255.0;

	r = r > 0.04045 ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
	g = g > 0.04045 ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
	b = b > 0.04045 ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

	r *= 100;
	g *= 100;
	b *= 100;

	y1 = 0.2126729 * r + 0.7151522 * g + 0.0721750 * b;

	// step 2: XYZ to Lab
	y2 = y1 / 100.0;
	y2 = y2 > 0.008856 ? pow(y2, 1.0 / 3) : 7.787 * y2 + 4.0 / 29;

	L = 116 * y2 - 16;

	// all done. take the L.
	return L;
}

// Moved from r_patch.c
void dsda_InitPlayPal(PlaypalIndex playpal_i)
{
	double lightness;
	double darkest, lightest;
	int lump;
	const byte* playpal;
	int i, j, found = 0;

	dsda_FreePlayPal(playpal_i);

	lump = W_CheckNumForName(playpal_data[std::to_underlying(playpal_i)].lump_name);
	if(lump == LUMP_NOT_FOUND)
		return;

	playpal = (const byte*)W_LumpByNum(lump);

	if(!playpal_data[std::to_underlying(playpal_i)].duplicate)
	{
		// find two duplicate palette entries. use one for transparency.
		// rewrite source pixels in patches to the other on composition.

		for(i = 0; i < 256; i++)
		{
			for(j = i + 1; j < 256; j++)
			{
				if(dsda_DuplicatePaletteEntry(playpal, i, j))
				{
					found = 1;
					break;
				}
			}

			if(found)
				break;
		}

		if(found)
		{
			// found duplicate
			playpal_data[std::to_underlying(playpal_i)].transparent = i;
			playpal_data[std::to_underlying(playpal_i)].duplicate = j;
		}
		else
		{
			// no duplicate: use 255 for transparency, as done previously
			playpal_data[std::to_underlying(playpal_i)].transparent = 255;
			playpal_data[std::to_underlying(playpal_i)].duplicate = -1;
		}
	}

	// find the brightness extremes (0-100)
	darkest = 101.0;
	lightest = -1.0;
	for(i = 0; i < 256; i++)
	{
		lightness = dsda_PaletteEntryLightness(playpal, i);

		if(lightness < darkest)
		{
			darkest = lightness;
			playpal_data[std::to_underlying(playpal_i)].darkest = i;
		}

		if(lightness > lightest)
		{
			lightest = lightness;
			playpal_data[std::to_underlying(playpal_i)].lightest = i;
		}
	}
}

void dsda_InitAllPlayPals()
{
	for(int32_t i = 0; i < std::to_underlying(PlaypalIndex::Count); ++i)
	{
		dsda_InitPlayPal(static_cast<PlaypalIndex>(i));
	}
}
