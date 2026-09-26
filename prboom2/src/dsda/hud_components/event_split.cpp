// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Event Split HUD Component

#include "cpp/EnumArray.hpp"

#include "base.hpp"

#include "event_split.hpp"

typedef struct
{
	const char* msg;
	int default_delay;
	int delay;
} dsda_split_state_t;

static constinit EnumArray<dsda_split_state_t, EnumCount<SplitClass>> dsda_split_state = {
	{At(SplitClass::BlueKey), {"Blue Key", 0, 0}},
	{At(SplitClass::YellowKey), {"Yellow Key", 0, 0}},
	{At(SplitClass::RedKey), {"Red Key", 0, 0}},
	{At(SplitClass::Use), {"Use", 2, 0}},
	{At(SplitClass::Secret), {"Secret", 0, 0}},
};

typedef struct
{
	dsda_text_t component;
} local_component_t;

static local_component_t* local;

static int ticks;

void dsda_InitEventSplitHC(int x_offset, int y_offset, PatchTranslation vpt, int* args, int arg_count, void** data)
{
	*data = Z_Calloc(1, sizeof(local_component_t));
	local = static_cast<decltype(local)>(*data);

	dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_AddSplit(SplitClass split_class, int lifetime)
{
	int minutes;
	float seconds;
	dsda_split_state_t* split_state;

	if(!local)
		return;

	split_state = &dsda_split_state[split_class];

	if(split_state->delay > 0)
	{
		split_state->delay = split_state->default_delay;
		return;
	}

	split_state->delay = split_state->default_delay;

	ticks = lifetime;

	// To match the timer, we use the leveltime value at the end of the frame
	minutes = (leveltime + 1) / TICRATE / 60;
	seconds = (float)((leveltime + 1) % (60 * TICRATE)) / TICRATE;
	snprintf(
		local->component.msg, sizeof(local->component.msg), "%s%d:%05.2f - %s",
		dsda_TextColor(TextColorIndex::ExhudEventSplit),
		minutes, seconds, split_state->msg
	);

	dsda_RefreshHudText(&local->component);
}

void dsda_UpdateEventSplitHC(void* data)
{
	local = (local_component_t*)data;

	if(ticks > 0)
		--ticks;

	for(dsda_split_state_t& split_state : dsda_split_state)
		if(split_state.delay > 0)
			--split_state.delay;
}

void dsda_DrawEventSplitHC(void* data)
{
	local = (local_component_t*)data;

	if(ticks > 0)
		dsda_DrawBasicText(&local->component);
}
