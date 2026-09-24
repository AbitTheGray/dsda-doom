// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Stretch

#include <utility>

#include "am_map.hpp"
#include "doomdef.hpp"
#include "doomtype.hpp"
#include "hu_stuff.hpp"
#include "r_main.hpp"
#include "st_stuff.hpp"
#include "v_video.hpp"

#include "dsda/configuration.hpp"
#include "dsda/font.hpp"

#include "stretch.hpp"

int wide_offsetx;
int wide_offset2x;
int wide_offsety;
int wide_offset2y;

int render_stretch_hud;

int patches_scalex;
int patches_scaley;

static cb_video_t video;
static cb_video_t video_stretch;
static cb_video_t video_full;
static cb_video_t video_ex_text;

static stretch_param_t* stretch_params;
static stretch_param_t stretch_params_table[std::to_underlying(PatchStretch::Max)][std::to_underlying(PatchTranslation::AlignMax)];

static int ex_text_screenwidth;
static int ex_text_screenheight;
static int ex_text_st_scaled_height;
static double ex_text_scale_x;
static double ex_text_scale_y;


static void GenLookup(short* lookup1, short* lookup2, int size, int max, int step)
{
	int i;
	fixed_t frac, lastfrac;

	memset(lookup1, 0, max * sizeof(lookup1[0]));
	memset(lookup2, 0, max * sizeof(lookup2[0]));

	lastfrac = frac = 0;

	// lookup1[0] = 0;
	// for (i = 1; i < max; ++i) {
	//   lookup1[i] = (float) i * size / max;
	//   lookup2[i - 1] = lookup1[i] - 1;
	// }

	// lookup2[max - 1] = size - 1;
	// lookup1[max] = lookup2[max] = size;

	for(i = 0; i < size; i++)
	{
		if(frac >> FRACBITS > lastfrac >> FRACBITS)
		{
			lookup1[frac >> FRACBITS] = i;
			lookup2[lastfrac >> FRACBITS] = i - 1;

			lastfrac = frac;
		}

		frac += step;
	}

	lookup2[max - 1] = size - 1;
	lookup1[max] = lookup2[max] = size;

	for(i = 1; i < max; i++)
	{
		if(lookup1[i] == 0 && lookup1[i - 1] != 0)
			lookup1[i] = lookup1[i - 1];

		if(lookup2[i] == 0 && lookup2[i - 1] != 0)
			lookup2[i] = lookup2[i - 1];
	}
}

static void EvaluateExTextScale()
{
	ex_text_scale_x = dsda_IntConfig(ConfigId::ExTextScaleX) / 100.0;
	ex_text_scale_y = dsda_IntConfig(ConfigId::ExTextRatioY) / 100.0;

	if(!ex_text_scale_x)
		ex_text_scale_x = (double)WIDE_SCREENWIDTH / 320;

	if(!ex_text_scale_y)
		ex_text_scale_y = 1.0;

	ex_text_scale_y *= ex_text_scale_x;

	ex_text_screenwidth = 320 * ex_text_scale_x;
	ex_text_screenheight = 200 * ex_text_scale_y;
	ex_text_st_scaled_height = g_st_height * ex_text_scale_y;
}

stretch_param_t* dsda_StretchParams(PatchTranslation flags)
{
	if((flags & PatchTranslation::ExText) != PatchTranslation{})
		return &stretch_params_table[std::to_underlying(PatchStretch::ExText)][std::to_underlying(PatchAlignment(flags))];

	return &stretch_params[std::to_underlying(PatchAlignment(flags))];
}

