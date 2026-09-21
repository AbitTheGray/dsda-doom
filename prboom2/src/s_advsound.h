// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Support MUSINFO lump (dynamic music changing)
 */

#ifndef __S_ADVSOUND__
#define __S_ADVSOUND__

#include "p_mobj.h"
#include "sounds.h"

//
//MUSINFO lump
//

#define MAX_MUS_ENTRIES 65

typedef struct musinfo_s
{
  mobj_t *mapthing;
  mobj_t *lastmapthing;
  int tics;
  int current_item;
  int items[MAX_MUS_ENTRIES];
} musinfo_t;

extern musicinfo_t *mus_playing;
extern musinfo_t musinfo;

void S_ParseMusInfo(const char *mapid);
void MusInfoThinker(mobj_t *thing);
void T_MAPMusic(void);

#endif
