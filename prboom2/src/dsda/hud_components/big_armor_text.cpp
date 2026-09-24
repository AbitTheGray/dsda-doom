// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Armor Text HUD Component

#include <utility>

#include "base.hpp"

#include "big_armor_text.hpp"

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
	int armor;

	player = &players[displayplayer];

	if(hexen)
	{
		armor = dsda_HexenArmor(player);
		cm = dsda_TextCR(TextColorIndex::StbarArmorZero);
	}
	else
	{
		armor = player->armorpoints[std::to_underlying(ArmorType::Armor)];
		if(armor <= 0)
			cm = dsda_TextCR(TextColorIndex::StbarArmorZero);
		else if(player->armortype < 2)
			cm = dsda_TextCR(TextColorIndex::StbarArmorOne);
		else
			cm = dsda_TextCR(TextColorIndex::StbarArmorTwo);
	}

	dsda_DrawBigNumber(local->component.x, local->component.y, patch_delta_x, 0,
		cm, local->component.vpt, 3, armor);
}

void dsda_InitBigArmorTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
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

void dsda_UpdateBigArmorTextHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawBigArmorTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawComponent();
}
