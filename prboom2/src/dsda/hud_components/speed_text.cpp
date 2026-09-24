// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Speed Text HUD Component

#include "base.hpp"

#include "speed_text.hpp"

typedef struct
{
	dsda_text_t component;
	char label[9];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	int speed;

	speed = dsda_GameSpeed();

	snprintf(
		str,
		max_size,
		"%s%s%d%%",
		local->label,
		speed < 100
		? dsda_TextColor(TextColorIndex::ExhudSpeedSlow)
		: speed == 100
		? dsda_TextColor(TextColorIndex::ExhudSpeedNormal)
		: dsda_TextColor(TextColorIndex::ExhudSpeedFast),
		speed
	);
}

void dsda_InitSpeedTextHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	if(arg_count < 1 || args[0])
		snprintf(local->label, sizeof(local->label), "%sSPEED ", dsda_TextColor(TextColorIndex::ExhudSpeedLabel));
	else
		local->label[0] = '\0';

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSpeedTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawSpeedTextHC(void* data)
{
	local = (local_component_t*)data;

	dsda_DrawBasicText(&local->component);
}
