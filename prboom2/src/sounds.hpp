// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Created by the sound utility written by Dave Taylor.
 *      Kept as a sample, DOOM2 sounds. Frozen.
 */

#pragma once

#include <utility>

#include "doomtype.hpp"

#include "cpp/EnumArray.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct SfxClass : int32_t
{
	None,
	Important,
	Secret,
};

typedef struct
{
	int volume;
	int separation;
	int pitch;
	int priority;
	dboolean ambient;
	float attenuation;
	float volume_factor;
	dboolean loop;
	int loop_timeout;
	SfxClass sfx_class;
} sfx_params_t;

//
// SoundFX struct.
//

struct sfxinfo_struct;

typedef struct sfxinfo_struct sfxinfo_t;

struct sfxinfo_struct
{
	// up to 6-character name
	const char* name; // CPhipps - const

	// Sfx priority
	int priority;

	// referenced sound if a link
	sfxinfo_t* link;

	// pitch if a link
	int pitch;

	// sound data
	void* data;

	// lump number of sfx
	int lumpnum;

	// heretic - total number of channels a sound type may occupy
	int numchannels;

	// hexen
	const char* tagname;

	int parallel_tic;
	int parallel_count;
};

//
// MusicInfo struct.
//

typedef struct
{
	// up to 6-character name
	const char* name; // CPhipps - const

	// lump number of music
	int lumpnum;

	/* music data - cphipps 4/11 made const void* */
	const void* data;

	// music handle once registered
	int handle;
} musicinfo_t;

//
// Identifiers for all music in game.
//

extern int mus_musinfo;

enum struct MusicId : int32_t
{
	None,
	E1m1,
	E1m2,
	E1m3,
	E1m4,
	E1m5,
	E1m6,
	E1m7,
	E1m8,
	E1m9,
	E2m1,
	E2m2,
	E2m3,
	E2m4,
	E2m5,
	E2m6,
	E2m7,
	E2m8,
	E2m9,
	E3m1,
	E3m2,
	E3m3,
	E3m4,
	E3m5,
	E3m6,
	E3m7,
	E3m8,
	E3m9,
	Inter,
	Intro,
	Bunny,
	Victor,
	Introa,
	Runnin,
	Stalks,
	Countd,
	Betwee,
	Doom,
	TheDa,
	Shawn,
	Ddtblu,
	InCit,
	Dead,
	Stlks2,
	Theda2,
	Doom2,
	Ddtbl2,
	Runni2,
	Dead2,
	Stlks3,
	Romero,
	Shawn2,
	Messag,
	Count2,
	Ddtbl3,
	Ampie,
	Theda3,
	Adrian,
	Messg2,
	Romer2,
	Tense,
	Shawn3,
	Openin,
	Evil,
	Ultima,
	ReadM,
	Dm2ttl,
	Dm2int,
	DoomMusinfo,
	DoomNummusic,

	// heretic
	HereticE1m1 = E1m1,
	HereticE1m2,
	HereticE1m3,
	HereticE1m4,
	HereticE1m5,
	HereticE1m6,
	HereticE1m7,
	HereticE1m8,
	HereticE1m9,

	HereticE2m1,
	HereticE2m2,
	HereticE2m3,
	HereticE2m4,
	HereticE2m5,
	HereticE2m6,
	HereticE2m7,
	HereticE2m8,
	HereticE2m9,

	HereticE3m1,
	HereticE3m2,
	HereticE3m3,
	HereticE3m4,
	HereticE3m5,
	HereticE3m6,
	HereticE3m7,
	HereticE3m8,
	HereticE3m9,

	HereticE4m1,
	HereticE4m2,
	HereticE4m3,
	HereticE4m4,
	HereticE4m5,
	HereticE4m6,
	HereticE4m7,
	HereticE4m8,
	HereticE4m9,

	HereticE5m1,
	HereticE5m2,
	HereticE5m3,
	HereticE5m4,
	HereticE5m5,
	HereticE5m6,
	HereticE5m7,
	HereticE5m8,
	HereticE5m9,

	HereticE6m1,
	HereticE6m2,
	HereticE6m3,

	HereticTitl,
	HereticIntr,
	HereticCptd,
	HereticMusInfo,
	HereticCount,

