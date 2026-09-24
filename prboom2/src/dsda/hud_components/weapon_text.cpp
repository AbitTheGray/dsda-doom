// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Weapon Text HUD Component

#include <utility>

#include "base.hpp"

#include "weapon_text.hpp"

typedef struct
{
	dsda_text_t component;
	dboolean grid;
	const char* label_w;
	const char* label_p;
	const char* label_n;
	const char* label_wpn;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	player_t* player;

	player = &players[displayplayer];

	if(local->grid)
		snprintf(
			str,
			max_size,
			"%s%s%s%c %s%c %c\n"
			"%s%s%s%c %c %c\n"
			"%s%s%s%c %c %c",
			dsda_TextColor(TextColorIndex::ExhudWeaponLabel),
			local->label_w,
			player->powers[std::to_underlying(PowerType::Strength)] ? dsda_TextColor(TextColorIndex::ExhudWeaponBerserk) : dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[0] ? '1' : ' ',
			dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[1] ? '2' : ' ',
			player->weaponowned[2] ? '3' : ' ',
			dsda_TextColor(TextColorIndex::ExhudWeaponLabel),
			local->label_p,
			dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[3] ? '4' : ' ',
			player->weaponowned[4] ? '5' : ' ',
			player->weaponowned[5] ? '6' : ' ',
			dsda_TextColor(TextColorIndex::ExhudWeaponLabel),
			local->label_n,
			dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[6] ? '7' : ' ',
			player->weaponowned[7] ? '8' : ' ',
			player->weaponowned[8] ? '9' : ' '
		);
	else
		snprintf(
			str,
			max_size,
			"%s%s%s%c %s%c %c %c %c %c %c %c %c",
			dsda_TextColor(TextColorIndex::ExhudWeaponLabel),
			local->label_wpn,
			player->powers[std::to_underlying(PowerType::Strength)] ? dsda_TextColor(TextColorIndex::ExhudWeaponBerserk) : dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[0] ? '1' : ' ',
			dsda_TextColor(TextColorIndex::ExhudWeaponOwned),
			player->weaponowned[1] ? '2' : ' ',
			player->weaponowned[2] ? '3' : ' ',
			player->weaponowned[3] ? '4' : ' ',
			player->weaponowned[4] ? '5' : ' ',
			player->weaponowned[5] ? '6' : ' ',
			player->weaponowned[6] ? '7' : ' ',
			player->weaponowned[7] ? '8' : ' ',
			player->weaponowned[8] ? '9' : ' '
		);
}

void dsda_InitWeaponTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	local->grid = args[0];

	if(arg_count < 2 || args[1])
	{
		local->label_w = "W ";
		local->label_p = "P ";
		local->label_n = "N ";
		local->label_wpn = "WPN ";
	}
	else
	{
		local->label_w = "";
		local->label_p = "";
		local->label_n = "";
		local->label_wpn = "";
	}

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateWeaponTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawWeaponTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
