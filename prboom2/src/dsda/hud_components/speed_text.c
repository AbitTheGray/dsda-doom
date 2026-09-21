// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Speed Text HUD Component

#include "base.h"

#include "speed_text.h"

typedef struct {
  dsda_text_t component;
  char label[9];
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int speed;

  speed = dsda_GameSpeed();

  snprintf(
    str,
    max_size,
    "%s%s%d%%",
    local->label,
    speed < 100 ? dsda_TextColor(dsda_tc_exhud_speed_slow)
                : speed == 100 ? dsda_TextColor(dsda_tc_exhud_speed_normal)
                               : dsda_TextColor(dsda_tc_exhud_speed_fast),
    speed
  );
}

void dsda_InitSpeedTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (arg_count < 1 || args[0])
    snprintf(local->label, sizeof(local->label), "%sSPEED ", dsda_TextColor(dsda_tc_exhud_speed_label));
  else
    local->label[0] = '\0';

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSpeedTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawSpeedTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
