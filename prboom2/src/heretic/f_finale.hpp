// SPDX-License-Identifier: GPL-2.0-or-later

// F_finale.h

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_event.hpp"

dboolean Heretic_F_Responder(event_t* event);
void Heretic_F_Drawer();
void Heretic_F_Ticker();
void Heretic_F_StartFinale();

#ifdef __cplusplus
}
#endif
