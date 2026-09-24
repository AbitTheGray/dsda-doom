// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Color Test HUD Component

#include "base.hpp"

#include "color_test.hpp"

typedef struct
{
	dsda_text_t component;
	dsda_text_t component_blocky;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	snprintf(
		str,
		max_size,
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n"
		"\x1b%c %02d TEST\n",
		HUlib_Color(static_cast<ColorRange>(0)), 0,
		HUlib_Color(static_cast<ColorRange>(1)), 1,
		HUlib_Color(static_cast<ColorRange>(2)), 2,
		HUlib_Color(static_cast<ColorRange>(3)), 3,
		HUlib_Color(static_cast<ColorRange>(4)), 4,
		HUlib_Color(static_cast<ColorRange>(5)), 5,
		HUlib_Color(static_cast<ColorRange>(6)), 6,
		HUlib_Color(static_cast<ColorRange>(7)), 7,
		HUlib_Color(static_cast<ColorRange>(8)), 8,
		HUlib_Color(static_cast<ColorRange>(9)), 9,
		HUlib_Color(static_cast<ColorRange>(10)), 10,
		HUlib_Color(static_cast<ColorRange>(11)), 11,
		HUlib_Color(static_cast<ColorRange>(12)), 12,
		HUlib_Color(static_cast<ColorRange>(13)), 13,
		HUlib_Color(static_cast<ColorRange>(14)), 14
	);
}

void dsda_InitColorTestHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
	dsda_InitBlockyHC(&local->component_blocky, x_offset + 64, y_offset, vpt);

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);

	dsda_UpdateComponentText(local->component_blocky.msg, sizeof(local->component_blocky.msg));
	dsda_RefreshHudText(&local->component_blocky);
}

void dsda_UpdateColorTestHC(void* data)
{
	// nothing to do
}

void dsda_DrawColorTestHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
	dsda_DrawBasicText(&local->component_blocky);
}
