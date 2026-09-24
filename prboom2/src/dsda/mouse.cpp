// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mouse

#include "SDL.h"

#include "dsda/configuration.hpp"
#include "dsda/features.hpp"

#include "i_video.hpp"
#include "mouse.hpp"

static int quickstart_cache_tics;
static int quickstart_queued;
static signed short angleturn_cache[35];
static unsigned int angleturn_cache_index;

extern "C" void dsda_InitQuickstartCache()
{
	quickstart_cache_tics = dsda_IntConfig(ConfigId::QuickstartCacheTics);
}

void dsda_ApplyQuickstartMouseCache(int* mousex)
{
	int i;

	if(!quickstart_cache_tics) return;

	if(quickstart_queued)
	{
		signed short result = 0;

		quickstart_queued = false;

		dsda_TrackFeature(FeatureFlag::Quickstartcache);

		for(i = 0; i < quickstart_cache_tics; ++i)
			result += angleturn_cache[i];

		*mousex = result;
	}
	else
	{
		angleturn_cache[angleturn_cache_index] = *mousex;
		++angleturn_cache_index;
		if(angleturn_cache_index >= quickstart_cache_tics)
			angleturn_cache_index = 0;
	}
}

void dsda_QueueQuickstart()
{
	quickstart_queued = true;
}

void dsda_GetMousePosition(int* x, int* y)
{
	SDL_GetMouseState(x, y);

	// Lets account for HiDPI displays since SDL_GetMouseState doesnt
	*x = (int)(*x * (float)renderer_rect.w / (float)window_rect.w);
	*y = (int)(*y * (float)renderer_rect.h / (float)window_rect.h);

	// x and y currently have the mouse position in the renderer_rect
	// but we want the mouse position in the viewport_rect
	if(viewport_rect.x < *x && *x < (viewport_rect.w + viewport_rect.x) &&
		viewport_rect.y < *y && *y < (viewport_rect.h + viewport_rect.y))
	{
		*x -= viewport_rect.x;
		*y -= viewport_rect.y;
	}
	else
	{
		*x = 0;
		*y = 0;
	}
}
