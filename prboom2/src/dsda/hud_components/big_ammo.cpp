// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Ammo HUD Component

#include <utility>

#include "base.hpp"

#include "big_ammo.hpp"

#define PATCH_DELTA_X 14

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

TextColorIndex dsda_AmmoColorBig(player_t* player)
{
	int ammo_percent;

	ammo_percent = P_AmmoPercent(player, player->readyweapon);

	if(ammo_percent < hud_ammo_red)
		return TextColorIndex::StbarAmmoBad;
	else if(ammo_percent < hud_ammo_yellow)
		return TextColorIndex::StbarAmmoWarning;
	else if(ammo_percent < 100)
		return TextColorIndex::StbarAmmoOk;
	else
		return TextColorIndex::StbarAmmoFull;
}

static void dsda_DrawComponent()
{
	player_t* player;
	AmmoType ammo_type;
	int ammo;

	if(hexen)
		return;

	player = &players[displayplayer];
	ammo_type = weaponinfo[std::to_underlying(player->readyweapon)].ammo;

	if(ammo_type == AmmoType::NoAmmo || !player->maxammo[std::to_underlying(ammo_type)])
		return;

	ammo = player->ammo[std::to_underlying(ammo_type)];

	dsda_DrawBigNumber(local->component.x, local->component.y, PATCH_DELTA_X, 0,
		dsda_TextCR(dsda_AmmoColorBig(player)), local->component.vpt, 3, ammo);
}

void dsda_InitBigAmmoHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigAmmoHC(void* data)
{
	local = (local_component_t*)data;
}

void dsda_DrawBigAmmoHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawComponent();
}
