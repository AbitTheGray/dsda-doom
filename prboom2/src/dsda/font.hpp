// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Font

#pragma once

#include "r_defs.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define HU_FONTSTART '!'  /* the first font characters */
#define HU_FONTEND (0x7f) /*jff 2/16/98 '_' the last font characters */
#define HU_FONTSIZE (HU_FONTEND - HU_FONTSTART + 1)

typedef struct
{
	const patchnum_t* font;
	int height;
	int line_height;
	int space_width;
	int start;

	int kerning; // Heretic/Hexen -1 kerning
} dsda_font_t;

extern dsda_font_t hud_font;
extern dsda_font_t exhud_font;

void dsda_InitFont();

#ifdef __cplusplus
}
#endif
