// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Keys HUD Component

#include <utility>

#include "base.hpp"

#include "keys.hpp"

#define PATCH_DELTA 10

typedef struct
{
	dsda_patch_component_t component;
	dboolean horizontal;
} local_component_t;

static local_component_t* local;

static int key_patch_num[std::to_underlying(Card::Count)];

static const char* dsda_Key1Name(player_t* player)
{
	if(heretic)
	{
		if(player->cards[std::to_underlying(Card::KeyYellow)])
			return "ykeyicon";
	}
	else
	{
		if(player->cards[0] && player->cards[3])
			return "STKEYS6";
		else if(player->cards[0])
			return "STKEYS0";
		else if(player->cards[3])
			return "STKEYS3";
	}

	return nullptr;
}

static const char* dsda_Key2Name(player_t* player)
{
	if(heretic)
	{
		if(player->cards[std::to_underlying(Card::KeyGreen)])
			return "gkeyicon";
	}
	else
	{
		if(player->cards[1] && player->cards[4])
			return "STKEYS7";
		else if(player->cards[1])
			return "STKEYS1";
		else if(player->cards[4])
			return "STKEYS4";
	}

	return nullptr;
}

static const char* dsda_Key3Name(player_t* player)
{
	if(heretic)
	{
		if(player->cards[std::to_underlying(Card::KeyBlue)])
			return "bkeyicon";
	}
	else
	{
		if(player->cards[2] && player->cards[5])
			return "STKEYS8";
		else if(player->cards[2])
			return "STKEYS2";
		else if(player->cards[5])
			return "STKEYS5";
	}

	return nullptr;
}

void drawKey(player_t* player, int* x, int* y, const char* (*key)(player_t*))
{
	const char* name;

	name = key(player);

	if(name)
		V_DrawNamePatch(*x, *y, FG, name, ColorRange::Default, static_cast<PatchTranslation>((PatchTranslation)local->component.vpt));

	if(local->horizontal)
		*x += PATCH_DELTA;
	else
		*y += PATCH_DELTA;
}

static void dsda_DrawComponent()
{
	player_t* player;
	int x, y;

	player = &players[displayplayer];

	x = local->component.x;
	y = local->component.y;

	if(hexen)
	{
		int i;

		for(i = 0; i < std::to_underlying(Card::Count); ++i)
			if(player->cards[i])
			{
				V_DrawNumPatch(x, y, 0, key_patch_num[i], ColorRange::Default, static_cast<PatchTranslation>((PatchTranslation)local->component.vpt));
				x += R_NumPatchWidth(key_patch_num[i]) + 4;
			}

		return;
	}

	drawKey(player, &x, &y, dsda_Key1Name);
	drawKey(player, &x, &y, dsda_Key2Name);
	drawKey(player, &x, &y, dsda_Key3Name);
}

void dsda_InitKeysHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	local->horizontal = arg_count > 0 ? !!args[0] : false;

	if(hexen)
	{
		int i;

		for(i = 0; i < std::to_underlying(Card::Count); ++i)
			key_patch_num[i] = R_NumPatchForSpriteIndex(static_cast<SpriteId>(std::to_underlying(SpriteId::HexenKey1) + i));
	}

	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateKeysHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawKeysHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawComponent();
}
