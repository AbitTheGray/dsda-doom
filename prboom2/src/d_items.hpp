// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Items: key cards, artifacts, weapon, ammunition.
 */

#pragma once

#include <utility>

#include "doomdef.hpp"

#include "cpp/EnumArray.hpp"

enum struct StateId : int32_t;

#ifdef __cplusplus
extern "C"
{
#endif

//
// Internal weapon flags
//
#define WIF_ENABLEAPS 0x00000001 // [XA] enable "ammo per shot" field for native Doom weapon codepointers

// haleyjd 09/11/07: weapon flags
//
#define WPF_NOFLAG         0x00000000 // no flag
#define WPF_NOTHRUST       0x00000001 // doesn't thrust Mobj's
#define WPF_SILENT         0x00000002 // weapon is silent
#define WPF_NOAUTOFIRE     0x00000004 // weapon won't autofire in A_WeaponReady
#define WPF_FLEEMELEE      0x00000008 // monsters consider it a melee weapon
#define WPF_AUTOSWITCHFROM 0x00000010 // can be switched away from when ammo is picked up
#define WPF_NOAUTOSWITCHTO   0x00000020 // cannot be switched to when ammo is picked up

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
	int intflags;
	int flags;
} weaponinfo_t;

extern weaponinfo_t doom_weaponinfo[std::to_underlying(WeaponType::Count) + 2];

// heretic

extern weaponinfo_t wpnlev1info[std::to_underlying(WeaponType::Count)];
extern weaponinfo_t wpnlev2info[std::to_underlying(WeaponType::Count)];

// hexen

extern EnumArray<EnumArray<weaponinfo_t, EnumCount<PClass>>, WeaponType::HexenCount> hexen_weaponinfo;

// dynamically selected in global.c

extern weaponinfo_t* weaponinfo;

#ifdef __cplusplus
}
#endif
