// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA HUD Component Base

#include <utility>

#include <math.h>

#include "base.hpp"

static char digit_lump[9];
static const char* digit_lump_format;

extern "C" int dsda_ExHudVerticalOffset();
int dsda_HudComponentY(int y_offset, PatchTranslation vpt, double ratio)
{

	int y = 0;
	PatchTranslation vpt_align;

	if(ratio)
		y_offset *= ratio;

	vpt_align = PatchAlignment(vpt);
	if(BOTTOM_ALIGNMENT(vpt_align))
	{
		y = 200;
		y_offset = -y_offset;

		y -= dsda_ExHudVerticalOffset();
	}

	return y + y_offset;
}

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, PatchTranslation vpt)
{
	static double ratio;
	int x, y;

	DO_ONCE
		if(exhud_font.line_height != 8)
			ratio = (double)exhud_font.line_height / 8.0;
	END_ONCE

	x = x_offset;
	y = dsda_HudComponentY(y_offset, vpt, ratio);

	HUlib_initTextLine(&component->text, x, y, &exhud_font, ColorRange::Gray, (PatchTranslation)vpt);
}

void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, PatchTranslation vpt)
{
	static double ratio;
	int x, y;

	DO_ONCE
		if(hud_font.line_height != 8)
			ratio = (double)hud_font.line_height / 8.0;
	END_ONCE

	x = x_offset;
	y = dsda_HudComponentY(y_offset, vpt, ratio);

	HUlib_initTextLine(&component->text, x, y, &hud_font, ColorRange::Gray, (PatchTranslation)vpt);
}

void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, PatchTranslation vpt)
{
	int x, y;

	x = x_offset;
	y = dsda_HudComponentY(y_offset, vpt, 0);

	component->x = x;
	component->y = y;
	component->vpt = vpt;

	if(raven)
		digit_lump_format = "IN%.1d";
	else
		digit_lump_format = "STTNUM%.1d";
}

int dsda_HexenArmor(player_t* player)
{
	int temp = pclass[player->pclass].auto_armor_save
		+ player->armorpoints[std::to_underlying(ArmorType::Armor)]
		+ player->armorpoints[std::to_underlying(ArmorType::Shield)]
		+ player->armorpoints[std::to_underlying(ArmorType::Helmet)]
		+ player->armorpoints[std::to_underlying(ArmorType::Amulet)];
	return FixedDiv(temp, 5 * FRACUNIT) >> FRACBITS;
}

static void dsda_DrawBigDigit(int x, int y, ColorRange cm, PatchTranslation vpt, int digit)
{
	extern int sts_colored_numbers;
	if(digit > 9 || digit < 0)
		return;

	snprintf(digit_lump, sizeof(digit_lump), digit_lump_format, digit);
	V_DrawNamePatch(x, y, FG, digit_lump, cm, static_cast<PatchTranslation>((PatchTranslation)(vpt | ((sts_colored_numbers ? PatchTranslation::Trans : PatchTranslation::None)))));
}

static int digit_mod[6] = {1, 10, 100, 1000, 10000, 100000};
static int digit_div[6] = {1, 1, 10, 100, 1000, 10000};

void dsda_DrawBigNumber(int x, int y, int delta_x, int delta_y, ColorRange cm, PatchTranslation vpt, int count, int n)
{
	int i;
	int digit, any_digit;

	if(count > 5)
		return;

	any_digit = 0;

	for(i = count; i > 0; --i)
	{
		digit = (n % digit_mod[i]) / digit_div[i];
		any_digit |= digit;

		if(any_digit || i == 1)
			dsda_DrawBigDigit(x, y, cm, vpt, digit);

		x += delta_x;
	}
}

void dsda_DrawBasicText(dsda_text_t* component)
{
	HUlib_drawTextLine(&component->text, false);
}

void dsda_RefreshHudText(dsda_text_t* component)
{
	const char* s;

	HUlib_clearTextLine(&component->text);

	s = component->msg;
	while(*s) HUlib_addCharToTextLine(&component->text, *(s++));
}
