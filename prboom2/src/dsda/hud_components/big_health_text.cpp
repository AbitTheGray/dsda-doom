// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Health Text HUD Component

#include "base.hpp"

#include "big_health_text.hpp"

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static int patch_delta_x;

static void dsda_DrawComponent()
{
	player_t* player;
	ColorRange cm;

	player = &players[displayplayer];

	cm = player->health <= hud_health_red ? dsda_TextCR(TextColorIndex::StbarHealthBad) : player->health <= hud_health_yellow ? dsda_TextCR(TextColorIndex::StbarHealthWarning) : player->health <= hud_health_green ? dsda_TextCR(TextColorIndex::StbarHealthOk) : dsda_TextCR(TextColorIndex::StbarHealthSuper);

	dsda_DrawBigNumber(local->component.x, local->component.y, patch_delta_x, 0,
		cm, local->component.vpt, 3, player->health);
}

void dsda_InitBigHealthTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(heretic)
		patch_delta_x = 10;
	else if(hexen)
		patch_delta_x = 10;
	else
		patch_delta_x = 14;

	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigHealthTextHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawBigHealthTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawComponent();
}
