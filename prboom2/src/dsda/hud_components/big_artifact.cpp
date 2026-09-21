// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Artifact HUD Component

#include "base.hpp"

#include "big_artifact.hpp"

typedef struct
{
	dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArtifactHC(void* data)
{
	local = (local_component_t*)data;
}

extern "C" void DrawArtifact(int x, int y, int vpt);
void dsda_DrawBigArtifactHC(void* data)
{

	local = (local_component_t*)data;

	DrawArtifact(local->component.x, local->component.y, local->component.vpt);
}
