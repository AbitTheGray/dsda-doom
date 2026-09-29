// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Items: key cards, artifacts, weapon, ammunition.
 */

#pragma once

#include <utility>

#include "doomdef.hpp"

#include "cpp/EnumArray.hpp"
#include "cpp/Util.hpp"

// Internal weapon flags, set by DEHACKED.
enum struct WeaponIntFlag : uint32_t
{
	EnableAps = Bit<uint32_t>(0u), // [XA] enable "ammo per shot" field for native Doom weapon codepointers
};
ENUM_FLAGS_FUNC(WeaponIntFlag)

// haleyjd 09/11/07: weapon flags
enum struct WeaponFlag : uint32_t
{
	NoThrust = Bit<uint32_t>(0u),       // doesn't thrust Mobj's
	Silent = Bit<uint32_t>(1u),         // weapon is silent
	NoAutoFire = Bit<uint32_t>(2u),     // weapon won't autofire in A_WeaponReady
	FleeMelee = Bit<uint32_t>(3u),      // monsters consider it a melee weapon
	AutoSwitchFrom = Bit<uint32_t>(4u), // can be switched away from when ammo is picked up
	NoAutoSwitchTo = Bit<uint32_t>(5u), // cannot be switched to when ammo is picked up
};
ENUM_FLAGS_FUNC(WeaponFlag)

enum struct StateId : int32_t;

#ifdef __cplusplus
extern "C"
{
#endif

/* Weapon info: sprite frames, ammunition use. */
typedef struct
{
	AmmoType ammo;
	StateId upstate;
	StateId downstate;
	StateId readystate;
	StateId atkstate;
	StateId holdatkstate;
	StateId flashstate;
	int ammopershot;
	WeaponIntFlag intflags;
	WeaponFlag flags;
} weaponinfo_t;

extern weaponinfo_t doom_weaponinfo[std::to_underlying(WeaponType::Count) + 2];

// heretic

extern weaponinfo_t wpnlev1info[std::to_underlying(WeaponType::Count)];
extern weaponinfo_t wpnlev2info[std::to_underlying(WeaponType::Count)];

// hexen

extern EnumArray<EnumArray<weaponinfo_t, PClass>, WeaponType, WeaponType::HexenCount> hexen_weaponinfo;

// dynamically selected in global.c

extern weaponinfo_t* weaponinfo;

#ifdef __cplusplus
}
#endif
