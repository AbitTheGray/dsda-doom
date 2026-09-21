// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Artifact HUD Component

#include "base.h"

#include "big_artifact.h"

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = *data;

	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArtifactHC(void* data)
{
	local = data;
}

void dsda_DrawBigArtifactHC(void* data)
{
	extern void DrawArtifact(int x, int y, int vpt);

	local = data;

	DrawArtifact(local->component.x, local->component.y, local->component.vpt);
}
