// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//      Load sound lumps with libsndfile.

#pragma once

#include "SDL_audio.h"

void *Load_SNDFile(const void *data, SDL_AudioSpec *sample, void **sampledata,
				   Uint32 *samplelen);
