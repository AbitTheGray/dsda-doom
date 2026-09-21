// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Attempts HUD Component

#include "dsda/split_tracker.h"

#include "base.h"

#include "attempts.h"

typedef struct
{
	dsda_text_t component;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size)
{
	if(!demorecording)
		return;

	snprintf(
		str,
		max_size,
		"%s%d/%d",
		dsda_TextColor(dsda_tc_exhud_attempts),
		dsda_SessionAttempts(),
		dsda_DemoAttempts()
	);
}

void dsda_InitAttemptsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = *data;

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateAttemptsHC(void* data)
{
	local = data;

	dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
	dsda_RefreshHudText(&local->component);
}

void dsda_DrawAttemptsHC(void* data)
{
	local = data;

	dsda_DrawBasicText(&local->component);
}
