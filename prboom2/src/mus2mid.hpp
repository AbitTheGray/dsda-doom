// SPDX-License-Identifier: GPL-2.0-or-later

// mus2mid.h - Ben Ryves 2006 - http://benryves.com - benryves@benryves.com
// Use to convert a MUS file into a single track, type 0 MIDI file.

// e6y
// All tabs are replaced with spaces.
// Fixed eol style of files.

#pragma once

#include "doomtype.hpp"
#include "memio.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// Structure to hold MUS file header
typedef struct
{
	byte id[4];
	unsigned short scorelength;
	unsigned short scorestart;
	unsigned short primarychannels;
	unsigned short secondarychannels;
	unsigned short instrumentcount;
} musheader;

dboolean mus2mid(MEMFILE* musinput, MEMFILE* midioutput);

#ifdef __cplusplus
}
#endif