	// hexen
	HexenMap   = E1m1,
	HexenHexen = HexenMap + 99, // hexen mapinfo supports up to 99 entries
	HexenHub,
	HexenHall,
	HexenOrb,
	HexenChess,
	HexenMusInfo,
	HexenCount
};

//
// Identifiers for all sfx in game.
//

enum struct SfxId : int32_t
{
	NoFallback = -1, // S_StartOptionalSound: do not play anything instead

	None       = 0,

	BaseCount,

	Pistol = BaseCount,
	Shotgn,
	Sgcock,
	Dshtgn,
	Dbopn,
	Dbcls,
	Dbload,
	Plasma,
	Bfg,
	Sawup,
	Sawidl,
	Sawful,
	Sawhit,
	Rlaunc,
	Rxplod,
	Firsht,
	Firxpl,
	Pstart,
	Pstop,
	Doropn,
	Dorcls,
	Stnmov,
	Swtchn,
	Swtchx,
	Plpain,
	Dmpain,
	Popain,
	Vipain,
	Mnpain,
	Pepain,
	Slop,
	Itemup,
	Wpnup,
	Oof,
	Telept,
	Posit1,
	Posit2,
	Posit3,
	Bgsit1,
	Bgsit2,
	Sgtsit,
	Cacsit,
	Brssit,
	Cybsit,
	Spisit,
	Bspsit,
	Kntsit,
	Vilsit,
	Mansit,
	Pesit,
	Sklatk,
	Sgtatk,
	Skepch,
	Vilatk,
	Claw,
	Skeswg,
	Pldeth,
	Pdiehi,
	Podth1,
	Podth2,
	Podth3,
	Bgdth1,
	Bgdth2,
	Sgtdth,
	Cacdth,
	Skldth,
	Brsdth,
	Cybdth,
	Spidth,
	Bspdth,
	Vildth,
	Kntdth,
	Pedth,
	Skedth,
	Posact,
	Bgact,
	Dmact,
	Bspact,
	Bspwlk,
	Vilact,
	Noway,
	Barexp,
	Punch,
	Hoof,
	Metal,
	Chgun,
	Tink,
	Bdopn,
	Bdcls,
	Itmbk,
	Flame,
	Flamst,
	Getpow,
	Bospit,
	Boscub,
	Bossit,
	Bospn,
	Bosdth,
	Manatk,
	Mandth,
	Sssit,
	Ssdth,
	Keenpn,
	Keendt,
	Skeact,
	Skesit,
	Skeatk,
	Radio,

	/* killough 11/98: dog sounds */
	Dgsit,
	Dgatk,
	Dgact,
	Dgdth,
	Dgpain,

	// DSDA
	Secret,

	// Optional menu/intermission sounds
	Mnuopn, // swtchn
	Mnucls, // swtchx
	Mnuact, // pistol
	Mnubak,
	Mnumov, // pstop
	Mnusli, // stnmov
	Mnusel, // itemup
	Mnuerr, // oof
	Inttic, // pistol
	Inttot, // barex
	Intnex, // sgcock
	Intnet, // pldeth
	Intdms, // slop

	// Everything from here to 500 is reserved

