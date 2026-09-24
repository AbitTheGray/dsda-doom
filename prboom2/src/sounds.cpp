// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Created by a sound utility.
 *      Kept as a sample, DOOM2 sounds.
 */

// killough 5/3/98: reformatted

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomtype.hpp"
#include "sounds.hpp"

//
// Information about all the music
//

musicinfo_t doom_S_music[] = {
	{nullptr},
	{"e1m1", 0},
	{"e1m2", 0},
	{"e1m3", 0},
	{"e1m4", 0},
	{"e1m5", 0},
	{"e1m6", 0},
	{"e1m7", 0},
	{"e1m8", 0},
	{"e1m9", 0},
	{"e2m1", 0},
	{"e2m2", 0},
	{"e2m3", 0},
	{"e2m4", 0},
	{"e2m5", 0},
	{"e2m6", 0},
	{"e2m7", 0},
	{"e2m8", 0},
	{"e2m9", 0},
	{"e3m1", 0},
	{"e3m2", 0},
	{"e3m3", 0},
	{"e3m4", 0},
	{"e3m5", 0},
	{"e3m6", 0},
	{"e3m7", 0},
	{"e3m8", 0},
	{"e3m9", 0},
	{"inter", 0},
	{"intro", 0},
	{"bunny", 0},
	{"victor", 0},
	{"introa", 0},
	{"runnin", 0},
	{"stalks", 0},
	{"countd", 0},
	{"betwee", 0},
	{"doom", 0},
	{"the_da", 0},
	{"shawn", 0},
	{"ddtblu", 0},
	{"in_cit", 0},
	{"dead", 0},
	{"stlks2", 0},
	{"theda2", 0},
	{"doom2", 0},
	{"ddtbl2", 0},
	{"runni2", 0},
	{"dead2", 0},
	{"stlks3", 0},
	{"romero", 0},
	{"shawn2", 0},
	{"messag", 0},
	{"count2", 0},
	{"ddtbl3", 0},
	{"ampie", 0},
	{"theda3", 0},
	{"adrian", 0},
	{"messg2", 0},
	{"romer2", 0},
	{"tense", 0},
	{"shawn3", 0},
	{"openin", 0},
	{"evil", 0},
	{"ultima", 0},
	{"read_m", 0},
	{"dm2ttl", 0},
	{"dm2int", 0},

	// custom music from MUSINFO lump
	{"musinfo", 0}
};


//
// Information about all the sfx
//

