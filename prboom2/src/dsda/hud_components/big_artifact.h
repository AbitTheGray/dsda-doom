// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Artifact HUD Component

#ifndef __DSDA_HUD_COMPONENT_BIG_ARTIFACT__
#define __DSDA_HUD_COMPONENT_BIG_ARTIFACT__

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigArtifactHC(void* data);
void dsda_DrawBigArtifactHC(void* data);

#endif