	/* Free sound effect slots for DEHEXTRA. Offset agreed upon with Eternity devs. -SH */
	Fre000 = 500,
	Fre001,
	Fre002,
	Fre003,
	Fre004,
	Fre005,
	Fre006,
	Fre007,
	Fre008,
	Fre009,
	Fre010,
	Fre011,
	Fre012,
	Fre013,
	Fre014,
	Fre015,
	Fre016,
	Fre017,
	Fre018,
	Fre019,
	Fre020,
	Fre021,
	Fre022,
	Fre023,
	Fre024,
	Fre025,
	Fre026,
	Fre027,
	Fre028,
	Fre029,
	Fre030,
	Fre031,
	Fre032,
	Fre033,
	Fre034,
	Fre035,
	Fre036,
	Fre037,
	Fre038,
	Fre039,
	Fre040,
	Fre041,
	Fre042,
	Fre043,
	Fre044,
	Fre045,
	Fre046,
	Fre047,
	Fre048,
	Fre049,
	Fre050,
	Fre051,
	Fre052,
	Fre053,
	Fre054,
	Fre055,
	Fre056,
	Fre057,
	Fre058,
	Fre059,
	Fre060,
	Fre061,
	Fre062,
	Fre063,
	Fre064,
	Fre065,
	Fre066,
	Fre067,
	Fre068,
	Fre069,
	Fre070,
	Fre071,
	Fre072,
	Fre073,
	Fre074,
	Fre075,
	Fre076,
	Fre077,
	Fre078,
	Fre079,
	Fre080,
	Fre081,
	Fre082,
	Fre083,
	Fre084,
	Fre085,
	Fre086,
	Fre087,
	Fre088,
	Fre089,
	Fre090,
	Fre091,
	Fre092,
	Fre093,
	Fre094,
	Fre095,
	Fre096,
	Fre097,
	Fre098,
	Fre099,
	Fre100,
	Fre101,
	Fre102,
	Fre103,
	Fre104,
	Fre105,
	Fre106,
	Fre107,
	Fre108,
	Fre109,
	Fre110,
	Fre111,
	Fre112,
	Fre113,
	Fre114,
	Fre115,
	Fre116,
	Fre117,
	Fre118,
	Fre119,
	Fre120,
	Fre121,
	Fre122,
	Fre123,
	Fre124,
	Fre125,
	Fre126,
	Fre127,
	Fre128,
	Fre129,
	Fre130,
	Fre131,
	Fre132,
	Fre133,
	Fre134,
	Fre135,
	Fre136,
	Fre137,
	Fre138,
	Fre139,
	Fre140,
	Fre141,
	Fre142,
	Fre143,
	Fre144,
	Fre145,
	Fre146,
	Fre147,
	Fre148,
	Fre149,
	Fre150,
	Fre151,
	Fre152,
	Fre153,
	Fre154,
	Fre155,
	Fre156,
	Fre157,
	Fre158,
	Fre159,
	Fre160,
	Fre161,
	Fre162,
	Fre163,
	Fre164,
	Fre165,
	Fre166,
	Fre167,
	Fre168,
	Fre169,
	Fre170,
	Fre171,
	Fre172,
	Fre173,
	Fre174,
	Fre175,
	Fre176,
	Fre177,
	Fre178,
	Fre179,
	Fre180,
	Fre181,
	Fre182,
	Fre183,
	Fre184,
	Fre185,
	Fre186,
	Fre187,
	Fre188,
	Fre189,
	Fre190,
	Fre191,
	Fre192,
	Fre193,
	Fre194,
	Fre195,
	Fre196,
	Fre197,
	Fre198,
	Fre199,

	DoomCount,

	// heretic
	HereticGldhit = BaseCount,
	HereticGntful,
	HereticGnthit,
	HereticGntpow,
	HereticGntact,
	HereticGntuse,
	HereticPhosht,
	HereticPhohit,
	HereticPhopow,
	HereticLobsht,
	HereticLobhit,
	HereticLobpow,
	HereticHrnsht,
	HereticHrnhit,
	HereticHrnpow,
	HereticRamphit,
	HereticRamrain,
	HereticBowsht,
	HereticStfhit,
	HereticStfpow,
	HereticStfcrk,
	HereticImpsit,
	HereticImpat1,
	HereticImpat2,
	HereticImpdth,
	HereticImpact,
	HereticImppai,
	HereticMumsit,
	HereticMumat1,
	HereticMumat2,
	HereticMumdth,
	HereticMumact,
	HereticMumpai,
	HereticMumhed,
	HereticBstsit,
	HereticBstatk,
	HereticBstdth,
	HereticBstact,
	HereticBstpai,
	HereticClksit,
	HereticClkatk,
	HereticClkdth,
	HereticClkact,
	HereticClkpai,
	HereticSnksit,
	HereticSnkatk,
	HereticSnkdth,
	HereticSnkact,
	HereticSnkpai,
	HereticKgtsit,
	HereticKgtatk,
	HereticKgtat2,
	HereticKgtdth,
	HereticKgtact,
	HereticKgtpai,
	HereticWizsit,
	HereticWizatk,
	HereticWizdth,
	HereticWizact,
	HereticWizpai,
	HereticMinsit,
	HereticMinat1,
	HereticMinat2,
	HereticMinat3,
	HereticMindth,
	HereticMinact,
	HereticMinpai,
	HereticHedsit,
	HereticHedat1,
	HereticHedat2,
	HereticHedat3,
	HereticHeddth,
	HereticHedact,
	HereticHedpai,
	HereticSorzap,
	HereticSorrise,
	HereticSorsit,
	HereticSoratk,
	HereticSoract,
	HereticSorpai,
	HereticSordsph,
	HereticSordexp,
	HereticSordbon,
	HereticSbtsit,
	HereticSbtatk,
	HereticSbtdth,
	HereticSbtact,
	HereticSbtpai,
	HereticPlroof,
	HereticPlrpai,
	HereticPlrdth,  // Normal
	HereticGibdth,  // Extreme
	HereticPlrwdth, // Wimpy
	HereticPlrcdth, // Crazy
	HereticItemup,
	HereticWpnup,
	HereticTelept,
	HereticDoropn,
	HereticDorcls,
	HereticDormov,
	HereticArtiup,
	HereticSwitch,
	HereticPstart,
	HereticPstop,
	HereticStnmov,
	HereticChicpai,
	HereticChicatk,
	HereticChicdth,
	HereticChicact,
	HereticChicpk1,
	HereticChicpk2,
	HereticChicpk3,
	HereticKeyup,
	HereticRipslop,
	HereticNewpod,
	HereticPodexp,
	HereticBounce,
	HereticVolsht,
	HereticVolhit,
	HereticBurn,
	HereticSplash,
	HereticGloop,
	HereticRespawn,
	HereticBlssht,
	HereticBlshit,
	HereticChat,
	HereticArtiuse,
	HereticGfrag,
	HereticWaterfl,

