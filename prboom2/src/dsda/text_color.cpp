// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text Color

#include <algorithm>
#include <utility>

#include "cpp/EnumArray.hpp"

#include "doomdef.hpp"
#include "hu_lib.hpp"
#include "lprintf.hpp"
#include "w_wad.hpp"
#include "v_video.hpp"

#include "dsda/utility.hpp"

#include "text_color.hpp"

typedef struct
{
	const char* key;
	ColorRange color_range;
	char color_str[3];
} dsda_text_color_t;

constinit EnumArray<dsda_text_color_t, EnumCount<TextColorIndex>> dsda_text_colors = {
	{At(TextColorIndex::ExhudTimeLabel), {"exhud_time_label", ColorRange::Gray}},
	{At(TextColorIndex::ExhudLevelTime), {"exhud_level_time", ColorRange::Green}},
	{At(TextColorIndex::ExhudTotalTime), {"exhud_total_time", ColorRange::Gold}},
	{At(TextColorIndex::ExhudDemoLength), {"exhud_demo_length", ColorRange::Brown}},
	{At(TextColorIndex::ExhudArmorZero), {"exhud_armor_zero", ColorRange::Gray}},
	{At(TextColorIndex::ExhudArmorOne), {"exhud_armor_one", ColorRange::Green}},
	{At(TextColorIndex::ExhudArmorTwo), {"exhud_armor_two", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudCommandEntry), {"exhud_command_entry", ColorRange::Gray}},
	{At(TextColorIndex::ExhudCommandQueue), {"exhud_command_queue", ColorRange::Gold}},
	{At(TextColorIndex::ExhudCoordsBase), {"exhud_coords_base", ColorRange::Green}},
	{At(TextColorIndex::ExhudCoordsMf50), {"exhud_coords_mf50", ColorRange::Gray}},
	{At(TextColorIndex::ExhudCoordsSr40), {"exhud_coords_sr40", ColorRange::Green}},
	{At(TextColorIndex::ExhudCoordsSr50), {"exhud_coords_sr50", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudCoordsFast), {"exhud_coords_fast", ColorRange::Red}},
	{At(TextColorIndex::ExhudFpsBad), {"exhud_fps_bad", ColorRange::Red}},
	{At(TextColorIndex::ExhudFpsFine), {"exhud_fps_fine", ColorRange::Gray}},
	{At(TextColorIndex::ExhudHealthBad), {"exhud_health_bad", ColorRange::Red}},
	{At(TextColorIndex::ExhudHealthWarning), {"exhud_health_warning", ColorRange::Gold}},
	{At(TextColorIndex::ExhudHealthOk), {"exhud_health_ok", ColorRange::Green}},
	{At(TextColorIndex::ExhudHealthSuper), {"exhud_health_super", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudLineClose), {"exhud_line_close", ColorRange::Green}},
	{At(TextColorIndex::ExhudLineFar), {"exhud_line_far", ColorRange::Gray}},
	{At(TextColorIndex::ExhudLineSpecial), {"exhud_line_special", ColorRange::Green}},
	{At(TextColorIndex::ExhudLineNormal), {"exhud_line_normal", ColorRange::Gray}},
	{At(TextColorIndex::ExhudMobjAlive), {"exhud_mobj_alive", ColorRange::Green}},
	{At(TextColorIndex::ExhudMobjDead), {"exhud_mobj_dead", ColorRange::Gray}},
	{At(TextColorIndex::ExhudPlayerDamage), {"exhud_player_damage", ColorRange::Green}},
	{At(TextColorIndex::ExhudPlayerNeutral), {"exhud_player_neutral", ColorRange::Gray}},
	{At(TextColorIndex::ExhudAmmoLabel), {"exhud_ammo_label", ColorRange::Gray}},
	{At(TextColorIndex::ExhudAmmoMana1), {"exhud_ammo_mana1", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudAmmoMana2), {"exhud_ammo_mana2", ColorRange::Green}},
	{At(TextColorIndex::ExhudAmmoValue), {"exhud_ammo_value", ColorRange::Gray}},
	{At(TextColorIndex::ExhudAmmoBad), {"exhud_ammo_bad", ColorRange::Red}},
	{At(TextColorIndex::ExhudAmmoWarning), {"exhud_ammo_warning", ColorRange::Gold}},
	{At(TextColorIndex::ExhudAmmoOk), {"exhud_ammo_ok", ColorRange::Green}},
	{At(TextColorIndex::ExhudAmmoFull), {"exhud_ammo_full", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudRenderLabel), {"exhud_render_label", ColorRange::Gray}},
	{At(TextColorIndex::ExhudRenderGood), {"exhud_render_good", ColorRange::Gold}},
	{At(TextColorIndex::ExhudRenderBad), {"exhud_render_bad", ColorRange::Red}},
	{At(TextColorIndex::ExhudSectorActive), {"exhud_sector_active", ColorRange::Red}},
	{At(TextColorIndex::ExhudSectorSpecial), {"exhud_sector_special", ColorRange::Green}},
	{At(TextColorIndex::ExhudSectorNormal), {"exhud_sector_normal", ColorRange::Gray}},
	{At(TextColorIndex::ExhudSpeedLabel), {"exhud_speed_label", ColorRange::Gray}},
	{At(TextColorIndex::ExhudSpeedSlow), {"exhud_speed_slow", ColorRange::Gold}},
	{At(TextColorIndex::ExhudSpeedNormal), {"exhud_speed_normal", ColorRange::Green}},
	{At(TextColorIndex::ExhudSpeedFast), {"exhud_speed_fast", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudTotalsLabel), {"exhud_totals_label", ColorRange::Red}},
	{At(TextColorIndex::ExhudTotalsValue), {"exhud_totals_value", ColorRange::Gold}},
	{At(TextColorIndex::ExhudTotalsMax), {"exhud_totals_max", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudWeaponLabel), {"exhud_weapon_label", ColorRange::Gray}},
	{At(TextColorIndex::ExhudWeaponOwned), {"exhud_weapon_owned", ColorRange::Green}},
	{At(TextColorIndex::ExhudWeaponBerserk), {"exhud_weapon_berserk", ColorRange::Lightblue}},
	{At(TextColorIndex::ExhudAttempts), {"exhud_attempts", ColorRange::Gray}},
	{At(TextColorIndex::ExhudEventSplit), {"exhud_event_split", ColorRange::Gray}},
	{At(TextColorIndex::ExhudLineActivation), {"exhud_line_activation", ColorRange::Gray}},
	{At(TextColorIndex::ExhudLocalTime), {"exhud_local_time", ColorRange::Gray}},
	{At(TextColorIndex::ExhudFreeText), {"exhud_free_text", ColorRange::Gray}},
	{At(TextColorIndex::HudMessage), {"hud_message", ColorRange::Default}},
	{At(TextColorIndex::HudSecretMessage), {"hud_secret_message", ColorRange::Gold}},
	{At(TextColorIndex::MapCoords), {"map_coords", ColorRange::Green}},
	{At(TextColorIndex::MapTimeLevel), {"map_time_level", ColorRange::Gray}},
	{At(TextColorIndex::MapTimeTotal), {"map_time_total", ColorRange::Gray}},
	{At(TextColorIndex::MapTitle), {"map_title", ColorRange::Gold}},
	{At(TextColorIndex::MapTotalsLabel), {"map_totals_label", ColorRange::Red}},
	{At(TextColorIndex::MapTotalsValue), {"map_totals_value", ColorRange::Gray}},
	{At(TextColorIndex::MapTotalsMax), {"map_totals_max", ColorRange::Lightblue}},
	{At(TextColorIndex::InterSplitNormal), {"inter_split_normal", ColorRange::Gray}},
	{At(TextColorIndex::InterSplitGood), {"inter_split_good", ColorRange::Green}},
	{At(TextColorIndex::InterSplitBest), {"inter_split_best", ColorRange::Gold}},
	{At(TextColorIndex::MenuTitle), {"menu_title", ColorRange::Gold}},
	{At(TextColorIndex::MenuTab), {"menu_tab", ColorRange::Tan}},
	{At(TextColorIndex::MenuTabHighlight), {"menu_tab_highlight", ColorRange::Gold}},
	{At(TextColorIndex::MenuLabel), {"menu_label", ColorRange::Red}},
	{At(TextColorIndex::MenuLabelHighlight), {"menu_label_highlight", ColorRange::Brick}},
	{At(TextColorIndex::MenuLabelEdit), {"menu_label_edit", ColorRange::Gray}},
	{At(TextColorIndex::MenuValue), {"menu_value", ColorRange::Green}},
	{At(TextColorIndex::MenuValueHighlight), {"menu_value_highlight", ColorRange::Brick}},
	{At(TextColorIndex::MenuValueEdit), {"menu_value_edit", ColorRange::Gray}},
	{At(TextColorIndex::MenuInfoHighlight), {"menu_info_highlight", ColorRange::Brick}},
	{At(TextColorIndex::MenuInfoEdit), {"menu_info_edit", ColorRange::Gray}},
	{At(TextColorIndex::MenuWarning), {"menu_warning", ColorRange::Red}},
	{At(TextColorIndex::MenuScrollbar), {"menu_scrollbar", ColorRange::Tan}},
	{At(TextColorIndex::StbarHealthBad), {"stbar_health_bad", ColorRange::Red}},
	{At(TextColorIndex::StbarHealthWarning), {"stbar_health_warning", ColorRange::Gold}},
	{At(TextColorIndex::StbarHealthOk), {"stbar_health_ok", ColorRange::Green}},
	{At(TextColorIndex::StbarHealthSuper), {"stbar_health_super", ColorRange::Blue}},
	{At(TextColorIndex::StbarArmorZero), {"stbar_armor_zero", ColorRange::Gray}},
	{At(TextColorIndex::StbarArmorOne), {"stbar_armor_one", ColorRange::Green}},
	{At(TextColorIndex::StbarArmorTwo), {"stbar_armor_two", ColorRange::Blue}},
	{At(TextColorIndex::StbarAmmoBad), {"stbar_ammo_bad", ColorRange::Red}},
	{At(TextColorIndex::StbarAmmoWarning), {"stbar_ammo_warning", ColorRange::Gold}},
	{At(TextColorIndex::StbarAmmoOk), {"stbar_ammo_ok", ColorRange::Green}},
	{At(TextColorIndex::StbarAmmoFull), {"stbar_ammo_full", ColorRange::Blue}},
};

