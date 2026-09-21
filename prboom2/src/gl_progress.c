// SPDX-License-Identifier: GPL-2.0-or-later

#include "gl_opengl.h"

#include <SDL.h>

#include "gl_intern.h"
#include "i_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"

#include "dsda/font.h"
#include "dsda/utility.h"

static GLuint progress_texid = 0;
static unsigned int lastupdate = 0;

int gld_ProgressStart(void)
{
	if(!progress_texid)
	{
		progress_texid = CaptureScreenAsTexID();
		lastupdate = SDL_GetTicks() - 100;
		return true;
	}

	return false;
}

int gld_ProgressRestoreScreen(void)
{
	int total_w, total_h;
	float fU1, fU2, fV1, fV2;

	if(progress_texid)
	{
		total_w = gld_GetTexDimension(SCREENWIDTH);
		total_h = gld_GetTexDimension(SCREENHEIGHT);

		fU1 = 0.0f;
		fV1 = (float)SCREENHEIGHT / (float)total_h;
		fU2 = (float)SCREENWIDTH / (float)total_w;
		fV2 = 0.0f;

		gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

		glBindTexture(GL_TEXTURE_2D, progress_texid);
		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(fU1, fV1);
			glVertex2f(0.0f, 0.0f);
			glTexCoord2f(fU1, fV2);
			glVertex2f(0.0f, (float)SCREENHEIGHT);
			glTexCoord2f(fU2, fV1);
			glVertex2f((float)SCREENWIDTH, 0.0f);
			glTexCoord2f(fU2, fV2);
			glVertex2f((float)SCREENWIDTH, (float)SCREENHEIGHT);
		}
		glEnd();

		return true;
	}

	return false;
}

int gld_ProgressEnd(void)
{
	if(progress_texid != 0)
	{
		gld_ProgressRestoreScreen();
		I_FinishUpdate();
		gld_ProgressRestoreScreen();
		glDeleteTextures(1, &progress_texid);
		progress_texid = 0;
		return true;
	}

	return false;
}

static hu_textline_t w_precache;

static void gld_InitProgressUpdate(void)
{
	HUlib_initTextLine
	(
		&w_precache,
		16,
		186,
		&hud_font,
		CR_DEFAULT,
		VPT_ALIGN_LEFT_BOTTOM
	);
}

void gld_ProgressUpdate(const char* text, int progress, int total)
{
	int len;
	static char last_text[32] = {0};
	unsigned int tic;

	if(!progress_texid)
		return;

	// do not do it often
	tic = SDL_GetTicks();
	if(tic - lastupdate < 100)
		return;
	lastupdate = tic;

	DO_ONCE
		gld_InitProgressUpdate();
	END_ONCE

	if(text && *text && strcmp(last_text, text))
	{
		const char* s;
		strcpy(last_text, text);

		HUlib_clearTextLine(&w_precache);
		s = text;
		while(*s)
			HUlib_addCharToTextLine(&w_precache, *(s++));
		HUlib_setTextXCenter(&w_precache);
	}

	gld_ProgressRestoreScreen();
	HUlib_drawTextLine(&w_precache, false);

	len = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * progress / total));
	V_FillRect(0, 0, SCREENHEIGHT - 4, len - 0, 4, 4);
	if(len > 4)
	{
		V_FillRect(0, 2, SCREENHEIGHT - 3, len - 4, 2, 31);
	}

	I_FinishUpdate();
}