	// Monophonic sounds

	HereticWind,
	HereticAmb1,
	HereticAmb2,
	HereticAmb3,
	HereticAmb4,
	HereticAmb5,
	HereticAmb6,
	HereticAmb7,
	HereticAmb8,
	HereticAmb9,
	HereticAmb10,
	HereticAmb11,

	// DSDA
	HereticSecret,

	// Optional menu/intermission sounds
	HereticMnuopn, // swtchn
	HereticMnucls, // swtchx
	HereticMnuact, // pistol
	HereticMnubak,
	HereticMnumov, // pstop
	HereticMnusli, // stnmov
	HereticMnusel, // itemup
	HereticMnuerr, // oof
	HereticInttic, // pistol
	HereticInttot, // barex
	HereticIntnex, // sgcock
	HereticIntnet, // pldeth
	HereticIntdms, // slop

	HereticCount,

	// hexen
	HexenPlayerFighterNormalDeath = BaseCount, // class specific death screams
	HexenPlayerFighterCrazyDeath,
	HexenPlayerFighterExtreme1Death,
	HexenPlayerFighterExtreme2Death,
	HexenPlayerFighterExtreme3Death,
	HexenPlayerFighterBurnDeath,
	HexenPlayerClericNormalDeath,
	HexenPlayerClericCrazyDeath,
	HexenPlayerClericExtreme1Death,
	HexenPlayerClericExtreme2Death,
	HexenPlayerClericExtreme3Death,
	HexenPlayerClericBurnDeath,
	HexenPlayerMageNormalDeath,
	HexenPlayerMageCrazyDeath,
	HexenPlayerMageExtreme1Death,
	HexenPlayerMageExtreme2Death,
	HexenPlayerMageExtreme3Death,
	HexenPlayerMageBurnDeath,
	HexenPlayerFighterPain,
	HexenPlayerClericPain,
	HexenPlayerMagePain,
	HexenPlayerFighterGrunt,
	HexenPlayerClericGrunt,
	HexenPlayerMageGrunt,
	HexenPlayerLand,
	HexenPlayerPoisoncough,
	HexenPlayerFighterFallingScream, // class specific falling screams
	HexenPlayerClericFallingScream,
	HexenPlayerMageFallingScream,
	HexenPlayerFallingSplat,
	HexenPlayerFighterFailedUse,
	HexenPlayerClericFailedUse,
	HexenPlayerMageFailedUse,
	HexenPlatformStart,
	HexenPlatformStartmetal,
	HexenPlatformStop,
	HexenStoneMove,
	HexenMetalMove,
	HexenDoorOpen,
	HexenDoorLocked,
	HexenDoorMetalOpen,
	HexenDoorMetalClose,
	HexenDoorLightClose,
	HexenDoorHeavyClose,
	HexenDoorCreak,
	HexenPickupWeapon,
	HexenPickupArtifact,
	HexenPickupKey,
	HexenPickupItem,
	HexenPickupPiece,
	HexenWeaponBuild,
	HexenArtifactUse,
	HexenArtifactBlast,
	HexenTeleport,
	HexenThunderCrash,
	HexenFighterPunchMiss,
	HexenFighterPunchHitthing,
	HexenFighterPunchHitwall,
	HexenFighterGrunt,
	HexenFighterAxeHitthing,
	HexenFighterHammerMiss,
	HexenFighterHammerHitthing,
	HexenFighterHammerHitwall,
	HexenFighterHammerContinuous,
	HexenFighterHammerExplode,
	HexenFighterSwordFire,
	HexenFighterSwordExplode,
	HexenClericCstaffFire,
	HexenClericCstaffExplode,
	HexenClericCstaffHitthing,
	HexenClericFlameFire,
	HexenClericFlameExplode,
	HexenClericFlameCircle,
	HexenMageWandFire,
	HexenMageLightningFire,
	HexenMageLightningZap,
	HexenMageLightningContinuous,
	HexenMageLightningReady,
	HexenMageShardsFire,
	HexenMageShardsExplode,
	HexenMageStaffFire,
	HexenMageStaffExplode,
	HexenSwitch1,
	HexenSwitch2,
	HexenSerpentSight,
	HexenSerpentActive,
	HexenSerpentPain,
	HexenSerpentAttack,
	HexenSerpentMeleehit,
	HexenSerpentDeath,
	HexenSerpentBirth,
	HexenSerpentfxContinuous,
	HexenSerpentfxHit,
	HexenPotteryExplode,
	HexenDrip,
	HexenCentaurSight,
	HexenCentaurActive,
	HexenCentaurPain,
	HexenCentaurAttack,
	HexenCentaurDeath,
	HexenCentaurleaderAttack,
	HexenCentaurMissileExplode,
	HexenWind,
	HexenBishopSight,
	HexenBishopActive,
	HexenBishopPain,
	HexenBishopAttack,
	HexenBishopDeath,
	HexenBishopMissileExplode,
	HexenBishopBlur,
	HexenDemonSight,
	HexenDemonActive,
	HexenDemonPain,
	HexenDemonAttack,
	HexenDemonMissileFire,
	HexenDemonMissileExplode,
	HexenDemonDeath,
	HexenWraithSight,
	HexenWraithActive,
	HexenWraithPain,
	HexenWraithAttack,
	HexenWraithMissileFire,
	HexenWraithMissileExplode,
	HexenWraithDeath,
	HexenPigActive1,
	HexenPigActive2,
	HexenPigPain,
	HexenPigAttack,
	HexenPigDeath,
	HexenMaulatorSight,
	HexenMaulatorActive,
	HexenMaulatorPain,
	HexenMaulatorHammerSwing,
	HexenMaulatorHammerHit,
	HexenMaulatorMissileHit,
	HexenMaulatorDeath,
	HexenFreezeDeath,
	HexenFreezeShatter,
	HexenEttinSight,
	HexenEttinActive,
	HexenEttinPain,
	HexenEttinAttack,
	HexenEttinDeath,
	HexenFiredSpawn,
	HexenFiredActive,
	HexenFiredPain,
	HexenFiredAttack,
	HexenFiredMissileHit,
	HexenFiredDeath,
	HexenIceguySight,
	HexenIceguyActive,
	HexenIceguyAttack,
	HexenIceguyFxExplode,
	HexenSorcererSight,
	HexenSorcererActive,
	HexenSorcererPain,
	HexenSorcererSpellcast,
	HexenSorcererBallwoosh,
	HexenSorcererDeathscream,
	HexenSorcererBishopspawn,
	HexenSorcererBallpop,
	HexenSorcererBallbounce,
	HexenSorcererBallexplode,
	HexenSorcererBigballexplode,
	HexenSorcererHeadscream,
	HexenDragonSight,
	HexenDragonActive,
	HexenDragonWingflap,
	HexenDragonAttack,
	HexenDragonPain,
	HexenDragonDeath,
	HexenDragonFireballExplode,
	HexenKoraxSight,
	HexenKoraxActive,
	HexenKoraxPain,
	HexenKoraxAttack,
	HexenKoraxCommand,
	HexenKoraxDeath,
	HexenKoraxStep,
	HexenThrustspikeRaise,
	HexenThrustspikeLower,
	HexenStainedglassShatter,
	HexenFlechetteBounce,
	HexenFlechetteExplode,
	HexenLavaMove,
	HexenWaterMove,
	HexenIceStartmove,
	HexenEarthStartmove,
	HexenWaterSplash,
	HexenLavaSizzle,
	HexenSludgeGloop,
	HexenCholyFire,
	HexenSpiritActive,
	HexenSpiritAttack,
	HexenSpiritDie,
	HexenValveTurn,
	HexenRopePull,
	HexenFlyBuzz,
	HexenIgnite,
	HexenPuzzleSuccess,
	HexenPuzzleFailFighter,
	HexenPuzzleFailCleric,
	HexenPuzzleFailMage,
	HexenEarthquake,
	HexenBellring,
	HexenTreeBreak,
	HexenTreeExplode,
	HexenSuitofarmorBreak,
	HexenPoisonshroomPain,
	HexenPoisonshroomDeath,
	HexenAmbient1,
	HexenAmbient2,
	HexenAmbient3,
	HexenAmbient4,
	HexenAmbient5,
	HexenAmbient6,
	HexenAmbient7,
	HexenAmbient8,
	HexenAmbient9,
	HexenAmbient10,
	HexenAmbient11,
	HexenAmbient12,
	HexenAmbient13,
	HexenAmbient14,
	HexenAmbient15,
	HexenStartupTick,
	HexenSwitchOtherlevel,
	HexenRespawn,
	HexenKoraxVoice1,
	HexenKoraxVoice2,
	HexenKoraxVoice3,
	HexenKoraxVoice4,
	HexenKoraxVoice5,
	HexenKoraxVoice6,
	HexenKoraxVoice7,
	HexenKoraxVoice8,
	HexenKoraxVoice9,
	HexenBatScream,
	HexenChat,
	HexenMenuMove,
	HexenClockTick,
	HexenFireball,
	HexenPuppybeat,
	HexenMysticincant,

