// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Something to do with weapon sprite frames. Don't ask me.
 */

// We are referring to sprite numbers.
#include <utility>

#include "doomtype.hpp"
#include "info.hpp"

#include "d_items.hpp"


//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//
weaponinfo_t doom_weaponinfo[std::to_underlying(WeaponType::Count) + 2] =
{
	{
		// fist
		AmmoType::NoAmmo,
		StateId::Punchup,
		StateId::Punchdown,
		StateId::Punch,
		StateId::Punch1,
		static_cast<StateId>(-1), // upstream wrote MT_NULL here, not a state id //TODO Check correctness
		StateId::Null,
		1,
		0,
		WPF_FLEEMELEE | WPF_AUTOSWITCHFROM | WPF_NOAUTOSWITCHTO
	},
	{
		// pistol
		AmmoType::Clip,
		StateId::Pistolup,
		StateId::Pistoldown,
		StateId::Pistol,
		StateId::Pistol1,
		StateId::Null,
		StateId::Pistolflash,
		1,
		0,
		WPF_AUTOSWITCHFROM
	},
	{
		// shotgun
		AmmoType::Shell,
		StateId::Sgunup,
		StateId::Sgundown,
		StateId::Sgun,
		StateId::Sgun1,
		StateId::Null,
		StateId::Sgunflash1,
		1,
		0,
		WPF_NOFLAG
	},
	{
		// chaingun
		AmmoType::Clip,
		StateId::Chainup,
		StateId::Chaindown,
		StateId::Chain,
		StateId::Chain1,
		StateId::Null,
		StateId::Chainflash1,
		1,
		0,
		WPF_NOFLAG
	},
	{
		// missile launcher
		AmmoType::Misl,
		StateId::Missileup,
		StateId::Missiledown,
		StateId::Missile,
		StateId::Missile1,
		StateId::Null,
		StateId::Missileflash1,
		1,
		0,
		WPF_NOAUTOFIRE
	},
	{
		// plasma rifle
		AmmoType::Cell,
		StateId::Plasmaup,
		StateId::Plasmadown,
		StateId::Plasma,
		StateId::Plasma1,
		StateId::Null,
		StateId::Plasmaflash1,
		1,
		0,
		WPF_NOFLAG
	},
	{
		// bfg 9000
		AmmoType::Cell,
		StateId::Bfgup,
		StateId::Bfgdown,
		StateId::Bfg,
		StateId::Bfg1,
		StateId::Null,
		StateId::Bfgflash1,
		40,
		0,
		WPF_NOAUTOFIRE
	},
	{
		// chainsaw
		AmmoType::NoAmmo,
		StateId::Sawup,
		StateId::Sawdown,
		StateId::Saw,
		StateId::Saw1,
		StateId::Null,
		StateId::Null,
		1,
		0,
		WPF_NOTHRUST | WPF_FLEEMELEE | WPF_NOAUTOSWITCHTO
	},
	{
		// super shotgun
		AmmoType::Shell,
		StateId::Dsgunup,
		StateId::Dsgundown,
		StateId::Dsgun,
		StateId::Dsgun1,
		StateId::Null,
		StateId::Dsgunflash1,
		2,
		0,
		WPF_NOFLAG
	},

	// dseg03:00082D90                 weaponinfo_t <5, 46h, 45h, 43h, 47h, 0>
	// dseg03:00082D90                 weaponinfo_t <1, 22h, 21h, 20h, 23h, 2Fh>
	// dseg03:00082E68 animdefs        dd 0                    ; istexture
	// dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '3', 2 dup(0); endname
	// dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '1', 2 dup(0); startname
	// dseg03:00082E68                 dd 8                    ; speed
	// dseg03:00082E68                 dd 0                    ; istexture
	{
		// ololo weapon
		AmmoType{},
		StateId::Null, // states are not used for emulation of weaponinfo overrun
		StateId::Null,
		StateId::Null,
		StateId::Null,
		StateId::Null,
		StateId::Null,
		0,
		0,
		WPF_NOFLAG
	},
	{
		// preved medved weapon
		AmmoType{},
		StateId::Null,
		StateId::Null,
		StateId::Null,
		StateId::Null,
		StateId::Null,
		StateId::Null,
		0,
		0,
		WPF_NOFLAG
	},
};

