// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Render Stats

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	int visplanes;
	int drawsegs;
	int vissprites;
} dsda_render_stats_t;

void dsda_BeginRenderStats();
void dsda_RecordVisSprite();
void dsda_RecordVisSprites(int n);
void dsda_RecordVisPlane();
void dsda_RecordVisPlanes(int n);
void dsda_RecordDrawSeg();
void dsda_RecordDrawSegs(int n);
void dsda_UpdateRenderStats();

#ifdef __cplusplus
}
#endif
