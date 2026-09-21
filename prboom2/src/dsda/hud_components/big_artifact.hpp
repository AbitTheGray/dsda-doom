// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Artifact HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigArtifactHC(void* data);
void dsda_DrawBigArtifactHC(void* data);

#ifdef __cplusplus
}
#endif
