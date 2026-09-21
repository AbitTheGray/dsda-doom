// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   DOOM strings, by language.
 *   Note:  In BOOM, some new strings hav ebeen defined that are
 *          not found in the French version.  A better approach is
 *          to create a BEX text-replacement file for other
 *          languages since any language can be supported that way
 *          without recompiling the program.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* All important printed strings.
 * Language selection (message strings).
 * Use -DFRENCH etc.
 */

#ifdef FRENCH
#include "d_french.h"
#else
#include "d_englsh.hpp"
#endif

/* Note this is not externally modifiable through DEH/BEX
 * Misc. other strings.
 * #define SAVEGAMENAME  "boomsav"      * killough 3/22/98 *
 * Ty 05/04/98 - replaced with a modifiable string, see d_deh.c
 */

/*
 * File locations,
 *  relative to current position.
 * Path names are OS-sensitive.
 */
#define DEVMAPS "devmaps"
#define DEVDATA "devdata"


/* Not done in french?
 * QuitDOOM messages *
 * killough 1/18/98:
 * replace hardcoded limit with extern var (silly hack, I know)
 */

#include <stddef.h>

extern const size_t NUM_QUITMESSAGES; /* Calculated in dstrings.c */

extern const char** endmsg[]; /* killough 1/18/98 const added */

#ifdef __cplusplus
}
#endif