sfxinfo_t doom_S_sfx[] = {
	// S_sfx[0] needs to be a dummy for odd reasons.
	{"dsnone", 0, nullptr, -1, nullptr, 0, 0, ""},

	// Doom sounds
	{"dspistol", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsshotgn", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dssgcock", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdshtgn", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdbopn", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdbcls", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdbload", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsplasma", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbfg", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dssawup", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dssawidl", 118, nullptr, -1, nullptr, 0, 0, ""},
	{"dssawful", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dssawhit", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsrlaunc", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dsrxplod", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsfirsht", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsfirxpl", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dspstart", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dspstop", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdoropn", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdorcls", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsstnmov", 119, nullptr, -1, nullptr, 0, 0, ""},
	{"dsswtchn", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dsswtchx", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dsplpain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdmpain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dspopain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dsvipain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnpain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dspepain", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dsslop", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dsitemup", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dswpnup", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dsoof", 96, nullptr, -1, nullptr, 0, 0, ""},
	{"dstelept", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsposit1", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsposit2", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsposit3", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbgsit1", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbgsit2", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dssgtsit", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dscacsit", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbrssit", 94, nullptr, -1, nullptr, 0, 0, ""},
	{"dscybsit", 92, nullptr, -1, nullptr, 0, 0, ""},
	{"dsspisit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbspsit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dskntsit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dsvilsit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmansit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dspesit", 90, nullptr, -1, nullptr, 0, 0, ""},
	{"dssklatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dssgtatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskepch", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsvilatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsclaw", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskeswg", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dspldeth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dspdiehi", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dspodth1", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dspodth2", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dspodth3", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbgdth1", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbgdth2", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dssgtdth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dscacdth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskldth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbrsdth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dscybdth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsspidth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbspdth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsvildth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dskntdth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dspedth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskedth", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsposact", 120, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbgact", 120, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdmact", 120, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbspact", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbspwlk", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsvilact", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsnoway", 78, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbarexp", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dspunch", 64, nullptr, -1, nullptr, 0, 0, ""},
	{"dshoof", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmetal", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dschgun", 64, &doom_S_sfx[std::to_underlying(SfxId::Pistol)], 150, nullptr, 0, 0, ""},
	{"dstink", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbdopn", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbdcls", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsitmbk", 100, nullptr, -1, nullptr, 0, 0, ""},
	{"dsflame", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsflamst", 32, nullptr, -1, nullptr, 0, 0, ""},
	{"dsgetpow", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbospit", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsboscub", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbossit", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbospn", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsbosdth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmanatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmandth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dssssit", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsssdth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dskeenpn", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dskeendt", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskeact", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskesit", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsskeatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsradio", 60, nullptr, -1, nullptr, 0, 0, ""},

	// killough 11/98: dog sounds
	{"dsdgsit", 98, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdgatk", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdgact", 120, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdgdth", 70, nullptr, -1, nullptr, 0, 0, ""},
	{"dsdgpain", 96, nullptr, -1, nullptr, 0, 0, ""},

	// DSDA
	{"dssecret", 60, nullptr, -1, nullptr, 0, 0, ""},

	// Optional menu/intermission sounds
	{"dsmnuopn", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnucls", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnuact", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnubak", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnumov", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnusli", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnusel", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsmnuerr", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsinttic", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsinttot", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsintnex", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsintnet", 60, nullptr, -1, nullptr, 0, 0, ""},
	{"dsintdms", 60, nullptr, -1, nullptr, 0, 0, ""},

	// Everything from here up to 500 is reserved for future use.

	// Free slots for DEHEXTRA. Priorities should be overridden by user.
	// There is a gap present to accomodate Eternity Engine - see their commit
	// @ https://github.com/team-eternity/eternity/commit/b8fb8f71 - which  means
	// I must use desginated initializers, or else supply an exact number of dummy
	// entries to pad it out. Not sure which would be uglier to maintain. -SH
	[500] = {"dsfre000", 127, nullptr, -1, nullptr, 0, 0, ""},
	[501] = {"dsfre001", 127, nullptr, -1, nullptr, 0, 0, ""},
	[502] = {"dsfre002", 127, nullptr, -1, nullptr, 0, 0, ""},
	[503] = {"dsfre003", 127, nullptr, -1, nullptr, 0, 0, ""},
	[504] = {"dsfre004", 127, nullptr, -1, nullptr, 0, 0, ""},
	[505] = {"dsfre005", 127, nullptr, -1, nullptr, 0, 0, ""},
	[506] = {"dsfre006", 127, nullptr, -1, nullptr, 0, 0, ""},
	[507] = {"dsfre007", 127, nullptr, -1, nullptr, 0, 0, ""},
	[508] = {"dsfre008", 127, nullptr, -1, nullptr, 0, 0, ""},
	[509] = {"dsfre009", 127, nullptr, -1, nullptr, 0, 0, ""},
	[510] = {"dsfre010", 127, nullptr, -1, nullptr, 0, 0, ""},
	[511] = {"dsfre011", 127, nullptr, -1, nullptr, 0, 0, ""},
	[512] = {"dsfre012", 127, nullptr, -1, nullptr, 0, 0, ""},
	[513] = {"dsfre013", 127, nullptr, -1, nullptr, 0, 0, ""},
	[514] = {"dsfre014", 127, nullptr, -1, nullptr, 0, 0, ""},
	[515] = {"dsfre015", 127, nullptr, -1, nullptr, 0, 0, ""},
	[516] = {"dsfre016", 127, nullptr, -1, nullptr, 0, 0, ""},
	[517] = {"dsfre017", 127, nullptr, -1, nullptr, 0, 0, ""},
	[518] = {"dsfre018", 127, nullptr, -1, nullptr, 0, 0, ""},
	[519] = {"dsfre019", 127, nullptr, -1, nullptr, 0, 0, ""},
	[520] = {"dsfre020", 127, nullptr, -1, nullptr, 0, 0, ""},
	[521] = {"dsfre021", 127, nullptr, -1, nullptr, 0, 0, ""},
	[522] = {"dsfre022", 127, nullptr, -1, nullptr, 0, 0, ""},
	[523] = {"dsfre023", 127, nullptr, -1, nullptr, 0, 0, ""},
	[524] = {"dsfre024", 127, nullptr, -1, nullptr, 0, 0, ""},
	[525] = {"dsfre025", 127, nullptr, -1, nullptr, 0, 0, ""},
	[526] = {"dsfre026", 127, nullptr, -1, nullptr, 0, 0, ""},
	[527] = {"dsfre027", 127, nullptr, -1, nullptr, 0, 0, ""},
	[528] = {"dsfre028", 127, nullptr, -1, nullptr, 0, 0, ""},
	[529] = {"dsfre029", 127, nullptr, -1, nullptr, 0, 0, ""},
	[530] = {"dsfre030", 127, nullptr, -1, nullptr, 0, 0, ""},
	[531] = {"dsfre031", 127, nullptr, -1, nullptr, 0, 0, ""},
	[532] = {"dsfre032", 127, nullptr, -1, nullptr, 0, 0, ""},
	[533] = {"dsfre033", 127, nullptr, -1, nullptr, 0, 0, ""},
	[534] = {"dsfre034", 127, nullptr, -1, nullptr, 0, 0, ""},
	[535] = {"dsfre035", 127, nullptr, -1, nullptr, 0, 0, ""},
	[536] = {"dsfre036", 127, nullptr, -1, nullptr, 0, 0, ""},
	[537] = {"dsfre037", 127, nullptr, -1, nullptr, 0, 0, ""},
	[538] = {"dsfre038", 127, nullptr, -1, nullptr, 0, 0, ""},
	[539] = {"dsfre039", 127, nullptr, -1, nullptr, 0, 0, ""},
	[540] = {"dsfre040", 127, nullptr, -1, nullptr, 0, 0, ""},
	[541] = {"dsfre041", 127, nullptr, -1, nullptr, 0, 0, ""},
	[542] = {"dsfre042", 127, nullptr, -1, nullptr, 0, 0, ""},
	[543] = {"dsfre043", 127, nullptr, -1, nullptr, 0, 0, ""},
	[544] = {"dsfre044", 127, nullptr, -1, nullptr, 0, 0, ""},
	[545] = {"dsfre045", 127, nullptr, -1, nullptr, 0, 0, ""},
	[546] = {"dsfre046", 127, nullptr, -1, nullptr, 0, 0, ""},
	[547] = {"dsfre047", 127, nullptr, -1, nullptr, 0, 0, ""},
	[548] = {"dsfre048", 127, nullptr, -1, nullptr, 0, 0, ""},
	[549] = {"dsfre049", 127, nullptr, -1, nullptr, 0, 0, ""},
	[550] = {"dsfre050", 127, nullptr, -1, nullptr, 0, 0, ""},
	[551] = {"dsfre051", 127, nullptr, -1, nullptr, 0, 0, ""},
	[552] = {"dsfre052", 127, nullptr, -1, nullptr, 0, 0, ""},
	[553] = {"dsfre053", 127, nullptr, -1, nullptr, 0, 0, ""},
	[554] = {"dsfre054", 127, nullptr, -1, nullptr, 0, 0, ""},
	[555] = {"dsfre055", 127, nullptr, -1, nullptr, 0, 0, ""},
	[556] = {"dsfre056", 127, nullptr, -1, nullptr, 0, 0, ""},
	[557] = {"dsfre057", 127, nullptr, -1, nullptr, 0, 0, ""},
	[558] = {"dsfre058", 127, nullptr, -1, nullptr, 0, 0, ""},
	[559] = {"dsfre059", 127, nullptr, -1, nullptr, 0, 0, ""},
	[560] = {"dsfre060", 127, nullptr, -1, nullptr, 0, 0, ""},
	[561] = {"dsfre061", 127, nullptr, -1, nullptr, 0, 0, ""},
	[562] = {"dsfre062", 127, nullptr, -1, nullptr, 0, 0, ""},
	[563] = {"dsfre063", 127, nullptr, -1, nullptr, 0, 0, ""},
	[564] = {"dsfre064", 127, nullptr, -1, nullptr, 0, 0, ""},
	[565] = {"dsfre065", 127, nullptr, -1, nullptr, 0, 0, ""},
	[566] = {"dsfre066", 127, nullptr, -1, nullptr, 0, 0, ""},
	[567] = {"dsfre067", 127, nullptr, -1, nullptr, 0, 0, ""},
	[568] = {"dsfre068", 127, nullptr, -1, nullptr, 0, 0, ""},
	[569] = {"dsfre069", 127, nullptr, -1, nullptr, 0, 0, ""},
	[570] = {"dsfre070", 127, nullptr, -1, nullptr, 0, 0, ""},
	[571] = {"dsfre071", 127, nullptr, -1, nullptr, 0, 0, ""},
	[572] = {"dsfre072", 127, nullptr, -1, nullptr, 0, 0, ""},
	[573] = {"dsfre073", 127, nullptr, -1, nullptr, 0, 0, ""},
	[574] = {"dsfre074", 127, nullptr, -1, nullptr, 0, 0, ""},
	[575] = {"dsfre075", 127, nullptr, -1, nullptr, 0, 0, ""},
	[576] = {"dsfre076", 127, nullptr, -1, nullptr, 0, 0, ""},
	[577] = {"dsfre077", 127, nullptr, -1, nullptr, 0, 0, ""},
	[578] = {"dsfre078", 127, nullptr, -1, nullptr, 0, 0, ""},
	[579] = {"dsfre079", 127, nullptr, -1, nullptr, 0, 0, ""},
	[580] = {"dsfre080", 127, nullptr, -1, nullptr, 0, 0, ""},
	[581] = {"dsfre081", 127, nullptr, -1, nullptr, 0, 0, ""},
	[582] = {"dsfre082", 127, nullptr, -1, nullptr, 0, 0, ""},
	[583] = {"dsfre083", 127, nullptr, -1, nullptr, 0, 0, ""},
	[584] = {"dsfre084", 127, nullptr, -1, nullptr, 0, 0, ""},
	[585] = {"dsfre085", 127, nullptr, -1, nullptr, 0, 0, ""},
	[586] = {"dsfre086", 127, nullptr, -1, nullptr, 0, 0, ""},
	[587] = {"dsfre087", 127, nullptr, -1, nullptr, 0, 0, ""},
	[588] = {"dsfre088", 127, nullptr, -1, nullptr, 0, 0, ""},
	[589] = {"dsfre089", 127, nullptr, -1, nullptr, 0, 0, ""},
	[590] = {"dsfre090", 127, nullptr, -1, nullptr, 0, 0, ""},
	[591] = {"dsfre091", 127, nullptr, -1, nullptr, 0, 0, ""},
	[592] = {"dsfre092", 127, nullptr, -1, nullptr, 0, 0, ""},
	[593] = {"dsfre093", 127, nullptr, -1, nullptr, 0, 0, ""},
	[594] = {"dsfre094", 127, nullptr, -1, nullptr, 0, 0, ""},
	[595] = {"dsfre095", 127, nullptr, -1, nullptr, 0, 0, ""},
	[596] = {"dsfre096", 127, nullptr, -1, nullptr, 0, 0, ""},
	[597] = {"dsfre097", 127, nullptr, -1, nullptr, 0, 0, ""},
	[598] = {"dsfre098", 127, nullptr, -1, nullptr, 0, 0, ""},
	[599] = {"dsfre099", 127, nullptr, -1, nullptr, 0, 0, ""},
	[600] = {"dsfre100", 127, nullptr, -1, nullptr, 0, 0, ""},
	[601] = {"dsfre101", 127, nullptr, -1, nullptr, 0, 0, ""},
	[602] = {"dsfre102", 127, nullptr, -1, nullptr, 0, 0, ""},
	[603] = {"dsfre103", 127, nullptr, -1, nullptr, 0, 0, ""},
	[604] = {"dsfre104", 127, nullptr, -1, nullptr, 0, 0, ""},
	[605] = {"dsfre105", 127, nullptr, -1, nullptr, 0, 0, ""},
	[606] = {"dsfre106", 127, nullptr, -1, nullptr, 0, 0, ""},
	[607] = {"dsfre107", 127, nullptr, -1, nullptr, 0, 0, ""},
	[608] = {"dsfre108", 127, nullptr, -1, nullptr, 0, 0, ""},
	[609] = {"dsfre109", 127, nullptr, -1, nullptr, 0, 0, ""},
	[610] = {"dsfre110", 127, nullptr, -1, nullptr, 0, 0, ""},
	[611] = {"dsfre111", 127, nullptr, -1, nullptr, 0, 0, ""},
	[612] = {"dsfre112", 127, nullptr, -1, nullptr, 0, 0, ""},
	[613] = {"dsfre113", 127, nullptr, -1, nullptr, 0, 0, ""},
	[614] = {"dsfre114", 127, nullptr, -1, nullptr, 0, 0, ""},
	[615] = {"dsfre115", 127, nullptr, -1, nullptr, 0, 0, ""},
	[616] = {"dsfre116", 127, nullptr, -1, nullptr, 0, 0, ""},
	[617] = {"dsfre117", 127, nullptr, -1, nullptr, 0, 0, ""},
	[618] = {"dsfre118", 127, nullptr, -1, nullptr, 0, 0, ""},
	[619] = {"dsfre119", 127, nullptr, -1, nullptr, 0, 0, ""},
	[620] = {"dsfre120", 127, nullptr, -1, nullptr, 0, 0, ""},
	[621] = {"dsfre121", 127, nullptr, -1, nullptr, 0, 0, ""},
	[622] = {"dsfre122", 127, nullptr, -1, nullptr, 0, 0, ""},
	[623] = {"dsfre123", 127, nullptr, -1, nullptr, 0, 0, ""},
	[624] = {"dsfre124", 127, nullptr, -1, nullptr, 0, 0, ""},
	[625] = {"dsfre125", 127, nullptr, -1, nullptr, 0, 0, ""},
	[626] = {"dsfre126", 127, nullptr, -1, nullptr, 0, 0, ""},
	[627] = {"dsfre127", 127, nullptr, -1, nullptr, 0, 0, ""},
	[628] = {"dsfre128", 127, nullptr, -1, nullptr, 0, 0, ""},
	[629] = {"dsfre129", 127, nullptr, -1, nullptr, 0, 0, ""},
	[630] = {"dsfre130", 127, nullptr, -1, nullptr, 0, 0, ""},
	[631] = {"dsfre131", 127, nullptr, -1, nullptr, 0, 0, ""},
	[632] = {"dsfre132", 127, nullptr, -1, nullptr, 0, 0, ""},
	[633] = {"dsfre133", 127, nullptr, -1, nullptr, 0, 0, ""},
	[634] = {"dsfre134", 127, nullptr, -1, nullptr, 0, 0, ""},
	[635] = {"dsfre135", 127, nullptr, -1, nullptr, 0, 0, ""},
	[636] = {"dsfre136", 127, nullptr, -1, nullptr, 0, 0, ""},
	[637] = {"dsfre137", 127, nullptr, -1, nullptr, 0, 0, ""},
	[638] = {"dsfre138", 127, nullptr, -1, nullptr, 0, 0, ""},
	[639] = {"dsfre139", 127, nullptr, -1, nullptr, 0, 0, ""},
	[640] = {"dsfre140", 127, nullptr, -1, nullptr, 0, 0, ""},
	[641] = {"dsfre141", 127, nullptr, -1, nullptr, 0, 0, ""},
	[642] = {"dsfre142", 127, nullptr, -1, nullptr, 0, 0, ""},
	[643] = {"dsfre143", 127, nullptr, -1, nullptr, 0, 0, ""},
	[644] = {"dsfre144", 127, nullptr, -1, nullptr, 0, 0, ""},
	[645] = {"dsfre145", 127, nullptr, -1, nullptr, 0, 0, ""},
	[646] = {"dsfre146", 127, nullptr, -1, nullptr, 0, 0, ""},
	[647] = {"dsfre147", 127, nullptr, -1, nullptr, 0, 0, ""},
	[648] = {"dsfre148", 127, nullptr, -1, nullptr, 0, 0, ""},
	[649] = {"dsfre149", 127, nullptr, -1, nullptr, 0, 0, ""},
	[650] = {"dsfre150", 127, nullptr, -1, nullptr, 0, 0, ""},
	[651] = {"dsfre151", 127, nullptr, -1, nullptr, 0, 0, ""},
	[652] = {"dsfre152", 127, nullptr, -1, nullptr, 0, 0, ""},
	[653] = {"dsfre153", 127, nullptr, -1, nullptr, 0, 0, ""},
	[654] = {"dsfre154", 127, nullptr, -1, nullptr, 0, 0, ""},
	[655] = {"dsfre155", 127, nullptr, -1, nullptr, 0, 0, ""},
	[656] = {"dsfre156", 127, nullptr, -1, nullptr, 0, 0, ""},
	[657] = {"dsfre157", 127, nullptr, -1, nullptr, 0, 0, ""},
	[658] = {"dsfre158", 127, nullptr, -1, nullptr, 0, 0, ""},
	[659] = {"dsfre159", 127, nullptr, -1, nullptr, 0, 0, ""},
	[660] = {"dsfre160", 127, nullptr, -1, nullptr, 0, 0, ""},
	[661] = {"dsfre161", 127, nullptr, -1, nullptr, 0, 0, ""},
	[662] = {"dsfre162", 127, nullptr, -1, nullptr, 0, 0, ""},
	[663] = {"dsfre163", 127, nullptr, -1, nullptr, 0, 0, ""},
	[664] = {"dsfre164", 127, nullptr, -1, nullptr, 0, 0, ""},
	[665] = {"dsfre165", 127, nullptr, -1, nullptr, 0, 0, ""},
	[666] = {"dsfre166", 127, nullptr, -1, nullptr, 0, 0, ""},
	[667] = {"dsfre167", 127, nullptr, -1, nullptr, 0, 0, ""},
	[668] = {"dsfre168", 127, nullptr, -1, nullptr, 0, 0, ""},
	[669] = {"dsfre169", 127, nullptr, -1, nullptr, 0, 0, ""},
	[670] = {"dsfre170", 127, nullptr, -1, nullptr, 0, 0, ""},
	[671] = {"dsfre171", 127, nullptr, -1, nullptr, 0, 0, ""},
	[672] = {"dsfre172", 127, nullptr, -1, nullptr, 0, 0, ""},
	[673] = {"dsfre173", 127, nullptr, -1, nullptr, 0, 0, ""},
	[674] = {"dsfre174", 127, nullptr, -1, nullptr, 0, 0, ""},
	[675] = {"dsfre175", 127, nullptr, -1, nullptr, 0, 0, ""},
	[676] = {"dsfre176", 127, nullptr, -1, nullptr, 0, 0, ""},
	[677] = {"dsfre177", 127, nullptr, -1, nullptr, 0, 0, ""},
	[678] = {"dsfre178", 127, nullptr, -1, nullptr, 0, 0, ""},
	[679] = {"dsfre179", 127, nullptr, -1, nullptr, 0, 0, ""},
	[680] = {"dsfre180", 127, nullptr, -1, nullptr, 0, 0, ""},
	[681] = {"dsfre181", 127, nullptr, -1, nullptr, 0, 0, ""},
	[682] = {"dsfre182", 127, nullptr, -1, nullptr, 0, 0, ""},
	[683] = {"dsfre183", 127, nullptr, -1, nullptr, 0, 0, ""},
	[684] = {"dsfre184", 127, nullptr, -1, nullptr, 0, 0, ""},
	[685] = {"dsfre185", 127, nullptr, -1, nullptr, 0, 0, ""},
	[686] = {"dsfre186", 127, nullptr, -1, nullptr, 0, 0, ""},
	[687] = {"dsfre187", 127, nullptr, -1, nullptr, 0, 0, ""},
	[688] = {"dsfre188", 127, nullptr, -1, nullptr, 0, 0, ""},
	[689] = {"dsfre189", 127, nullptr, -1, nullptr, 0, 0, ""},
	[690] = {"dsfre190", 127, nullptr, -1, nullptr, 0, 0, ""},
	[691] = {"dsfre191", 127, nullptr, -1, nullptr, 0, 0, ""},
	[692] = {"dsfre192", 127, nullptr, -1, nullptr, 0, 0, ""},
	[693] = {"dsfre193", 127, nullptr, -1, nullptr, 0, 0, ""},
	[694] = {"dsfre194", 127, nullptr, -1, nullptr, 0, 0, ""},
	[695] = {"dsfre195", 127, nullptr, -1, nullptr, 0, 0, ""},
	[696] = {"dsfre196", 127, nullptr, -1, nullptr, 0, 0, ""},
	[697] = {"dsfre197", 127, nullptr, -1, nullptr, 0, 0, ""},
	[698] = {"dsfre198", 127, nullptr, -1, nullptr, 0, 0, ""},
	[699] = {"dsfre199", 127, nullptr, -1, nullptr, 0, 0, ""},
};

#define DISAMBIGUATED_SFX(id, tag) { "", 0, &doom_S_sfx[std::to_underlying(id)], 0, 0, 0, 0, tag }

sfxinfo_t doom_disambiguated_sfx[] = {
	DISAMBIGUATED_SFX(SfxId::Pistol, "weapons/pistol"),
	DISAMBIGUATED_SFX(SfxId::Pistol, "grunt/attack"),
	DISAMBIGUATED_SFX(SfxId::Pistol, "menu/choose"),
	DISAMBIGUATED_SFX(SfxId::Pistol, "intermission/tick"),

	DISAMBIGUATED_SFX(SfxId::Shotgn, "weapons/shotgf"),
	DISAMBIGUATED_SFX(SfxId::Shotgn, "shotguy/attack"),
	DISAMBIGUATED_SFX(SfxId::Shotgn, "chainguy/attack"),
	DISAMBIGUATED_SFX(SfxId::Shotgn, "spider/attack"),
	DISAMBIGUATED_SFX(SfxId::Shotgn, "wolfss/attack"),

	DISAMBIGUATED_SFX(SfxId::Sgcock, "weapons/shotgr"),
	DISAMBIGUATED_SFX(SfxId::Sgcock, "intermission/paststats"),
	DISAMBIGUATED_SFX(SfxId::Sgcock, "intermission/pastcoopstats"),

	DISAMBIGUATED_SFX(SfxId::Dshtgn, "weapons/sshotf"),

	DISAMBIGUATED_SFX(SfxId::Dbopn, "weapons/sshoto"),

	DISAMBIGUATED_SFX(SfxId::Dbcls, "weapons/sshotc"),

	DISAMBIGUATED_SFX(SfxId::Dbload, "weapons/sshotl"),

	DISAMBIGUATED_SFX(SfxId::Plasma, "weapons/plasmaf"),
	DISAMBIGUATED_SFX(SfxId::Plasma, "baby/attack"),

	DISAMBIGUATED_SFX(SfxId::Bfg, "weapons/bfgf"),

	DISAMBIGUATED_SFX(SfxId::Sawup, "weapons/sawup"),

	DISAMBIGUATED_SFX(SfxId::Sawidl, "weapons/sawidle"),

	DISAMBIGUATED_SFX(SfxId::Sawful, "weapons/sawfull"),

	DISAMBIGUATED_SFX(SfxId::Sawhit, "weapons/sawhit"),

	DISAMBIGUATED_SFX(SfxId::Rlaunc, "weapons/rocklf"),

	DISAMBIGUATED_SFX(SfxId::Rxplod, "weapons/bfgx"),

	DISAMBIGUATED_SFX(SfxId::Firsht, "baron/attack"),
	DISAMBIGUATED_SFX(SfxId::Firsht, "fatso/attack"),
	DISAMBIGUATED_SFX(SfxId::Firsht, "imp/attack"),
	DISAMBIGUATED_SFX(SfxId::Firsht, "caco/attack"),

	DISAMBIGUATED_SFX(SfxId::Firxpl, "weapons/plasmax"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "fatso/shotx"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "imp/shotx"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "caco/shotx"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "baron/shotx"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "skull/death"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "baby/shotx"),
	DISAMBIGUATED_SFX(SfxId::Firxpl, "brain/cubeboom"),

	DISAMBIGUATED_SFX(SfxId::Pstart, "plats/pt1_strt"),

	DISAMBIGUATED_SFX(SfxId::Pstop, "plats/pt1_stop"),
	DISAMBIGUATED_SFX(SfxId::Pstop, "menu/cursor"),

	DISAMBIGUATED_SFX(SfxId::Doropn, "doors/dr1_open"),

	DISAMBIGUATED_SFX(SfxId::Dorcls, "doors/dr1_clos"),

	DISAMBIGUATED_SFX(SfxId::Stnmov, "plats/pt1_mid"),
	DISAMBIGUATED_SFX(SfxId::Stnmov, "menu/change"),

	DISAMBIGUATED_SFX(SfxId::Swtchn, "switches/normbutn"),
	DISAMBIGUATED_SFX(SfxId::Swtchn, "menu/activate"),
	DISAMBIGUATED_SFX(SfxId::Swtchn, "menu/backup"),
	DISAMBIGUATED_SFX(SfxId::Swtchn, "menu/prompt"),

	DISAMBIGUATED_SFX(SfxId::Swtchx, "switches/exitbutn"),
	DISAMBIGUATED_SFX(SfxId::Swtchx, "menu/dismiss"),
	DISAMBIGUATED_SFX(SfxId::Swtchx, "menu/clear"),

	DISAMBIGUATED_SFX(SfxId::Plpain, "*pain100"),
	DISAMBIGUATED_SFX(SfxId::Plpain, "*pain75"),
	DISAMBIGUATED_SFX(SfxId::Plpain, "*pain50"),
	DISAMBIGUATED_SFX(SfxId::Plpain, "*pain25"),

	DISAMBIGUATED_SFX(SfxId::Dmpain, "demon/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "spectre/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "caco/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "baron/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "knight/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "skull/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "spider/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "baby/pain"),
	DISAMBIGUATED_SFX(SfxId::Dmpain, "cyber/pain"),

	DISAMBIGUATED_SFX(SfxId::Popain, "grunt/pain"),
	DISAMBIGUATED_SFX(SfxId::Popain, "shotguy/pain"),
	DISAMBIGUATED_SFX(SfxId::Popain, "skeleton/pain"),
	DISAMBIGUATED_SFX(SfxId::Popain, "chainguy/pain"),
	DISAMBIGUATED_SFX(SfxId::Popain, "imp/pain"),
	DISAMBIGUATED_SFX(SfxId::Popain, "wolfss/pain"),

	DISAMBIGUATED_SFX(SfxId::Vipain, "vile/pain"),

	DISAMBIGUATED_SFX(SfxId::Mnpain, "fatso/pain"),

	DISAMBIGUATED_SFX(SfxId::Pepain, "pain/pain"),

	DISAMBIGUATED_SFX(SfxId::Slop, "*gibbed"),
	DISAMBIGUATED_SFX(SfxId::Slop, "misc/gibbed"),
	DISAMBIGUATED_SFX(SfxId::Slop, "vile/raise"),
	DISAMBIGUATED_SFX(SfxId::Slop, "intermission/pastdmstats"),

	DISAMBIGUATED_SFX(SfxId::Itemup, "misc/i_pkup"),
	DISAMBIGUATED_SFX(SfxId::Itemup, "misc/k_pkup"),
	DISAMBIGUATED_SFX(SfxId::Itemup, "misc/health_pkup"),
	DISAMBIGUATED_SFX(SfxId::Itemup, "misc/armor_pkup"),
	DISAMBIGUATED_SFX(SfxId::Itemup, "misc/ammo_pkup"),

	DISAMBIGUATED_SFX(SfxId::Wpnup, "misc/w_pkup"),

	DISAMBIGUATED_SFX(SfxId::Oof, "*grunt"),
	DISAMBIGUATED_SFX(SfxId::Oof, "*land"),
	DISAMBIGUATED_SFX(SfxId::Oof, "menu/invalid"),

	DISAMBIGUATED_SFX(SfxId::Telept, "misc/teleport"),
	DISAMBIGUATED_SFX(SfxId::Telept, "brain/spawn"),

	DISAMBIGUATED_SFX(SfxId::Posit1, "grunt/sight1"),
	DISAMBIGUATED_SFX(SfxId::Posit1, "shotguy/sight1"),
	DISAMBIGUATED_SFX(SfxId::Posit1, "chainguy/sight1"),

	DISAMBIGUATED_SFX(SfxId::Posit2, "grunt/sight2"),
	DISAMBIGUATED_SFX(SfxId::Posit2, "shotguy/sight2"),
	DISAMBIGUATED_SFX(SfxId::Posit2, "chainguy/sight2"),

	DISAMBIGUATED_SFX(SfxId::Posit3, "grunt/sight3"),
	DISAMBIGUATED_SFX(SfxId::Posit3, "shotguy/sight3"),
	DISAMBIGUATED_SFX(SfxId::Posit3, "chainguy/sight3"),

	DISAMBIGUATED_SFX(SfxId::Bgsit1, "imp/sight1"),

	DISAMBIGUATED_SFX(SfxId::Bgsit2, "imp/sight2"),

	DISAMBIGUATED_SFX(SfxId::Sgtsit, "demon/sight"),
	DISAMBIGUATED_SFX(SfxId::Sgtsit, "spectre/sight"),

	DISAMBIGUATED_SFX(SfxId::Cacsit, "caco/sight"),

	DISAMBIGUATED_SFX(SfxId::Brssit, "baron/sight"),

	DISAMBIGUATED_SFX(SfxId::Cybsit, "cyber/sight"),

	DISAMBIGUATED_SFX(SfxId::Spisit, "spider/sight"),

	DISAMBIGUATED_SFX(SfxId::Bspsit, "baby/sight"),

	DISAMBIGUATED_SFX(SfxId::Kntsit, "knight/sight"),

	DISAMBIGUATED_SFX(SfxId::Vilsit, "vile/sight"),

	DISAMBIGUATED_SFX(SfxId::Mansit, "fatso/sight"),

	DISAMBIGUATED_SFX(SfxId::Pesit, "pain/sight"),

	DISAMBIGUATED_SFX(SfxId::Sklatk, "skull/melee"),

	DISAMBIGUATED_SFX(SfxId::Sgtatk, "demon/melee"),
	DISAMBIGUATED_SFX(SfxId::Sgtatk, "spectre/melee"),

	DISAMBIGUATED_SFX(SfxId::Skepch, "skeleton/melee"),

	DISAMBIGUATED_SFX(SfxId::Vilatk, "vile/start"),

	DISAMBIGUATED_SFX(SfxId::Claw, "imp/melee"),
	DISAMBIGUATED_SFX(SfxId::Claw, "baron/melee"),

	DISAMBIGUATED_SFX(SfxId::Skeswg, "skeleton/swing"),

	DISAMBIGUATED_SFX(SfxId::Pldeth, "*death"),
	DISAMBIGUATED_SFX(SfxId::Pldeth, "intermission/cooptotal"),

	DISAMBIGUATED_SFX(SfxId::Pdiehi, "*xdeath"),

	DISAMBIGUATED_SFX(SfxId::Podth1, "grunt/death1"),
	DISAMBIGUATED_SFX(SfxId::Podth1, "shotguy/death1"),
	DISAMBIGUATED_SFX(SfxId::Podth1, "chainguy/death1"),

	DISAMBIGUATED_SFX(SfxId::Podth2, "grunt/death2"),
	DISAMBIGUATED_SFX(SfxId::Podth2, "shotguy/death2"),
	DISAMBIGUATED_SFX(SfxId::Podth2, "chainguy/death2"),

	DISAMBIGUATED_SFX(SfxId::Podth3, "grunt/death3"),
	DISAMBIGUATED_SFX(SfxId::Podth3, "shotguy/death3"),
	DISAMBIGUATED_SFX(SfxId::Podth3, "chainguy/death3"),

	DISAMBIGUATED_SFX(SfxId::Bgdth1, "imp/death1"),

	DISAMBIGUATED_SFX(SfxId::Bgdth2, "imp/death2"),

	DISAMBIGUATED_SFX(SfxId::Sgtdth, "demon/death"),
	DISAMBIGUATED_SFX(SfxId::Sgtdth, "spectre/death"),

	DISAMBIGUATED_SFX(SfxId::Cacdth, "caco/death"),

	DISAMBIGUATED_SFX(SfxId::Skldth, "misc/unused"),

	DISAMBIGUATED_SFX(SfxId::Brsdth, "baron/death"),

	DISAMBIGUATED_SFX(SfxId::Cybdth, "cyber/death"),

	DISAMBIGUATED_SFX(SfxId::Spidth, "spider/death"),

	DISAMBIGUATED_SFX(SfxId::Bspdth, "baby/death"),

	DISAMBIGUATED_SFX(SfxId::Vildth, "vile/death"),

	DISAMBIGUATED_SFX(SfxId::Kntdth, "knight/death"),

	DISAMBIGUATED_SFX(SfxId::Pedth, "pain/death"),

	DISAMBIGUATED_SFX(SfxId::Skedth, "skeleton/death"),

	DISAMBIGUATED_SFX(SfxId::Posact, "grunt/active"),
	DISAMBIGUATED_SFX(SfxId::Posact, "shotguy/active"),
	DISAMBIGUATED_SFX(SfxId::Posact, "fatso/active"),
	DISAMBIGUATED_SFX(SfxId::Posact, "chainguy/active"),
	DISAMBIGUATED_SFX(SfxId::Posact, "wolfss/active"),

	DISAMBIGUATED_SFX(SfxId::Bgact, "imp/active"),

	DISAMBIGUATED_SFX(SfxId::Dmact, "demon/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "spectre/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "caco/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "baron/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "knight/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "skull/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "spider/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "cyber/active"),
	DISAMBIGUATED_SFX(SfxId::Dmact, "pain/active"),

	DISAMBIGUATED_SFX(SfxId::Bspact, "baby/active"),

	DISAMBIGUATED_SFX(SfxId::Bspwlk, "baby/walk"),

	DISAMBIGUATED_SFX(SfxId::Vilact, "vile/active"),

	DISAMBIGUATED_SFX(SfxId::Noway, "*usefail"),
	DISAMBIGUATED_SFX(SfxId::Noway, "misc/keytry"),

	DISAMBIGUATED_SFX(SfxId::Barexp, "weapons/rocklx"),
	DISAMBIGUATED_SFX(SfxId::Barexp, "vile/stop"),
	DISAMBIGUATED_SFX(SfxId::Barexp, "skeleton/tracex"),
	DISAMBIGUATED_SFX(SfxId::Barexp, "world/barrelx"),
	DISAMBIGUATED_SFX(SfxId::Barexp, "misc/brainexplode"),
	DISAMBIGUATED_SFX(SfxId::Barexp, "intermission/nextstage"),

	DISAMBIGUATED_SFX(SfxId::Punch, "*fist"),

	DISAMBIGUATED_SFX(SfxId::Hoof, "cyber/hoof"),

	DISAMBIGUATED_SFX(SfxId::Metal, "spider/walk"),

	DISAMBIGUATED_SFX(SfxId::Chgun, "weapons/chngun"), // -> chgun -> pistol

	DISAMBIGUATED_SFX(SfxId::Tink, "misc/chat2"),

	DISAMBIGUATED_SFX(SfxId::Bdopn, "doors/dr2_open"),

	DISAMBIGUATED_SFX(SfxId::Bdcls, "doors/dr2_clos"),

	DISAMBIGUATED_SFX(SfxId::Itmbk, "misc/spawn"),

	DISAMBIGUATED_SFX(SfxId::Flame, "vile/firecrkl"),

	DISAMBIGUATED_SFX(SfxId::Flamst, "vile/firestrt"),

	DISAMBIGUATED_SFX(SfxId::Getpow, "misc/p_pkup"),

	DISAMBIGUATED_SFX(SfxId::Bospit, "brain/spit"),

	DISAMBIGUATED_SFX(SfxId::Boscub, "brain/cube"),

	DISAMBIGUATED_SFX(SfxId::Bossit, "brain/sight"),

	DISAMBIGUATED_SFX(SfxId::Bospn, "brain/pain"),

	DISAMBIGUATED_SFX(SfxId::Bosdth, "brain/death"),

	DISAMBIGUATED_SFX(SfxId::Manatk, "fatso/raiseguns"),

	DISAMBIGUATED_SFX(SfxId::Mandth, "fatso/death"),

	DISAMBIGUATED_SFX(SfxId::Sssit, "wolfss/sight"),

	DISAMBIGUATED_SFX(SfxId::Ssdth, "wolfss/death"),

	DISAMBIGUATED_SFX(SfxId::Keenpn, "keen/pain"),

	DISAMBIGUATED_SFX(SfxId::Keendt, "keen/death"),

	DISAMBIGUATED_SFX(SfxId::Skeact, "skeleton/active"),

	DISAMBIGUATED_SFX(SfxId::Skesit, "skeleton/sight"),

	DISAMBIGUATED_SFX(SfxId::Skeatk, "skeleton/attack"),

	DISAMBIGUATED_SFX(SfxId::Radio, "misc/chat"),

	DISAMBIGUATED_SFX(SfxId::Dgsit, "dog/sight"),

	DISAMBIGUATED_SFX(SfxId::Dgatk, "dog/attack"),

	DISAMBIGUATED_SFX(SfxId::Dgact, "dog/active"),

	DISAMBIGUATED_SFX(SfxId::Dgdth, "dog/death"),

	DISAMBIGUATED_SFX(SfxId::Dgpain, "dog/pain"),

	DISAMBIGUATED_SFX(SfxId::Secret, "misc/secret"),
};