// heretic

#include "heretic/def.hpp"

weaponinfo_t wpnlev1info[std::to_underlying(WeaponType::Count)] = {
	{
		// Staff
		AmmoType::NoAmmo,             // ammo
		StateId::HereticStaffup,     // upstate
		StateId::HereticStaffdown,   // downstate
		StateId::HereticStaffready,  // readystate
		StateId::HereticStaffatk11, // atkstate
		StateId::HereticStaffatk11, // holdatkstate
		StateId::HereticNull,        // flashstate
		0,                     // ammopershot
		0,                     // intflags
		WPF_NOFLAG
	},
	{
		// Gold wand
		AmmoType::GoldWand,              // ammo
		StateId::HereticGoldwandup,     // upstate
		StateId::HereticGoldwanddown,   // downstate
		StateId::HereticGoldwandready,  // readystate
		StateId::HereticGoldwandatk11, // atkstate
		StateId::HereticGoldwandatk11, // holdatkstate
		StateId::HereticNull,           // flashstate
		USE_GWND_AMMO_1,          // ammopershot
		0,                        // intflags
		WPF_NOFLAG
	},
	{
		// Crossbow
		AmmoType::Crossbow,           // ammo
		StateId::HereticCrbowup,     // upstate
		StateId::HereticCrbowdown,   // downstate
		StateId::HereticCrbow1,      // readystate
		StateId::HereticCrbowatk11, // atkstate
		StateId::HereticCrbowatk11, // holdatkstate
		StateId::HereticNull,        // flashstate
		USE_CBOW_AMMO_1,       // ammopershot
		0,                     // intflags
		WPF_NOFLAG
	},
	{
		// Blaster
		AmmoType::Blaster,              // ammo
		StateId::HereticBlasterup,     // upstate
		StateId::HereticBlasterdown,   // downstate
		StateId::HereticBlasterready,  // readystate
		StateId::HereticBlasteratk11, // atkstate
		StateId::HereticBlasteratk13, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_BLSR_AMMO_1,         // ammopershot
		0,                       // intflags
		WPF_NOFLAG
	},
	{
		// Skull rod
		AmmoType::SkullRod,             // ammo
		StateId::HereticHornrodup,     // upstate
		StateId::HereticHornroddown,   // downstate
		StateId::HereticHornrodready,  // readystae
		StateId::HereticHornrodatk11, // atkstate
		StateId::HereticHornrodatk11, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_SKRD_AMMO_1,         // ammopershot
		0,                       // intflags
		WPF_NOFLAG
	},
	{
		// Phoenix rod
		AmmoType::PhoenixRod,           // ammo
		StateId::HereticPhoenixup,     // upstate
		StateId::HereticPhoenixdown,   // downstate
		StateId::HereticPhoenixready,  // readystate
		StateId::HereticPhoenixatk11, // atkstate
		StateId::HereticPhoenixatk11, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_PHRD_AMMO_1,         // ammopershot
		0,                       // intflags
		WPF_NOAUTOFIRE
	},
	{
		// Mace
		AmmoType::Mace,              // ammo
		StateId::HereticMaceup,     // upstate
		StateId::HereticMacedown,   // downstate
		StateId::HereticMaceready,  // readystate
		StateId::HereticMaceatk11, // atkstate
		StateId::HereticMaceatk12, // holdatkstate
		StateId::HereticNull,       // flashstate
		USE_MACE_AMMO_1,      // ammopershot
		0,                    // intflags
		WPF_NOFLAG
	},
	{
		// Gauntlets
		AmmoType::NoAmmo,                // ammo
		StateId::HereticGauntletup,     // upstate
		StateId::HereticGauntletdown,   // downstate
		StateId::HereticGauntletready,  // readystate
		StateId::HereticGauntletatk11, // atkstate
		StateId::HereticGauntletatk13, // holdatkstate
		StateId::HereticNull,           // flashstate
		0,                        // ammopershot
		0,                        // intflags
		WPF_NOTHRUST
	},
	{
		// Beak
		AmmoType::NoAmmo,            // ammo
		StateId::HereticBeakup,     // upstate
		StateId::HereticBeakdown,   // downstate
		StateId::HereticBeakready,  // readystate
		StateId::HereticBeakatk11, // atkstate
		StateId::HereticBeakatk11, // holdatkstate
		StateId::HereticNull,       // flashstate
		0,                    // ammopershot
		0,                    // intflags
		WPF_NOFLAG
	}
};

