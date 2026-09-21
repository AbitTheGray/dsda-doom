// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Font

#include "r_data.h"

#include "font.h"

static patchnum_t hu_font[HU_FONTSIZE];
static patchnum_t hu_font2[HU_FONTSIZE];

dsda_font_t hud_font;
dsda_font_t exhud_font;

void dsda_InitFont(void)
{
	int i;
	int j;
	char buffer[9];

	j = HU_FONTSTART;
	for(i = 0; i < HU_FONTSIZE - 1; i++, j++)
	{
		snprintf(buffer, sizeof(buffer), "DIG%.3d", j);
		R_SetPatchNum(&hu_font2[i], buffer);
	}

	j = HU_FONTSTART;
	for(i = 0; i < HU_FONTSIZE; ++i, ++j)
	{
		if('0' <= j && j <= '9')
		{
			if(raven)
				snprintf(buffer, sizeof(buffer), "FONTA%.2d", j - 32);
			else
				snprintf(buffer, sizeof(buffer), "STCFN%.3d", j);
			R_SetPatchNum(&hu_font[i], buffer);
		}
		else if('A' <= j && j <= 'Z')
		{
			if(raven)
				snprintf(buffer, sizeof(buffer), "FONTA%.2d", j - 32);
			else
				snprintf(buffer, sizeof(buffer), "STCFN%.3d", j);
			R_SetPatchNum(&hu_font[i], buffer);
		}
		else if(!raven && j < 97)
		{
			snprintf(buffer, sizeof(buffer), "STCFN%.3d", j);
			R_SetPatchNum(&hu_font[i], buffer);
			//jff 2/23/98 make all font chars defined, useful or not
		}
		else if(raven && j < 91)
		{
			snprintf(buffer, sizeof(buffer), "FONTA%.2d", j - 32);
			R_SetPatchNum(&hu_font[i], buffer);
			//jff 2/23/98 make all font chars defined, useful or not
		}
		else
		{
			hu_font[i] = hu_font2[i]; //jff 2/16/98 account for gap
		}
	}

	hud_font.font = hu_font;
	hud_font.height = hu_font['0' - HU_FONTSTART].height;
	hud_font.line_height = hud_font.height + 1;
	hud_font.space_width = raven ? 5 : 4;
	hud_font.start = HU_FONTSTART;
	hud_font.kerning = raven ? -1 : 0;

	exhud_font.font = hu_font2;
	exhud_font.height = hu_font2['0' - HU_FONTSTART].height;
	exhud_font.line_height = exhud_font.height + 1;
	exhud_font.space_width = 5;
	exhud_font.start = HU_FONTSTART;
	exhud_font.kerning = 0;
}
