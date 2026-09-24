// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Related to f_finale.c, which is called at the end of a level
 */

#pragma once

#include "doomtype.hpp"
#include "d_event.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

/* Called by main loop. */
dboolean F_Responder(event_t* ev);

/* Called by main loop. */
void F_Ticker();

/* Called by main loop. */
void F_Drawer();

dboolean F_ShowCast();

void F_StartFinale();
void F_StartCast(const char* background, const char* music, dboolean loop_music);
void F_StartScroll(const char* right, const char* left, const char* music, dboolean loop_music);
void F_StartPostFinale();

enum struct FinaleScreen : int32_t
{
	Text,
	Art,
	Cast,
	Title
};

#ifdef __cplusplus
}
#endif