weaponinfo_t wpnlev2info[std::to_underlying(WeaponType::Count)] = {
	{
		// Staff
		AmmoType::NoAmmo,               // ammo
		StateId::HereticStaffup2,      // upstate
		StateId::HereticStaffdown2,    // downstate
		StateId::HereticStaffready21, // readystate
		StateId::HereticStaffatk21,   // atkstate
		StateId::HereticStaffatk21,   // holdatkstate
		StateId::HereticNull,          // flashstate
		0,                       // ammopershot
		0,                       // intflags
		WPF_NOFLAG
	},
	{
		// Gold wand
		AmmoType::GoldWand,              // ammo
		StateId::HereticGoldwandup,     // upstate
		StateId::HereticGoldwanddown,   // downstate
		StateId::HereticGoldwandready,  // readystate
		StateId::HereticGoldwandatk21, // atkstate
		StateId::HereticGoldwandatk21, // holdatkstate
		StateId::HereticNull,           // flashstate
		USE_GWND_AMMO_2,          // ammopershot
		0,                        // intflags
		WPF_NOFLAG
	},
	{
		// Crossbow
		AmmoType::Crossbow,           // ammo
		StateId::HereticCrbowup,     // upstate
		StateId::HereticCrbowdown,   // downstate
		StateId::HereticCrbow1,      // readystate
		StateId::HereticCrbowatk21, // atkstate
		StateId::HereticCrbowatk21, // holdatkstate
		StateId::HereticNull,        // flashstate
		USE_CBOW_AMMO_2,       // ammopershot
		0,                     // intflags
		WPF_NOFLAG
	},
	{
		// Blaster
		AmmoType::Blaster,              // ammo
		StateId::HereticBlasterup,     // upstate
		StateId::HereticBlasterdown,   // downstate
		StateId::HereticBlasterready,  // readystate
		StateId::HereticBlasteratk21, // atkstate
		StateId::HereticBlasteratk23, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_BLSR_AMMO_2,         // ammopershot
		0,                       // intflags
		WPF_NOFLAG
	},
	{
		// Skull rod
		AmmoType::SkullRod,             // ammo
		StateId::HereticHornrodup,     // upstate
		StateId::HereticHornroddown,   // downstate
		StateId::HereticHornrodready,  // readystae
		StateId::HereticHornrodatk21, // atkstate
		StateId::HereticHornrodatk21, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_SKRD_AMMO_2,         // ammopershot
		0,                       // intflags
		WPF_NOFLAG
	},
	{
		// Phoenix rod
		AmmoType::PhoenixRod,           // ammo
		StateId::HereticPhoenixup,     // upstate
		StateId::HereticPhoenixdown,   // downstate
		StateId::HereticPhoenixready,  // readystate
		StateId::HereticPhoenixatk21, // atkstate
		StateId::HereticPhoenixatk22, // holdatkstate
		StateId::HereticNull,          // flashstate
		USE_PHRD_AMMO_2,         // ammopershot
		0,                       // intflags
		WPF_NOAUTOFIRE
	},
	{
		// Mace
		AmmoType::Mace,              // ammo
		StateId::HereticMaceup,     // upstate
		StateId::HereticMacedown,   // downstate
		StateId::HereticMaceready,  // readystate
		StateId::HereticMaceatk21, // atkstate
		StateId::HereticMaceatk21, // holdatkstate
		StateId::HereticNull,       // flashstate
		USE_MACE_AMMO_2,      // ammopershot
		0,                    // intflags
		WPF_NOFLAG
	},
	{
		// Gauntlets
		AmmoType::NoAmmo,                  // ammo
		StateId::HereticGauntletup2,      // upstate
		StateId::HereticGauntletdown2,    // downstate
		StateId::HereticGauntletready21, // readystate
		StateId::HereticGauntletatk21,   // atkstate
		StateId::HereticGauntletatk23,   // holdatkstate
		StateId::HereticNull,             // flashstate
		0,                          // ammopershot
		0,                          // intflags
		WPF_NOTHRUST
	},
	{
		// Beak
		AmmoType::NoAmmo,            // ammo
		StateId::HereticBeakup,     // upstate
		StateId::HereticBeakdown,   // downstate
		StateId::HereticBeakready,  // readystate
		StateId::HereticBeakatk21, // atkstate
		StateId::HereticBeakatk21, // holdatkstate
		StateId::HereticNull,       // flashstate
		0,                    // ammopershot
		0,                    // intflags
		WPF_NOFLAG
	}
};

