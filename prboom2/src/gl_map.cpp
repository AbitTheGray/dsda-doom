// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION: Cool automap things
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include <math.h>

#include "SDL.h"
#ifdef HAVE_LIBSDL2_IMAGE
#include "SDL_image.h"
#endif

#include "gl_opengl.hpp"
#include "gl_intern.hpp"
#include "w_wad.hpp"
#include "m_misc.hpp"
#include "am_map.hpp"
#include "lprintf.hpp"
#include "dsda/gl/render_scale.hpp"

am_icon_t am_icons[std::to_underlying(AutomapIcon::Count) + 1] =
{
	{static_cast<GLuint>(-1), "M_SHADOW"},

	{static_cast<GLuint>(-1), "M_ARROW"},
	{static_cast<GLuint>(-1), "M_NORMAL"},
	{static_cast<GLuint>(-1), "M_HEALTH"},
	{static_cast<GLuint>(-1), "M_ARMOUR"},
	{static_cast<GLuint>(-1), "M_AMMO"},
	{static_cast<GLuint>(-1), "M_KEY"},
	{static_cast<GLuint>(-1), "M_POWER"},
	{static_cast<GLuint>(-1), "M_WEAP"},

	{static_cast<GLuint>(-1), "M_ARROW"},
	{static_cast<GLuint>(-1), "M_ARROW"},
	{static_cast<GLuint>(-1), "M_ARROW"},
	{static_cast<GLuint>(-1), "M_MARK"},
	{static_cast<GLuint>(-1), "M_NORMAL"},

	{static_cast<GLuint>(-1), nullptr},
};

typedef struct map_nice_thing_s
{
	vbo_xy_uv_rgba_t v[4];
}
	PACKEDATTR map_nice_thing_t;

static array_t map_things[std::to_underlying(AutomapIcon::Count)];

void gld_InitMapPics()
{
	int i, lump;

	i = 0;
	while(am_icons[i].name)
	{
		lump = W_CheckNumForName2(am_icons[i].name, LumpNamespace::Prboom);
		am_icons[i].lumpnum = lump;
		if(lump != LUMP_NOT_FOUND)
		{
			SDL_Surface* surf = nullptr;
#ifdef HAVE_LIBSDL2_IMAGE
			SDL_Surface* surf_raw;

			surf_raw = IMG_Load_RW(SDL_RWFromConstMem(W_LumpByNum(lump), W_LumpLength(lump)), true);

			surf = SDL_ConvertSurface(surf_raw, &RGBAFormat, 0);
			SDL_FreeSurface(surf_raw);
#endif

			if(surf)
			{
				glGenTextures(1, &am_icons[i].tex_id);
				glBindTexture(GL_TEXTURE_2D, am_icons[i].tex_id);

				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				SDL_FreeSurface(surf);
			}
		}

		i++;
	}
}

void gld_AddNiceThing(AutomapIcon type, float x, float y, float radius, float angle,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	map_nice_thing_t* thing = static_cast<map_nice_thing_t*>(M_ArrayGetNewItem(&map_things[std::to_underlying(type)], sizeof(thing[0])));

	float sina_r = (float)sin(angle) * radius;
	float cosa_r = (float)cos(angle) * radius;

#define MAP_NICE_THING_INIT(index, _x, _y, _u, _v) \
  { \
    thing->v[index].x = _x; \
    thing->v[index].y = _y; \
    thing->v[index].u = _u; \
    thing->v[index].v = _v; \
    thing->v[index].r = r; \
    thing->v[index].g = g; \
    thing->v[index].b = b; \
    thing->v[index].a = a; \
  }
	MAP_NICE_THING_INIT(0, x + sina_r + cosa_r, y - cosa_r + sina_r, 1.0f, 0.0f);
	MAP_NICE_THING_INIT(1, x + sina_r - cosa_r, y - cosa_r - sina_r, 0.0f, 0.0f);
	MAP_NICE_THING_INIT(2, x - sina_r - cosa_r, y + cosa_r - sina_r, 0.0f, 1.0f);
	MAP_NICE_THING_INIT(3, x - sina_r + cosa_r, y + cosa_r + sina_r, 1.0f, 1.0f);

#undef MAP_NICE_THING_INIT
}

void gld_DrawNiceThings(int fx, int fy, int fw, int fh)
{
	int i;

	dsda_GLSetScreenSpaceScissor(fx, fy, fw, fh);
	glEnable(GL_SCISSOR_TEST);

	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

	// activate vertex array, texture coord array and color arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	for(i = 0; i < std::to_underlying(AutomapIcon::Count); i++)
	{
		array_t* things = &map_things[i];

		if(things->count == 0)
			continue;

		glBindTexture(GL_TEXTURE_2D, am_icons[i].tex_id);

		{
			map_nice_thing_t* thing = &((map_nice_thing_t*)things->data)[0];

			// activate and specify pointers to arrays
			glVertexPointer(2, GL_FLOAT, sizeof(thing->v[0]), &thing->v[0].x);
			glTexCoordPointer(2, GL_FLOAT, sizeof(thing->v[0]), &thing->v[0].u);
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(thing->v[0]), &thing->v[0].r);

			glDrawArrays(GL_QUADS, 0, things->count * 4);
		}
	}

	// deactivate vertex array, texture coord array and color arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	gld_ResetLastTexture();
	glDisable(GL_SCISSOR_TEST);
}

void gld_ClearNiceThings()
{
	int type;

	for(type = 0; type < std::to_underlying(AutomapIcon::Count); type++)
	{
		M_ArrayClear(&map_things[type]);
	}
}

void gld_DrawMapLines()
{
	if(map_lines.count > 0)
	{
		map_point_t* point = (map_point_t*)map_lines.data;

		gld_EnableTexture2D(GL_TEXTURE0_ARB, false);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(point[0]), &point->x);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(point[0]), &point->r);

		glDrawArrays(GL_LINES, 0, map_lines.count * 2);

		gld_EnableTexture2D(GL_TEXTURE0_ARB, true);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
}
