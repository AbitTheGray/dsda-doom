// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Coordinates HUD Component

#include "base.hpp"

#include "map_coordinates.hpp"

typedef struct
{
	dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	snprintf(
		str,
		max_size,
		"%sX: %-5d\n"
		"Y: %-5d\n"
		"Z: %-5d",
		dsda_TextColor(TextColorIndex::MapCoords),
		(players[displayplayer].mo->x >> FRACBITS),
		(players[displayplayer].mo->y >> FRACBITS),
		(players[displayplayer].mo->z >> FRACBITS)
	);
}

void dsda_InitMapCoordinatesHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapCoordinatesHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapCoordinatesHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
