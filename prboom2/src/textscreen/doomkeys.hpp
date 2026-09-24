// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//       Key definitions

#pragma once

#include <utility>

#ifdef __cplusplus
extern "C"
{
#endif

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define NUMKEYS 256
#define KEY_RIGHTARROW 0xae
#define KEY_LEFTARROW  0xac
#define KEY_UPARROW    0xad
#define KEY_DOWNARROW  0xaf
#define KEY_ESCAPE     27
#define KEY_ENTER      13
#define KEY_TAB        9

#define KEY_BACKSPACE  0x7f
#define KEY_PAUSE      0xff

#define KEY_EQUALS     0x3d
#define KEY_MINUS      0x2d

enum struct Key : int32_t
{
	// Keys without character representations

	F1 = 0x80,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	Rshift,
	Rctrl,
	Ralt,
	Lalt = Ralt,
	Capslock,
	Numlock,
	Scrlck,
	Prtscr,
	Home,
	End,
	Pgup,
	Pgdn,
	Ins,
	Del,

	// Keys on the numerics keypad

	Keyp0,
	Keyp1,
	Keyp2,
	Keyp3,
	Keyp4,
	Keyp5,
	Keyp6,
	Keyp7,
	Keyp8,
	Keyp9,
	KeypDivide,
	KeypPlus,
	KeypMinus,
	KeypMultiply,
	KeypPeriod,
	KeypEquals = KEY_EQUALS,
	KeypEnter  = KEY_ENTER,
};

#define SCANCODE_TO_KEYS_ARRAY {                                            \
    0,   0,   0,   0,   'a',                                  /* 0-9 */     \
    'b', 'c', 'd', 'e', 'f',                                                \
    'g', 'h', 'i', 'j', 'k',                                  /* 10-19 */   \
    'l', 'm', 'n', 'o', 'p',                                                \
    'q', 'r', 's', 't', 'u',                                  /* 20-29 */   \
    'v', 'w', 'x', 'y', 'z',                                                \
    '1', '2', '3', '4', '5',                                  /* 30-39 */   \
    '6', '7', '8', '9', '0',                                                \
    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ',       /* 40-49 */   \
    KEY_MINUS, KEY_EQUALS, '[', ']', '\\',                                  \
    0,   ';', '\'', '`', ',',                                 /* 50-59 */   \
    '.', '/', std::to_underlying(Key::Capslock), std::to_underlying(Key::F1), std::to_underlying(Key::F2),                                 \
    std::to_underlying(Key::F3), std::to_underlying(Key::F4), std::to_underlying(Key::F5), std::to_underlying(Key::F6), std::to_underlying(Key::F7),                   /* 60-69 */   \
    std::to_underlying(Key::F8), std::to_underlying(Key::F9), std::to_underlying(Key::F10), std::to_underlying(Key::F11), std::to_underlying(Key::F12),                              \
    std::to_underlying(Key::Prtscr), std::to_underlying(Key::Scrlck), KEY_PAUSE, std::to_underlying(Key::Ins), std::to_underlying(Key::Home),     /* 70-79 */   \
    std::to_underlying(Key::Pgup), std::to_underlying(Key::Del), std::to_underlying(Key::End), std::to_underlying(Key::Pgdn), KEY_RIGHTARROW,                   \
    KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW,                /* 80-89 */   \
    std::to_underlying(Key::Numlock), std::to_underlying(Key::KeypDivide),                                               \
    std::to_underlying(Key::KeypMultiply), std::to_underlying(Key::KeypMinus), std::to_underlying(Key::KeypPlus), std::to_underlying(Key::KeypEnter), std::to_underlying(Key::Keyp1),               \
    std::to_underlying(Key::Keyp2), std::to_underlying(Key::Keyp3), std::to_underlying(Key::Keyp4), std::to_underlying(Key::Keyp5), std::to_underlying(Key::Keyp6),                   /* 90-99 */   \
    std::to_underlying(Key::Keyp7), std::to_underlying(Key::Keyp8), std::to_underlying(Key::Keyp9), std::to_underlying(Key::Keyp0), std::to_underlying(Key::KeypPeriod),                            \
    0, 0, 0, std::to_underlying(Key::KeypEquals),                                     /* 100-103 */ \
}