static void InitExTextParam(stretch_param_t* offsets, PatchTranslation flags)
{
	int offset2x, offset2y;

	offset2x = SCREENWIDTH - ex_text_screenwidth;
	offset2y = (SCREENHEIGHT - ex_text_screenheight) -
		R_PartialView() * (ST_SCALED_HEIGHT - ex_text_st_scaled_height);

	memset(offsets, 0, sizeof(*offsets));

	offsets->video = &video_ex_text;

	if(flags == PatchTranslation::AlignLeft || flags == PatchTranslation::AlignLeftBottom || flags == PatchTranslation::AlignLeftTop)
	{
		offsets->deltax1 = 0;
		offsets->deltax2 = 0;
	}

	if(flags == PatchTranslation::AlignRight || flags == PatchTranslation::AlignRightBottom || flags == PatchTranslation::AlignRightTop)
	{
		offsets->deltax1 = offset2x;
		offsets->deltax2 = offset2x;
	}

	if(flags == PatchTranslation::AlignBottom || flags == PatchTranslation::AlignLeftBottom || flags == PatchTranslation::AlignRightBottom)
		offsets->deltay1 = offset2y;
}

extern "C" void dsda_UpdateExTextOffset(PatchTranslation flags, int offset)
{
	stretch_params_table[std::to_underlying(PatchStretch::ExText)][std::to_underlying(flags)].deltay1 +=
		(ST_SCALED_HEIGHT - ex_text_st_scaled_height) * offset * hud_font.line_height / g_st_height +
		(hud_font.line_height - exhud_font.line_height) * ex_text_scale_y * (offset > 0 ? 1 : -1);
}

extern "C" void dsda_ResetExTextOffsets()
{
	int k;

	for(k = 0; k < std::to_underlying(PatchTranslation::AlignMax); k++)
		InitExTextParam(&stretch_params_table[std::to_underlying(PatchStretch::ExText)][k], (PatchTranslation)k);
}

static void InitStretchParam(stretch_param_t* offsets, PatchStretch stretch, PatchTranslation flags)
{
	memset(offsets, 0, sizeof(*offsets));

	switch(stretch_hud(stretch))
	{
		case PatchStretch::NotAdjusted:
			if(flags == PatchTranslation::AlignWide)
			{
				offsets->video = &video_stretch;
				offsets->deltax1 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
				offsets->deltax2 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
			}
			else
			{
				offsets->video = &video;
				offsets->deltax1 = wide_offsetx;
				offsets->deltax2 = wide_offsetx;
			}
			break;
		case PatchStretch::DoomFormat:
			offsets->video = &video_stretch;
			offsets->deltax1 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
			offsets->deltax2 = (SCREENWIDTH - WIDE_SCREENWIDTH) / 2;
			break;
		case PatchStretch::FitToWidth:
			offsets->video = &video_full;
			offsets->deltax1 = 0;
			offsets->deltax2 = 0;
			break;
	}

	if(flags == PatchTranslation::AlignLeft || flags == PatchTranslation::AlignLeftBottom || flags == PatchTranslation::AlignLeftTop)
	{
		offsets->deltax1 = 0;
		offsets->deltax2 = 0;
	}

	if(flags == PatchTranslation::AlignRight || flags == PatchTranslation::AlignRightBottom || flags == PatchTranslation::AlignRightTop)
	{
		offsets->deltax1 *= 2;
		offsets->deltax2 *= 2;
	}

	offsets->deltay1 = wide_offsety;

	if(flags == PatchTranslation::AlignBottom || flags == PatchTranslation::AlignLeftBottom || flags == PatchTranslation::AlignRightBottom)
		offsets->deltay1 = wide_offset2y;

	if(flags == PatchTranslation::AlignTop || flags == PatchTranslation::AlignLeftTop || flags == PatchTranslation::AlignRightTop)
		offsets->deltay1 = 0;

	if(flags == PatchTranslation::AlignWide && !tallscreen)
		offsets->deltay1 = 0;
}

