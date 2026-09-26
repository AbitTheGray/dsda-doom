// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA CR Table

#include <utility>

#include "cpp/EnumArray.hpp"

#include "doomdef.hpp"
#include "lprintf.hpp"
#include "w_wad.hpp"
#include "v_video.hpp"

#include "dsda/palette.hpp"
#include "dsda/utility.hpp"

#include "cr_table.hpp"

typedef struct
{
	int r1, g1, b1;
	int r2, g2, b2;
} cr_range_t;

// Default values - overridden by DSDACR lump
constinit EnumArray<cr_range_t, ColorRange::HudLimit> cr_range = {
	{At(ColorRange::Default), {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF}},
	{At(ColorRange::Brick), {0x47, 0x00, 0x00, 0xFF, 0xB8, 0xB8}},
	{At(ColorRange::Tan), {0x33, 0x2B, 0x13, 0xFF, 0xEB, 0xDF}},
	{At(ColorRange::Gray), {0x27, 0x27, 0x27, 0xEF, 0xEF, 0xEF}},
	{At(ColorRange::Green), {0x0B, 0x17, 0x07, 0x77, 0xFF, 0x6F}},
	{At(ColorRange::Brown), {0x53, 0x3F, 0x2F, 0xBF, 0xA7, 0x8F}},
	{At(ColorRange::Gold), {0x73, 0x2B, 0x00, 0xFF, 0xFF, 0x73}},
	{At(ColorRange::Red), {0x3F, 0x00, 0x00, 0xFF, 0x00, 0x00}},
	{At(ColorRange::Blue), {0x00, 0x00, 0x27, 0x00, 0x00, 0xFF}},
	{At(ColorRange::Orange), {0x20, 0x00, 0x00, 0xFF, 0x80, 0x00}},
	{At(ColorRange::Yellow), {0x77, 0x77, 0x00, 0xFF, 0xFF, 0x00}},
	{At(ColorRange::Lightblue), {0x00, 0x00, 0x73, 0xB4, 0xB4, 0xFF}},
	{At(ColorRange::Black), {0x13, 0x13, 0x13, 0x50, 0x50, 0x50}},
	{At(ColorRange::Purple), {0x23, 0x00, 0x23, 0xCF, 0x00, 0xCF}},
	{At(ColorRange::White), {0x24, 0x24, 0x24, 0xFF, 0xFF, 0xFF}},
};

static char ref_lump_doom[9] = "STCFN065";
static char ref_lump_heretic[9] = "FONTA33";
static char ref_lump_hexen[9] = "FONTA33";

typedef struct
{
	double light_lower_bound;
	double light_upper_bound;
	double multiplier;
} cr_font_t;

static cr_font_t cr_font = {
	.light_lower_bound = 1.0,
	.light_upper_bound = 0.0,
	.multiplier = 1.0,
};

static void dsda_RegisterFontLightness(double lightness)
{
	if(cr_font.light_lower_bound > lightness)
		cr_font.light_lower_bound = lightness;

	if(cr_font.light_upper_bound < lightness)
		cr_font.light_upper_bound = lightness;
}

static void dsda_CalculateFontBounds(const byte* playpal)
{
	int i, j;
	const byte* lump;
	const byte* p;
	short width;
	byte length;
	byte entry;
	double lightness;

	if(heretic)
		lump = static_cast<const byte *>(W_LumpByName(ref_lump_heretic));
	else if(hexen)
		lump = static_cast<const byte *>(W_LumpByName(ref_lump_hexen));
	else
		lump = static_cast<const byte *>(W_LumpByName(ref_lump_doom));

	width = *((const int16_t*)lump);
	width = LittleShort(width);

	for(i = 0; i < width; ++i)
	{
		int32_t offset;
		p = lump + 8 + 4 * i;
		offset = *((const int32_t*)p);
		p = lump + LittleLong(offset);

		while(*p != 0xff)
		{
			p++;
			length = *p++;
			p++;

			for(j = 0; j < length; ++j)
			{
				entry = *p++;
				lightness = dsda_PaletteEntryLightness(playpal, entry) / 100.0;
				dsda_RegisterFontLightness(lightness);
			}

			p++;
		}
	}

	cr_font.multiplier = 1.0 / (cr_font.light_upper_bound - cr_font.light_lower_bound);

	lprintf(OutputLevels::Debug, "Font Bounds: %lf:%lf x%lf\n",
		cr_font.light_lower_bound, cr_font.light_upper_bound, cr_font.multiplier);
}

