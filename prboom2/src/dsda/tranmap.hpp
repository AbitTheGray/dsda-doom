// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA TRANMAP

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

const byte* dsda_TranMap(unsigned int alpha);
const byte* dsda_DefaultTranMap();

#ifdef __cplusplus
}
#endif
