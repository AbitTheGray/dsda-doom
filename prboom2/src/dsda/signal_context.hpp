// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Signal Context

#pragma once

enum struct SignalContext : uint32_t
{
	Display             = 0x0001,
	PlayerView         = 0x0002,
	SetupFrame         = 0x0004,
	Clear               = 0x0008,
	InitScene          = 0x0010,
	GlFrustum          = 0x0020,
	BspNodes           = 0x0040,
	DrawPlanes         = 0x0080,
	ResetColumnBuffer = 0x0100,
	DrawMasked         = 0x0200,
	DrawScene          = 0x0400,
	StatusBar          = 0x0800,
	Hud                 = 0x1000,
};
ENUM_FLAGS_FUNC(SignalContext)

#ifdef __cplusplus
extern "C"
{
#endif

extern SignalContext signal_context;

#define DSDA_ADD_CONTEXT(x) signal_context |= (x)
#define DSDA_REMOVE_CONTEXT(x) signal_context -= (x)

#ifdef __cplusplus
}
#endif
