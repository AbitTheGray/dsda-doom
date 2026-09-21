// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//      Load sound lumps with libsndfile.

#ifndef I_SNDFILE_H
#define I_SNDFILE_H

#include "SDL_audio.h"

void *Load_SNDFile(const void *data, SDL_AudioSpec *sample, void **sampledata,
				   Uint32 *samplelen);

#endif
