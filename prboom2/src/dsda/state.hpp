// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA State

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "info.hpp"

typedef struct
{
	state_t* state;
	actionf_t* codeptr;
	byte* defined_codeptr_args;
} dsda_deh_state_t;

dsda_deh_state_t dsda_GetDehState(int index);
void dsda_InitializeStates(state_t* source, int count);
void dsda_FreeDehStates();

#ifdef __cplusplus
}
#endif
