// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Related to f_finale.c, which is called at the end of a level
 */

#ifndef __F_FINALE__
#define __F_FINALE__

#include "doomtype.h"
#include "d_event.h"

/* Called by main loop. */
dboolean F_Responder (event_t* ev);

/* Called by main loop. */
void F_Ticker (void);

/* Called by main loop. */
void F_Drawer (void);

dboolean F_ShowCast(void);

void F_StartFinale (void);
void F_StartCast (const char* background, const char* music, dboolean loop_music);
void F_StartScroll (const char* right, const char* left, const char* music, dboolean loop_music);
void F_StartPostFinale (void);

typedef enum finalestage_e
{
    FINALE_STAGE_TEXT,
    FINALE_STAGE_ART,
    FINALE_STAGE_CAST,
    FINALE_STAGE_TITLE
} finalestage_t;

#endif
