// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Render Stats

#ifndef __RENDER_STATS__
#define __RENDER_STATS__

typedef struct {
  int visplanes;
  int drawsegs;
  int vissprites;
} dsda_render_stats_t;

void dsda_BeginRenderStats(void);
void dsda_RecordVisSprite(void);
void dsda_RecordVisSprites(int n);
void dsda_RecordVisPlane(void);
void dsda_RecordVisPlanes(int n);
void dsda_RecordDrawSeg(void);
void dsda_RecordDrawSegs(int n);
void dsda_UpdateRenderStats(void);

#endif