const char* dsda_TextColor(TextColorIndex i)
{
	return dsda_text_colors[i].color_str;
}

ColorRange dsda_TextCR(TextColorIndex i)
{
	return dsda_text_colors[i].color_range;
}

void dsda_LoadTextColor()
{
	char* lump;
	char** lines;
	const char* line;
	int line_i;
	char key[33] = {0};

	lump = W_ReadLumpToString(W_GetNumForName("DSDATC"));

	lines = dsda_SplitString(lump, "\n\r");

	for(line_i = 0; lines[line_i]; ++line_i)
	{
		line = lines[line_i];

		if(!line[0] || line[0] == '/')
			continue;

		int color_range_value;

		if(sscanf(line, "%32s %d", key, &color_range_value) != 2)
			I_Error("DSDATC lump has unknown format! (%s)", line);

		const auto color = std::ranges::find_if(dsda_text_colors, [&](const dsda_text_color_t& entry)
		{
			return !strcasecmp(key, entry.key);
		});

		if(color == dsda_text_colors.end())
			I_Error("DSDATC lump has unknown key %s!", key);

		color->color_range = static_cast<ColorRange>(color_range_value);
	}

	for(dsda_text_color_t& color : dsda_text_colors)
	{
		color.color_str[0] = '\x1b';
		color.color_str[1] = HUlib_Color(color.color_range);
	}

	Z_Free(lines);
	Z_Free(lump);
}

static const char* color_name_to_index[std::to_underlying(ColorRange::HudLimit)] = {
	"",
	"brick",
	"tan",
	"gray",
	"green",
	"brown",
	"gold",
	"red",
	"blue",
	"orange",
	"yellow",
	"light blue",
	"black",
	"purple",
	"white",
};

int dsda_ColorNameToIndex(const char* name)
{
	int i;

	if(!name)
		return std::to_underlying(ColorRange::Default);

	for(i = std::to_underlying(ColorRange::Default) + 1; i < std::to_underlying(ColorRange::HudLimit); ++i)
		if(!stricmp(color_name_to_index[i], name))
			return i;

	return std::to_underlying(ColorRange::Default);
}
