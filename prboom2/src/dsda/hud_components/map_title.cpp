// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Title HUD Component

#include "base.hpp"

#include "map_title.hpp"

typedef struct
{
	dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	extern dsda_string_t hud_title;

	snprintf(
		str,
		max_size,
		"%s%s",
		dsda_TextColor(TextColorIndex::MapTitle),
		hud_title.string
	);
}

void dsda_InitMapTitleHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTitleHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapTitleHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
