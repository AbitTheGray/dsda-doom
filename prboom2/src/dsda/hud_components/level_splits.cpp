// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Level Splits HUD Component

#include "dsda/split_tracker.hpp"

#include "base.hpp"

#include "level_splits.hpp"

typedef struct
{
	dsda_text_t time_component;
	dsda_text_t total_component;
} local_component_t;

static local_component_t* local;

extern int leveltime, totalleveltimes;

static int dsda_SplitComparisonDelta(dsda_split_time_t* split_time)
{
	return split_time->ref != -1 ? split_time->ref_delta : split_time->best_delta;
}

static void dsda_UpdateIntermissionTime(dsda_split_t* split)
{
	char delta[18];
	const char* color;

	delta[0] = '\0';
	color = dsda_TextColor(TextColorIndex::InterSplitNormal);

	if(split && !split->first_time)
	{
		const char* sign;
		int diff;

		diff = dsda_SplitComparisonDelta(&split->leveltime);
		sign = diff >= 0 ? "+" : "-";
		color = diff >= 0 ? dsda_TextColor(TextColorIndex::InterSplitNormal) : split->leveltime.best_delta >= 0 ? dsda_TextColor(TextColorIndex::InterSplitGood) : dsda_TextColor(TextColorIndex::InterSplitBest);
		diff = abs(diff);

		if(diff >= 2100)
		{
			snprintf(
				delta, sizeof(delta),
				" (%s%d:%05.2f)",
				sign, diff / TICRATE / 60, (float)(diff % (60 * TICRATE)) / TICRATE
			);
		}
		else
		{
			snprintf(
				delta, sizeof(delta),
				" (%s%04.2f)",
				sign, (float)(diff % (60 * TICRATE)) / TICRATE
			);
		}
	}

	snprintf(
		local->time_component.msg,
		sizeof(local->time_component.msg),
		"%s%d:%05.2f",
		color, leveltime / TICRATE / 60,
		(float)(leveltime % (60 * TICRATE)) / TICRATE
	);

	strcat(local->time_component.msg, delta);

	dsda_RefreshHudText(&local->time_component);
}

static void dsda_UpdateIntermissionTotal(dsda_split_t* split)
{
	char delta[16];
	const char* color;

	delta[0] = '\0';
	color = dsda_TextColor(TextColorIndex::InterSplitNormal);

	if(split && !split->first_time)
	{
		const char* sign;
		int diff;

		diff = dsda_SplitComparisonDelta(&split->totalleveltimes) / TICRATE;
		sign = diff >= 0 ? "+" : "-";
		color = diff >= 0 ? dsda_TextColor(TextColorIndex::InterSplitNormal) : dsda_TextColor(TextColorIndex::InterSplitGood);
		diff = abs(diff);

		if(diff >= 60)
		{
			snprintf(
				delta, sizeof(delta),
				" (%s%d:%02d)",
				sign, diff / 60, diff % 60
			);
		}
		else
		{
			snprintf(
				delta, sizeof(delta),
				" (%s%d)",
				sign, diff % 60
			);
		}
	}

	snprintf(
		local->total_component.msg,
		sizeof(local->total_component.msg),
		"%s%d:%02d",
		color, totalleveltimes / TICRATE / 60,
		(totalleveltimes / TICRATE) % 60
	);

	strcat(local->total_component.msg, delta);

	dsda_RefreshHudText(&local->total_component);
}

void dsda_InitLevelSplitsHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitTextHC(&local->time_component, x_offset, y_offset, vpt);
	dsda_InitTextHC(&local->total_component, x_offset, y_offset + 8, vpt);
}

void dsda_UpdateLevelSplitsHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawLevelSplitsHC(void* data)
{
	dsda_split_t* split;

	local = (local_component_t*)data;

	split = dsda_CurrentSplit();

	dsda_UpdateIntermissionTime(split);
	dsda_UpdateIntermissionTotal(split);

	dsda_DrawBasicText(&local->time_component);
	dsda_DrawBasicText(&local->total_component);
}
