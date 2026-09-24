// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ammo Text HUD Component

#include <utility>

#include "base.hpp"

#include "ammo_text.hpp"

typedef struct
{
	dsda_text_t component[6];
	dboolean show_names;
} local_component_t;

static local_component_t* local;

typedef struct
{
	const char** ammo_name;
	const int* ammo_type;
	const int count;
} ammo_component_config_t;

static const char* doom_ammo_name[4] = {
	"BULL",
	"SHEL",
	"RCKT",
	"CELL",
};

static const int doom_ammo_type[4] = {0, 1, 3, 2};

static const char* heretic_ammo_name[6] = {
	"CRYS",
	"BOLT",
	"CLAW",
	"RUNE",
	"FLAM",
	"MACE",
};

static const int heretic_ammo_type[6] = {0, 1, 2, 3, 4, 5};

static const char* hexen_ammo_name[2] = {
	"BLUE",
	"GREN",
};

static const int hexen_ammo_type[2] = {0, 1};

static const ammo_component_config_t doom_ammo = {doom_ammo_name, doom_ammo_type, 4};
static const ammo_component_config_t heretic_ammo = {heretic_ammo_name, heretic_ammo_type, 6};
static const ammo_component_config_t hexen_ammo = {hexen_ammo_name, hexen_ammo_type, 2};

static const ammo_component_config_t* component_config;

static void dsda_UpdateComponentText(char* str, size_t max_size, int i)
{
	player_t* player;
	int current_ammo, max_ammo;
	const char* name;

	player = &players[displayplayer];
	name = component_config->ammo_name[i];
	i = component_config->ammo_type[i];
	current_ammo = player->ammo[i];
	max_ammo = hexen ? MAX_MANA : player->maxammo[i];

	if(local->show_names)
		snprintf(
			str,
			max_size,
			"%s%s %s%3d\x1b\x01/\x1b\x01%3d",
			dsda_TextColor(TextColorIndex::ExhudAmmoLabel),
			name,
			dsda_TextColor(TextColorIndex::ExhudAmmoValue),
			current_ammo,
			max_ammo
		);
	else
		snprintf(
			str,
			max_size,
			"%s%3d\x1b\x01/\x1b\x01%3d",
			dsda_TextColor(TextColorIndex::ExhudAmmoValue),
			current_ammo,
			max_ammo
		);
}

void dsda_InitAmmoTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	int i;
	int y_delta;

	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(arg_count > 0)
		local->show_names = args[0];
	else
		local->show_names = true;

	if(heretic)
		component_config = &heretic_ammo;
	else if(hexen)
		component_config = &hexen_ammo;
	else
		component_config = &doom_ammo;

	y_delta = BOTTOM_ALIGNMENT(PatchAlignment(vpt)) ? -8 : 8;

	for(i = 0; i < component_config->count; ++i)
		dsda_InitTextHC(&local->component[i], x_offset, y_offset + i * y_delta, vpt);
}

void dsda_UpdateAmmoTextHC(void* data)
{
	int i;

	local = (local_component_t*)data;

	for(i = 0; i < component_config->count; ++i)
	{
		dsda_UpdateComponentText(local->component[i].msg, sizeof(local->component[i].msg), i);
		dsda_RefreshHudText(&local->component[i]);
	}
}

void dsda_DrawAmmoTextHC(void* data)
{
	int i;

	local = (local_component_t*)data;

	for(i = 0; i < component_config->count; ++i)
		dsda_DrawBasicText(&local->component[i]);
}