// Default names for keys, to use in English or as fallback.
#define KEY_NAMES_ARRAY {                                            \
    { KEY_BACKSPACE,  "BACKSP" },   { KEY_TAB,        "TAB" },       \
    { Key::Ins,        "INS" },      { Key::Del,        "DEL" },       \
    { Key::Pgup,       "PGUP" },     { Key::Pgdn,       "PGDN" },      \
    { KEY_ENTER,      "ENTER" },    { KEY_ESCAPE,     "ESC" },       \
    { Key::F1,         "F1" },       { Key::F2,         "F2" },        \
    { Key::F3,         "F3" },       { Key::F4,         "F4" },        \
    { Key::F5,         "F5" },       { Key::F6,         "F6" },        \
    { Key::F7,         "F7" },       { Key::F8,         "F8" },        \
    { Key::F9,         "F9" },       { Key::F10,        "F10" },       \
    { Key::F11,        "F11" },      { Key::F12,        "F12" },       \
    { Key::Home,       "HOME" },     { Key::End,        "END" },       \
    { KEY_MINUS,      "-" },        { KEY_EQUALS,     "=" },         \
    { Key::Numlock,    "NUMLCK" },   { Key::Scrlck,     "SCRLCK" },    \
    { KEY_PAUSE,      "PAUSE" },    { Key::Prtscr,     "PRTSC" },     \
    { KEY_UPARROW,    "UP" },       { KEY_DOWNARROW,  "DOWN" },      \
    { KEY_LEFTARROW,  "LEFT" },     { KEY_RIGHTARROW, "RIGHT" },     \
    { Key::Ralt,       "ALT" },      { Key::Lalt,       "ALT" },       \
    { Key::Rshift,     "SHIFT" },    { Key::Capslock,   "CAPS" },      \
    { Key::Rctrl,      "CTRL" },     { Key::Keyp5,         "NUM5" },      \
    { ' ',            "SPACE" },                                     \
    { 'a', "A" },   { 'b', "B" },   { 'c', "C" },   { 'd', "D" },    \
    { 'e', "E" },   { 'f', "F" },   { 'g', "G" },   { 'h', "H" },    \
    { 'i', "I" },   { 'j', "J" },   { 'k', "K" },   { 'l', "L" },    \
    { 'm', "M" },   { 'n', "N" },   { 'o', "O" },   { 'p', "P" },    \
    { 'q', "Q" },   { 'r', "R" },   { 's', "S" },   { 't', "T" },    \
    { 'u', "U" },   { 'v', "V" },   { 'w', "W" },   { 'x', "X" },    \
    { 'y', "Y" },   { 'z', "Z" },   { '0', "0" },   { '1', "1" },    \
    { '2', "2" },   { '3', "3" },   { '4', "4" },   { '5', "5" },    \
    { '6', "6" },   { '7', "7" },   { '8', "8" },   { '9', "9" },    \
    { '[', "[" },   { ']', "]" },   { ';', ";" },   { '`', "`" },    \
    { ',', "," },   { '.', "." },   { '/', "/" },   { '\\', "\\" },  \
    { '\'', "\'" },                                                  \
}

enum struct GamepadButton : int32_t
{
	A,
	B,
	X,
	Y,
	Back,
	Guide,
	Start,
	LeftStick,
	RightStick,
	LeftShoulder,
	RightShoulder,
	DpadUp,
	DpadDown,
	DpadLeft,
	DpadRight,
	Misc1,
	Paddle1,
	Paddle2,
	Paddle3,
	Paddle4,
	TouchpadPress,
	TouchpadTouch,
	LeftTrigger,
	RightTrigger,
	LeftStickUp,
	LeftStickDown,
	LeftStickLeft,
	LeftStickRight,
	RightStickUp,
	RightStickDown,
	RightStickLeft,
	RightStickRight,

	NumGamepadButtons
};

enum struct MouseButton : int32_t
{
	Left,
	Right,
	Middle,
	X1,
	X2,
	Wheelup,
	Wheeldown,
	Wheelleft,
	Wheelright,

	NumMouseButtons
};

enum struct GamepadAxis : int32_t
{
	LeftX,
	LeftY,
	RightX,
	RightY,

	Count
};

enum struct AxisAction : int32_t
{
	Strafe,
	Forward,
	Turn,
	Look,
};

enum struct GyroAxis : int32_t
{
	Turn,
	Look,

	Count
};

#ifdef __cplusplus
}
#endif