// hexen

weaponinfo_t hexen_weaponinfo[std::to_underlying(WeaponType::HexenCount)][std::to_underlying(PClass::Count)] = {
	{
		// First Weapons
		[std::to_underlying(PClass::Fighter)] = {
			// Fighter First Weapon - Punch
			AmmoType::ManaNone,           // mana
			StateId::HexenPunchup,     // upstate
			StateId::HexenPunchdown,   // downstate
			StateId::HexenPunchready,  // readystate
			StateId::HexenPunchatk11, // atkstate
			StateId::HexenPunchatk11, // holdatkstate
			StateId::HexenNull,        // flashstate
			0,                   // ammopershot
			0,                   // intflags
			WPF_NOFLAG
		},
		{
			// Cleric First Weapon - Mace
			AmmoType::ManaNone,          // mana
			StateId::HexenCmaceup,    // upstate
			StateId::HexenCmacedown,  // downstate
			StateId::HexenCmaceready, // readystate
			StateId::HexenCmaceatk1, // atkstate
			StateId::HexenCmaceatk1, // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		},
		{
			// Mage First Weapon - Wand
			AmmoType::ManaNone,
			StateId::HexenMwandup,    // upstate
			StateId::HexenMwanddown,  // downstate
			StateId::HexenMwandready, // readystate
			StateId::HexenMwandatk1, // atkstate
			StateId::HexenMwandatk1, // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		},
		{
			// Pig - Snout
			AmmoType::ManaNone,          // mana
			StateId::HexenSnoutup,    // upstate
			StateId::HexenSnoutdown,  // downstate
			StateId::HexenSnoutready, // readystate
			StateId::HexenSnoutatk1,  // atkstate
			StateId::HexenSnoutatk1,  // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		}
	},
	{
		// Second Weapons
		[std::to_underlying(PClass::Fighter)] = {
			// Fighter - Axe
			AmmoType::ManaNone,         // mana
			StateId::HexenFaxeup,    // upstate
			StateId::HexenFaxedown,  // downstate
			StateId::HexenFaxeready, // readystate
			StateId::HexenFaxeatk1, // atkstate
			StateId::HexenFaxeatk1, // holdatkstate
			StateId::HexenNull,      // flashstate
			2,                 // ammopershot
			0,                 // intflags
			WPF_NOFLAG
		},
		{
			// Cleric - Serpent Staff
			AmmoType::Mana1,              // mana
			StateId::HexenCstaffup,    // upstate
			StateId::HexenCstaffdown,  // downstate
			StateId::HexenCstaffready, // readystate
			StateId::HexenCstaffatk1, // atkstate
			StateId::HexenCstaffatk1, // holdatkstate
			StateId::HexenNull,        // flashstate
			1,                   // ammopershot
			0,                   // intflags
			WPF_NOFLAG
		},
		{
			// Mage - Cone of shards
			AmmoType::Mana1,             // mana
			StateId::HexenConeup,     // upstate
			StateId::HexenConedown,   // downstate
			StateId::HexenConeready,  // readystate
			StateId::HexenConeatk11, // atkstate
			StateId::HexenConeatk13, // holdatkstate
			StateId::HexenNull,       // flashstate
			3,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		},
		{
			// Pig - Snout
			AmmoType::ManaNone,          // mana
			StateId::HexenSnoutup,    // upstate
			StateId::HexenSnoutdown,  // downstate
			StateId::HexenSnoutready, // readystate
			StateId::HexenSnoutatk1,  // atkstate
			StateId::HexenSnoutatk1,  // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		}
	},
	{
		// Third Weapons
		[std::to_underlying(PClass::Fighter)] = {
			// Fighter - Hammer
			AmmoType::ManaNone,            // mana
			StateId::HexenFhammerup,    // upstate
			StateId::HexenFhammerdown,  // downstate
			StateId::HexenFhammerready, // readystate
			StateId::HexenFhammeratk1, // atkstate
			StateId::HexenFhammeratk1, // holdatkstate
			StateId::HexenNull,         // flashstate
			3,                    // ammopershot
			0,                    // intflags
			WPF_NOFLAG
		},
		{
			// Cleric - Flame Strike
			AmmoType::Mana2,               // mana
			StateId::HexenCflameup,     // upstate
			StateId::HexenCflamedown,   // downstate
			StateId::HexenCflameready1, // readystate
			StateId::HexenCflameatk1,  // atkstate
			StateId::HexenCflameatk1,  // holdatkstate
			StateId::HexenNull,         // flashstate
			4,                    // ammopershot
			0,                    // intflags
			WPF_NOFLAG
		},
		{
			// Mage - Lightning
			AmmoType::Mana2,                  // mana
			StateId::HexenMlightningup,    // upstate
			StateId::HexenMlightningdown,  // downstate
			StateId::HexenMlightningready, // readystate
			StateId::HexenMlightningatk1, // atkstate
			StateId::HexenMlightningatk1, // holdatkstate
			StateId::HexenNull,            // flashstate
			5,                       // ammopershot
			0,                       // intflags
			WPF_NOFLAG
		},
		{
			// Pig - Snout
			AmmoType::ManaNone,          // mana
			StateId::HexenSnoutup,    // upstate
			StateId::HexenSnoutdown,  // downstate
			StateId::HexenSnoutready, // readystate
			StateId::HexenSnoutatk1,  // atkstate
			StateId::HexenSnoutatk1,  // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		}
	},
	{
		// Fourth Weapons
		[std::to_underlying(PClass::Fighter)] = {
			// Fighter - Rune Sword
			AmmoType::ManaBoth,           // mana
			StateId::HexenFswordup,    // upstate
			StateId::HexenFsworddown,  // downstate
			StateId::HexenFswordready, // readystate
			StateId::HexenFswordatk1, // atkstate
			StateId::HexenFswordatk1, // holdatkstate
			StateId::HexenNull,        // flashstate
			14,                  // ammopershot
			0,                   // intflags
			WPF_NOFLAG
		},
		{
			// Cleric - Holy Symbol
			AmmoType::ManaBoth,          // mana
			StateId::HexenCholyup,    // upstate
			StateId::HexenCholydown,  // downstate
			StateId::HexenCholyready, // readystate
			StateId::HexenCholyatk1, // atkstate
			StateId::HexenCholyatk1, // holdatkstate
			StateId::HexenNull,       // flashstate
			18,                 // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		},
		{
			// Mage - Staff
			AmmoType::ManaBoth,           // mana
			StateId::HexenMstaffup,    // upstate
			StateId::HexenMstaffdown,  // downstate
			StateId::HexenMstaffready, // readystate
			StateId::HexenMstaffatk1, // atkstate
			StateId::HexenMstaffatk1, // holdatkstate
			StateId::HexenNull,        // flashstate
			15,                  // ammopershot
			0,                   // intflags
			WPF_NOFLAG
		},
		{
			// Pig - Snout
			AmmoType::ManaNone,          // mana
			StateId::HexenSnoutup,    // upstate
			StateId::HexenSnoutdown,  // downstate
			StateId::HexenSnoutready, // readystate
			StateId::HexenSnoutatk1,  // atkstate
			StateId::HexenSnoutatk1,  // holdatkstate
			StateId::HexenNull,       // flashstate
			0,                  // ammopershot
			0,                  // intflags
			WPF_NOFLAG
		}
	}
};
