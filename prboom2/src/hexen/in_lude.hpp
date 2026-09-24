// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "doomdef.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void Hexen_IN_Ticker();
void Hexen_IN_Drawer();
void Hexen_IN_Start(wbstartstruct_t* wbstartstruct);

#ifdef __cplusplus
}
#endif
