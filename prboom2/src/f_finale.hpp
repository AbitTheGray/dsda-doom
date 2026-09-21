// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Related to f_finale.c, which is called at the end of a level
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"
#include "d_event.hpp"

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

typedef enum finalestage_e
{
	FINALE_STAGE_TEXT,
	FINALE_STAGE_ART,
	FINALE_STAGE_CAST,
	FINALE_STAGE_TITLE
} finalestage_t;

#ifdef __cplusplus
}
#endif
