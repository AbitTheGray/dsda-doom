// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Memory

#include "i_sound.hpp"
#include "sounds.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"

#include "dsda/time.hpp"

#include "memory.hpp"

void dsda_CacheSoundLumps()
{
	int i;

	for(i = 0; i < num_sfx; ++i)
	{
		sfxinfo_t* sfx = &S_sfx[i];
		sfx->lumpnum = I_GetSfxLumpNum(sfx);

		if(sfx->lumpnum >= 0)
			W_LockLumpNum(sfx->lumpnum);
	}
}
