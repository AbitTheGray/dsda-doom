// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Render Stats

#include "dsda/time.hpp"
#include "dsda/utility.hpp"

#include "render_stats.hpp"
#include "doomdef.hpp"

static dsda_render_stats_t frame_stats;
static dsda_render_stats_t interval_stats;
static int frame_count;

dsda_render_stats_t dsda_render_stats;
dsda_render_stats_t dsda_render_stats_max;
int dsda_render_stats_fps = TICRATE;

static void dsda_UpdateMaxValues(dsda_render_stats_t* x, dsda_render_stats_t* y)
{
	if(x->visplanes < y->visplanes)
		x->visplanes = y->visplanes;

	if(x->drawsegs < y->drawsegs)
		x->drawsegs = y->drawsegs;

	if(x->vissprites < y->vissprites)
		x->vissprites = y->vissprites;
}

void dsda_BeginRenderStats()
{
	ZERO_DATA(frame_stats);
	ZERO_DATA(interval_stats);
	ZERO_DATA(dsda_render_stats);
	ZERO_DATA(dsda_render_stats_max);

	dsda_StartTimer(dsda_timer_render_stats);
}

void dsda_RecordVisSprite()
{
	++frame_stats.vissprites;
}

void dsda_RecordVisSprites(int n)
{
	frame_stats.vissprites += n;
}

void dsda_RecordVisPlane()
{
	++frame_stats.visplanes;
}

void dsda_RecordVisPlanes(int n)
{
	frame_stats.visplanes += n;
}

void dsda_RecordDrawSeg()
{
	++frame_stats.drawsegs;
}

void dsda_RecordDrawSegs(int n)
{
	frame_stats.drawsegs += n;
}

void dsda_UpdateRenderStats()
{
	dsda_UpdateMaxValues(&interval_stats, &frame_stats);

	++frame_count;
	ZERO_DATA(frame_stats);

	if(dsda_ElapsedTimeMS(dsda_timer_render_stats) >= 1000)
	{
		dsda_render_stats = interval_stats;
		ZERO_DATA(interval_stats);
		dsda_UpdateMaxValues(&dsda_render_stats_max, &dsda_render_stats);
		dsda_render_stats_fps = frame_count * 1000 / dsda_ElapsedTimeMS(dsda_timer_render_stats);
		frame_count = 0;
		dsda_StartTimer(dsda_timer_render_stats);
	}
}
