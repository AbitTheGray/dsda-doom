// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Health Text HUD Component

#include "base.h"

#include "health_text.h"

typedef struct {
  dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  snprintf(
    str,
    max_size,
    "%sHEL %3d%%",
    player->health <= hud_health_red ? dsda_TextColor(dsda_tc_exhud_health_bad) :
      player->health <= hud_health_yellow ? dsda_TextColor(dsda_tc_exhud_health_warning) :
      player->health <= hud_health_green ? dsda_TextColor(dsda_tc_exhud_health_ok) :
      dsda_TextColor(dsda_tc_exhud_health_super),
    player->health
  );
}

void dsda_InitHealthTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateHealthTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawHealthTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
