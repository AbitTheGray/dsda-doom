// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ready Ammo Text HUD Component

#include <utility>

#include "base.hpp"

#include "ready_ammo_text.hpp"

typedef struct
{
	dsda_text_t component;
} local_component_t;

static local_component_t* local;

TextColorIndex dsda_AmmoColor(player_t* player)
{
	int ammo_percent;

	ammo_percent = P_AmmoPercent(player, player->readyweapon);

	if(ammo_percent < hud_ammo_red)
		return TextColorIndex::ExhudAmmoBad;
	else if(ammo_percent < hud_ammo_yellow)
		return TextColorIndex::ExhudAmmoWarning;
	else if(ammo_percent < 100)
		return TextColorIndex::ExhudAmmoOk;
	else
		return TextColorIndex::ExhudAmmoFull;
}

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	player_t* player;

	player = &players[displayplayer];

	if(hexen)
	{
		snprintf(str, max_size, "%sAMM %s%3d %s%3d",
			dsda_TextColor(TextColorIndex::ExhudAmmoLabel),
			dsda_TextColor(TextColorIndex::ExhudAmmoMana1), player->ammo[0],
			dsda_TextColor(TextColorIndex::ExhudAmmoMana2), player->ammo[1]
		);
	}
	else
	{
		AmmoType ammo_type = weaponinfo[std::to_underlying(player->readyweapon)].ammo;

		if(ammo_type == AmmoType::NoAmmo || !player->maxammo[std::to_underlying(ammo_type)])
		{
			snprintf(str, max_size, "%sAMM %sN/A",
				dsda_TextColor(TextColorIndex::ExhudAmmoLabel),
				dsda_TextColor(TextColorIndex::ExhudAmmoValue)
			);
		}
		else
		{
			snprintf(str, max_size, "%sAMM %s%3d",
				dsda_TextColor(TextColorIndex::ExhudAmmoLabel),
				dsda_TextColor(dsda_AmmoColor(player)),
				player->ammo[std::to_underlying(ammo_type)]
			);
		}
	}
}

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateReadyAmmoTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawReadyAmmoTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
