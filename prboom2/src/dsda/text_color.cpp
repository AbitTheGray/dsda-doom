// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text Color

#include <utility>

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

dsda_text_color_t dsda_text_colors[] = {
	[std::to_underlying(TextColorIndex::ExhudTimeLabel)] = {"exhud_time_label", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudLevelTime)] = {"exhud_level_time", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudTotalTime)] = {"exhud_total_time", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudDemoLength)] = {"exhud_demo_length", ColorRange::Brown},
	[std::to_underlying(TextColorIndex::ExhudArmorZero)] = {"exhud_armor_zero", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudArmorOne)] = {"exhud_armor_one", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudArmorTwo)] = {"exhud_armor_two", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudCommandEntry)] = {"exhud_command_entry", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudCommandQueue)] = {"exhud_command_queue", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudCoordsBase)] = {"exhud_coords_base", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudCoordsMf50)] = {"exhud_coords_mf50", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudCoordsSr40)] = {"exhud_coords_sr40", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudCoordsSr50)] = {"exhud_coords_sr50", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudCoordsFast)] = {"exhud_coords_fast", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudFpsBad)] = {"exhud_fps_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudFpsFine)] = {"exhud_fps_fine", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudHealthBad)] = {"exhud_health_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudHealthWarning)] = {"exhud_health_warning", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudHealthOk)] = {"exhud_health_ok", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudHealthSuper)] = {"exhud_health_super", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudLineClose)] = {"exhud_line_close", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudLineFar)] = {"exhud_line_far", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudLineSpecial)] = {"exhud_line_special", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudLineNormal)] = {"exhud_line_normal", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudMobjAlive)] = {"exhud_mobj_alive", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudMobjDead)] = {"exhud_mobj_dead", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudPlayerDamage)] = {"exhud_player_damage", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudPlayerNeutral)] = {"exhud_player_neutral", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudAmmoLabel)] = {"exhud_ammo_label", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudAmmoMana1)] = {"exhud_ammo_mana1", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudAmmoMana2)] = {"exhud_ammo_mana2", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudAmmoValue)] = {"exhud_ammo_value", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudAmmoBad)] = {"exhud_ammo_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudAmmoWarning)] = {"exhud_ammo_warning", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudAmmoOk)] = {"exhud_ammo_ok", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudAmmoFull)] = {"exhud_ammo_full", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudRenderLabel)] = {"exhud_render_label", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudRenderGood)] = {"exhud_render_good", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudRenderBad)] = {"exhud_render_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudSectorActive)] = {"exhud_sector_active", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudSectorSpecial)] = {"exhud_sector_special", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudSectorNormal)] = {"exhud_sector_normal", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudSpeedLabel)] = {"exhud_speed_label", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudSpeedSlow)] = {"exhud_speed_slow", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudSpeedNormal)] = {"exhud_speed_normal", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudSpeedFast)] = {"exhud_speed_fast", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudTotalsLabel)] = {"exhud_totals_label", ColorRange::Red},
	[std::to_underlying(TextColorIndex::ExhudTotalsValue)] = {"exhud_totals_value", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::ExhudTotalsMax)] = {"exhud_totals_max", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudWeaponLabel)] = {"exhud_weapon_label", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudWeaponOwned)] = {"exhud_weapon_owned", ColorRange::Green},
	[std::to_underlying(TextColorIndex::ExhudWeaponBerserk)] = {"exhud_weapon_berserk", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::ExhudAttempts)] = {"exhud_attempts", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudEventSplit)] = {"exhud_event_split", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudLineActivation)] = {"exhud_line_activation", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudLocalTime)] = {"exhud_local_time", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::ExhudFreeText)] = {"exhud_free_text", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::HudMessage)] = {"hud_message", ColorRange::Default},
	[std::to_underlying(TextColorIndex::HudSecretMessage)] = {"hud_secret_message", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::MapCoords)] = {"map_coords", ColorRange::Green},
	[std::to_underlying(TextColorIndex::MapTimeLevel)] = {"map_time_level", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MapTimeTotal)] = {"map_time_total", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MapTitle)] = {"map_title", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::MapTotalsLabel)] = {"map_totals_label", ColorRange::Red},
	[std::to_underlying(TextColorIndex::MapTotalsValue)] = {"map_totals_value", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MapTotalsMax)] = {"map_totals_max", ColorRange::Lightblue},
	[std::to_underlying(TextColorIndex::InterSplitNormal)] = {"inter_split_normal", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::InterSplitGood)] = {"inter_split_good", ColorRange::Green},
	[std::to_underlying(TextColorIndex::InterSplitBest)] = {"inter_split_best", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::MenuTitle)] = {"menu_title", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::MenuTab)] = {"menu_tab", ColorRange::Tan},
	[std::to_underlying(TextColorIndex::MenuTabHighlight)] = {"menu_tab_highlight", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::MenuLabel)] = {"menu_label", ColorRange::Red},
	[std::to_underlying(TextColorIndex::MenuLabelHighlight)] = {"menu_label_highlight", ColorRange::Brick},
	[std::to_underlying(TextColorIndex::MenuLabelEdit)] = {"menu_label_edit", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MenuValue)] = {"menu_value", ColorRange::Green},
	[std::to_underlying(TextColorIndex::MenuValueHighlight)] = {"menu_value_highlight", ColorRange::Brick},
	[std::to_underlying(TextColorIndex::MenuValueEdit)] = {"menu_value_edit", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MenuInfoHighlight)] = {"menu_info_highlight", ColorRange::Brick},
	[std::to_underlying(TextColorIndex::MenuInfoEdit)] = {"menu_info_edit", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::MenuWarning)] = {"menu_warning", ColorRange::Red},
	[std::to_underlying(TextColorIndex::MenuScrollbar)] = {"menu_scrollbar", ColorRange::Tan},
	[std::to_underlying(TextColorIndex::StbarHealthBad)] = {"stbar_health_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::StbarHealthWarning)] = {"stbar_health_warning", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::StbarHealthOk)] = {"stbar_health_ok", ColorRange::Green},
	[std::to_underlying(TextColorIndex::StbarHealthSuper)] = {"stbar_health_super", ColorRange::Blue},
	[std::to_underlying(TextColorIndex::StbarArmorZero)] = {"stbar_armor_zero", ColorRange::Gray},
	[std::to_underlying(TextColorIndex::StbarArmorOne)] = {"stbar_armor_one", ColorRange::Green},
	[std::to_underlying(TextColorIndex::StbarArmorTwo)] = {"stbar_armor_two", ColorRange::Blue},
	[std::to_underlying(TextColorIndex::StbarAmmoBad)] = {"stbar_ammo_bad", ColorRange::Red},
	[std::to_underlying(TextColorIndex::StbarAmmoWarning)] = {"stbar_ammo_warning", ColorRange::Gold},
	[std::to_underlying(TextColorIndex::StbarAmmoOk)] = {"stbar_ammo_ok", ColorRange::Green},
	[std::to_underlying(TextColorIndex::StbarAmmoFull)] = {"stbar_ammo_full", ColorRange::Blue},
	{nullptr},
};

const char* dsda_TextColor(TextColorIndex i)
{
	return dsda_text_colors[std::to_underlying(i)].color_str;
}

ColorRange dsda_TextCR(TextColorIndex i)
{
	return dsda_text_colors[std::to_underlying(i)].color_range;
}

void dsda_LoadTextColor()
{
	char* lump;
	char** lines;
	const char* line;
	int line_i;
	char key[33] = {0};
	dsda_text_color_t* p;

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

		for(p = dsda_text_colors; p->key; p++)
			if(!strcasecmp(key, p->key))
			{
				p->color_range = static_cast<ColorRange>(color_range_value);
				break;
			}

		if(!p->key)
			I_Error("DSDATC lump has unknown key %s!", key);
	}

	for(p = dsda_text_colors; p->key; p++)
	{
		p->color_str[0] = '\x1b';
		p->color_str[1] = HUlib_Color(p->color_range);
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