void dsda_SetupStretchParams()
{
	int i, k;

	EvaluateExTextScale();

	for(k = 0; k < std::to_underlying(PatchTranslation::AlignMax); k++)
		for(i = 0; i < std::to_underlying(PatchStretch::MaxConfig); i++)
			InitStretchParam(&stretch_params_table[i][k], static_cast<PatchStretch>(i), (PatchTranslation)k);

	dsda_ResetExTextOffsets();

	stretch_params = stretch_params_table[render_stretch_hud];

	video.xstep = ((320 << FRACBITS) / 320 / patches_scalex) + 1;
	video.ystep = ((200 << FRACBITS) / 200 / patches_scaley) + 1;
	video_stretch.xstep = ((320 << FRACBITS) / WIDE_SCREENWIDTH) + 1;
	video_stretch.ystep = ((200 << FRACBITS) / WIDE_SCREENHEIGHT) + 1;
	video_full.xstep = ((320 << FRACBITS) / SCREENWIDTH) + 1;
	video_full.ystep = ((200 << FRACBITS) / SCREENHEIGHT) + 1;
	video_ex_text.xstep = ((320 << FRACBITS) / ex_text_screenwidth) + 1;
	video_ex_text.ystep = ((200 << FRACBITS) / ex_text_screenheight) + 1;

	video.width = 320 * patches_scalex;
	video.height = 200 * patches_scaley;
	GenLookup(video.x1lookup, video.x2lookup, video.width, 320, video.xstep);
	GenLookup(video.y1lookup, video.y2lookup, video.height, 200, video.ystep);

	video_stretch.width = WIDE_SCREENWIDTH;
	video_stretch.height = WIDE_SCREENHEIGHT;
	GenLookup(video_stretch.x1lookup, video_stretch.x2lookup, video_stretch.width, 320, video_stretch.xstep);
	GenLookup(video_stretch.y1lookup, video_stretch.y2lookup, video_stretch.height, 200, video_stretch.ystep);

	video_full.width = SCREENWIDTH;
	video_full.height = SCREENHEIGHT;
	GenLookup(video_full.x1lookup, video_full.x2lookup, video_full.width, 320, video_full.xstep);
	GenLookup(video_full.y1lookup, video_full.y2lookup, video_full.height, 200, video_full.ystep);

	video_ex_text.width = ex_text_screenwidth;
	video_ex_text.height = ex_text_screenheight;
	GenLookup(video_ex_text.x1lookup, video_ex_text.x2lookup, video_ex_text.width, 320, video_ex_text.xstep);
	GenLookup(video_ex_text.y1lookup, video_ex_text.y2lookup, video_ex_text.height, 200, video_ex_text.ystep);
}

void dsda_UpdateStretchParams()
{
	dsda_SetupStretchParams();
	AM_RefreshMinimap();
}

void dsda_EvaluatePatchScale()
{
	int render_patches_scalex;
	int render_patches_scaley;

	render_patches_scalex = dsda_IntConfig(ConfigId::RenderPatchesScalex);
	render_patches_scaley = dsda_IntConfig(ConfigId::RenderPatchesScaley);

	patches_scalex = MIN(SCREENWIDTH / 320, SCREENHEIGHT / 200);
	patches_scalex = MAX(1, patches_scalex);
	patches_scaley = patches_scalex;

	if(render_patches_scalex > 0)
		patches_scalex = MIN(render_patches_scalex, patches_scalex);

	if(render_patches_scaley > 0)
		patches_scaley = MIN(render_patches_scaley, patches_scaley);

	ST_SCALED_HEIGHT = g_st_height * patches_scaley;

	if(SCREENWIDTH < 320 || WIDE_SCREENWIDTH < 320 ||
		SCREENHEIGHT < 200 || WIDE_SCREENHEIGHT < 200)
		render_stretch_hud = std::to_underlying(PatchStretch::FitToWidth);

	switch(stretch_hud(static_cast<PatchStretch>(render_stretch_hud)))
	{
		case PatchStretch::NotAdjusted:
			wide_offset2x = SCREENWIDTH - patches_scalex * 320;
			wide_offset2y = SCREENHEIGHT - patches_scaley * 200;
			break;
		case PatchStretch::DoomFormat:
			ST_SCALED_HEIGHT = g_st_height * WIDE_SCREENHEIGHT / 200;

			wide_offset2x = SCREENWIDTH - WIDE_SCREENWIDTH;
			wide_offset2y = SCREENHEIGHT - WIDE_SCREENHEIGHT;
			break;
		case PatchStretch::FitToWidth:
			ST_SCALED_HEIGHT = g_st_height * SCREENHEIGHT / 200;

			wide_offset2x = 0;
			wide_offset2y = 0;
			break;
	}

	wide_offsetx = wide_offset2x / 2;
	wide_offsety = wide_offset2y / 2;
}