static void dsda_LoadCRLump()
{
	char* lump;
	char** lines;
	const char* line;
	int line_i;
	int i, r1, g1, b1, r2, g2, b2;

	lump = W_ReadLumpToString(W_GetNumForName("DSDACR"));

	lines = dsda_SplitString(lump, "\n\r");

	if(lines[0])
		sscanf(lines[0], "%8s %8s %8s", ref_lump_doom, ref_lump_heretic, ref_lump_hexen);

	for(line_i = 1; lines[line_i]; ++line_i)
	{
		line = lines[line_i];

		if(sscanf(line, "%d %i %i %i %i %i %i", &i, &r1, &g1, &b1, &r2, &g2, &b2) != 7)
			I_Error("DSDACR lump has unknown format!");

		if(i < 1 || i >= std::to_underlying(ColorRange::HudLimit))
			I_Error("DSDACR index %d is out of bounds!", i);

		if(r1 < 0 || g1 < 0 || b1 < 0 || r2 < 0 || g2 < 0 || b2 < 0 ||
			r1 > 255 || g1 > 255 || b1 > 255 || r2 > 255 || g2 > 255 || b2 > 255)
			I_Error("DSDACR index %d has color out of range (0-255)", i);

		cr_range_t& range = cr_range[static_cast<ColorRange>(i)];
		range.r1 = r1;
		range.g1 = g1;
		range.b1 = b1;
		range.r2 = r2;
		range.g2 = g2;
		range.b2 = b2;
	}

	Z_Free(lines);
	Z_Free(lump);
}

typedef struct
{
	const char* name;
	ColorRange fallback;
} blood_load_t;

static blood_load_t blood_data[std::to_underlying(ColorRange::Limit) - std::to_underlying(ColorRange::Blood)] = {
	{"CRGRAY", ColorRange::Gray},
	{"CRGREEN", ColorRange::Green},
	{"CRBLUE2", ColorRange::Blue},
	{"CRYELLOW", ColorRange::Yellow},
	{"CRBLACK", ColorRange::Black},
	{"CRPURPLE", ColorRange::Purple},
	{"CRWHITE", ColorRange::White},
	{"CRORANGE", ColorRange::Orange},
};

static void dsda_LoadCRLumps(byte* buffer)
{
	int i;
	byte* blood_buffer;

	blood_buffer = buffer + std::to_underlying(ColorRange::Blood) * 256;

	for(i = 0; i < std::to_underlying(ColorRange::Limit) - std::to_underlying(ColorRange::Blood); ++i)
	{
		int lump;

		lump = W_CheckNumForName(blood_data[i].name);
		if(lump != LUMP_NOT_FOUND && W_LumpLength(lump) == 256)
			memcpy(blood_buffer + i * 256, W_LumpByNum(lump), 256);
		else
			memcpy(blood_buffer + i * 256, buffer + std::to_underlying(blood_data[i].fallback) * 256, 256);
	}
}

