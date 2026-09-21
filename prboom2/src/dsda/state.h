// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA State

#ifndef __DSDA_STATE__
#define __DSDA_STATE__

#include "info.h"

typedef struct {
  state_t* state;
  actionf_t* codeptr;
  byte* defined_codeptr_args;
} dsda_deh_state_t;

dsda_deh_state_t dsda_GetDehState(int index);
void dsda_InitializeStates(state_t* source, int count);
void dsda_FreeDehStates(void);

#endif
