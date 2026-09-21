// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Armor HUD Component

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
	int cm;
	int lump;
	int armor;

	player = &players[displayplayer];
	x = local->component.x;
	y = local->component.y;

	if(hexen)
	{
		armor = dsda_HexenArmor(player);
		cm = dsda_TextCR(dsda_tc_stbar_armor_zero);
		lump = armor_lump_green;
	}
	else
	{
		armor = player->armorpoints[ARMOR_ARMOR];
		if(armor <= 0)
		{
			cm = dsda_TextCR(dsda_tc_stbar_armor_zero);
			lump = armor_lump_green;
		}
		else if(player->armortype < 2)
		{
			cm = dsda_TextCR(dsda_tc_stbar_armor_one);
			lump = armor_lump_green;
		}
		else
		{
			cm = dsda_TextCR(dsda_tc_stbar_armor_two);
			lump = armor_lump_blue;
		}
	}

	V_DrawNumPatch(x, y, FG, lump, CR_DEFAULT, static_cast<enum patch_translation_e>((enum patch_translation_e)local->component.vpt));

	x += patch_spacing;
	y += patch_vertical_spacing;

	dsda_DrawBigNumber(x, y, patch_delta_x, 0,
		cm, local->component.vpt, 3, armor);
}

void dsda_InitBigArmorHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(heretic)
	{
		armor_lump_green = R_NumPatchForSpriteIndex(HERETIC_SPR_SHLD);
		armor_lump_blue = R_NumPatchForSpriteIndex(HERETIC_SPR_SHD2);
		patch_delta_x = 10;
		patch_vertical_spacing = 6;
		patch_spacing = 2;
	}
	else if(hexen)
	{
		armor_lump_green = R_NumPatchForSpriteIndex(HEXEN_SPR_ARM3);
		armor_lump_blue = R_NumPatchForSpriteIndex(HEXEN_SPR_ARM3);
		patch_delta_x = 10;
		patch_vertical_spacing = 4;
		patch_spacing = 2;
	}
	else
	{
		armor_lump_green = R_NumPatchForSpriteIndex(SPR_ARM1);
		armor_lump_blue = R_NumPatchForSpriteIndex(SPR_ARM2);
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
