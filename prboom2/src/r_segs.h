// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh module, drawing LineSegs from BSP.
 */

#pragma once

void R_RenderMaskedSegRange(drawseg_t* ds, int x1, int x2);
void R_StoreWallRange(const int start, const int stop);

int R_TopLightLevel(side_t* side, int base_lightlevel);
int R_MidLightLevel(side_t* side, int base_lightlevel);
int R_BottomLightLevel(side_t* side, int base_lightlevel);
void R_AddContrast(seg_t* seg, int* base_lightlevel);

typedef enum
{
	FAKE_CONTRAST_MODE_OFF,
	FAKE_CONTRAST_MODE_ON,
	FAKE_CONTRAST_MODE_SMOOTH
} fake_contrast_mode_t;

extern fake_contrast_mode_t fake_contrast_mode;