	// DSDA
	HexenSecret,

	// Optional menu/intermission sounds
	HexenMnuopn, // swtchn
	HexenMnucls, // swtchx
	HexenMnuact, // pistol
	HexenMnubak,
	HexenMnumov, // pstop
	HexenMnusli, // stnmov
	HexenMnusel, // itemup
	HexenMnuerr, // oof
	HexenInttic, // pistol
	HexenInttot, // barex
	HexenIntnex, // sgcock
	HexenIntnet, // pldeth
	HexenIntdms, // slop

	HexenCount
};

// many sounds come in consecutive variants that are picked at random
inline constexpr SfxId SfxVariant(const SfxId first, const int32_t offset) noexcept
{
	return static_cast<SfxId>(std::to_underlying(first) + offset);
}

// a pickup sound is tagged with a high bit so it is never dropped when the
// channel budget is tight - see S_StartSoundAtVolume
inline constexpr SfxId SfxPickupBit{0x8000};

inline constexpr SfxId SfxAsPickup(const SfxId id) noexcept
{
	return static_cast<SfxId>(std::to_underlying(id) | std::to_underlying(SfxPickupBit));
}

inline constexpr bool SfxIsPickup(const SfxId id) noexcept
{
	return (std::to_underlying(id) & std::to_underlying(SfxPickupBit)) != 0;
}

inline constexpr SfxId SfxWithoutPickup(const SfxId id) noexcept
{
	return static_cast<SfxId>(std::to_underlying(id) & ~std::to_underlying(SfxPickupBit));
}

// all the stuff - dynamically selected in global.c

extern sfxinfo_t hexen_S_sfx[];
extern EnumArray<musicinfo_t, MusicId, MusicId::HexenCount> hexen_S_music;

extern sfxinfo_t heretic_S_sfx[];
extern musicinfo_t heretic_S_music[];

extern sfxinfo_t doom_S_sfx[];
extern musicinfo_t doom_S_music[];

extern sfxinfo_t* S_sfx;
extern int num_sfx;
extern musicinfo_t* S_music;
extern int num_music;

#ifdef __cplusplus
}
#endif
