// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Game Controller

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// Must match SDL
enum struct GameControllerButton : int32_t
{
	A,
	B,
	X,
	Y,
	Back,
	Guide,
	Start,
	Leftstick,
	Rightstick,
	Leftshoulder,
	Rightshoulder,
	DpadUp,
	DpadDown,
	DpadLeft,
	DpadRight,
	Misc1,
	Paddle1,
	Paddle2,
	Paddle3,
	Paddle4,
	Touchpad,
	Triggerleft,
	Triggerright,
	Max,
};

const char* dsda_GameControllerButtonName(int button);
void dsda_PollGameController();
void dsda_PollGameControllerButtons();
void dsda_InitGameController();

#ifdef __cplusplus
}
#endif
