// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Game Controller

#include <utility>

#include "SDL.h"

#include "d_event.hpp"
#include "d_main.hpp"
#include "lprintf.hpp"

#include "dsda/args.hpp"
#include "dsda/configuration.hpp"

#include "game_controller.hpp"

static int use_game_controller;
static SDL_GameController* game_controller;

typedef struct
{
	SDL_GameControllerAxis axis;
	int deadzone;
	int sensitivity;
} axis_t;

static axis_t left_analog_x = {SDL_CONTROLLER_AXIS_LEFTX};
static axis_t left_analog_y = {SDL_CONTROLLER_AXIS_LEFTY};
static axis_t right_analog_x = {SDL_CONTROLLER_AXIS_RIGHTX};
static axis_t right_analog_y = {SDL_CONTROLLER_AXIS_RIGHTY};
static axis_t left_trigger = {SDL_CONTROLLER_AXIS_TRIGGERLEFT};
static axis_t right_trigger = {SDL_CONTROLLER_AXIS_TRIGGERRIGHT};

static int swap_analogs;

static const char* button_names[] = {
	[std::to_underlying(GameControllerButton::A)] = "pad a",
	[std::to_underlying(GameControllerButton::B)] = "pad b",
	[std::to_underlying(GameControllerButton::X)] = "pad x",
	[std::to_underlying(GameControllerButton::Y)] = "pad y",
	[std::to_underlying(GameControllerButton::Back)] = "pad back",
	[std::to_underlying(GameControllerButton::Guide)] = "pad guide",
	[std::to_underlying(GameControllerButton::Start)] = "pad start",
	[std::to_underlying(GameControllerButton::Leftstick)] = "lstick",
	[std::to_underlying(GameControllerButton::Rightstick)] = "rstick",
	[std::to_underlying(GameControllerButton::Leftshoulder)] = "pad l",
	[std::to_underlying(GameControllerButton::Rightshoulder)] = "pad r",
	[std::to_underlying(GameControllerButton::DpadUp)] = "dpad u",
	[std::to_underlying(GameControllerButton::DpadDown)] = "dpad d",
	[std::to_underlying(GameControllerButton::DpadLeft)] = "dpad l",
	[std::to_underlying(GameControllerButton::DpadRight)] = "dpad r",
	[std::to_underlying(GameControllerButton::Misc1)] = "misc 1",
	[std::to_underlying(GameControllerButton::Paddle1)] = "paddle 1",
	[std::to_underlying(GameControllerButton::Paddle2)] = "paddle 2",
	[std::to_underlying(GameControllerButton::Paddle3)] = "paddle 3",
	[std::to_underlying(GameControllerButton::Paddle4)] = "paddle 4",
	[std::to_underlying(GameControllerButton::Touchpad)] = "touchpad",
	[std::to_underlying(GameControllerButton::Triggerleft)] = "pad lt",
	[std::to_underlying(GameControllerButton::Triggerright)] = "pad rt",
};

const char* dsda_GameControllerButtonName(int button)
{
	if(button >= sizeof(button_names) || !button_names[button])
		return "misc";

	return button_names[button];
}

static float dsda_AxisValue(axis_t* axis)
{
	int value;

	value = SDL_GameControllerGetAxis(game_controller, axis->axis);

	// the positive axis max is 1 less
	if(value > (axis->deadzone - 1))
		value -= (axis->deadzone - 1);
	else if(value < -axis->deadzone)
		value += axis->deadzone;
	else
		value = 0;

	return (float)value * axis->sensitivity / (32768 - axis->deadzone);
}

static void dsda_PollLeftStick()
{
	event_t ev;

	ev.type = swap_analogs ? EventType::LookAnalog : EventType::MoveAnalog;
	ev.data1.f = dsda_AxisValue(&left_analog_x);
	ev.data2.f = -dsda_AxisValue(&left_analog_y);

	if(ev.data1.f || ev.data2.f)
		D_PostEvent(&ev);
}

static void dsda_PollRightStick()
{
	event_t ev;

	ev.type = swap_analogs ? EventType::MoveAnalog : EventType::LookAnalog;
	ev.data1.f = dsda_AxisValue(&right_analog_x);
	ev.data2.f = -dsda_AxisValue(&right_analog_y);

	if(ev.data1.f || ev.data2.f)
		D_PostEvent(&ev);
}

