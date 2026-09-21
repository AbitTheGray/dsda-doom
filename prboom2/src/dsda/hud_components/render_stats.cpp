// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Render Stats HUD Component

#include "dsda/render_stats.hpp"

#include "base.hpp"

#include "render_stats.hpp"

typedef struct
{
	dsda_text_t component[2];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateCurrentComponentText(char* str, size_t max_size)
{
	extern dsda_render_stats_t dsda_render_stats;
	extern int dsda_render_stats_fps;

	snprintf(
		str, max_size,
		"%sFPS %s%4d %sSEGS %s%4d %sPLANES %s%4d %sSPRITES %s%4d",
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats_fps < TICRATE ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats_fps,
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats.drawsegs > 256 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats.drawsegs,
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats.visplanes > 128 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats.visplanes,
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats.vissprites > 128 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats.vissprites
	);
}

static void dsda_UpdateMaxComponentText(char* str, size_t max_size)
{
	extern dsda_render_stats_t dsda_render_stats_max;

	snprintf(
		str, max_size,
		"%sMAX      SEGS %s%4d %sPLANES %s%4d %sSPRITES %s%4d",
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats_max.drawsegs > 256 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats_max.drawsegs,
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats_max.visplanes > 128 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats_max.visplanes,
		dsda_TextColor(dsda_tc_exhud_render_label),
		dsda_render_stats_max.vissprites > 128 ? dsda_TextColor(dsda_tc_exhud_render_bad) : dsda_TextColor(dsda_tc_exhud_render_good),
		dsda_render_stats_max.vissprites
	);
}

void dsda_InitRenderStatsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitTextHC(&local->component[0], x_offset, y_offset, vpt);
	dsda_InitTextHC(&local->component[1], x_offset, y_offset + 8, vpt);
}

void dsda_UpdateRenderStatsHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateCurrentComponentText(local->component[0].msg, sizeof(local->component[0].msg));
	dsda_UpdateMaxComponentText(local->component[1].msg, sizeof(local->component[1].msg));
	dsda_RefreshHudText(&local->component[0]);
	dsda_RefreshHudText(&local->component[1]);
}

void dsda_DrawRenderStatsHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component[0]);
	dsda_DrawBasicText(&local->component[1]);
}
