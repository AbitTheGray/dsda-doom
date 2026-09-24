// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Armor HUD Component

#include <utility>

#include "base.hpp"

#include "big_armor.hpp"

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static int armor_lump_green;
static int armor_lump_blue;
static int patch_delta_x;
static int patch_vertical_spacing;
static int patch_spacing;

static void dsda_DrawComponent()
{
	player_t* player;
	int x, y;
	ColorRange cm;
	int lump;
	int armor;

	player = &players[displayplayer];
	x = local->component.x;
	y = local->component.y;

	if(hexen)
	{
		armor = dsda_HexenArmor(player);
		cm = dsda_TextCR(TextColorIndex::StbarArmorZero);
		lump = armor_lump_green;
	}
	else
	{
		armor = player->armorpoints[std::to_underlying(ArmorType::Armor)];
		if(armor <= 0)
		{
			cm = dsda_TextCR(TextColorIndex::StbarArmorZero);
			lump = armor_lump_green;
		}
		else if(player->armortype < 2)
		{
			cm = dsda_TextCR(TextColorIndex::StbarArmorOne);
			lump = armor_lump_green;
		}
		else
		{
			cm = dsda_TextCR(TextColorIndex::StbarArmorTwo);
			lump = armor_lump_blue;
		}
	}

	V_DrawNumPatch(x, y, FG, lump, ColorRange::Default, static_cast<PatchTranslation>((PatchTranslation)local->component.vpt));

	x += patch_spacing;
	y += patch_vertical_spacing;

	dsda_DrawBigNumber(x, y, patch_delta_x, 0,
		cm, local->component.vpt, 3, armor);
}

void dsda_InitBigArmorHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(heretic)
	{
		armor_lump_green = R_NumPatchForSpriteIndex(SpriteId::HereticShld);
		armor_lump_blue = R_NumPatchForSpriteIndex(SpriteId::HereticShd2);
		patch_delta_x = 10;
		patch_vertical_spacing = 6;
		patch_spacing = 2;
	}
	else if(hexen)
	{
		armor_lump_green = R_NumPatchForSpriteIndex(SpriteId::HexenArm3);
		armor_lump_blue = R_NumPatchForSpriteIndex(SpriteId::HexenArm3);
		patch_delta_x = 10;
		patch_vertical_spacing = 4;
		patch_spacing = 2;
	}
	else
	{
		armor_lump_green = R_NumPatchForSpriteIndex(SpriteId::Arm1);
		armor_lump_blue = R_NumPatchForSpriteIndex(SpriteId::Arm2);
		patch_delta_x = 14;
		patch_vertical_spacing = 1;
		patch_spacing = 2;
	}
	patch_spacing += MAX(R_NumPatchWidth(armor_lump_green), R_NumPatchWidth(armor_lump_blue));
	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawBigArmorHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawComponent();
}