static inline int PollButton(GameControllerButton button)
{
	// This depends on enums having same values
	return SDL_GameControllerGetButton(game_controller, (SDL_GameControllerButton)button) << std::to_underlying(button);
}

void dsda_PollGameControllerButtons()
{
	event_t ev;
	float trigger;

	if(!game_controller)
		return;

	ev.type = EventType::Joystick;
	ev.data1.i = PollButton(GameControllerButton::A) |
		PollButton(GameControllerButton::B) |
		PollButton(GameControllerButton::X) |
		PollButton(GameControllerButton::Y) |
		PollButton(GameControllerButton::Back) |
		PollButton(GameControllerButton::Guide) |
		PollButton(GameControllerButton::Start) |
		PollButton(GameControllerButton::Leftstick) |
		PollButton(GameControllerButton::Rightstick) |
		PollButton(GameControllerButton::Leftshoulder) |
		PollButton(GameControllerButton::Rightshoulder) |
		PollButton(GameControllerButton::DpadUp) |
		PollButton(GameControllerButton::DpadDown) |
		PollButton(GameControllerButton::DpadLeft) |
		PollButton(GameControllerButton::DpadRight) |
		PollButton(GameControllerButton::Misc1) |
		PollButton(GameControllerButton::Paddle1) |
		PollButton(GameControllerButton::Paddle2) |
		PollButton(GameControllerButton::Paddle3) |
		PollButton(GameControllerButton::Paddle4) |
		PollButton(GameControllerButton::Touchpad);

	trigger = dsda_AxisValue(&left_trigger);
	if(trigger)
		ev.data1.i |= (1 << std::to_underlying(GameControllerButton::Triggerleft));

	trigger = dsda_AxisValue(&right_trigger);
	if(trigger)
		ev.data1.i |= (1 << std::to_underlying(GameControllerButton::Triggerright));

	D_PostEvent(&ev);
}

void dsda_PollGameController()
{
	if(!game_controller)
		return;

	dsda_PollGameControllerButtons();
	dsda_PollLeftStick();
	dsda_PollRightStick();
}

extern "C" void dsda_InitGameControllerParameters()
{
	left_analog_x.deadzone = dsda_IntConfig(ConfigId::LeftAnalogDeadzone);
	left_analog_x.sensitivity = dsda_IntConfig(ConfigId::LeftAnalogSensitivityX);
	left_analog_y.deadzone = left_analog_x.deadzone;
	left_analog_y.sensitivity = dsda_IntConfig(ConfigId::LeftAnalogSensitivityY);

	right_analog_x.deadzone = dsda_IntConfig(ConfigId::RightAnalogDeadzone);
	right_analog_x.sensitivity = dsda_IntConfig(ConfigId::RightAnalogSensitivityX);
	right_analog_y.deadzone = right_analog_x.deadzone;
	right_analog_y.sensitivity = dsda_IntConfig(ConfigId::RightAnalogSensitivityY);

	left_trigger.deadzone = dsda_IntConfig(ConfigId::LeftTriggerDeadzone);
	left_trigger.sensitivity = 1;
	right_trigger.deadzone = dsda_IntConfig(ConfigId::RightTriggerDeadzone);
	right_trigger.sensitivity = 1;

	swap_analogs = dsda_IntConfig(ConfigId::SwapAnalogs);
}

void dsda_InitGameController()
{
	int num_joysticks;

	game_controller = nullptr;
	use_game_controller =
		dsda_IntConfig(ConfigId::UseGameController) && !dsda_Flag(ArgId::Nojoy);

	if(!use_game_controller)
		return;

	dsda_InitGameControllerParameters();
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

	num_joysticks = SDL_NumJoysticks();

	if(use_game_controller > num_joysticks)
	{
		lprintf(OutputLevels::Warn, "dsda_InitGameController: invalid joystick %d\n",
			use_game_controller);
		return;
	}

	if(!SDL_IsGameController(use_game_controller - 1))
	{
		lprintf(OutputLevels::Warn, "dsda_InitGameController: unsupported joystick %d\n",
			use_game_controller);
		return;
	}

	game_controller = SDL_GameControllerOpen(use_game_controller - 1);

	if(!game_controller)
	{
		lprintf(OutputLevels::Error, "dsda_InitGameController: error opening game controller %d\n",
			use_game_controller);
		return;
	}

	lprintf(OutputLevels::Debug, "Opened game controller %s\n", SDL_GameControllerName(game_controller));
}
