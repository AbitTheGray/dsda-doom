// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Preferences

#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"

#include "dsda/args.h"
#include "dsda/messenger.h"
#include "dsda/utility.h"

#include "preferences.h"

typedef struct {
  dboolean opengl;
  dboolean software;
} preferences_t;

static preferences_t map_preferences;
static preferences_t wad_preferences;

void dsda_LoadWadPreferences(void) {
  char* lump;
  char** lines;
  const char* line;
  int line_i;
  int value;
  char key[33] = { 0 };

  lump = W_ReadLumpToString(W_CheckNumForName("DSDAPREF"));

  if (!lump)
    return;

  lines = dsda_SplitString(lump, "\n\r");

  for (line_i = 0; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (!line[0] || line[0] == '/')
      continue;

    value = 1;
    if (!sscanf(line, "%32s %d", key, &value))
      I_Error("DSDAPREF lump has unknown format! (%s)", line);

    if (!strcasecmp(key, "prefer_opengl"))
      wad_preferences.opengl = !!value;
    else if (!strcasecmp(key, "prefer_software"))
      wad_preferences.software = !!value;
    else
      lprintf(LO_WARN, "Unknown DSDAPREF key: %s\n", key);
  }

  Z_Free(lines);
  Z_Free(lump);
}

static void dsda_HandleWadPreferences(void) {
  DO_ONCE
    if (wad_preferences.opengl && V_IsSoftwareMode())
      dsda_AddAlert("This wad may have rendering errors\nin software mode!");

    if (wad_preferences.software && V_IsOpenGLMode())
      dsda_AddAlert("This wad may have rendering errors\nin opengl mode!");
  END_ONCE
}

void dsda_HandleMapPreferences(void) {
  dsda_HandleWadPreferences();

  if (map_preferences.opengl && V_IsSoftwareMode())
    dsda_AddAlert("This level may have rendering errors\nin software mode!");

  if (map_preferences.software && V_IsOpenGLMode())
    dsda_AddAlert("This level may have rendering errors\nin opengl mode!");

  memset(&map_preferences, 0, sizeof(map_preferences));
}

void dsda_PreferOpenGL(void) {
  map_preferences.opengl = true;
}

void dsda_PreferSoftware(void) {
  map_preferences.software = true;
}

