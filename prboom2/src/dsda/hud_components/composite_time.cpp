// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Composite Time HUD Component

#include "base.hpp"

#include "composite_time.hpp"

typedef struct
{
	dsda_text_t component;
	char label[8];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	extern dboolean dsda_reborn;

	int total_time;
	int length;

	total_time = hexen ? players[consoleplayer].worldTimer : totalleveltimes + leveltime;

	if(total_time != leveltime)
		length = snprintf(
			str,
			max_size,
			"%s%s%d:%02d %s%d:%05.2f ",
			local->label,
			dsda_TextColor(dsda_tc_exhud_total_time),
			total_time / TICRATE / 60,
			(total_time % (60 * TICRATE)) / TICRATE,
			dsda_TextColor(dsda_tc_exhud_level_time),
			leveltime / TICRATE / 60,
			(float)(leveltime % (60 * TICRATE)) / TICRATE
		);
	else
		length = snprintf(
			str,
			max_size,
			"%s%s%d:%05.2f ",
			local->label,
			dsda_TextColor(dsda_tc_exhud_level_time),
			leveltime / TICRATE / 60,
			(float)(leveltime % (60 * TICRATE)) / TICRATE
		);

	if(dsda_reborn && (demorecording || demoplayback))
	{
		int demo_tic = dsda_DemoTic();

		snprintf(
			str + length,
			max_size - length,
			"%s%d:%02d ",
			dsda_TextColor(dsda_tc_exhud_demo_length),
			demo_tic / TICRATE / 60,
			(demo_tic % (60 * TICRATE)) / TICRATE
		);
	}
}

void dsda_InitCompositeTimeHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(arg_count < 1 || args[0])
		snprintf(local->label, sizeof(local->label), "%stime ", dsda_TextColor(dsda_tc_exhud_time_label));
	else
		local->label[0] = '\0';

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateCompositeTimeHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawCompositeTimeHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
