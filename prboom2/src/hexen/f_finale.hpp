// SPDX-License-Identifier: GPL-2.0-or-later

// F_finale.h

#pragma once

#include "d_event.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

dboolean Hexen_F_Responder(event_t* event);
void Hexen_F_Drawer();
void Hexen_F_Ticker();
void Hexen_F_StartFinale();

#ifdef __cplusplus
}
#endif