static int dsda_BrightenPaletteEntry(const byte* playpal, int entry)
{
	int palette_i = entry * 3;
	int target_r = playpal[palette_i + 0];
	int target_g = playpal[palette_i + 1];
	int target_b = playpal[palette_i + 2];
	int max = target_r;
	double scale;

	if(target_g > max)
		max = target_g;
	if(target_b > max)
		max = target_b;
	if(!max)
		return entry;

	scale = 1.4;
	if(max * scale > 255.0)
		scale = 255.0 / max;

	target_r = (int)(target_r * scale + 0.5);
	target_g = (int)(target_g * scale + 0.5);
	target_b = (int)(target_b * scale + 0.5);

	return V_BestColor(playpal, target_r, target_g, target_b);
}

byte* dsda_GenerateCRTable()
{
	int cr_i;
	int orig_i;
	int check_i;
	byte* buffer;
	const byte* playpal;
	int dark_i;

	dsda_LoadCRLump();

	playpal = static_cast<const byte *>(W_LumpByName("PLAYPAL"));

	buffer = static_cast<byte *>(Z_Malloc(256 * std::to_underlying(ColorRange::Limit)));

	dsda_CalculateFontBounds(playpal);

	for(orig_i = 0; orig_i < 256; ++orig_i)
	{
		double length;

		length = dsda_PaletteEntryLightness(playpal, orig_i) / 100.0;
		length -= cr_font.light_lower_bound;
		length *= cr_font.multiplier;
		if(length < 0)
			length = 0;
		if(length > 1)
			length = 1;

		// This is the exhud font bright value
		if(orig_i == 176)
			length = 1;

		for(dark_i = 0; dark_i < 2; ++dark_i)
		{
			for(const ColorRange color : cr_range.Keys())
			{
				const cr_range_t& range = cr_range[color];
				int target_r, target_g, target_b;
				int best_i = 0;
				int best_dist = INT_MAX;

				target_r = range.r1 +
					(int)(length * (range.r2 - range.r1));
				target_g = range.g1 +
					(int)(length * (range.g2 - range.g1));
				target_b = range.b1 +
					(int)(length * (range.b2 - range.b1));

				if(dark_i)
				{
					target_r /= 2;
					target_g /= 2;
					target_b /= 2;
				}

				for(check_i = 0; check_i < 768; check_i += 3)
				{
					int dist;
					int dist_r, dist_g, dist_b;
					int avg_r;

					avg_r = (target_r + playpal[check_i + 0]) / 2;
					dist_r = target_r - playpal[check_i + 0];
					dist_g = target_g - playpal[check_i + 1];
					dist_b = target_b - playpal[check_i + 2];

					// This equation seems to fix issues with red-dominant translation,
					// e.g., aaliens CR_BRICK, which has artifacts in the second equation.
					//
					// I experimented with more "sophisticated" approaches,
					// but they don't seem to do well with common palettes.
					if(target_r > target_g && target_r > target_b)
						dist = (((512 + avg_r) * dist_r * dist_r) >> 8) +
							4 * dist_g * dist_g +
							(((767 - avg_r) * dist_b * dist_b) >> 8);
					else
						dist = dist_r * dist_r +
							dist_g * dist_g +
							dist_b * dist_b;

					if(dist < best_dist)
					{
						best_dist = dist;
						best_i = check_i / 3;
					}
				}

				buffer[(dark_i ? std::to_underlying(ColorRange::Darken) * 256 : 0) + std::to_underlying(color) * 256 + orig_i] = best_i;
			}
		}

		buffer[std::to_underlying(ColorRange::Bright) * 256 + orig_i] =
			dsda_BrightenPaletteEntry(playpal, orig_i);
	}

	for(cr_i = std::to_underlying(ColorRange::Default) + 1; cr_i < std::to_underlying(ColorRange::HudLimit); ++cr_i)
	{
		for(orig_i = 0; orig_i < 256; ++orig_i)
		{
			buffer[(std::to_underlying(ColorRange::Bright) + cr_i) * 256 + orig_i] =
				buffer[std::to_underlying(ColorRange::Bright) * 256 + buffer[cr_i * 256 + orig_i]];
		}
	}

	dsda_LoadCRLumps(buffer);

	return buffer;
}
