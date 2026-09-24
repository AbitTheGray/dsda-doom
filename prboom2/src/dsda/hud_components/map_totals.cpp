// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Totals HUD Component

#include "dsda/skill_info.hpp"

#include "base.hpp"

#include "map_totals.hpp"

typedef struct
{
	dsda_text_t component;
	dboolean include_kills, include_items, include_secrets;
	dboolean hide_totals;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	int i;
	size_t length;
	int fullkillcount, fullitemcount, fullsecretcount;
	const char* killcolor;
	const char* itemcolor;
	const char* secretcolor;
	int kill_percent_count;
	int max_kill_requirement;

	length = 0;
	fullkillcount = 0;
	fullitemcount = 0;
	fullsecretcount = 0;
	kill_percent_count = 0;
	max_kill_requirement = dsda_MaxKillRequirement();

	for(i = 0; i < g_maxplayers; ++i)
	{
		if(playeringame[i])
		{
			fullkillcount += players[i].killcount - players[i].maxkilldiscount;
			fullitemcount += players[i].itemcount;
			fullsecretcount += players[i].secretcount;
			kill_percent_count += players[i].killcount;
		}
	}

	if(skill_info.respawn_time)
	{
		fullkillcount = kill_percent_count;
		max_kill_requirement = totalkills;
	}

	killcolor = (fullkillcount >= max_kill_requirement ? dsda_TextColor(TextColorIndex::MapTotalsMax) : dsda_TextColor(TextColorIndex::MapTotalsValue));
	itemcolor = (fullitemcount >= totalitems ? dsda_TextColor(TextColorIndex::MapTotalsMax) : dsda_TextColor(TextColorIndex::MapTotalsValue));
	secretcolor = (fullsecretcount >= totalsecret ? dsda_TextColor(TextColorIndex::MapTotalsMax) : dsda_TextColor(TextColorIndex::MapTotalsValue));

	if(local->include_kills)
	{
		if(!local->hide_totals || fullkillcount >= max_kill_requirement)
			length += snprintf(
				str,
				max_size,
				"%sMonsters: %s%d/%d\n",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				killcolor, fullkillcount, max_kill_requirement
			);
		else
			length += snprintf(
				str,
				max_size,
				"%sMonsters: %s%d\n",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				killcolor, fullkillcount
			);
	}

	if(local->include_items)
	{
		if(!local->hide_totals || fullitemcount >= totalitems)
			length += snprintf(
				str + length,
				max_size - length,
				"%sItems: %s%d/%d\n",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				itemcolor, fullitemcount, totalitems
			);
		else
			length += snprintf(
				str + length,
				max_size - length,
				"%sItems: %s%d\n",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				itemcolor, fullitemcount
			);
	}

	if(local->include_secrets)
	{
		if(!local->hide_totals || fullsecretcount >= totalsecret)
			snprintf(
				str + length,
				max_size - length,
				"%sSecrets: %s%d/%d",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				secretcolor, fullsecretcount, totalsecret
			);
		else
			snprintf(
				str + length,
				max_size - length,
				"%sSecrets: %s%d",
				dsda_TextColor(TextColorIndex::MapTotalsLabel),
				secretcolor, fullsecretcount
			);
	}
}

void dsda_InitMapTotalsHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	local->include_kills = args[0];
	local->include_items = args[1];
	local->include_secrets = args[2];

	local->hide_totals = args[3];

	if(!local->include_kills && !local->include_items && !local->include_secrets)
		local->include_kills = local->include_items = local->include_secrets = true;

	dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTotalsHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapTotalsHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
