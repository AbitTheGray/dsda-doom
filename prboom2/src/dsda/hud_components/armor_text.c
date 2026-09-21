// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Armor Text HUD Component

#include "base.h"

#include "armor_text.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  if (hexen) {
    fixed_t armor_percent;

    armor_percent = dsda_HexenArmor(player);

    snprintf(
      str,
      max_size,
      "%sA.C. %2d",
      armor_percent == 0 ? dsda_TextColor(dsda_tc_exhud_armor_zero) :
        armor_percent <= 50 ? dsda_TextColor(dsda_tc_exhud_armor_one) :
        dsda_TextColor(dsda_tc_exhud_armor_two),
      armor_percent
    );
  }
  else {
    snprintf(
      str,
      max_size,
      "%sARM %3d%%",
      player->armorpoints[ARMOR_ARMOR] <= 0 ? dsda_TextColor(dsda_tc_exhud_armor_zero) :
        player->armortype == 1 ? dsda_TextColor(dsda_tc_exhud_armor_one) :
        dsda_TextColor(dsda_tc_exhud_armor_two),
      player->armorpoints[ARMOR_ARMOR]
    );
  }
}

void dsda_InitArmorTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateArmorTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawArmorTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
