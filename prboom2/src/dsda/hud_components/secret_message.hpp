// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Secret Message HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitSecretMessageHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateSecretMessageHC(void* data);
void dsda_DrawSecretMessageHC(void* data);

#ifdef __cplusplus
}
#endif
