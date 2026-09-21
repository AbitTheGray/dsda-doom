// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh, visplane stuff (floor, ceilings).
 */

#pragma once

#include "r_data.h"

#define PL_SKYFLAT_LINE (0x80000000)
#define PL_SKYFLAT_SECTOR (0x40000000)
#define PL_SKYFLAT (PL_SKYFLAT_LINE|PL_SKYFLAT_SECTOR)

/* Visplane related. */
extern int *lastopening; // dropoff overflow

// e6y: resolution limitation is removed
extern int *floorclip, *ceilingclip; // dropoff overflow
extern fixed_t *yslope, *distscale;

void R_InitVisplanesRes(void);
void R_InitPlanesRes(void);
void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_DrawPlanes (void);

void dsda_RefreshLinearSky (void);

const rpatch_t *R_HackedSkyPatch(texture_t *texture);

visplane_t *R_FindPlane(
                        fixed_t height,
                        int picnum,
                        int lightlevel,
                        int special,
                        fixed_t xoffs,  /* killough 2/28/98: add x-y offsets */
                        fixed_t yoffs,
                        angle_t rotation,
                        fixed_t xscale,
                        fixed_t yscale
                       );

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop);
