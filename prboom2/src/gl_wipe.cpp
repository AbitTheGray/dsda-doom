// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gl_opengl.hpp"

#include "v_video.hpp"
#include "i_video.hpp"
#include "gl_intern.hpp"
#include "m_random.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"

#include "dsda/gl/render_scale.hpp"

static GLuint wipe_scr_start_tex = 0;
static GLuint wipe_scr_end_tex = 0;

GLuint CaptureScreenAsTexID()
{
	GLuint id;

	gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		gld_GetTexDimension(viewport_rect.w), gld_GetTexDimension(viewport_rect.h),
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewport_rect.x, viewport_rect.y, viewport_rect.w, viewport_rect.h);

	return id;
}

int gld_wipe_doMelt(int ticks, int* y_lookup)
{
	int i, scaled_i, scaled_i2;
	int total_w, total_h;
	float fU1, fU2, fV1, fV2;
	int yoffs;
	float tx, tx2;
	int dx = MAX(1, (SCREENWIDTH > SCREENHEIGHT) ? SCREENHEIGHT / 200 : SCREENWIDTH / 200);

	total_w = gld_GetTexDimension(viewport_rect.w);
	total_h = gld_GetTexDimension(viewport_rect.h);

	fU1 = 0.0f;
	fV1 = (float)viewport_rect.h / (float)total_h;
	fU2 = (float)viewport_rect.w / (float)total_w;
	fV2 = 0.0f;


	glBindTexture(GL_TEXTURE_2D, wipe_scr_end_tex);
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

	glBindTexture(GL_TEXTURE_2D, wipe_scr_start_tex);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_TRIANGLE_STRIP);
	for(i = 0; i < SCREENWIDTH; i += dx)
	{
		int i2 = (i + dx > SCREENWIDTH ? SCREENWIDTH : i + dx);
		yoffs = MAX(0, y_lookup[i]);

		// elim - melt texture is the pixel size of the GL viewport, not the game scene texture size
		scaled_i = MIN(viewport_rect.w, (int)((float)i * gl_scale_x));
		scaled_i2 = MIN(viewport_rect.w, (int)((float)i2 * gl_scale_x));

		// elim - texel coordinates don't necessarily match texture buffer dimensions, since textures
		//        have to be stored in dimensions that are power-of-2
		tx = (float)MIN(fU2, (float)scaled_i / (float)total_w);
		tx2 = (float)MIN(fU2, (float)scaled_i2 / (float)total_w);

		glTexCoord2f(tx, fV1);
		glVertex2f(i, yoffs);
		glTexCoord2f(tx, fV2);
		glVertex2f(i, yoffs + SCREENHEIGHT);
		glTexCoord2f(tx2, fV1);
		glVertex2f(i2, yoffs);
		glTexCoord2f(tx2, fV2);
		glVertex2f(i2, yoffs + SCREENHEIGHT);
	}
	glEnd();

	return 0;
}

int gld_wipe_exitMelt(int ticks)
{
	if(wipe_scr_start_tex != 0)
	{
		glDeleteTextures(1, &wipe_scr_start_tex);
		wipe_scr_start_tex = 0;
	}
	if(wipe_scr_end_tex != 0)
	{
		glDeleteTextures(1, &wipe_scr_end_tex);
		wipe_scr_end_tex = 0;
	}

	gld_ResetLastTexture();

	return 0;
}

int gld_wipe_StartScreen()
{
	wipe_scr_start_tex = CaptureScreenAsTexID();

	return 0;
}

int gld_wipe_EndScreen()
{
	glFlush();
	wipe_scr_end_tex = CaptureScreenAsTexID();

	return 0;
}
