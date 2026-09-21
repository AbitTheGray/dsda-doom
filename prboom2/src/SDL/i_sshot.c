// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Screenshot functions, moved out of i_video.c
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "SDL.h"

#ifdef HAVE_LIBSDL2_IMAGE
#include <SDL_image.h>
#endif

#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "lprintf.h"

#include "dsda/gl/render_scale.h"

int renderW;
int renderH;

void I_UpdateRenderSize(void)
{
	renderW = renderer_rect.w;
	renderH = renderer_rect.h;
}

//
// I_ScreenShot // Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

int I_ScreenShot(const char* fname)
{
	int result = -1;
	unsigned char* pixels = I_GrabScreen();
	SDL_Surface* screenshot = NULL;

	if(pixels)
	{
		screenshot = SDL_CreateRGBSurfaceFrom(pixels, renderW, renderH, 24,
			renderW * 3, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
	}

	if(screenshot)
	{
#ifdef HAVE_LIBSDL2_IMAGE
		result = IMG_SavePNG(screenshot, fname);
#else
		result = SDL_SaveBMP(screenshot, fname);
#endif
		SDL_FreeSurface(screenshot);
	}
	return result;
}

// NSM
// returns current screen contents as RGB24 (raw)
// returned pointer should be freed when done
//
// Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

unsigned char* I_GrabScreen(void)
{
	static unsigned char* pixels = NULL;
	static int pixels_size = 0;
	int size;

	I_UpdateRenderSize();

	if(V_IsOpenGLMode())
	{
		return gld_ReadScreen();
	}

	size = renderW * renderH * 3;
	if(!pixels || size > pixels_size)
	{
		pixels_size = size;
		pixels = (unsigned char*)Z_Realloc(pixels, size);
	}

	if(pixels && size)
	{
		SDL_Rect screen = {0, 0, renderW, renderH};
		SDL_RenderReadPixels(sdl_renderer, &screen, SDL_PIXELFORMAT_RGB24, pixels, renderW * 3);
	}

	return pixels;
}
