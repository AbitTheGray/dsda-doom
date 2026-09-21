// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include <math.h>

#include "doomstat.hpp"
#include "lprintf.hpp"
#include "v_video.hpp"
#include "r_main.hpp"
#include "gl_intern.hpp"
#include "gl_opengl.hpp"
#include "e6y.hpp"

#include "dsda/configuration.hpp"

dboolean gl_ui_lightmode_indexed = false;
dboolean gl_automap_lightmode_indexed = false;
dboolean gl_menu_lightmode_indexed = false;

static float lighttable[5][256];

/*
 * lookuptable for lightvalues
 * calculated as follow:
 * floatlight=(1.0-exp((light^3)*gamma)) / (1.0-exp(1.0*gamma));
 * gamma=-0,2;-2,0;-4,0;-6,0;-8,0
 * light=0,0 .. 1,0
 */
void gld_InitLightTable()
{
	int i, g;
	float gamma[5] = {-0.2f, -2.0f, -4.0f, -6.0f, -8.0f};

	for(g = 0; g < 5; g++)
	{
		for(i = 0; i < 256; i++)
		{
			lighttable[g][i] = (float)((1.0f - exp(pow(i / 255.0f, 3) * gamma[g])) / (1.0f - exp(1.0f * gamma[g])));
		}
	}
}

float gld_Calc2DLightLevel(int lightlevel)
{
	return lighttable[usegamma][BETWEEN(0, 255, lightlevel)];
}

float gld_CalcLightLevel(int lightlevel)
{
	int light;

	light = BETWEEN(0, 255, lightlevel);

	return (float)light / 255.0f;
}

void gld_StaticLightAlpha(float light, float alpha)
{
	player_t* player = &players[displayplayer];

	glColor4f(1.0f, 1.0f, 1.0f, alpha);

	glsl_SetLightLevel((player->fixedcolormap ? 1.0f : light));
}

// [XA] return amount of light to add from the player's gun flash.
int gld_GetGunFlashLight()
{
	return (extralight << 4);
}
