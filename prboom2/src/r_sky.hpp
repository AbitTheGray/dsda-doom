// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Sky rendering.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "m_fixed.hpp"

/* The sky map is 256*128*4 maps. */
#define ANGLETOSKYSHIFT         22

extern int skyflatnum;
extern int skytexture;
extern int skytexturemid;

#define SKYSTRETCH_HEIGHT 228
extern int skystretch;
extern fixed_t freelookviewheight;

/* Called whenever the view size changes. */
void R_InitSkyMap();

#ifdef __cplusplus
}
#endif
