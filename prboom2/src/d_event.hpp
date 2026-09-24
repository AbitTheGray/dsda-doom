// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Event information structures.
 */

#pragma once

#include <utility>

#include "doomtype.hpp"

enum struct ButtonCode : uint8_t
{
	Attack = 1, // Press "Fire".
	Use    = 2, // Use button, to open doors, activate switches.

	// Flag, weapon change pending.
	// If true, the next 4 bits hold weapon num.
	Change = 4,

	// The 4bit weapon mask and shift, convenience.
	WeaponMaskOld = (8 + 16 + 32),      // e6y
	WeaponMask     = (8 + 16 + 32 + 64), // extended to pick up SSG // phares
	WeaponShift    = 3,

	// Special events
	Special     = 128,
	SpecialMask = 3,
	Pause       = 1,  // Pause the game.
	Join        = 64, // Demo joined.
};
ENUM_FLAGS_FUNC(ButtonCode)

// the button byte packs a weapon number and a special code beside its flags,
// and those fields need their value, not a flag test
inline constexpr ButtonCode ButtonSpecial(const ButtonCode buttons) noexcept
{
	return static_cast<ButtonCode>(
		std::to_underlying(buttons) & std::to_underlying(ButtonCode::SpecialMask));
}

inline constexpr int32_t ButtonWeapon(const ButtonCode buttons) noexcept
{
	return (std::to_underlying(buttons) & std::to_underlying(ButtonCode::WeaponMask))
		>> std::to_underlying(ButtonCode::WeaponShift);
}

#ifdef __cplusplus
extern "C"
{
#endif

//
// Event handling.
//

// Input event types.
enum struct EventType : int32_t
{
	KeyDown,
	KeyUp,
	Mouse,
	MouseMotion,
	Joystick,
	MoveAnalog,
	LookAnalog,
	Trigger,
	Text,
};

typedef union
{
	int i;
	float f;
} event_data_t;

typedef struct
{
	EventType type;
	event_data_t data1;
	event_data_t data2;
	char* text;
} event_t;

enum struct GameAction : int32_t
{
	Nothing,
	LoadLevel,
	NewGame,
	LoadGame,
	PlayDemo,
	Completed,
	Victory,
	WorldDone,

	// hexen
	LeaveMap
};

//
// Button/action code definitions.
//

//
// GLOBAL VARIABLES
//

extern GameAction gameaction;

#ifdef __cplusplus
}
#endif
