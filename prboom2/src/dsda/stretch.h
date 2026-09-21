// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Stretch

#pragma once

typedef struct
{
	fixed_t xstep;
	fixed_t ystep;
	int width, height;
	short x1lookup[321];
	short y1lookup[201];
	short x2lookup[321];
	short y2lookup[201];
} cb_video_t;

typedef struct stretch_param_s
{
	cb_video_t* video;
	int deltax1;
	int deltay1;
	int deltax2;
	int deltay2;
} stretch_param_t;

typedef enum
{
	patch_stretch_not_adjusted,
	patch_stretch_doom_format,
	patch_stretch_fit_to_width,

	patch_stretch_max_config,

	patch_stretch_ex_text = patch_stretch_max_config,

	patch_stretch_max
} patch_stretch_t;

// Raven HUD breaks in "patch_stretch_not_adjusted",
// so let's use "patch_stretch_doom_format" settings
#define stretch_hud(a) ((raven && (a) == patch_stretch_not_adjusted) ? patch_stretch_doom_format : (a))

extern int wide_offsetx;
extern int wide_offset2x;
extern int wide_offsety;
extern int wide_offset2y;

extern int render_stretch_hud;

extern int patches_scalex;
extern int patches_scaley;

stretch_param_t* dsda_StretchParams(int flags);
void dsda_SetupStretchParams(void);
void dsda_EvaluatePatchScale(void);
void dsda_UpdateStretchParams(void);
