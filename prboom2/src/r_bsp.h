// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh module, BSP traversal and handling.
 */

#pragma once

extern seg_t    *curline;
extern side_t   *sidedef;
extern line_t   *linedef;
extern sector_t *frontsector;
extern sector_t *backsector;

/* old code -- killough:
 * extern drawseg_t drawsegs[MAXDRAWSEGS];
 * new code -- killough: */
extern drawseg_t *drawsegs;
extern unsigned maxdrawsegs;

// e6y: resolution limitation is removed
extern byte *solidcol;

extern drawseg_t *ds_p;

void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);
void R_RenderBSPNode(int bspnum);
void R_ForceRenderPolyObjs(void);

/* killough 4/13/98: fake floors/ceilings for deep water / fake ceilings: */
sector_t *R_FakeFlat(sector_t *, sector_t *, int *, int *, dboolean);
