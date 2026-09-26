// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh module, drawing LineSegs from BSP.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void R_RenderMaskedSegRange(drawseg_t* ds, int x1, int x2);
void R_StoreWallRange(const int start, const int stop);

int R_TopLightLevel(side_t* side, int base_lightlevel);
int R_MidLightLevel(side_t* side, int base_lightlevel);
int R_BottomLightLevel(side_t* side, int base_lightlevel);
void R_AddContrast(seg_t* seg, int* base_lightlevel);

enum struct FakeContrastMode : int32_t
{
	Off,
	On,
	Smooth
};

extern int fake_contrast_mode; // a FakeContrastMode; int because the config binds it as int*

#ifdef __cplusplus
}
#endif
