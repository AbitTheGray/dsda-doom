// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Sky rendering. The DOOM sky is a texture map like any
 *  wall, wrapping around. A 1024 columns equal 360 degrees.
 *  The default sky map is 256 columns and repeats 4 times
 *  on a 320 screen?
 */

#include "r_sky.hpp"
#include "r_main.hpp"
#include "e6y.hpp"

#include "dsda/configuration.hpp"
#include "dsda/excmd.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/settings.hpp"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;

int skystretch;
fixed_t freelookviewheight;

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap()
{
	int r_stretchsky;

	r_stretchsky = dsda_IntConfig(dsda_config_render_stretchsky);

	if(raven || !dsda_FreeAim())
	{
		skystretch = false;
		skytexturemid = (raven ? 200 : 100) * FRACUNIT;
		skyiscale = (200 << FRACBITS) / SCREENHEIGHT;
	}
	else
	{
		int skyheight;

		if(!textureheight)
			return;

		// There are various combinations for sky rendering depending on how tall the sky is:
		//        h <  128: Unstretched and tiled, centered on horizon
		// 128 <= h <  200: Can possibly be stretched. When unstretched, the baseline is
		//                  28 rows below the horizon so that the top of the texture
		//                  aligns with the top of the screen when looking straight ahead.
		//                  When stretched, it is scaled to 228 pixels with the baseline
		//                  in the same location as an unstretched 128-tall sky, so the top
		//					of the texture aligns with the top of the screen when looking
		//                  fully up.
		//        h == 200: Unstretched, baseline is on horizon, and top is at the top of
		//                  the screen when looking fully up.
		//        h >  200: Unstretched, but the baseline is shifted down so that the top
		//                  of the texture is at the top of the screen when looking fully up.

		skyheight = textureheight[skytexture] >> FRACBITS;
		skystretch = false;
		skytexturemid = 0;
		if(skyheight >= 128 && skyheight < 200)
		{
			skystretch = (r_stretchsky && skyheight >= 128);
			skytexturemid = -28 * FRACUNIT;
		}
		else if(skyheight > 200)
		{
			skytexturemid = (200 - skyheight) << FRACBITS;
		}

		skyiscale = (200 << FRACBITS) / SCREENHEIGHT;

		if(skystretch)
		{
			skyiscale = (fixed_t)((int64_t)skyiscale * skyheight / SKYSTRETCH_HEIGHT);
			skytexturemid = (int)((int64_t)skytexturemid * skyheight / SKYSTRETCH_HEIGHT);
		}
		else
		{
			skytexturemid = 100 * FRACUNIT;
		}
	}
}
