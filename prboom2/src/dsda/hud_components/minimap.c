// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Minimap HUD Component

#include "base.h"

#include "minimap.h"

typedef struct {
  int x, y, width, height, scale;

  // backups of original values
  // (so we can access them later)
  int xx, yy, ww, hh;
  int yy_offset, flags;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateMinimapCoordinates(void) {

  // Must recalculate y each update
  local->yy = dsda_HudComponentY(local->yy_offset, local->flags, 0);

  // Varables
  local->x      = local->xx;
  local->y      = local->yy;
  local->width  = local->ww;
  local->height = local->hh;

  if (local->x < 0)
    local->x = 0;

  if (local->width <= 0 || local->width > 320)
    local->width = 48;

  if (local->x + local->width > 320)
    local->x = 320 - local->width;

  if (local->y < 0)
    local->y = 0;

  if (local->height <= 0 || local->height > 200)
    local->height = 48;

  if (local->y + local->height > 200)
    local->y = 200 - local->height;

  V_GetWideRect(&local->x, &local->y, &local->width, &local->height, local->flags);
}

void dsda_InitMinimapHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  // Varables
  local->x = x_offset;
  local->width = args[0];
  local->height = args[1];
  local->scale = args[2];

  // Constants
  local->xx = local->x;
  local->ww = local->width;
  local->hh = local->height;

  local->yy_offset = y_offset;
  local->flags = vpt;

  // Init scale
  if (local->scale < 64)
    local->scale = 1024;

  dsda_UpdateMinimapCoordinates();
}

void dsda_UpdateMinimapHC(void* data) {
  local = data;
}

void dsda_DrawMinimapHC(void* data) {
  local = data;

  AM_Drawer(true);
}

void dsda_CopyMinimapCoordinates(int* f_x, int* f_y, int* f_w, int* f_h) {
  if (!local)
    return;

  dsda_UpdateMinimapCoordinates();

  *f_x = local->x;
  *f_y = local->y;
  *f_w = local->width;
  *f_h = local->height;
}

int dsda_MinimapScale(void) {
  if (!local)
    return 1024;

  return local->scale;
}
