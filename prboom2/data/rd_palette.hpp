// Copyright (c) 1993-2011 PrBoom developers (see AUTHORS)
// Licence: GPLv2 or later (see COPYING)
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// Chained hash lookup to convert rgb triples to palette indices

/// load raw palette, create hash table etc.
void palette_init(const char* filename);

/// lookup colour index of rgb triple
int palette_getindex(const unsigned char* rgb);

#ifdef __cplusplus
}
#endif
