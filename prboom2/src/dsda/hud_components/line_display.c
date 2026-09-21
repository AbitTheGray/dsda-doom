// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Line Display HUD Component

#include "base.h"

#include "line_display.h"

typedef struct
{
	dsda_text_t line_display[LINE_ACTIVATION_INDEX_MAX];
} local_component_t;

static local_component_t* local;

void dsda_InitLineDisplayHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	int i;

	*data = Z_Calloc(1, sizeof(local_component_t));
	local = *data;

	for(i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
		dsda_InitTextHC(&local->line_display[i], x_offset, y_offset + i * 8, vpt);
}

void dsda_UpdateLineDisplayHC(void* data)
{
	int* line_ids;
	int i;

	local = data;

	line_ids = dsda_PlayerActivatedLines();

	for(i = 0; line_ids[i] != -1; ++i)
	{
		snprintf(local->line_display[i].msg, sizeof(local->line_display[i].msg), "%s%d",
			dsda_TextColor(dsda_tc_exhud_line_activation), line_ids[i]);
		dsda_RefreshHudText(&local->line_display[i]);
	}

	if(line_ids[0] != -1 && i < LINE_ACTIVATION_INDEX_MAX)
	{
		local->line_display[i].msg[0] = '\0';
		dsda_RefreshHudText(&local->line_display[i]);
	}
}

void dsda_DrawLineDisplayHC(void* data)
{
	int i;

	local = data;

	for(i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
	{
		if(!local->line_display[i].msg[0])
			break;

		dsda_DrawBasicText(&local->line_display[i]);
	}
}
