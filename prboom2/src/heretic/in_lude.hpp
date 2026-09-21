// SPDX-License-Identifier: GPL-2.0-or-later

/*
========================
=
= IN_lude.h
=
========================
*/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomdef.hpp"

void IN_Ticker();
void IN_Drawer();
void IN_Start(wbstartstruct_t* wbstartstruct);

#ifdef __cplusplus
}
#endif
