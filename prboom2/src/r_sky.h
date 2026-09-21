// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Sky rendering.
 */

#ifndef __R_SKY__
#define __R_SKY__

#include "m_fixed.h"

/* The sky map is 256*128*4 maps. */
#define ANGLETOSKYSHIFT         22

extern int skyflatnum;
extern int skytexture;
extern int skytexturemid;

#define SKYSTRETCH_HEIGHT 228
extern int skystretch;
extern fixed_t freelookviewheight;

/* Called whenever the view size changes. */
void R_InitSkyMap(void);

#endif
