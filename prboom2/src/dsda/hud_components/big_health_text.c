// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Health Text HUD Component

#include "base.h"

#include "big_health_text.h"

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static int patch_delta_x;

static void dsda_DrawComponent(void)
{
	player_t* player;
	int cm;

	player = &players[displayplayer];

	cm = player->health <= hud_health_red ? dsda_TextCR(dsda_tc_stbar_health_bad) : player->health <= hud_health_yellow ? dsda_TextCR(dsda_tc_stbar_health_warning) : player->health <= hud_health_green ? dsda_TextCR(dsda_tc_stbar_health_ok) : dsda_TextCR(dsda_tc_stbar_health_super);

	dsda_DrawBigNumber(local->component.x, local->component.y, patch_delta_x, 0,
		cm, local->component.vpt, 3, player->health);
}

void dsda_InitBigHealthTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = *data;

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
	local = data;
}

void dsda_DrawBigHealthTextHC(void* data)
{
	local = data;

	dsda_DrawComponent();
}
