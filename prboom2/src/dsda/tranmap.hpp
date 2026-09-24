// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA TRANMAP

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

const byte* dsda_TranMap(unsigned int alpha);
const byte* dsda_DefaultTranMap();

#ifdef __cplusplus
}
#endif
