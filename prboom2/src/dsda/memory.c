// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Memory

#include "i_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "lprintf.h"

#include "dsda/time.h"

#include "memory.h"

void dsda_CacheSoundLumps(void) {
  int i;

  for (i = 0; i < num_sfx; ++i) {
    sfxinfo_t *sfx = &S_sfx[i];
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

    if (sfx->lumpnum >= 0)
      W_LockLumpNum(sfx->lumpnum);
  }
}
