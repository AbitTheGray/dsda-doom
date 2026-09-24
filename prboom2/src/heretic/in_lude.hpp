// SPDX-License-Identifier: GPL-2.0-or-later

/*
========================
=
= IN_lude.h
=
========================
*/

#pragma once

#include "doomdef.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void IN_Ticker();
void IN_Drawer();
void IN_Start(wbstartstruct_t* wbstartstruct);

#ifdef __cplusplus
}
#endif
