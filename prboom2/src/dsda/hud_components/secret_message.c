// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Secret Message HUD Component

#include "base.h"

#include "secret_message.h"

typedef struct {
  dsda_text_t component;
  dboolean center;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  char* HU_SecretMessage(void);

  char* message;

  message = HU_SecretMessage();

  if (message)
    snprintf(
      str,
      max_size,
      "%s%s",
      dsda_TextColor(dsda_tc_hud_secret_message),
      message
    );
  else
    str[0] = '\0';
}

void dsda_InitSecretMessageHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : true;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSecretMessageHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  if (local->center)
    HUlib_setTextXCenter(&local->component.text);
}

void dsda_DrawSecretMessageHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
