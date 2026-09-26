// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include "p_mobj.hpp"
#include "p_action.hpp"
#include "sounds.hpp"
#include "info.hpp"

const char* heretic_sprnames[std::to_underlying(SpriteId::HereticCount) + 1] = {
	"IMPX", "ACLO", "PTN1", "SHLD", "SHD2", "BAGH", "SPMP", "INVS", "PTN2", "SOAR",
	"INVU", "PWBK", "EGGC", "EGGM", "FX01", "SPHL", "TRCH", "FBMB", "XPL1", "ATLP",
	"PPOD", "AMG1", "SPSH", "LVAS", "SLDG", "SKH1", "SKH2", "SKH3", "SKH4", "CHDL",
	"SRTC", "SMPL", "STGS", "STGL", "STCS", "STCL", "KFR1", "BARL", "BRPL", "MOS1",
	"MOS2", "WTRH", "HCOR", "KGZ1", "KGZB", "KGZG", "KGZY", "VLCO", "VFBL", "VTFB",
	"SFFI", "TGLT", "TELE", "STFF", "PUF3", "PUF4", "BEAK", "WGNT", "GAUN", "PUF1",
	"WBLS", "BLSR", "FX18", "FX17", "WMCE", "MACE", "FX02", "WSKL", "HROD", "FX00",
	"FX20", "FX21", "FX22", "FX23", "GWND", "PUF2", "WPHX", "PHNX", "FX04", "FX08",
	"FX09", "WBOW", "CRBW", "FX03", "BLOD", "PLAY", "FDTH", "BSKL", "CHKN", "MUMM",
	"FX15", "BEAS", "FRB1", "SNKE", "SNFX", "HEAD", "FX05", "FX06", "FX07", "CLNK",
	"WZRD", "FX11", "FX10", "KNIG", "SPAX", "RAXE", "SRCR", "FX14", "SOR2", "SDTH",
	"FX16", "MNTR", "FX12", "FX13", "AKYY", "BKYY", "CKYY", "AMG2", "AMM1", "AMM2",
	"AMC1", "AMC2", "AMS1", "AMS2", "AMP1", "AMP2", "AMB1", "AMB2",
	nullptr
};

// The action functions do not share a parameter list, so each entry needs a cast.
#define DOOM_ACTION(a_function) reinterpret_cast<actionf_t>(a_function)

state_t heretic_states[std::to_underlying(StateId::HereticCount)] = {
	{SpriteId::HereticImpx, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_NULL
	{SpriteId::HereticAclo, 4, 1050, DOOM_ACTION(A_FreeTargMobj), StateId::HereticNull, 0, 0},                 // HERETIC_S_FREETARGMOBJ
	{SpriteId::HereticPtn1, 0, 3, nullptr, StateId::HereticItemPtn12, 0, 0},                       // HERETIC_S_ITEM_PTN1_1
	{SpriteId::HereticPtn1, 1, 3, nullptr, StateId::HereticItemPtn13, 0, 0},                       // HERETIC_S_ITEM_PTN1_2
	{SpriteId::HereticPtn1, 2, 3, nullptr, StateId::HereticItemPtn11, 0, 0},                       // HERETIC_S_ITEM_PTN1_3
	{SpriteId::HereticShld, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_ITEM_SHLD1
	{SpriteId::HereticShd2, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_ITEM_SHD2_1
	{SpriteId::HereticBagh, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_ITEM_BAGH1
	{SpriteId::HereticSpmp, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_ITEM_SPMP1
	{SpriteId::HereticAclo, 4, 1400, nullptr, StateId::HereticHidespecial2, 0, 0},                   // HERETIC_S_HIDESPECIAL1
	{SpriteId::HereticAclo, 0, 4, DOOM_ACTION(A_RestoreSpecialThing1), StateId::HereticHidespecial3, 0, 0},    // HERETIC_S_HIDESPECIAL2
	{SpriteId::HereticAclo, 1, 4, nullptr, StateId::HereticHidespecial4, 0, 0},                      // HERETIC_S_HIDESPECIAL3
	{SpriteId::HereticAclo, 0, 4, nullptr, StateId::HereticHidespecial5, 0, 0},                      // HERETIC_S_HIDESPECIAL4
	{SpriteId::HereticAclo, 1, 4, nullptr, StateId::HereticHidespecial6, 0, 0},                      // HERETIC_S_HIDESPECIAL5
	{SpriteId::HereticAclo, 2, 4, nullptr, StateId::HereticHidespecial7, 0, 0},                      // HERETIC_S_HIDESPECIAL6
	{SpriteId::HereticAclo, 1, 4, nullptr, StateId::HereticHidespecial8, 0, 0},                      // HERETIC_S_HIDESPECIAL7
	{SpriteId::HereticAclo, 2, 4, nullptr, StateId::HereticHidespecial9, 0, 0},                      // HERETIC_S_HIDESPECIAL8
	{SpriteId::HereticAclo, 3, 4, nullptr, StateId::HereticHidespecial10, 0, 0},                     // HERETIC_S_HIDESPECIAL9
	{SpriteId::HereticAclo, 2, 4, nullptr, StateId::HereticHidespecial11, 0, 0},                     // HERETIC_S_HIDESPECIAL10
	{SpriteId::HereticAclo, 3, 4, DOOM_ACTION(A_RestoreSpecialThing2), StateId::HereticNull, 0, 0},            // HERETIC_S_HIDESPECIAL11
	{SpriteId::HereticAclo, 3, 3, nullptr, StateId::HereticDormantarti2, 0, 0},                      // HERETIC_S_DORMANTARTI1
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti3, 0, 0},                      // HERETIC_S_DORMANTARTI2
	{SpriteId::HereticAclo, 3, 3, nullptr, StateId::HereticDormantarti4, 0, 0},                      // HERETIC_S_DORMANTARTI3
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti5, 0, 0},                      // HERETIC_S_DORMANTARTI4
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti6, 0, 0},                      // HERETIC_S_DORMANTARTI5
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti7, 0, 0},                      // HERETIC_S_DORMANTARTI6
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti8, 0, 0},                      // HERETIC_S_DORMANTARTI7
	{SpriteId::HereticAclo, 0, 3, nullptr, StateId::HereticDormantarti9, 0, 0},                      // HERETIC_S_DORMANTARTI8
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti10, 0, 0},                     // HERETIC_S_DORMANTARTI9
	{SpriteId::HereticAclo, 0, 3, nullptr, StateId::HereticDormantarti11, 0, 0},                     // HERETIC_S_DORMANTARTI10
	{SpriteId::HereticAclo, 0, 1400, DOOM_ACTION(A_HideThing), StateId::HereticDormantarti12, 0, 0},           // HERETIC_S_DORMANTARTI11
	{SpriteId::HereticAclo, 0, 3, DOOM_ACTION(A_UnHideThing), StateId::HereticDormantarti13, 0, 0},            // HERETIC_S_DORMANTARTI12
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti14, 0, 0},                     // HERETIC_S_DORMANTARTI13
	{SpriteId::HereticAclo, 0, 3, nullptr, StateId::HereticDormantarti15, 0, 0},                     // HERETIC_S_DORMANTARTI14
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti16, 0, 0},                     // HERETIC_S_DORMANTARTI15
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti17, 0, 0},                     // HERETIC_S_DORMANTARTI16
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDormantarti18, 0, 0},                     // HERETIC_S_DORMANTARTI17
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti19, 0, 0},                     // HERETIC_S_DORMANTARTI18
	{SpriteId::HereticAclo, 3, 3, nullptr, StateId::HereticDormantarti20, 0, 0},                     // HERETIC_S_DORMANTARTI19
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDormantarti21, 0, 0},                     // HERETIC_S_DORMANTARTI20
	{SpriteId::HereticAclo, 3, 3, DOOM_ACTION(A_RestoreArtifact), StateId::HereticNull, 0, 0},                 // HERETIC_S_DORMANTARTI21
	{SpriteId::HereticAclo, 3, 3, nullptr, StateId::HereticDeadarti2, 0, 0},                         // HERETIC_S_DEADARTI1
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDeadarti3, 0, 0},                         // HERETIC_S_DEADARTI2
	{SpriteId::HereticAclo, 3, 3, nullptr, StateId::HereticDeadarti4, 0, 0},                         // HERETIC_S_DEADARTI3
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDeadarti5, 0, 0},                         // HERETIC_S_DEADARTI4
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDeadarti6, 0, 0},                         // HERETIC_S_DEADARTI5
	{SpriteId::HereticAclo, 2, 3, nullptr, StateId::HereticDeadarti7, 0, 0},                         // HERETIC_S_DEADARTI6
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDeadarti8, 0, 0},                         // HERETIC_S_DEADARTI7
	{SpriteId::HereticAclo, 0, 3, nullptr, StateId::HereticDeadarti9, 0, 0},                         // HERETIC_S_DEADARTI8
	{SpriteId::HereticAclo, 1, 3, nullptr, StateId::HereticDeadarti10, 0, 0},                        // HERETIC_S_DEADARTI9
	{SpriteId::HereticAclo, 0, 3, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_DEADARTI10
	{SpriteId::HereticInvs, 32768, 350, nullptr, StateId::HereticArtiInvs1, 0, 0},                  // HERETIC_S_ARTI_INVS1
	{SpriteId::HereticPtn2, 0, 4, nullptr, StateId::HereticArtiPtn22, 0, 0},                       // HERETIC_S_ARTI_PTN2_1
	{SpriteId::HereticPtn2, 1, 4, nullptr, StateId::HereticArtiPtn23, 0, 0},                       // HERETIC_S_ARTI_PTN2_2
	{SpriteId::HereticPtn2, 2, 4, nullptr, StateId::HereticArtiPtn21, 0, 0},                       // HERETIC_S_ARTI_PTN2_3
	{SpriteId::HereticSoar, 0, 5, nullptr, StateId::HereticArtiSoar2, 0, 0},                        // HERETIC_S_ARTI_SOAR1
	{SpriteId::HereticSoar, 1, 5, nullptr, StateId::HereticArtiSoar3, 0, 0},                        // HERETIC_S_ARTI_SOAR2
	{SpriteId::HereticSoar, 2, 5, nullptr, StateId::HereticArtiSoar4, 0, 0},                        // HERETIC_S_ARTI_SOAR3
	{SpriteId::HereticSoar, 1, 5, nullptr, StateId::HereticArtiSoar1, 0, 0},                        // HERETIC_S_ARTI_SOAR4
	{SpriteId::HereticInvu, 0, 3, nullptr, StateId::HereticArtiInvu2, 0, 0},                        // HERETIC_S_ARTI_INVU1
	{SpriteId::HereticInvu, 1, 3, nullptr, StateId::HereticArtiInvu3, 0, 0},                        // HERETIC_S_ARTI_INVU2
	{SpriteId::HereticInvu, 2, 3, nullptr, StateId::HereticArtiInvu4, 0, 0},                        // HERETIC_S_ARTI_INVU3
	{SpriteId::HereticInvu, 3, 3, nullptr, StateId::HereticArtiInvu1, 0, 0},                        // HERETIC_S_ARTI_INVU4
	{SpriteId::HereticPwbk, 0, 350, nullptr, StateId::HereticArtiPwbk1, 0, 0},                      // HERETIC_S_ARTI_PWBK1
	{SpriteId::HereticEggc, 0, 6, nullptr, StateId::HereticArtiEggc2, 0, 0},                        // HERETIC_S_ARTI_EGGC1
	{SpriteId::HereticEggc, 1, 6, nullptr, StateId::HereticArtiEggc3, 0, 0},                        // HERETIC_S_ARTI_EGGC2
	{SpriteId::HereticEggc, 2, 6, nullptr, StateId::HereticArtiEggc4, 0, 0},                        // HERETIC_S_ARTI_EGGC3
	{SpriteId::HereticEggc, 1, 6, nullptr, StateId::HereticArtiEggc1, 0, 0},                        // HERETIC_S_ARTI_EGGC4
	{SpriteId::HereticEggm, 0, 4, nullptr, StateId::HereticEggfx2, 0, 0},                            // HERETIC_S_EGGFX1
	{SpriteId::HereticEggm, 1, 4, nullptr, StateId::HereticEggfx3, 0, 0},                            // HERETIC_S_EGGFX2
	{SpriteId::HereticEggm, 2, 4, nullptr, StateId::HereticEggfx4, 0, 0},                            // HERETIC_S_EGGFX3
	{SpriteId::HereticEggm, 3, 4, nullptr, StateId::HereticEggfx5, 0, 0},                            // HERETIC_S_EGGFX4
	{SpriteId::HereticEggm, 4, 4, nullptr, StateId::HereticEggfx1, 0, 0},                            // HERETIC_S_EGGFX5
	{SpriteId::HereticFx01, 32772, 3, nullptr, StateId::HereticEggfxi12, 0, 0},                     // HERETIC_S_EGGFXI1_1
	{SpriteId::HereticFx01, 32773, 3, nullptr, StateId::HereticEggfxi13, 0, 0},                     // HERETIC_S_EGGFXI1_2
	{SpriteId::HereticFx01, 32774, 3, nullptr, StateId::HereticEggfxi14, 0, 0},                     // HERETIC_S_EGGFXI1_3
	{SpriteId::HereticFx01, 32775, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_EGGFXI1_4
	{SpriteId::HereticSphl, 0, 350, nullptr, StateId::HereticArtiSphl1, 0, 0},                      // HERETIC_S_ARTI_SPHL1
	{SpriteId::HereticTrch, 32768, 3, nullptr, StateId::HereticArtiTrch2, 0, 0},                    // HERETIC_S_ARTI_TRCH1
	{SpriteId::HereticTrch, 32769, 3, nullptr, StateId::HereticArtiTrch3, 0, 0},                    // HERETIC_S_ARTI_TRCH2
	{SpriteId::HereticTrch, 32770, 3, nullptr, StateId::HereticArtiTrch1, 0, 0},                    // HERETIC_S_ARTI_TRCH3
	{SpriteId::HereticFbmb, 4, 350, nullptr, StateId::HereticArtiFbmb1, 0, 0},                      // HERETIC_S_ARTI_FBMB1
	{SpriteId::HereticFbmb, 0, 10, nullptr, StateId::HereticFirebomb2, 0, 0},                        // HERETIC_S_FIREBOMB1
	{SpriteId::HereticFbmb, 1, 10, nullptr, StateId::HereticFirebomb3, 0, 0},                        // HERETIC_S_FIREBOMB2
	{SpriteId::HereticFbmb, 2, 10, nullptr, StateId::HereticFirebomb4, 0, 0},                        // HERETIC_S_FIREBOMB3
	{SpriteId::HereticFbmb, 3, 10, nullptr, StateId::HereticFirebomb5, 0, 0},                        // HERETIC_S_FIREBOMB4
	{SpriteId::HereticFbmb, 4, 6, DOOM_ACTION(A_Scream), StateId::HereticFirebomb6, 0, 0},                     // HERETIC_S_FIREBOMB5
	{SpriteId::HereticXpl1, 32768, 4, DOOM_ACTION(A_Explode), StateId::HereticFirebomb7, 0, 0},                // HERETIC_S_FIREBOMB6
	{SpriteId::HereticXpl1, 32769, 4, nullptr, StateId::HereticFirebomb8, 0, 0},                     // HERETIC_S_FIREBOMB7
	{SpriteId::HereticXpl1, 32770, 4, nullptr, StateId::HereticFirebomb9, 0, 0},                     // HERETIC_S_FIREBOMB8
	{SpriteId::HereticXpl1, 32771, 4, nullptr, StateId::HereticFirebomb10, 0, 0},                    // HERETIC_S_FIREBOMB9
	{SpriteId::HereticXpl1, 32772, 4, nullptr, StateId::HereticFirebomb11, 0, 0},                    // HERETIC_S_FIREBOMB10
	{SpriteId::HereticXpl1, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_FIREBOMB11
	{SpriteId::HereticAtlp, 0, 4, nullptr, StateId::HereticArtiAtlp2, 0, 0},                        // HERETIC_S_ARTI_ATLP1
	{SpriteId::HereticAtlp, 1, 4, nullptr, StateId::HereticArtiAtlp3, 0, 0},                        // HERETIC_S_ARTI_ATLP2
	{SpriteId::HereticAtlp, 2, 4, nullptr, StateId::HereticArtiAtlp4, 0, 0},                        // HERETIC_S_ARTI_ATLP3
	{SpriteId::HereticAtlp, 1, 4, nullptr, StateId::HereticArtiAtlp1, 0, 0},                        // HERETIC_S_ARTI_ATLP4
	{SpriteId::HereticPpod, 0, 10, nullptr, StateId::HereticPodWait1, 0, 0},                        // HERETIC_S_POD_WAIT1
	{SpriteId::HereticPpod, 1, 14, DOOM_ACTION(A_PodPain), StateId::HereticPodWait1, 0, 0},                   // HERETIC_S_POD_PAIN1
	{SpriteId::HereticPpod, 32770, 5, DOOM_ACTION(A_RemovePod), StateId::HereticPodDie2, 0, 0},               // HERETIC_S_POD_DIE1
	{SpriteId::HereticPpod, 32771, 5, DOOM_ACTION(A_Scream), StateId::HereticPodDie3, 0, 0},                  // HERETIC_S_POD_DIE2
	{SpriteId::HereticPpod, 32772, 5, DOOM_ACTION(A_Explode), StateId::HereticPodDie4, 0, 0},                 // HERETIC_S_POD_DIE3
	{SpriteId::HereticPpod, 32773, 10, nullptr, StateId::HereticFreetargmobj, 0, 0},                 // HERETIC_S_POD_DIE4
	{SpriteId::HereticPpod, 8, 3, nullptr, StateId::HereticPodGrow2, 0, 0},                         // HERETIC_S_POD_GROW1
	{SpriteId::HereticPpod, 9, 3, nullptr, StateId::HereticPodGrow3, 0, 0},                         // HERETIC_S_POD_GROW2
	{SpriteId::HereticPpod, 10, 3, nullptr, StateId::HereticPodGrow4, 0, 0},                        // HERETIC_S_POD_GROW3
	{SpriteId::HereticPpod, 11, 3, nullptr, StateId::HereticPodGrow5, 0, 0},                        // HERETIC_S_POD_GROW4
	{SpriteId::HereticPpod, 12, 3, nullptr, StateId::HereticPodGrow6, 0, 0},                        // HERETIC_S_POD_GROW5
	{SpriteId::HereticPpod, 13, 3, nullptr, StateId::HereticPodGrow7, 0, 0},                        // HERETIC_S_POD_GROW6
	{SpriteId::HereticPpod, 14, 3, nullptr, StateId::HereticPodGrow8, 0, 0},                        // HERETIC_S_POD_GROW7
	{SpriteId::HereticPpod, 15, 3, nullptr, StateId::HereticPodWait1, 0, 0},                        // HERETIC_S_POD_GROW8
	{SpriteId::HereticPpod, 6, 8, nullptr, StateId::HereticPodgoo2, 0, 0},                           // HERETIC_S_PODGOO1
	{SpriteId::HereticPpod, 7, 8, nullptr, StateId::HereticPodgoo1, 0, 0},                           // HERETIC_S_PODGOO2
	{SpriteId::HereticPpod, 6, 10, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_PODGOOX
	{SpriteId::HereticAmg1, 0, 35, DOOM_ACTION(A_MakePod), StateId::HereticPodgenerator, 0, 0},                // HERETIC_S_PODGENERATOR
	{SpriteId::HereticSpsh, 0, 8, nullptr, StateId::HereticSplash2, 0, 0},                           // HERETIC_S_SPLASH1
	{SpriteId::HereticSpsh, 1, 8, nullptr, StateId::HereticSplash3, 0, 0},                           // HERETIC_S_SPLASH2
	{SpriteId::HereticSpsh, 2, 8, nullptr, StateId::HereticSplash4, 0, 0},                           // HERETIC_S_SPLASH3
	{SpriteId::HereticSpsh, 3, 16, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SPLASH4
	{SpriteId::HereticSpsh, 3, 10, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SPLASHX
	{SpriteId::HereticSpsh, 4, 5, nullptr, StateId::HereticSplashbase2, 0, 0},                       // HERETIC_S_SPLASHBASE1
	{SpriteId::HereticSpsh, 5, 5, nullptr, StateId::HereticSplashbase3, 0, 0},                       // HERETIC_S_SPLASHBASE2
	{SpriteId::HereticSpsh, 6, 5, nullptr, StateId::HereticSplashbase4, 0, 0},                       // HERETIC_S_SPLASHBASE3
	{SpriteId::HereticSpsh, 7, 5, nullptr, StateId::HereticSplashbase5, 0, 0},                       // HERETIC_S_SPLASHBASE4
	{SpriteId::HereticSpsh, 8, 5, nullptr, StateId::HereticSplashbase6, 0, 0},                       // HERETIC_S_SPLASHBASE5
	{SpriteId::HereticSpsh, 9, 5, nullptr, StateId::HereticSplashbase7, 0, 0},                       // HERETIC_S_SPLASHBASE6
	{SpriteId::HereticSpsh, 10, 5, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SPLASHBASE7
	{SpriteId::HereticLvas, 32768, 5, nullptr, StateId::HereticLavasplash2, 0, 0},                   // HERETIC_S_LAVASPLASH1
	{SpriteId::HereticLvas, 32769, 5, nullptr, StateId::HereticLavasplash3, 0, 0},                   // HERETIC_S_LAVASPLASH2
	{SpriteId::HereticLvas, 32770, 5, nullptr, StateId::HereticLavasplash4, 0, 0},                   // HERETIC_S_LAVASPLASH3
	{SpriteId::HereticLvas, 32771, 5, nullptr, StateId::HereticLavasplash5, 0, 0},                   // HERETIC_S_LAVASPLASH4
	{SpriteId::HereticLvas, 32772, 5, nullptr, StateId::HereticLavasplash6, 0, 0},                   // HERETIC_S_LAVASPLASH5
	{SpriteId::HereticLvas, 32773, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_LAVASPLASH6
	{SpriteId::HereticLvas, 32774, 5, nullptr, StateId::HereticLavasmoke2, 0, 0},                    // HERETIC_S_LAVASMOKE1
	{SpriteId::HereticLvas, 32775, 5, nullptr, StateId::HereticLavasmoke3, 0, 0},                    // HERETIC_S_LAVASMOKE2
	{SpriteId::HereticLvas, 32776, 5, nullptr, StateId::HereticLavasmoke4, 0, 0},                    // HERETIC_S_LAVASMOKE3
	{SpriteId::HereticLvas, 32777, 5, nullptr, StateId::HereticLavasmoke5, 0, 0},                    // HERETIC_S_LAVASMOKE4
	{SpriteId::HereticLvas, 32778, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_LAVASMOKE5
	{SpriteId::HereticSldg, 0, 8, nullptr, StateId::HereticSludgechunk2, 0, 0},                      // HERETIC_S_SLUDGECHUNK1
	{SpriteId::HereticSldg, 1, 8, nullptr, StateId::HereticSludgechunk3, 0, 0},                      // HERETIC_S_SLUDGECHUNK2
	{SpriteId::HereticSldg, 2, 8, nullptr, StateId::HereticSludgechunk4, 0, 0},                      // HERETIC_S_SLUDGECHUNK3
	{SpriteId::HereticSldg, 3, 8, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_SLUDGECHUNK4
	{SpriteId::HereticSldg, 3, 6, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_SLUDGECHUNKX
	{SpriteId::HereticSldg, 4, 5, nullptr, StateId::HereticSludgesplash2, 0, 0},                     // HERETIC_S_SLUDGESPLASH1
	{SpriteId::HereticSldg, 5, 5, nullptr, StateId::HereticSludgesplash3, 0, 0},                     // HERETIC_S_SLUDGESPLASH2
	{SpriteId::HereticSldg, 6, 5, nullptr, StateId::HereticSludgesplash4, 0, 0},                     // HERETIC_S_SLUDGESPLASH3
	{SpriteId::HereticSldg, 7, 5, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_SLUDGESPLASH4
	{SpriteId::HereticSkh1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SKULLHANG70_1
	{SpriteId::HereticSkh2, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SKULLHANG60_1
	{SpriteId::HereticSkh3, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SKULLHANG45_1
	{SpriteId::HereticSkh4, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SKULLHANG35_1
	{SpriteId::HereticChdl, 0, 4, nullptr, StateId::HereticChandelier2, 0, 0},                       // HERETIC_S_CHANDELIER1
	{SpriteId::HereticChdl, 1, 4, nullptr, StateId::HereticChandelier3, 0, 0},                       // HERETIC_S_CHANDELIER2
	{SpriteId::HereticChdl, 2, 4, nullptr, StateId::HereticChandelier1, 0, 0},                       // HERETIC_S_CHANDELIER3
	{SpriteId::HereticSrtc, 0, 4, nullptr, StateId::HereticSerptorch2, 0, 0},                        // HERETIC_S_SERPTORCH1
	{SpriteId::HereticSrtc, 1, 4, nullptr, StateId::HereticSerptorch3, 0, 0},                        // HERETIC_S_SERPTORCH2
	{SpriteId::HereticSrtc, 2, 4, nullptr, StateId::HereticSerptorch1, 0, 0},                        // HERETIC_S_SERPTORCH3
	{SpriteId::HereticSmpl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SMALLPILLAR
	{SpriteId::HereticStgs, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_STALAGMITESMALL
	{SpriteId::HereticStgl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_STALAGMITELARGE
	{SpriteId::HereticStcs, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_STALACTITESMALL
	{SpriteId::HereticStcl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_STALACTITELARGE
	{SpriteId::HereticKfr1, 32768, 3, nullptr, StateId::HereticFirebrazier2, 0, 0},                  // HERETIC_S_FIREBRAZIER1
	{SpriteId::HereticKfr1, 32769, 3, nullptr, StateId::HereticFirebrazier3, 0, 0},                  // HERETIC_S_FIREBRAZIER2
	{SpriteId::HereticKfr1, 32770, 3, nullptr, StateId::HereticFirebrazier4, 0, 0},                  // HERETIC_S_FIREBRAZIER3
	{SpriteId::HereticKfr1, 32771, 3, nullptr, StateId::HereticFirebrazier5, 0, 0},                  // HERETIC_S_FIREBRAZIER4
	{SpriteId::HereticKfr1, 32772, 3, nullptr, StateId::HereticFirebrazier6, 0, 0},                  // HERETIC_S_FIREBRAZIER5
	{SpriteId::HereticKfr1, 32773, 3, nullptr, StateId::HereticFirebrazier7, 0, 0},                  // HERETIC_S_FIREBRAZIER6
	{SpriteId::HereticKfr1, 32774, 3, nullptr, StateId::HereticFirebrazier8, 0, 0},                  // HERETIC_S_FIREBRAZIER7
	{SpriteId::HereticKfr1, 32775, 3, nullptr, StateId::HereticFirebrazier1, 0, 0},                  // HERETIC_S_FIREBRAZIER8
	{SpriteId::HereticBarl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_BARREL
	{SpriteId::HereticBrpl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_BRPILLAR
	{SpriteId::HereticMos1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_MOSS1
	{SpriteId::HereticMos2, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_MOSS2
	{SpriteId::HereticWtrh, 32768, 6, nullptr, StateId::HereticWalltorch2, 0, 0},                    // HERETIC_S_WALLTORCH1
	{SpriteId::HereticWtrh, 32769, 6, nullptr, StateId::HereticWalltorch3, 0, 0},                    // HERETIC_S_WALLTORCH2
	{SpriteId::HereticWtrh, 32770, 6, nullptr, StateId::HereticWalltorch1, 0, 0},                    // HERETIC_S_WALLTORCH3
	{SpriteId::HereticHcor, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_HANGINGCORPSE
	{SpriteId::HereticKgz1, 0, 1, nullptr, StateId::HereticKeygizmo2, 0, 0},                         // HERETIC_S_KEYGIZMO1
	{SpriteId::HereticKgz1, 0, 1, DOOM_ACTION(A_InitKeyGizmo), StateId::HereticKeygizmo3, 0, 0},               // HERETIC_S_KEYGIZMO2
	{SpriteId::HereticKgz1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_KEYGIZMO3
	{SpriteId::HereticKgzb, 0, 1, nullptr, StateId::HereticKgzStart, 0, 0},                         // HERETIC_S_KGZ_START
	{SpriteId::HereticKgzb, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_KGZ_BLUEFLOAT1
	{SpriteId::HereticKgzg, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_KGZ_GREENFLOAT1
	{SpriteId::HereticKgzy, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_KGZ_YELLOWFLOAT1
	{SpriteId::HereticVlco, 0, 350, nullptr, StateId::HereticVolcano2, 0, 0},                        // HERETIC_S_VOLCANO1
	{SpriteId::HereticVlco, 0, 35, DOOM_ACTION(A_VolcanoSet), StateId::HereticVolcano3, 0, 0},                 // HERETIC_S_VOLCANO2
	{SpriteId::HereticVlco, 1, 3, nullptr, StateId::HereticVolcano4, 0, 0},                          // HERETIC_S_VOLCANO3
	{SpriteId::HereticVlco, 2, 3, nullptr, StateId::HereticVolcano5, 0, 0},                          // HERETIC_S_VOLCANO4
	{SpriteId::HereticVlco, 3, 3, nullptr, StateId::HereticVolcano6, 0, 0},                          // HERETIC_S_VOLCANO5
	{SpriteId::HereticVlco, 1, 3, nullptr, StateId::HereticVolcano7, 0, 0},                          // HERETIC_S_VOLCANO6
	{SpriteId::HereticVlco, 2, 3, nullptr, StateId::HereticVolcano8, 0, 0},                          // HERETIC_S_VOLCANO7
	{SpriteId::HereticVlco, 3, 3, nullptr, StateId::HereticVolcano9, 0, 0},                          // HERETIC_S_VOLCANO8
	{SpriteId::HereticVlco, 4, 10, DOOM_ACTION(A_VolcanoBlast), StateId::HereticVolcano2, 0, 0},               // HERETIC_S_VOLCANO9
	{SpriteId::HereticVfbl, 0, 4, DOOM_ACTION(A_BeastPuff), StateId::HereticVolcanoball2, 0, 0},               // HERETIC_S_VOLCANOBALL1
	{SpriteId::HereticVfbl, 1, 4, DOOM_ACTION(A_BeastPuff), StateId::HereticVolcanoball1, 0, 0},               // HERETIC_S_VOLCANOBALL2
	{SpriteId::HereticXpl1, 0, 4, DOOM_ACTION(A_VolcBallImpact), StateId::HereticVolcanoballx2, 0, 0},         // HERETIC_S_VOLCANOBALLX1
	{SpriteId::HereticXpl1, 1, 4, nullptr, StateId::HereticVolcanoballx3, 0, 0},                     // HERETIC_S_VOLCANOBALLX2
	{SpriteId::HereticXpl1, 2, 4, nullptr, StateId::HereticVolcanoballx4, 0, 0},                     // HERETIC_S_VOLCANOBALLX3
	{SpriteId::HereticXpl1, 3, 4, nullptr, StateId::HereticVolcanoballx5, 0, 0},                     // HERETIC_S_VOLCANOBALLX4
	{SpriteId::HereticXpl1, 4, 4, nullptr, StateId::HereticVolcanoballx6, 0, 0},                     // HERETIC_S_VOLCANOBALLX5
	{SpriteId::HereticXpl1, 5, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_VOLCANOBALLX6
	{SpriteId::HereticVtfb, 0, 4, nullptr, StateId::HereticVolcanotball2, 0, 0},                     // HERETIC_S_VOLCANOTBALL1
	{SpriteId::HereticVtfb, 1, 4, nullptr, StateId::HereticVolcanotball1, 0, 0},                     // HERETIC_S_VOLCANOTBALL2
	{SpriteId::HereticSffi, 2, 4, nullptr, StateId::HereticVolcanotballx2, 0, 0},                    // HERETIC_S_VOLCANOTBALLX1
	{SpriteId::HereticSffi, 1, 4, nullptr, StateId::HereticVolcanotballx3, 0, 0},                    // HERETIC_S_VOLCANOTBALLX2
	{SpriteId::HereticSffi, 0, 4, nullptr, StateId::HereticVolcanotballx4, 0, 0},                    // HERETIC_S_VOLCANOTBALLX3
	{SpriteId::HereticSffi, 1, 4, nullptr, StateId::HereticVolcanotballx5, 0, 0},                    // HERETIC_S_VOLCANOTBALLX4
	{SpriteId::HereticSffi, 2, 4, nullptr, StateId::HereticVolcanotballx6, 0, 0},                    // HERETIC_S_VOLCANOTBALLX5
	{SpriteId::HereticSffi, 3, 4, nullptr, StateId::HereticVolcanotballx7, 0, 0},                    // HERETIC_S_VOLCANOTBALLX6
	{SpriteId::HereticSffi, 4, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_VOLCANOTBALLX7
	{SpriteId::HereticTglt, 0, 8, DOOM_ACTION(A_SpawnTeleGlitter), StateId::HereticTeleglitgen1, 0, 0},        // HERETIC_S_TELEGLITGEN1
	{SpriteId::HereticTglt, 5, 8, DOOM_ACTION(A_SpawnTeleGlitter2), StateId::HereticTeleglitgen2, 0, 0},       // HERETIC_S_TELEGLITGEN2
	{SpriteId::HereticTglt, 32768, 2, nullptr, StateId::HereticTeleglitter12, 0, 0},                // HERETIC_S_TELEGLITTER1_1
	{SpriteId::HereticTglt, 32769, 2, DOOM_ACTION(A_AccTeleGlitter), StateId::HereticTeleglitter13, 0, 0},    // HERETIC_S_TELEGLITTER1_2
	{SpriteId::HereticTglt, 32770, 2, nullptr, StateId::HereticTeleglitter14, 0, 0},                // HERETIC_S_TELEGLITTER1_3
	{SpriteId::HereticTglt, 32771, 2, DOOM_ACTION(A_AccTeleGlitter), StateId::HereticTeleglitter15, 0, 0},    // HERETIC_S_TELEGLITTER1_4
	{SpriteId::HereticTglt, 32772, 2, nullptr, StateId::HereticTeleglitter11, 0, 0},                // HERETIC_S_TELEGLITTER1_5
	{SpriteId::HereticTglt, 32773, 2, nullptr, StateId::HereticTeleglitter22, 0, 0},                // HERETIC_S_TELEGLITTER2_1
	{SpriteId::HereticTglt, 32774, 2, DOOM_ACTION(A_AccTeleGlitter), StateId::HereticTeleglitter23, 0, 0},    // HERETIC_S_TELEGLITTER2_2
	{SpriteId::HereticTglt, 32775, 2, nullptr, StateId::HereticTeleglitter24, 0, 0},                // HERETIC_S_TELEGLITTER2_3
	{SpriteId::HereticTglt, 32776, 2, DOOM_ACTION(A_AccTeleGlitter), StateId::HereticTeleglitter25, 0, 0},    // HERETIC_S_TELEGLITTER2_4
	{SpriteId::HereticTglt, 32777, 2, nullptr, StateId::HereticTeleglitter21, 0, 0},                // HERETIC_S_TELEGLITTER2_5
	{SpriteId::HereticTele, 32768, 6, nullptr, StateId::HereticTfog2, 0, 0},                         // HERETIC_S_TFOG1
	{SpriteId::HereticTele, 32769, 6, nullptr, StateId::HereticTfog3, 0, 0},                         // HERETIC_S_TFOG2
	{SpriteId::HereticTele, 32770, 6, nullptr, StateId::HereticTfog4, 0, 0},                         // HERETIC_S_TFOG3
	{SpriteId::HereticTele, 32771, 6, nullptr, StateId::HereticTfog5, 0, 0},                         // HERETIC_S_TFOG4
	{SpriteId::HereticTele, 32772, 6, nullptr, StateId::HereticTfog6, 0, 0},                         // HERETIC_S_TFOG5
	{SpriteId::HereticTele, 32773, 6, nullptr, StateId::HereticTfog7, 0, 0},                         // HERETIC_S_TFOG6
	{SpriteId::HereticTele, 32774, 6, nullptr, StateId::HereticTfog8, 0, 0},                         // HERETIC_S_TFOG7
	{SpriteId::HereticTele, 32775, 6, nullptr, StateId::HereticTfog9, 0, 0},                         // HERETIC_S_TFOG8
	{SpriteId::HereticTele, 32774, 6, nullptr, StateId::HereticTfog10, 0, 0},                        // HERETIC_S_TFOG9
	{SpriteId::HereticTele, 32773, 6, nullptr, StateId::HereticTfog11, 0, 0},                        // HERETIC_S_TFOG10
	{SpriteId::HereticTele, 32772, 6, nullptr, StateId::HereticTfog12, 0, 0},                        // HERETIC_S_TFOG11
	{SpriteId::HereticTele, 32771, 6, nullptr, StateId::HereticTfog13, 0, 0},                        // HERETIC_S_TFOG12
	{SpriteId::HereticTele, 32770, 6, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_TFOG13
	{SpriteId::HereticStff, 0, 0, DOOM_ACTION(A_Light0), StateId::HereticNull, 0, 0},                          // HERETIC_S_LIGHTDONE
	{SpriteId::HereticStff, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticStaffready, 0, 0},               // HERETIC_S_STAFFREADY
	{SpriteId::HereticStff, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticStaffdown, 0, 0},                      // HERETIC_S_STAFFDOWN
	{SpriteId::HereticStff, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticStaffup, 0, 0},                        // HERETIC_S_STAFFUP
	{SpriteId::HereticStff, 3, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticStaffready22, 0, 0},            // HERETIC_S_STAFFREADY2_1
	{SpriteId::HereticStff, 4, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticStaffready23, 0, 0},            // HERETIC_S_STAFFREADY2_2
	{SpriteId::HereticStff, 5, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticStaffready21, 0, 0},            // HERETIC_S_STAFFREADY2_3
	{SpriteId::HereticStff, 3, 1, DOOM_ACTION(A_Lower), StateId::HereticStaffdown2, 0, 0},                     // HERETIC_S_STAFFDOWN2
	{SpriteId::HereticStff, 3, 1, DOOM_ACTION(A_Raise), StateId::HereticStaffup2, 0, 0},                       // HERETIC_S_STAFFUP2
	{SpriteId::HereticStff, 1, 6, nullptr, StateId::HereticStaffatk12, 0, 0},                       // HERETIC_S_STAFFATK1_1
	{SpriteId::HereticStff, 2, 8, DOOM_ACTION(A_StaffAttackPL1), StateId::HereticStaffatk13, 0, 0},           // HERETIC_S_STAFFATK1_2
	{SpriteId::HereticStff, 1, 8, DOOM_ACTION(A_ReFire), StateId::HereticStaffready, 0, 0},                    // HERETIC_S_STAFFATK1_3
	{SpriteId::HereticStff, 6, 6, nullptr, StateId::HereticStaffatk22, 0, 0},                       // HERETIC_S_STAFFATK2_1
	{SpriteId::HereticStff, 7, 8, DOOM_ACTION(A_StaffAttackPL2), StateId::HereticStaffatk23, 0, 0},           // HERETIC_S_STAFFATK2_2
	{SpriteId::HereticStff, 6, 8, DOOM_ACTION(A_ReFire), StateId::HereticStaffready21, 0, 0},                 // HERETIC_S_STAFFATK2_3
	{SpriteId::HereticPuf3, 32768, 4, nullptr, StateId::HereticStaffpuff2, 0, 0},                    // HERETIC_S_STAFFPUFF1
	{SpriteId::HereticPuf3, 1, 4, nullptr, StateId::HereticStaffpuff3, 0, 0},                        // HERETIC_S_STAFFPUFF2
	{SpriteId::HereticPuf3, 2, 4, nullptr, StateId::HereticStaffpuff4, 0, 0},                        // HERETIC_S_STAFFPUFF3
	{SpriteId::HereticPuf3, 3, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_STAFFPUFF4
	{SpriteId::HereticPuf4, 32768, 4, nullptr, StateId::HereticStaffpuff22, 0, 0},                  // HERETIC_S_STAFFPUFF2_1
	{SpriteId::HereticPuf4, 32769, 4, nullptr, StateId::HereticStaffpuff23, 0, 0},                  // HERETIC_S_STAFFPUFF2_2
	{SpriteId::HereticPuf4, 32770, 4, nullptr, StateId::HereticStaffpuff24, 0, 0},                  // HERETIC_S_STAFFPUFF2_3
	{SpriteId::HereticPuf4, 32771, 4, nullptr, StateId::HereticStaffpuff25, 0, 0},                  // HERETIC_S_STAFFPUFF2_4
	{SpriteId::HereticPuf4, 32772, 4, nullptr, StateId::HereticStaffpuff26, 0, 0},                  // HERETIC_S_STAFFPUFF2_5
	{SpriteId::HereticPuf4, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_STAFFPUFF2_6
	{SpriteId::HereticBeak, 0, 1, DOOM_ACTION(A_BeakReady), StateId::HereticBeakready, 0, 0},                  // HERETIC_S_BEAKREADY
	{SpriteId::HereticBeak, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticBeakdown, 0, 0},                       // HERETIC_S_BEAKDOWN
	{SpriteId::HereticBeak, 0, 1, DOOM_ACTION(A_BeakRaise), StateId::HereticBeakup, 0, 0},                     // HERETIC_S_BEAKUP
	{SpriteId::HereticBeak, 0, 18, DOOM_ACTION(A_BeakAttackPL1), StateId::HereticBeakready, 0, 0},             // HERETIC_S_BEAKATK1_1
	{SpriteId::HereticBeak, 0, 12, DOOM_ACTION(A_BeakAttackPL2), StateId::HereticBeakready, 0, 0},             // HERETIC_S_BEAKATK2_1
	{SpriteId::HereticWgnt, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_WGNT
	{SpriteId::HereticGaun, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticGauntletready, 0, 0},            // HERETIC_S_GAUNTLETREADY
	{SpriteId::HereticGaun, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticGauntletdown, 0, 0},                   // HERETIC_S_GAUNTLETDOWN
	{SpriteId::HereticGaun, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticGauntletup, 0, 0},                     // HERETIC_S_GAUNTLETUP
	{SpriteId::HereticGaun, 6, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticGauntletready22, 0, 0},         // HERETIC_S_GAUNTLETREADY2_1
	{SpriteId::HereticGaun, 7, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticGauntletready23, 0, 0},         // HERETIC_S_GAUNTLETREADY2_2
	{SpriteId::HereticGaun, 8, 4, DOOM_ACTION(A_WeaponReady), StateId::HereticGauntletready21, 0, 0},         // HERETIC_S_GAUNTLETREADY2_3
	{SpriteId::HereticGaun, 6, 1, DOOM_ACTION(A_Lower), StateId::HereticGauntletdown2, 0, 0},                  // HERETIC_S_GAUNTLETDOWN2
	{SpriteId::HereticGaun, 6, 1, DOOM_ACTION(A_Raise), StateId::HereticGauntletup2, 0, 0},                    // HERETIC_S_GAUNTLETUP2
	{SpriteId::HereticGaun, 1, 4, nullptr, StateId::HereticGauntletatk12, 0, 0},                    // HERETIC_S_GAUNTLETATK1_1
	{SpriteId::HereticGaun, 2, 4, nullptr, StateId::HereticGauntletatk13, 0, 0},                    // HERETIC_S_GAUNTLETATK1_2
	{SpriteId::HereticGaun, 32771, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk14, 0, 0},    // HERETIC_S_GAUNTLETATK1_3
	{SpriteId::HereticGaun, 32772, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk15, 0, 0},    // HERETIC_S_GAUNTLETATK1_4
	{SpriteId::HereticGaun, 32773, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk16, 0, 0},    // HERETIC_S_GAUNTLETATK1_5
	{SpriteId::HereticGaun, 2, 4, DOOM_ACTION(A_ReFire), StateId::HereticGauntletatk17, 0, 0},                // HERETIC_S_GAUNTLETATK1_6
	{SpriteId::HereticGaun, 1, 4, DOOM_ACTION(A_Light0), StateId::HereticGauntletready, 0, 0},                 // HERETIC_S_GAUNTLETATK1_7
	{SpriteId::HereticGaun, 9, 4, nullptr, StateId::HereticGauntletatk22, 0, 0},                    // HERETIC_S_GAUNTLETATK2_1
	{SpriteId::HereticGaun, 10, 4, nullptr, StateId::HereticGauntletatk23, 0, 0},                   // HERETIC_S_GAUNTLETATK2_2
	{SpriteId::HereticGaun, 32779, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk24, 0, 0},    // HERETIC_S_GAUNTLETATK2_3
	{SpriteId::HereticGaun, 32780, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk25, 0, 0},    // HERETIC_S_GAUNTLETATK2_4
	{SpriteId::HereticGaun, 32781, 4, DOOM_ACTION(A_GauntletAttack), StateId::HereticGauntletatk26, 0, 0},    // HERETIC_S_GAUNTLETATK2_5
	{SpriteId::HereticGaun, 10, 4, DOOM_ACTION(A_ReFire), StateId::HereticGauntletatk27, 0, 0},               // HERETIC_S_GAUNTLETATK2_6
	{SpriteId::HereticGaun, 9, 4, DOOM_ACTION(A_Light0), StateId::HereticGauntletready21, 0, 0},              // HERETIC_S_GAUNTLETATK2_7
	{SpriteId::HereticPuf1, 32768, 4, nullptr, StateId::HereticGauntletpuff12, 0, 0},               // HERETIC_S_GAUNTLETPUFF1_1
	{SpriteId::HereticPuf1, 32769, 4, nullptr, StateId::HereticGauntletpuff13, 0, 0},               // HERETIC_S_GAUNTLETPUFF1_2
	{SpriteId::HereticPuf1, 32770, 4, nullptr, StateId::HereticGauntletpuff14, 0, 0},               // HERETIC_S_GAUNTLETPUFF1_3
	{SpriteId::HereticPuf1, 32771, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_GAUNTLETPUFF1_4
	{SpriteId::HereticPuf1, 32772, 4, nullptr, StateId::HereticGauntletpuff22, 0, 0},               // HERETIC_S_GAUNTLETPUFF2_1
	{SpriteId::HereticPuf1, 32773, 4, nullptr, StateId::HereticGauntletpuff23, 0, 0},               // HERETIC_S_GAUNTLETPUFF2_2
	{SpriteId::HereticPuf1, 32774, 4, nullptr, StateId::HereticGauntletpuff24, 0, 0},               // HERETIC_S_GAUNTLETPUFF2_3
	{SpriteId::HereticPuf1, 32775, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_GAUNTLETPUFF2_4
	{SpriteId::HereticWbls, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_BLSR
	{SpriteId::HereticBlsr, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticBlasterready, 0, 0},             // HERETIC_S_BLASTERREADY
	{SpriteId::HereticBlsr, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticBlasterdown, 0, 0},                    // HERETIC_S_BLASTERDOWN
	{SpriteId::HereticBlsr, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticBlasterup, 0, 0},                      // HERETIC_S_BLASTERUP
	{SpriteId::HereticBlsr, 1, 3, nullptr, StateId::HereticBlasteratk12, 0, 0},                     // HERETIC_S_BLASTERATK1_1
	{SpriteId::HereticBlsr, 2, 3, nullptr, StateId::HereticBlasteratk13, 0, 0},                     // HERETIC_S_BLASTERATK1_2
	{SpriteId::HereticBlsr, 3, 2, DOOM_ACTION(A_FireBlasterPL1), StateId::HereticBlasteratk14, 0, 0},         // HERETIC_S_BLASTERATK1_3
	{SpriteId::HereticBlsr, 2, 2, nullptr, StateId::HereticBlasteratk15, 0, 0},                     // HERETIC_S_BLASTERATK1_4
	{SpriteId::HereticBlsr, 1, 2, nullptr, StateId::HereticBlasteratk16, 0, 0},                     // HERETIC_S_BLASTERATK1_5
	{SpriteId::HereticBlsr, 0, 0, DOOM_ACTION(A_ReFire), StateId::HereticBlasterready, 0, 0},                  // HERETIC_S_BLASTERATK1_6
	{SpriteId::HereticBlsr, 1, 0, nullptr, StateId::HereticBlasteratk22, 0, 0},                     // HERETIC_S_BLASTERATK2_1
	{SpriteId::HereticBlsr, 2, 0, nullptr, StateId::HereticBlasteratk23, 0, 0},                     // HERETIC_S_BLASTERATK2_2
	{SpriteId::HereticBlsr, 3, 3, DOOM_ACTION(A_FireBlasterPL2), StateId::HereticBlasteratk24, 0, 0},         // HERETIC_S_BLASTERATK2_3
	{SpriteId::HereticBlsr, 2, 4, nullptr, StateId::HereticBlasteratk25, 0, 0},                     // HERETIC_S_BLASTERATK2_4
	{SpriteId::HereticBlsr, 1, 4, nullptr, StateId::HereticBlasteratk26, 0, 0},                     // HERETIC_S_BLASTERATK2_5
	{SpriteId::HereticBlsr, 0, 0, DOOM_ACTION(A_ReFire), StateId::HereticBlasterready, 0, 0},                  // HERETIC_S_BLASTERATK2_6
	{SpriteId::HereticAclo, 4, 200, nullptr, StateId::HereticBlasterfx11, 0, 0},                    // HERETIC_S_BLASTERFX1_1
	{SpriteId::HereticFx18, 32768, 3, DOOM_ACTION(A_SpawnRippers), StateId::HereticBlasterfxi12, 0, 0},       // HERETIC_S_BLASTERFXI1_1
	{SpriteId::HereticFx18, 32769, 3, nullptr, StateId::HereticBlasterfxi13, 0, 0},                 // HERETIC_S_BLASTERFXI1_2
	{SpriteId::HereticFx18, 32770, 4, nullptr, StateId::HereticBlasterfxi14, 0, 0},                 // HERETIC_S_BLASTERFXI1_3
	{SpriteId::HereticFx18, 32771, 4, nullptr, StateId::HereticBlasterfxi15, 0, 0},                 // HERETIC_S_BLASTERFXI1_4
	{SpriteId::HereticFx18, 32772, 4, nullptr, StateId::HereticBlasterfxi16, 0, 0},                 // HERETIC_S_BLASTERFXI1_5
	{SpriteId::HereticFx18, 32773, 4, nullptr, StateId::HereticBlasterfxi17, 0, 0},                 // HERETIC_S_BLASTERFXI1_6
	{SpriteId::HereticFx18, 32774, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_BLASTERFXI1_7
	{SpriteId::HereticFx18, 7, 4, nullptr, StateId::HereticBlastersmoke2, 0, 0},                     // HERETIC_S_BLASTERSMOKE1
	{SpriteId::HereticFx18, 8, 4, nullptr, StateId::HereticBlastersmoke3, 0, 0},                     // HERETIC_S_BLASTERSMOKE2
	{SpriteId::HereticFx18, 9, 4, nullptr, StateId::HereticBlastersmoke4, 0, 0},                     // HERETIC_S_BLASTERSMOKE3
	{SpriteId::HereticFx18, 10, 4, nullptr, StateId::HereticBlastersmoke5, 0, 0},                    // HERETIC_S_BLASTERSMOKE4
	{SpriteId::HereticFx18, 11, 4, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_BLASTERSMOKE5
	{SpriteId::HereticFx18, 12, 4, nullptr, StateId::HereticRipper2, 0, 0},                          // HERETIC_S_RIPPER1
	{SpriteId::HereticFx18, 13, 5, nullptr, StateId::HereticRipper1, 0, 0},                          // HERETIC_S_RIPPER2
	{SpriteId::HereticFx18, 32782, 4, nullptr, StateId::HereticRipperx2, 0, 0},                      // HERETIC_S_RIPPERX1
	{SpriteId::HereticFx18, 32783, 4, nullptr, StateId::HereticRipperx3, 0, 0},                      // HERETIC_S_RIPPERX2
	{SpriteId::HereticFx18, 32784, 4, nullptr, StateId::HereticRipperx4, 0, 0},                      // HERETIC_S_RIPPERX3
	{SpriteId::HereticFx18, 32785, 4, nullptr, StateId::HereticRipperx5, 0, 0},                      // HERETIC_S_RIPPERX4
	{SpriteId::HereticFx18, 32786, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RIPPERX5
	{SpriteId::HereticFx17, 32768, 4, nullptr, StateId::HereticBlasterpuff12, 0, 0},                // HERETIC_S_BLASTERPUFF1_1
	{SpriteId::HereticFx17, 32769, 4, nullptr, StateId::HereticBlasterpuff13, 0, 0},                // HERETIC_S_BLASTERPUFF1_2
	{SpriteId::HereticFx17, 32770, 4, nullptr, StateId::HereticBlasterpuff14, 0, 0},                // HERETIC_S_BLASTERPUFF1_3
	{SpriteId::HereticFx17, 32771, 4, nullptr, StateId::HereticBlasterpuff15, 0, 0},                // HERETIC_S_BLASTERPUFF1_4
	{SpriteId::HereticFx17, 32772, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_BLASTERPUFF1_5
	{SpriteId::HereticFx17, 32773, 3, nullptr, StateId::HereticBlasterpuff22, 0, 0},                // HERETIC_S_BLASTERPUFF2_1
	{SpriteId::HereticFx17, 32774, 3, nullptr, StateId::HereticBlasterpuff23, 0, 0},                // HERETIC_S_BLASTERPUFF2_2
	{SpriteId::HereticFx17, 32775, 4, nullptr, StateId::HereticBlasterpuff24, 0, 0},                // HERETIC_S_BLASTERPUFF2_3
	{SpriteId::HereticFx17, 32776, 4, nullptr, StateId::HereticBlasterpuff25, 0, 0},                // HERETIC_S_BLASTERPUFF2_4
	{SpriteId::HereticFx17, 32777, 4, nullptr, StateId::HereticBlasterpuff26, 0, 0},                // HERETIC_S_BLASTERPUFF2_5
	{SpriteId::HereticFx17, 32778, 4, nullptr, StateId::HereticBlasterpuff27, 0, 0},                // HERETIC_S_BLASTERPUFF2_6
	{SpriteId::HereticFx17, 32779, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_BLASTERPUFF2_7
	{SpriteId::HereticWmce, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_WMCE
	{SpriteId::HereticMace, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticMaceready, 0, 0},                // HERETIC_S_MACEREADY
	{SpriteId::HereticMace, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticMacedown, 0, 0},                       // HERETIC_S_MACEDOWN
	{SpriteId::HereticMace, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticMaceup, 0, 0},                         // HERETIC_S_MACEUP
	{SpriteId::HereticMace, 1, 4, nullptr, StateId::HereticMaceatk12, 0, 0},                        // HERETIC_S_MACEATK1_1
	{SpriteId::HereticMace, 2, 3, DOOM_ACTION(A_FireMacePL1), StateId::HereticMaceatk13, 0, 0},               // HERETIC_S_MACEATK1_2
	{SpriteId::HereticMace, 3, 3, DOOM_ACTION(A_FireMacePL1), StateId::HereticMaceatk14, 0, 0},               // HERETIC_S_MACEATK1_3
	{SpriteId::HereticMace, 4, 3, DOOM_ACTION(A_FireMacePL1), StateId::HereticMaceatk15, 0, 0},               // HERETIC_S_MACEATK1_4
	{SpriteId::HereticMace, 5, 3, DOOM_ACTION(A_FireMacePL1), StateId::HereticMaceatk16, 0, 0},               // HERETIC_S_MACEATK1_5
	{SpriteId::HereticMace, 2, 4, DOOM_ACTION(A_ReFire), StateId::HereticMaceatk17, 0, 0},                    // HERETIC_S_MACEATK1_6
	{SpriteId::HereticMace, 3, 4, nullptr, StateId::HereticMaceatk18, 0, 0},                        // HERETIC_S_MACEATK1_7
	{SpriteId::HereticMace, 4, 4, nullptr, StateId::HereticMaceatk19, 0, 0},                        // HERETIC_S_MACEATK1_8
	{SpriteId::HereticMace, 5, 4, nullptr, StateId::HereticMaceatk110, 0, 0},                       // HERETIC_S_MACEATK1_9
	{SpriteId::HereticMace, 1, 4, nullptr, StateId::HereticMaceready, 0, 0},                         // HERETIC_S_MACEATK1_10
	{SpriteId::HereticMace, 1, 4, nullptr, StateId::HereticMaceatk22, 0, 0},                        // HERETIC_S_MACEATK2_1
	{SpriteId::HereticMace, 3, 4, DOOM_ACTION(A_FireMacePL2), StateId::HereticMaceatk23, 0, 0},               // HERETIC_S_MACEATK2_2
	{SpriteId::HereticMace, 1, 4, nullptr, StateId::HereticMaceatk24, 0, 0},                        // HERETIC_S_MACEATK2_3
	{SpriteId::HereticMace, 0, 8, DOOM_ACTION(A_ReFire), StateId::HereticMaceready, 0, 0},                     // HERETIC_S_MACEATK2_4
	{SpriteId::HereticFx02, 0, 4, DOOM_ACTION(A_MacePL1Check), StateId::HereticMacefx12, 0, 0},               // HERETIC_S_MACEFX1_1
	{SpriteId::HereticFx02, 1, 4, DOOM_ACTION(A_MacePL1Check), StateId::HereticMacefx11, 0, 0},               // HERETIC_S_MACEFX1_2
	{SpriteId::HereticFx02, 32773, 4, DOOM_ACTION(A_MaceBallImpact), StateId::HereticMacefxi12, 0, 0},        // HERETIC_S_MACEFXI1_1
	{SpriteId::HereticFx02, 32774, 4, nullptr, StateId::HereticMacefxi13, 0, 0},                    // HERETIC_S_MACEFXI1_2
	{SpriteId::HereticFx02, 32775, 4, nullptr, StateId::HereticMacefxi14, 0, 0},                    // HERETIC_S_MACEFXI1_3
	{SpriteId::HereticFx02, 32776, 4, nullptr, StateId::HereticMacefxi15, 0, 0},                    // HERETIC_S_MACEFXI1_4
	{SpriteId::HereticFx02, 32777, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_MACEFXI1_5
	{SpriteId::HereticFx02, 2, 4, nullptr, StateId::HereticMacefx22, 0, 0},                         // HERETIC_S_MACEFX2_1
	{SpriteId::HereticFx02, 3, 4, nullptr, StateId::HereticMacefx21, 0, 0},                         // HERETIC_S_MACEFX2_2
	{SpriteId::HereticFx02, 32773, 4, DOOM_ACTION(A_MaceBallImpact2), StateId::HereticMacefxi12, 0, 0},       // HERETIC_S_MACEFXI2_1
	{SpriteId::HereticFx02, 0, 4, nullptr, StateId::HereticMacefx32, 0, 0},                         // HERETIC_S_MACEFX3_1
	{SpriteId::HereticFx02, 1, 4, nullptr, StateId::HereticMacefx31, 0, 0},                         // HERETIC_S_MACEFX3_2
	{SpriteId::HereticFx02, 4, 99, nullptr, StateId::HereticMacefx41, 0, 0},                        // HERETIC_S_MACEFX4_1
	{SpriteId::HereticFx02, 32770, 4, DOOM_ACTION(A_DeathBallImpact), StateId::HereticMacefxi12, 0, 0},       // HERETIC_S_MACEFXI4_1
	{SpriteId::HereticWskl, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_WSKL
	{SpriteId::HereticHrod, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticHornrodready, 0, 0},             // HERETIC_S_HORNRODREADY
	{SpriteId::HereticHrod, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticHornroddown, 0, 0},                    // HERETIC_S_HORNRODDOWN
	{SpriteId::HereticHrod, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticHornrodup, 0, 0},                      // HERETIC_S_HORNRODUP
	{SpriteId::HereticHrod, 0, 4, DOOM_ACTION(A_FireSkullRodPL1), StateId::HereticHornrodatk12, 0, 0},        // HERETIC_S_HORNRODATK1_1
	{SpriteId::HereticHrod, 1, 4, DOOM_ACTION(A_FireSkullRodPL1), StateId::HereticHornrodatk13, 0, 0},        // HERETIC_S_HORNRODATK1_2
	{SpriteId::HereticHrod, 1, 0, DOOM_ACTION(A_ReFire), StateId::HereticHornrodready, 0, 0},                  // HERETIC_S_HORNRODATK1_3
	{SpriteId::HereticHrod, 2, 2, nullptr, StateId::HereticHornrodatk22, 0, 0},                     // HERETIC_S_HORNRODATK2_1
	{SpriteId::HereticHrod, 3, 3, nullptr, StateId::HereticHornrodatk23, 0, 0},                     // HERETIC_S_HORNRODATK2_2
	{SpriteId::HereticHrod, 4, 2, nullptr, StateId::HereticHornrodatk24, 0, 0},                     // HERETIC_S_HORNRODATK2_3
	{SpriteId::HereticHrod, 5, 3, nullptr, StateId::HereticHornrodatk25, 0, 0},                     // HERETIC_S_HORNRODATK2_4
	{SpriteId::HereticHrod, 6, 4, DOOM_ACTION(A_FireSkullRodPL2), StateId::HereticHornrodatk26, 0, 0},        // HERETIC_S_HORNRODATK2_5
	{SpriteId::HereticHrod, 5, 2, nullptr, StateId::HereticHornrodatk27, 0, 0},                     // HERETIC_S_HORNRODATK2_6
	{SpriteId::HereticHrod, 4, 3, nullptr, StateId::HereticHornrodatk28, 0, 0},                     // HERETIC_S_HORNRODATK2_7
	{SpriteId::HereticHrod, 3, 2, nullptr, StateId::HereticHornrodatk29, 0, 0},                     // HERETIC_S_HORNRODATK2_8
	{SpriteId::HereticHrod, 2, 2, DOOM_ACTION(A_ReFire), StateId::HereticHornrodready, 0, 0},                  // HERETIC_S_HORNRODATK2_9
	{SpriteId::HereticFx00, 32768, 6, nullptr, StateId::HereticHrodfx12, 0, 0},                     // HERETIC_S_HRODFX1_1
	{SpriteId::HereticFx00, 32769, 6, nullptr, StateId::HereticHrodfx11, 0, 0},                     // HERETIC_S_HRODFX1_2
	{SpriteId::HereticFx00, 32775, 5, nullptr, StateId::HereticHrodfxi12, 0, 0},                    // HERETIC_S_HRODFXI1_1
	{SpriteId::HereticFx00, 32776, 5, nullptr, StateId::HereticHrodfxi13, 0, 0},                    // HERETIC_S_HRODFXI1_2
	{SpriteId::HereticFx00, 32777, 4, nullptr, StateId::HereticHrodfxi14, 0, 0},                    // HERETIC_S_HRODFXI1_3
	{SpriteId::HereticFx00, 32778, 4, nullptr, StateId::HereticHrodfxi15, 0, 0},                    // HERETIC_S_HRODFXI1_4
	{SpriteId::HereticFx00, 32779, 3, nullptr, StateId::HereticHrodfxi16, 0, 0},                    // HERETIC_S_HRODFXI1_5
	{SpriteId::HereticFx00, 32780, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_HRODFXI1_6
	{SpriteId::HereticFx00, 32770, 3, nullptr, StateId::HereticHrodfx22, 0, 0},                     // HERETIC_S_HRODFX2_1
	{SpriteId::HereticFx00, 32771, 3, DOOM_ACTION(A_SkullRodPL2Seek), StateId::HereticHrodfx23, 0, 0},        // HERETIC_S_HRODFX2_2
	{SpriteId::HereticFx00, 32772, 3, nullptr, StateId::HereticHrodfx24, 0, 0},                     // HERETIC_S_HRODFX2_3
	{SpriteId::HereticFx00, 32773, 3, DOOM_ACTION(A_SkullRodPL2Seek), StateId::HereticHrodfx21, 0, 0},        // HERETIC_S_HRODFX2_4
	{SpriteId::HereticFx00, 32775, 5, DOOM_ACTION(A_AddPlayerRain), StateId::HereticHrodfxi22, 0, 0},         // HERETIC_S_HRODFXI2_1
	{SpriteId::HereticFx00, 32776, 5, nullptr, StateId::HereticHrodfxi23, 0, 0},                    // HERETIC_S_HRODFXI2_2
	{SpriteId::HereticFx00, 32777, 4, nullptr, StateId::HereticHrodfxi24, 0, 0},                    // HERETIC_S_HRODFXI2_3
	{SpriteId::HereticFx00, 32778, 3, nullptr, StateId::HereticHrodfxi25, 0, 0},                    // HERETIC_S_HRODFXI2_4
	{SpriteId::HereticFx00, 32779, 3, nullptr, StateId::HereticHrodfxi26, 0, 0},                    // HERETIC_S_HRODFXI2_5
	{SpriteId::HereticFx00, 32780, 3, nullptr, StateId::HereticHrodfxi27, 0, 0},                    // HERETIC_S_HRODFXI2_6
	{SpriteId::HereticFx00, 6, 1, DOOM_ACTION(A_HideInCeiling), StateId::HereticHrodfxi28, 0, 0},             // HERETIC_S_HRODFXI2_7
	{SpriteId::HereticFx00, 6, 1, DOOM_ACTION(A_SkullRodStorm), StateId::HereticHrodfxi28, 0, 0},             // HERETIC_S_HRODFXI2_8
	{SpriteId::HereticFx20, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_RAINPLR1_1
	{SpriteId::HereticFx21, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_RAINPLR2_1
	{SpriteId::HereticFx22, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_RAINPLR3_1
	{SpriteId::HereticFx23, 32768, -1, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_RAINPLR4_1
	{SpriteId::HereticFx20, 32769, 4, DOOM_ACTION(A_RainImpact), StateId::HereticRainplr1x2, 0, 0},           // HERETIC_S_RAINPLR1X_1
	{SpriteId::HereticFx20, 32770, 4, nullptr, StateId::HereticRainplr1x3, 0, 0},                   // HERETIC_S_RAINPLR1X_2
	{SpriteId::HereticFx20, 32771, 4, nullptr, StateId::HereticRainplr1x4, 0, 0},                   // HERETIC_S_RAINPLR1X_3
	{SpriteId::HereticFx20, 32772, 4, nullptr, StateId::HereticRainplr1x5, 0, 0},                   // HERETIC_S_RAINPLR1X_4
	{SpriteId::HereticFx20, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINPLR1X_5
	{SpriteId::HereticFx21, 32769, 4, DOOM_ACTION(A_RainImpact), StateId::HereticRainplr2x2, 0, 0},           // HERETIC_S_RAINPLR2X_1
	{SpriteId::HereticFx21, 32770, 4, nullptr, StateId::HereticRainplr2x3, 0, 0},                   // HERETIC_S_RAINPLR2X_2
	{SpriteId::HereticFx21, 32771, 4, nullptr, StateId::HereticRainplr2x4, 0, 0},                   // HERETIC_S_RAINPLR2X_3
	{SpriteId::HereticFx21, 32772, 4, nullptr, StateId::HereticRainplr2x5, 0, 0},                   // HERETIC_S_RAINPLR2X_4
	{SpriteId::HereticFx21, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINPLR2X_5
	{SpriteId::HereticFx22, 32769, 4, DOOM_ACTION(A_RainImpact), StateId::HereticRainplr3x2, 0, 0},           // HERETIC_S_RAINPLR3X_1
	{SpriteId::HereticFx22, 32770, 4, nullptr, StateId::HereticRainplr3x3, 0, 0},                   // HERETIC_S_RAINPLR3X_2
	{SpriteId::HereticFx22, 32771, 4, nullptr, StateId::HereticRainplr3x4, 0, 0},                   // HERETIC_S_RAINPLR3X_3
	{SpriteId::HereticFx22, 32772, 4, nullptr, StateId::HereticRainplr3x5, 0, 0},                   // HERETIC_S_RAINPLR3X_4
	{SpriteId::HereticFx22, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINPLR3X_5
	{SpriteId::HereticFx23, 32769, 4, DOOM_ACTION(A_RainImpact), StateId::HereticRainplr4x2, 0, 0},           // HERETIC_S_RAINPLR4X_1
	{SpriteId::HereticFx23, 32770, 4, nullptr, StateId::HereticRainplr4x3, 0, 0},                   // HERETIC_S_RAINPLR4X_2
	{SpriteId::HereticFx23, 32771, 4, nullptr, StateId::HereticRainplr4x4, 0, 0},                   // HERETIC_S_RAINPLR4X_3
	{SpriteId::HereticFx23, 32772, 4, nullptr, StateId::HereticRainplr4x5, 0, 0},                   // HERETIC_S_RAINPLR4X_4
	{SpriteId::HereticFx23, 32773, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINPLR4X_5
	{SpriteId::HereticFx20, 32774, 4, nullptr, StateId::HereticRainairxplr12, 0, 0},                // HERETIC_S_RAINAIRXPLR1_1
	{SpriteId::HereticFx21, 32774, 4, nullptr, StateId::HereticRainairxplr22, 0, 0},                // HERETIC_S_RAINAIRXPLR2_1
	{SpriteId::HereticFx22, 32774, 4, nullptr, StateId::HereticRainairxplr32, 0, 0},                // HERETIC_S_RAINAIRXPLR3_1
	{SpriteId::HereticFx23, 32774, 4, nullptr, StateId::HereticRainairxplr42, 0, 0},                // HERETIC_S_RAINAIRXPLR4_1
	{SpriteId::HereticFx20, 32775, 4, nullptr, StateId::HereticRainairxplr13, 0, 0},                // HERETIC_S_RAINAIRXPLR1_2
	{SpriteId::HereticFx21, 32775, 4, nullptr, StateId::HereticRainairxplr23, 0, 0},                // HERETIC_S_RAINAIRXPLR2_2
	{SpriteId::HereticFx22, 32775, 4, nullptr, StateId::HereticRainairxplr33, 0, 0},                // HERETIC_S_RAINAIRXPLR3_2
	{SpriteId::HereticFx23, 32775, 4, nullptr, StateId::HereticRainairxplr43, 0, 0},                // HERETIC_S_RAINAIRXPLR4_2
	{SpriteId::HereticFx20, 32776, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINAIRXPLR1_3
	{SpriteId::HereticFx21, 32776, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINAIRXPLR2_3
	{SpriteId::HereticFx22, 32776, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINAIRXPLR3_3
	{SpriteId::HereticFx23, 32776, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_RAINAIRXPLR4_3
	{SpriteId::HereticGwnd, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticGoldwandready, 0, 0},            // HERETIC_S_GOLDWANDREADY
	{SpriteId::HereticGwnd, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticGoldwanddown, 0, 0},                   // HERETIC_S_GOLDWANDDOWN
	{SpriteId::HereticGwnd, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticGoldwandup, 0, 0},                     // HERETIC_S_GOLDWANDUP
	{SpriteId::HereticGwnd, 1, 3, nullptr, StateId::HereticGoldwandatk12, 0, 0},                    // HERETIC_S_GOLDWANDATK1_1
	{SpriteId::HereticGwnd, 2, 5, DOOM_ACTION(A_FireGoldWandPL1), StateId::HereticGoldwandatk13, 0, 0},       // HERETIC_S_GOLDWANDATK1_2
	{SpriteId::HereticGwnd, 3, 3, nullptr, StateId::HereticGoldwandatk14, 0, 0},                    // HERETIC_S_GOLDWANDATK1_3
	{SpriteId::HereticGwnd, 3, 0, DOOM_ACTION(A_ReFire), StateId::HereticGoldwandready, 0, 0},                 // HERETIC_S_GOLDWANDATK1_4
	{SpriteId::HereticGwnd, 1, 3, nullptr, StateId::HereticGoldwandatk22, 0, 0},                    // HERETIC_S_GOLDWANDATK2_1
	{SpriteId::HereticGwnd, 2, 4, DOOM_ACTION(A_FireGoldWandPL2), StateId::HereticGoldwandatk23, 0, 0},       // HERETIC_S_GOLDWANDATK2_2
	{SpriteId::HereticGwnd, 3, 3, nullptr, StateId::HereticGoldwandatk24, 0, 0},                    // HERETIC_S_GOLDWANDATK2_3
	{SpriteId::HereticGwnd, 3, 0, DOOM_ACTION(A_ReFire), StateId::HereticGoldwandready, 0, 0},                 // HERETIC_S_GOLDWANDATK2_4
	{SpriteId::HereticFx01, 32768, 6, nullptr, StateId::HereticGwandfx12, 0, 0},                    // HERETIC_S_GWANDFX1_1
	{SpriteId::HereticFx01, 32769, 6, nullptr, StateId::HereticGwandfx11, 0, 0},                    // HERETIC_S_GWANDFX1_2
	{SpriteId::HereticFx01, 32772, 3, nullptr, StateId::HereticGwandfxi12, 0, 0},                   // HERETIC_S_GWANDFXI1_1
	{SpriteId::HereticFx01, 32773, 3, nullptr, StateId::HereticGwandfxi13, 0, 0},                   // HERETIC_S_GWANDFXI1_2
	{SpriteId::HereticFx01, 32774, 3, nullptr, StateId::HereticGwandfxi14, 0, 0},                   // HERETIC_S_GWANDFXI1_3
	{SpriteId::HereticFx01, 32775, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_GWANDFXI1_4
	{SpriteId::HereticFx01, 32770, 6, nullptr, StateId::HereticGwandfx22, 0, 0},                    // HERETIC_S_GWANDFX2_1
	{SpriteId::HereticFx01, 32771, 6, nullptr, StateId::HereticGwandfx21, 0, 0},                    // HERETIC_S_GWANDFX2_2
	{SpriteId::HereticPuf2, 32768, 3, nullptr, StateId::HereticGwandpuff12, 0, 0},                  // HERETIC_S_GWANDPUFF1_1
	{SpriteId::HereticPuf2, 32769, 3, nullptr, StateId::HereticGwandpuff13, 0, 0},                  // HERETIC_S_GWANDPUFF1_2
	{SpriteId::HereticPuf2, 32770, 3, nullptr, StateId::HereticGwandpuff14, 0, 0},                  // HERETIC_S_GWANDPUFF1_3
	{SpriteId::HereticPuf2, 32771, 3, nullptr, StateId::HereticGwandpuff15, 0, 0},                  // HERETIC_S_GWANDPUFF1_4
	{SpriteId::HereticPuf2, 32772, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_GWANDPUFF1_5
	{SpriteId::HereticWphx, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_WPHX
	{SpriteId::HereticPhnx, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticPhoenixready, 0, 0},             // HERETIC_S_PHOENIXREADY
	{SpriteId::HereticPhnx, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticPhoenixdown, 0, 0},                    // HERETIC_S_PHOENIXDOWN
	{SpriteId::HereticPhnx, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticPhoenixup, 0, 0},                      // HERETIC_S_PHOENIXUP
	{SpriteId::HereticPhnx, 1, 5, nullptr, StateId::HereticPhoenixatk12, 0, 0},                     // HERETIC_S_PHOENIXATK1_1
	{SpriteId::HereticPhnx, 2, 7, DOOM_ACTION(A_FirePhoenixPL1), StateId::HereticPhoenixatk13, 0, 0},         // HERETIC_S_PHOENIXATK1_2
	{SpriteId::HereticPhnx, 3, 4, nullptr, StateId::HereticPhoenixatk14, 0, 0},                     // HERETIC_S_PHOENIXATK1_3
	{SpriteId::HereticPhnx, 1, 4, nullptr, StateId::HereticPhoenixatk15, 0, 0},                     // HERETIC_S_PHOENIXATK1_4
	{SpriteId::HereticPhnx, 1, 0, DOOM_ACTION(A_ReFire), StateId::HereticPhoenixready, 0, 0},                  // HERETIC_S_PHOENIXATK1_5
	{SpriteId::HereticPhnx, 1, 3, DOOM_ACTION(A_InitPhoenixPL2), StateId::HereticPhoenixatk22, 0, 0},         // HERETIC_S_PHOENIXATK2_1
	{SpriteId::HereticPhnx, 32770, 1, DOOM_ACTION(A_FirePhoenixPL2), StateId::HereticPhoenixatk23, 0, 0},     // HERETIC_S_PHOENIXATK2_2
	{SpriteId::HereticPhnx, 1, 4, DOOM_ACTION(A_ReFire), StateId::HereticPhoenixatk24, 0, 0},                 // HERETIC_S_PHOENIXATK2_3
	{SpriteId::HereticPhnx, 1, 4, DOOM_ACTION(A_ShutdownPhoenixPL2), StateId::HereticPhoenixready, 0, 0},      // HERETIC_S_PHOENIXATK2_4
	{SpriteId::HereticFx04, 32768, 4, DOOM_ACTION(A_PhoenixPuff), StateId::HereticPhoenixfx11, 0, 0},         // HERETIC_S_PHOENIXFX1_1
	{SpriteId::HereticFx08, 32768, 6, DOOM_ACTION(A_Explode), StateId::HereticPhoenixfxi12, 0, 0},            // HERETIC_S_PHOENIXFXI1_1
	{SpriteId::HereticFx08, 32769, 5, nullptr, StateId::HereticPhoenixfxi13, 0, 0},                 // HERETIC_S_PHOENIXFXI1_2
	{SpriteId::HereticFx08, 32770, 5, nullptr, StateId::HereticPhoenixfxi14, 0, 0},                 // HERETIC_S_PHOENIXFXI1_3
	{SpriteId::HereticFx08, 32771, 4, nullptr, StateId::HereticPhoenixfxi15, 0, 0},                 // HERETIC_S_PHOENIXFXI1_4
	{SpriteId::HereticFx08, 32772, 4, nullptr, StateId::HereticPhoenixfxi16, 0, 0},                 // HERETIC_S_PHOENIXFXI1_5
	{SpriteId::HereticFx08, 32773, 4, nullptr, StateId::HereticPhoenixfxi17, 0, 0},                 // HERETIC_S_PHOENIXFXI1_6
	{SpriteId::HereticFx08, 32774, 4, nullptr, StateId::HereticPhoenixfxi18, 0, 0},                 // HERETIC_S_PHOENIXFXI1_7
	{SpriteId::HereticFx08, 32775, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_PHOENIXFXI1_8
	{SpriteId::HereticFx08, 32776, 8, nullptr, StateId::HereticPhoenixfxix1, 0, 0},                 // HERETIC_S_PHOENIXFXIX_1
	{SpriteId::HereticFx08, 32777, 8, DOOM_ACTION(A_RemovedPhoenixFunc), StateId::HereticPhoenixfxix2, 0, 0}, // HERETIC_S_PHOENIXFXIX_2
	{SpriteId::HereticFx08, 32778, 8, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_PHOENIXFXIX_3
	{SpriteId::HereticFx04, 1, 4, nullptr, StateId::HereticPhoenixpuff2, 0, 0},                      // HERETIC_S_PHOENIXPUFF1
	{SpriteId::HereticFx04, 2, 4, nullptr, StateId::HereticPhoenixpuff3, 0, 0},                      // HERETIC_S_PHOENIXPUFF2
	{SpriteId::HereticFx04, 3, 4, nullptr, StateId::HereticPhoenixpuff4, 0, 0},                      // HERETIC_S_PHOENIXPUFF3
	{SpriteId::HereticFx04, 4, 4, nullptr, StateId::HereticPhoenixpuff5, 0, 0},                      // HERETIC_S_PHOENIXPUFF4
	{SpriteId::HereticFx04, 5, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_PHOENIXPUFF5
	{SpriteId::HereticFx09, 32768, 2, nullptr, StateId::HereticPhoenixfx22, 0, 0},                  // HERETIC_S_PHOENIXFX2_1
	{SpriteId::HereticFx09, 32769, 2, nullptr, StateId::HereticPhoenixfx23, 0, 0},                  // HERETIC_S_PHOENIXFX2_2
	{SpriteId::HereticFx09, 32768, 2, nullptr, StateId::HereticPhoenixfx24, 0, 0},                  // HERETIC_S_PHOENIXFX2_3
	{SpriteId::HereticFx09, 32769, 2, nullptr, StateId::HereticPhoenixfx25, 0, 0},                  // HERETIC_S_PHOENIXFX2_4
	{SpriteId::HereticFx09, 32768, 2, nullptr, StateId::HereticPhoenixfx26, 0, 0},                  // HERETIC_S_PHOENIXFX2_5
	{SpriteId::HereticFx09, 32769, 2, DOOM_ACTION(A_FlameEnd), StateId::HereticPhoenixfx27, 0, 0},            // HERETIC_S_PHOENIXFX2_6
	{SpriteId::HereticFx09, 32770, 2, nullptr, StateId::HereticPhoenixfx28, 0, 0},                  // HERETIC_S_PHOENIXFX2_7
	{SpriteId::HereticFx09, 32771, 2, nullptr, StateId::HereticPhoenixfx29, 0, 0},                  // HERETIC_S_PHOENIXFX2_8
	{SpriteId::HereticFx09, 32772, 2, nullptr, StateId::HereticPhoenixfx210, 0, 0},                 // HERETIC_S_PHOENIXFX2_9
	{SpriteId::HereticFx09, 32773, 2, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_PHOENIXFX2_10
	{SpriteId::HereticFx09, 32774, 3, nullptr, StateId::HereticPhoenixfxi22, 0, 0},                 // HERETIC_S_PHOENIXFXI2_1
	{SpriteId::HereticFx09, 32775, 3, DOOM_ACTION(A_FloatPuff), StateId::HereticPhoenixfxi23, 0, 0},          // HERETIC_S_PHOENIXFXI2_2
	{SpriteId::HereticFx09, 32776, 4, nullptr, StateId::HereticPhoenixfxi24, 0, 0},                 // HERETIC_S_PHOENIXFXI2_3
	{SpriteId::HereticFx09, 32777, 5, nullptr, StateId::HereticPhoenixfxi25, 0, 0},                 // HERETIC_S_PHOENIXFXI2_4
	{SpriteId::HereticFx09, 32778, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_PHOENIXFXI2_5
	{SpriteId::HereticWbow, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_WBOW
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow2, 0, 0},                   // HERETIC_S_CRBOW1
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow3, 0, 0},                   // HERETIC_S_CRBOW2
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow4, 0, 0},                   // HERETIC_S_CRBOW3
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow5, 0, 0},                   // HERETIC_S_CRBOW4
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow6, 0, 0},                   // HERETIC_S_CRBOW5
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow7, 0, 0},                   // HERETIC_S_CRBOW6
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow8, 0, 0},                   // HERETIC_S_CRBOW7
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow9, 0, 0},                   // HERETIC_S_CRBOW8
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow10, 0, 0},                  // HERETIC_S_CRBOW9
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow11, 0, 0},                  // HERETIC_S_CRBOW10
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow12, 0, 0},                  // HERETIC_S_CRBOW11
	{SpriteId::HereticCrbw, 1, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow13, 0, 0},                  // HERETIC_S_CRBOW12
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow14, 0, 0},                  // HERETIC_S_CRBOW13
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow15, 0, 0},                  // HERETIC_S_CRBOW14
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow16, 0, 0},                  // HERETIC_S_CRBOW15
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow17, 0, 0},                  // HERETIC_S_CRBOW16
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow18, 0, 0},                  // HERETIC_S_CRBOW17
	{SpriteId::HereticCrbw, 2, 1, DOOM_ACTION(A_WeaponReady), StateId::HereticCrbow1, 0, 0},                   // HERETIC_S_CRBOW18
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_Lower), StateId::HereticCrbowdown, 0, 0},                      // HERETIC_S_CRBOWDOWN
	{SpriteId::HereticCrbw, 0, 1, DOOM_ACTION(A_Raise), StateId::HereticCrbowup, 0, 0},                        // HERETIC_S_CRBOWUP
	{SpriteId::HereticCrbw, 3, 6, DOOM_ACTION(A_FireCrossbowPL1), StateId::HereticCrbowatk12, 0, 0},          // HERETIC_S_CRBOWATK1_1
	{SpriteId::HereticCrbw, 4, 3, nullptr, StateId::HereticCrbowatk13, 0, 0},                       // HERETIC_S_CRBOWATK1_2
	{SpriteId::HereticCrbw, 5, 3, nullptr, StateId::HereticCrbowatk14, 0, 0},                       // HERETIC_S_CRBOWATK1_3
	{SpriteId::HereticCrbw, 6, 3, nullptr, StateId::HereticCrbowatk15, 0, 0},                       // HERETIC_S_CRBOWATK1_4
	{SpriteId::HereticCrbw, 7, 3, nullptr, StateId::HereticCrbowatk16, 0, 0},                       // HERETIC_S_CRBOWATK1_5
	{SpriteId::HereticCrbw, 0, 4, nullptr, StateId::HereticCrbowatk17, 0, 0},                       // HERETIC_S_CRBOWATK1_6
	{SpriteId::HereticCrbw, 1, 4, nullptr, StateId::HereticCrbowatk18, 0, 0},                       // HERETIC_S_CRBOWATK1_7
	{SpriteId::HereticCrbw, 2, 5, DOOM_ACTION(A_ReFire), StateId::HereticCrbow1, 0, 0},                        // HERETIC_S_CRBOWATK1_8
	{SpriteId::HereticCrbw, 3, 5, DOOM_ACTION(A_FireCrossbowPL2), StateId::HereticCrbowatk22, 0, 0},          // HERETIC_S_CRBOWATK2_1
	{SpriteId::HereticCrbw, 4, 3, nullptr, StateId::HereticCrbowatk23, 0, 0},                       // HERETIC_S_CRBOWATK2_2
	{SpriteId::HereticCrbw, 5, 2, nullptr, StateId::HereticCrbowatk24, 0, 0},                       // HERETIC_S_CRBOWATK2_3
	{SpriteId::HereticCrbw, 6, 3, nullptr, StateId::HereticCrbowatk25, 0, 0},                       // HERETIC_S_CRBOWATK2_4
	{SpriteId::HereticCrbw, 7, 2, nullptr, StateId::HereticCrbowatk26, 0, 0},                       // HERETIC_S_CRBOWATK2_5
	{SpriteId::HereticCrbw, 0, 3, nullptr, StateId::HereticCrbowatk27, 0, 0},                       // HERETIC_S_CRBOWATK2_6
	{SpriteId::HereticCrbw, 1, 3, nullptr, StateId::HereticCrbowatk28, 0, 0},                       // HERETIC_S_CRBOWATK2_7
	{SpriteId::HereticCrbw, 2, 4, DOOM_ACTION(A_ReFire), StateId::HereticCrbow1, 0, 0},                        // HERETIC_S_CRBOWATK2_8
	{SpriteId::HereticFx03, 32769, 1, nullptr, StateId::HereticCrbowfx1, 0, 0},                      // HERETIC_S_CRBOWFX1
	{SpriteId::HereticFx03, 32775, 8, nullptr, StateId::HereticCrbowfxi12, 0, 0},                   // HERETIC_S_CRBOWFXI1_1
	{SpriteId::HereticFx03, 32776, 8, nullptr, StateId::HereticCrbowfxi13, 0, 0},                   // HERETIC_S_CRBOWFXI1_2
	{SpriteId::HereticFx03, 32777, 8, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_CRBOWFXI1_3
	{SpriteId::HereticFx03, 32769, 1, DOOM_ACTION(A_BoltSpark), StateId::HereticCrbowfx2, 0, 0},               // HERETIC_S_CRBOWFX2
	{SpriteId::HereticFx03, 32768, 1, nullptr, StateId::HereticCrbowfx3, 0, 0},                      // HERETIC_S_CRBOWFX3
	{SpriteId::HereticFx03, 32770, 8, nullptr, StateId::HereticCrbowfxi32, 0, 0},                   // HERETIC_S_CRBOWFXI3_1
	{SpriteId::HereticFx03, 32771, 8, nullptr, StateId::HereticCrbowfxi33, 0, 0},                   // HERETIC_S_CRBOWFXI3_2
	{SpriteId::HereticFx03, 32772, 8, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_CRBOWFXI3_3
	{SpriteId::HereticFx03, 32773, 8, nullptr, StateId::HereticCrbowfx42, 0, 0},                    // HERETIC_S_CRBOWFX4_1
	{SpriteId::HereticFx03, 32774, 8, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_CRBOWFX4_2
	{SpriteId::HereticBlod, 2, 8, nullptr, StateId::HereticBlood2, 0, 0},                            // HERETIC_S_BLOOD1
	{SpriteId::HereticBlod, 1, 8, nullptr, StateId::HereticBlood3, 0, 0},                            // HERETIC_S_BLOOD2
	{SpriteId::HereticBlod, 0, 8, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_BLOOD3
	{SpriteId::HereticBlod, 2, 8, nullptr, StateId::HereticBloodsplatter2, 0, 0},                    // HERETIC_S_BLOODSPLATTER1
	{SpriteId::HereticBlod, 1, 8, nullptr, StateId::HereticBloodsplatter3, 0, 0},                    // HERETIC_S_BLOODSPLATTER2
	{SpriteId::HereticBlod, 0, 8, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_BLOODSPLATTER3
	{SpriteId::HereticBlod, 0, 6, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_BLOODSPLATTERX
	{SpriteId::HereticPlay, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_PLAY
	{SpriteId::HereticPlay, 0, 4, nullptr, StateId::HereticPlayRun2, 0, 0},                         // HERETIC_S_PLAY_RUN1
	{SpriteId::HereticPlay, 1, 4, nullptr, StateId::HereticPlayRun3, 0, 0},                         // HERETIC_S_PLAY_RUN2
	{SpriteId::HereticPlay, 2, 4, nullptr, StateId::HereticPlayRun4, 0, 0},                         // HERETIC_S_PLAY_RUN3
	{SpriteId::HereticPlay, 3, 4, nullptr, StateId::HereticPlayRun1, 0, 0},                         // HERETIC_S_PLAY_RUN4
	{SpriteId::HereticPlay, 4, 12, nullptr, StateId::HereticPlay, 0, 0},                             // HERETIC_S_PLAY_ATK1
	{SpriteId::HereticPlay, 32773, 6, nullptr, StateId::HereticPlayAtk1, 0, 0},                     // HERETIC_S_PLAY_ATK2
	{SpriteId::HereticPlay, 6, 4, nullptr, StateId::HereticPlayPain2, 0, 0},                        // HERETIC_S_PLAY_PAIN
	{SpriteId::HereticPlay, 6, 4, DOOM_ACTION(A_Pain), StateId::HereticPlay, 0, 0},                            // HERETIC_S_PLAY_PAIN2
	{SpriteId::HereticPlay, 7, 6, nullptr, StateId::HereticPlayDie2, 0, 0},                         // HERETIC_S_PLAY_DIE1
	{SpriteId::HereticPlay, 8, 6, DOOM_ACTION(A_Scream), StateId::HereticPlayDie3, 0, 0},                     // HERETIC_S_PLAY_DIE2
	{SpriteId::HereticPlay, 9, 6, nullptr, StateId::HereticPlayDie4, 0, 0},                         // HERETIC_S_PLAY_DIE3
	{SpriteId::HereticPlay, 10, 6, nullptr, StateId::HereticPlayDie5, 0, 0},                        // HERETIC_S_PLAY_DIE4
	{SpriteId::HereticPlay, 11, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticPlayDie6, 0, 0},                // HERETIC_S_PLAY_DIE5
	{SpriteId::HereticPlay, 12, 6, nullptr, StateId::HereticPlayDie7, 0, 0},                        // HERETIC_S_PLAY_DIE6
	{SpriteId::HereticPlay, 13, 6, nullptr, StateId::HereticPlayDie8, 0, 0},                        // HERETIC_S_PLAY_DIE7
	{SpriteId::HereticPlay, 14, 6, nullptr, StateId::HereticPlayDie9, 0, 0},                        // HERETIC_S_PLAY_DIE8
	{SpriteId::HereticPlay, 15, -1, DOOM_ACTION(A_AddPlayerCorpse), StateId::HereticNull, 0, 0},               // HERETIC_S_PLAY_DIE9
	{SpriteId::HereticPlay, 16, 5, DOOM_ACTION(A_Scream), StateId::HereticPlayXdie2, 0, 0},                   // HERETIC_S_PLAY_XDIE1
	{SpriteId::HereticPlay, 17, 5, DOOM_ACTION(A_SkullPop), StateId::HereticPlayXdie3, 0, 0},                 // HERETIC_S_PLAY_XDIE2
	{SpriteId::HereticPlay, 18, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticPlayXdie4, 0, 0},               // HERETIC_S_PLAY_XDIE3
	{SpriteId::HereticPlay, 19, 5, nullptr, StateId::HereticPlayXdie5, 0, 0},                       // HERETIC_S_PLAY_XDIE4
	{SpriteId::HereticPlay, 20, 5, nullptr, StateId::HereticPlayXdie6, 0, 0},                       // HERETIC_S_PLAY_XDIE5
	{SpriteId::HereticPlay, 21, 5, nullptr, StateId::HereticPlayXdie7, 0, 0},                       // HERETIC_S_PLAY_XDIE6
	{SpriteId::HereticPlay, 22, 5, nullptr, StateId::HereticPlayXdie8, 0, 0},                       // HERETIC_S_PLAY_XDIE7
	{SpriteId::HereticPlay, 23, 5, nullptr, StateId::HereticPlayXdie9, 0, 0},                       // HERETIC_S_PLAY_XDIE8
	{SpriteId::HereticPlay, 24, -1, DOOM_ACTION(A_AddPlayerCorpse), StateId::HereticNull, 0, 0},               // HERETIC_S_PLAY_XDIE9
	{SpriteId::HereticFdth, 32768, 5, DOOM_ACTION(A_FlameSnd), StateId::HereticPlayFdth2, 0, 0},              // HERETIC_S_PLAY_FDTH1
	{SpriteId::HereticFdth, 32769, 4, nullptr, StateId::HereticPlayFdth3, 0, 0},                    // HERETIC_S_PLAY_FDTH2
	{SpriteId::HereticFdth, 32770, 5, nullptr, StateId::HereticPlayFdth4, 0, 0},                    // HERETIC_S_PLAY_FDTH3
	{SpriteId::HereticFdth, 32771, 4, DOOM_ACTION(A_Scream), StateId::HereticPlayFdth5, 0, 0},                // HERETIC_S_PLAY_FDTH4
	{SpriteId::HereticFdth, 32772, 5, nullptr, StateId::HereticPlayFdth6, 0, 0},                    // HERETIC_S_PLAY_FDTH5
	{SpriteId::HereticFdth, 32773, 4, nullptr, StateId::HereticPlayFdth7, 0, 0},                    // HERETIC_S_PLAY_FDTH6
	{SpriteId::HereticFdth, 32774, 5, DOOM_ACTION(A_FlameSnd), StateId::HereticPlayFdth8, 0, 0},              // HERETIC_S_PLAY_FDTH7
	{SpriteId::HereticFdth, 32775, 4, nullptr, StateId::HereticPlayFdth9, 0, 0},                    // HERETIC_S_PLAY_FDTH8
	{SpriteId::HereticFdth, 32776, 5, nullptr, StateId::HereticPlayFdth10, 0, 0},                   // HERETIC_S_PLAY_FDTH9
	{SpriteId::HereticFdth, 32777, 4, nullptr, StateId::HereticPlayFdth11, 0, 0},                   // HERETIC_S_PLAY_FDTH10
	{SpriteId::HereticFdth, 32778, 5, nullptr, StateId::HereticPlayFdth12, 0, 0},                   // HERETIC_S_PLAY_FDTH11
	{SpriteId::HereticFdth, 32779, 4, nullptr, StateId::HereticPlayFdth13, 0, 0},                   // HERETIC_S_PLAY_FDTH12
	{SpriteId::HereticFdth, 32780, 5, nullptr, StateId::HereticPlayFdth14, 0, 0},                   // HERETIC_S_PLAY_FDTH13
	{SpriteId::HereticFdth, 32781, 4, nullptr, StateId::HereticPlayFdth15, 0, 0},                   // HERETIC_S_PLAY_FDTH14
	{SpriteId::HereticFdth, 32782, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticPlayFdth16, 0, 0},           // HERETIC_S_PLAY_FDTH15
	{SpriteId::HereticFdth, 32783, 4, nullptr, StateId::HereticPlayFdth17, 0, 0},                   // HERETIC_S_PLAY_FDTH16
	{SpriteId::HereticFdth, 32784, 5, nullptr, StateId::HereticPlayFdth18, 0, 0},                   // HERETIC_S_PLAY_FDTH17
	{SpriteId::HereticFdth, 32785, 4, nullptr, StateId::HereticPlayFdth19, 0, 0},                   // HERETIC_S_PLAY_FDTH18
	{SpriteId::HereticAclo, 4, 35, DOOM_ACTION(A_CheckBurnGone), StateId::HereticPlayFdth19, 0, 0},           // HERETIC_S_PLAY_FDTH19
	{SpriteId::HereticAclo, 4, 8, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_PLAY_FDTH20
	{SpriteId::HereticBskl, 0, 5, DOOM_ACTION(A_CheckSkullFloor), StateId::HereticBloodyskull2, 0, 0},         // HERETIC_S_BLOODYSKULL1
	{SpriteId::HereticBskl, 1, 5, DOOM_ACTION(A_CheckSkullFloor), StateId::HereticBloodyskull3, 0, 0},         // HERETIC_S_BLOODYSKULL2
	{SpriteId::HereticBskl, 2, 5, DOOM_ACTION(A_CheckSkullFloor), StateId::HereticBloodyskull4, 0, 0},         // HERETIC_S_BLOODYSKULL3
	{SpriteId::HereticBskl, 3, 5, DOOM_ACTION(A_CheckSkullFloor), StateId::HereticBloodyskull5, 0, 0},         // HERETIC_S_BLOODYSKULL4
	{SpriteId::HereticBskl, 4, 5, DOOM_ACTION(A_CheckSkullFloor), StateId::HereticBloodyskull1, 0, 0},         // HERETIC_S_BLOODYSKULL5
	{SpriteId::HereticBskl, 5, 16, DOOM_ACTION(A_CheckSkullDone), StateId::HereticBloodyskullx1, 0, 0},        // HERETIC_S_BLOODYSKULLX1
	{SpriteId::HereticBskl, 5, 1050, nullptr, StateId::HereticNull, 0, 0},                           // HERETIC_S_BLOODYSKULLX2
	{SpriteId::HereticChkn, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_CHICPLAY
	{SpriteId::HereticChkn, 0, 3, nullptr, StateId::HereticChicplayRun2, 0, 0},                     // HERETIC_S_CHICPLAY_RUN1
	{SpriteId::HereticChkn, 1, 3, nullptr, StateId::HereticChicplayRun3, 0, 0},                     // HERETIC_S_CHICPLAY_RUN2
	{SpriteId::HereticChkn, 0, 3, nullptr, StateId::HereticChicplayRun4, 0, 0},                     // HERETIC_S_CHICPLAY_RUN3
	{SpriteId::HereticChkn, 1, 3, nullptr, StateId::HereticChicplayRun1, 0, 0},                     // HERETIC_S_CHICPLAY_RUN4
	{SpriteId::HereticChkn, 2, 12, nullptr, StateId::HereticChicplay, 0, 0},                         // HERETIC_S_CHICPLAY_ATK1
	{SpriteId::HereticChkn, 3, 4, DOOM_ACTION(A_Feathers), StateId::HereticChicplayPain2, 0, 0},              // HERETIC_S_CHICPLAY_PAIN
	{SpriteId::HereticChkn, 2, 4, DOOM_ACTION(A_Pain), StateId::HereticChicplay, 0, 0},                        // HERETIC_S_CHICPLAY_PAIN2
	{SpriteId::HereticChkn, 0, 10, DOOM_ACTION(A_ChicLook), StateId::HereticChickenLook2, 0, 0},              // HERETIC_S_CHICKEN_LOOK1
	{SpriteId::HereticChkn, 1, 10, DOOM_ACTION(A_ChicLook), StateId::HereticChickenLook1, 0, 0},              // HERETIC_S_CHICKEN_LOOK2
	{SpriteId::HereticChkn, 0, 3, DOOM_ACTION(A_ChicChase), StateId::HereticChickenWalk2, 0, 0},              // HERETIC_S_CHICKEN_WALK1
	{SpriteId::HereticChkn, 1, 3, DOOM_ACTION(A_ChicChase), StateId::HereticChickenWalk1, 0, 0},              // HERETIC_S_CHICKEN_WALK2
	{SpriteId::HereticChkn, 3, 5, DOOM_ACTION(A_Feathers), StateId::HereticChickenPain2, 0, 0},               // HERETIC_S_CHICKEN_PAIN1
	{SpriteId::HereticChkn, 2, 5, DOOM_ACTION(A_ChicPain), StateId::HereticChickenWalk1, 0, 0},               // HERETIC_S_CHICKEN_PAIN2
	{SpriteId::HereticChkn, 0, 8, DOOM_ACTION(A_FaceTarget), StateId::HereticChickenAtk2, 0, 0},              // HERETIC_S_CHICKEN_ATK1
	{SpriteId::HereticChkn, 2, 10, DOOM_ACTION(A_ChicAttack), StateId::HereticChickenWalk1, 0, 0},            // HERETIC_S_CHICKEN_ATK2
	{SpriteId::HereticChkn, 4, 6, DOOM_ACTION(A_Scream), StateId::HereticChickenDie2, 0, 0},                  // HERETIC_S_CHICKEN_DIE1
	{SpriteId::HereticChkn, 5, 6, DOOM_ACTION(A_Feathers), StateId::HereticChickenDie3, 0, 0},                // HERETIC_S_CHICKEN_DIE2
	{SpriteId::HereticChkn, 6, 6, nullptr, StateId::HereticChickenDie4, 0, 0},                      // HERETIC_S_CHICKEN_DIE3
	{SpriteId::HereticChkn, 7, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticChickenDie5, 0, 0},              // HERETIC_S_CHICKEN_DIE4
	{SpriteId::HereticChkn, 8, 6, nullptr, StateId::HereticChickenDie6, 0, 0},                      // HERETIC_S_CHICKEN_DIE5
	{SpriteId::HereticChkn, 9, 6, nullptr, StateId::HereticChickenDie7, 0, 0},                      // HERETIC_S_CHICKEN_DIE6
	{SpriteId::HereticChkn, 10, 6, nullptr, StateId::HereticChickenDie8, 0, 0},                     // HERETIC_S_CHICKEN_DIE7
	{SpriteId::HereticChkn, 11, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_CHICKEN_DIE8
	{SpriteId::HereticChkn, 12, 3, nullptr, StateId::HereticFeather2, 0, 0},                         // HERETIC_S_FEATHER1
	{SpriteId::HereticChkn, 13, 3, nullptr, StateId::HereticFeather3, 0, 0},                         // HERETIC_S_FEATHER2
	{SpriteId::HereticChkn, 14, 3, nullptr, StateId::HereticFeather4, 0, 0},                         // HERETIC_S_FEATHER3
	{SpriteId::HereticChkn, 15, 3, nullptr, StateId::HereticFeather5, 0, 0},                         // HERETIC_S_FEATHER4
	{SpriteId::HereticChkn, 16, 3, nullptr, StateId::HereticFeather6, 0, 0},                         // HERETIC_S_FEATHER5
	{SpriteId::HereticChkn, 15, 3, nullptr, StateId::HereticFeather7, 0, 0},                         // HERETIC_S_FEATHER6
	{SpriteId::HereticChkn, 14, 3, nullptr, StateId::HereticFeather8, 0, 0},                         // HERETIC_S_FEATHER7
	{SpriteId::HereticChkn, 13, 3, nullptr, StateId::HereticFeather1, 0, 0},                         // HERETIC_S_FEATHER8
	{SpriteId::HereticChkn, 13, 6, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_FEATHERX
	{SpriteId::HereticMumm, 0, 10, DOOM_ACTION(A_Look), StateId::HereticMummyLook2, 0, 0},                    // HERETIC_S_MUMMY_LOOK1
	{SpriteId::HereticMumm, 1, 10, DOOM_ACTION(A_Look), StateId::HereticMummyLook1, 0, 0},                    // HERETIC_S_MUMMY_LOOK2
	{SpriteId::HereticMumm, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticMummyWalk2, 0, 0},                    // HERETIC_S_MUMMY_WALK1
	{SpriteId::HereticMumm, 1, 4, DOOM_ACTION(A_Chase), StateId::HereticMummyWalk3, 0, 0},                    // HERETIC_S_MUMMY_WALK2
	{SpriteId::HereticMumm, 2, 4, DOOM_ACTION(A_Chase), StateId::HereticMummyWalk4, 0, 0},                    // HERETIC_S_MUMMY_WALK3
	{SpriteId::HereticMumm, 3, 4, DOOM_ACTION(A_Chase), StateId::HereticMummyWalk1, 0, 0},                    // HERETIC_S_MUMMY_WALK4
	{SpriteId::HereticMumm, 4, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticMummyAtk2, 0, 0},                // HERETIC_S_MUMMY_ATK1
	{SpriteId::HereticMumm, 5, 6, DOOM_ACTION(A_MummyAttack), StateId::HereticMummyAtk3, 0, 0},               // HERETIC_S_MUMMY_ATK2
	{SpriteId::HereticMumm, 6, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticMummyWalk1, 0, 0},               // HERETIC_S_MUMMY_ATK3
	{SpriteId::HereticMumm, 23, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticMummylAtk2, 0, 0},              // HERETIC_S_MUMMYL_ATK1
	{SpriteId::HereticMumm, 32792, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticMummylAtk3, 0, 0},           // HERETIC_S_MUMMYL_ATK2
	{SpriteId::HereticMumm, 23, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticMummylAtk4, 0, 0},              // HERETIC_S_MUMMYL_ATK3
	{SpriteId::HereticMumm, 32792, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticMummylAtk5, 0, 0},           // HERETIC_S_MUMMYL_ATK4
	{SpriteId::HereticMumm, 23, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticMummylAtk6, 0, 0},              // HERETIC_S_MUMMYL_ATK5
	{SpriteId::HereticMumm, 32792, 15, DOOM_ACTION(A_MummyAttack2), StateId::HereticMummyWalk1, 0, 0},        // HERETIC_S_MUMMYL_ATK6
	{SpriteId::HereticMumm, 7, 4, nullptr, StateId::HereticMummyPain2, 0, 0},                       // HERETIC_S_MUMMY_PAIN1
	{SpriteId::HereticMumm, 7, 4, DOOM_ACTION(A_Pain), StateId::HereticMummyWalk1, 0, 0},                     // HERETIC_S_MUMMY_PAIN2
	{SpriteId::HereticMumm, 8, 5, nullptr, StateId::HereticMummyDie2, 0, 0},                        // HERETIC_S_MUMMY_DIE1
	{SpriteId::HereticMumm, 9, 5, DOOM_ACTION(A_Scream), StateId::HereticMummyDie3, 0, 0},                    // HERETIC_S_MUMMY_DIE2
	{SpriteId::HereticMumm, 10, 5, DOOM_ACTION(A_MummySoul), StateId::HereticMummyDie4, 0, 0},                // HERETIC_S_MUMMY_DIE3
	{SpriteId::HereticMumm, 11, 5, nullptr, StateId::HereticMummyDie5, 0, 0},                       // HERETIC_S_MUMMY_DIE4
	{SpriteId::HereticMumm, 12, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticMummyDie6, 0, 0},               // HERETIC_S_MUMMY_DIE5
	{SpriteId::HereticMumm, 13, 5, nullptr, StateId::HereticMummyDie7, 0, 0},                       // HERETIC_S_MUMMY_DIE6
	{SpriteId::HereticMumm, 14, 5, nullptr, StateId::HereticMummyDie8, 0, 0},                       // HERETIC_S_MUMMY_DIE7
	{SpriteId::HereticMumm, 15, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_MUMMY_DIE8
	{SpriteId::HereticMumm, 16, 5, nullptr, StateId::HereticMummySoul2, 0, 0},                      // HERETIC_S_MUMMY_SOUL1
	{SpriteId::HereticMumm, 17, 5, nullptr, StateId::HereticMummySoul3, 0, 0},                      // HERETIC_S_MUMMY_SOUL2
	{SpriteId::HereticMumm, 18, 5, nullptr, StateId::HereticMummySoul4, 0, 0},                      // HERETIC_S_MUMMY_SOUL3
	{SpriteId::HereticMumm, 19, 9, nullptr, StateId::HereticMummySoul5, 0, 0},                      // HERETIC_S_MUMMY_SOUL4
	{SpriteId::HereticMumm, 20, 5, nullptr, StateId::HereticMummySoul6, 0, 0},                      // HERETIC_S_MUMMY_SOUL5
	{SpriteId::HereticMumm, 21, 5, nullptr, StateId::HereticMummySoul7, 0, 0},                      // HERETIC_S_MUMMY_SOUL6
	{SpriteId::HereticMumm, 22, 5, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_MUMMY_SOUL7
	{SpriteId::HereticFx15, 32768, 5, DOOM_ACTION(A_ContMobjSound), StateId::HereticMummyfx12, 0, 0},         // HERETIC_S_MUMMYFX1_1
	{SpriteId::HereticFx15, 32769, 5, DOOM_ACTION(A_MummyFX1Seek), StateId::HereticMummyfx13, 0, 0},          // HERETIC_S_MUMMYFX1_2
	{SpriteId::HereticFx15, 32770, 5, nullptr, StateId::HereticMummyfx14, 0, 0},                    // HERETIC_S_MUMMYFX1_3
	{SpriteId::HereticFx15, 32769, 5, DOOM_ACTION(A_MummyFX1Seek), StateId::HereticMummyfx11, 0, 0},          // HERETIC_S_MUMMYFX1_4
	{SpriteId::HereticFx15, 32771, 5, nullptr, StateId::HereticMummyfxi12, 0, 0},                   // HERETIC_S_MUMMYFXI1_1
	{SpriteId::HereticFx15, 32772, 5, nullptr, StateId::HereticMummyfxi13, 0, 0},                   // HERETIC_S_MUMMYFXI1_2
	{SpriteId::HereticFx15, 32773, 5, nullptr, StateId::HereticMummyfxi14, 0, 0},                   // HERETIC_S_MUMMYFXI1_3
	{SpriteId::HereticFx15, 32774, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_MUMMYFXI1_4
	{SpriteId::HereticBeas, 0, 10, DOOM_ACTION(A_Look), StateId::HereticBeastLook2, 0, 0},                    // HERETIC_S_BEAST_LOOK1
	{SpriteId::HereticBeas, 1, 10, DOOM_ACTION(A_Look), StateId::HereticBeastLook1, 0, 0},                    // HERETIC_S_BEAST_LOOK2
	{SpriteId::HereticBeas, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk2, 0, 0},                    // HERETIC_S_BEAST_WALK1
	{SpriteId::HereticBeas, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk3, 0, 0},                    // HERETIC_S_BEAST_WALK2
	{SpriteId::HereticBeas, 2, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk4, 0, 0},                    // HERETIC_S_BEAST_WALK3
	{SpriteId::HereticBeas, 3, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk5, 0, 0},                    // HERETIC_S_BEAST_WALK4
	{SpriteId::HereticBeas, 4, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk6, 0, 0},                    // HERETIC_S_BEAST_WALK5
	{SpriteId::HereticBeas, 5, 3, DOOM_ACTION(A_Chase), StateId::HereticBeastWalk1, 0, 0},                    // HERETIC_S_BEAST_WALK6
	{SpriteId::HereticBeas, 7, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticBeastAtk2, 0, 0},               // HERETIC_S_BEAST_ATK1
	{SpriteId::HereticBeas, 8, 10, DOOM_ACTION(A_BeastAttack), StateId::HereticBeastWalk1, 0, 0},             // HERETIC_S_BEAST_ATK2
	{SpriteId::HereticBeas, 6, 3, nullptr, StateId::HereticBeastPain2, 0, 0},                       // HERETIC_S_BEAST_PAIN1
	{SpriteId::HereticBeas, 6, 3, DOOM_ACTION(A_Pain), StateId::HereticBeastWalk1, 0, 0},                     // HERETIC_S_BEAST_PAIN2
	{SpriteId::HereticBeas, 17, 6, nullptr, StateId::HereticBeastDie2, 0, 0},                       // HERETIC_S_BEAST_DIE1
	{SpriteId::HereticBeas, 18, 6, DOOM_ACTION(A_Scream), StateId::HereticBeastDie3, 0, 0},                   // HERETIC_S_BEAST_DIE2
	{SpriteId::HereticBeas, 19, 6, nullptr, StateId::HereticBeastDie4, 0, 0},                       // HERETIC_S_BEAST_DIE3
	{SpriteId::HereticBeas, 20, 6, nullptr, StateId::HereticBeastDie5, 0, 0},                       // HERETIC_S_BEAST_DIE4
	{SpriteId::HereticBeas, 21, 6, nullptr, StateId::HereticBeastDie6, 0, 0},                       // HERETIC_S_BEAST_DIE5
	{SpriteId::HereticBeas, 22, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticBeastDie7, 0, 0},               // HERETIC_S_BEAST_DIE6
	{SpriteId::HereticBeas, 23, 6, nullptr, StateId::HereticBeastDie8, 0, 0},                       // HERETIC_S_BEAST_DIE7
	{SpriteId::HereticBeas, 24, 6, nullptr, StateId::HereticBeastDie9, 0, 0},                       // HERETIC_S_BEAST_DIE8
	{SpriteId::HereticBeas, 25, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_BEAST_DIE9
	{SpriteId::HereticBeas, 9, 5, nullptr, StateId::HereticBeastXdie2, 0, 0},                       // HERETIC_S_BEAST_XDIE1
	{SpriteId::HereticBeas, 10, 6, DOOM_ACTION(A_Scream), StateId::HereticBeastXdie3, 0, 0},                  // HERETIC_S_BEAST_XDIE2
	{SpriteId::HereticBeas, 11, 5, nullptr, StateId::HereticBeastXdie4, 0, 0},                      // HERETIC_S_BEAST_XDIE3
	{SpriteId::HereticBeas, 12, 6, nullptr, StateId::HereticBeastXdie5, 0, 0},                      // HERETIC_S_BEAST_XDIE4
	{SpriteId::HereticBeas, 13, 5, nullptr, StateId::HereticBeastXdie6, 0, 0},                      // HERETIC_S_BEAST_XDIE5
	{SpriteId::HereticBeas, 14, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticBeastXdie7, 0, 0},              // HERETIC_S_BEAST_XDIE6
	{SpriteId::HereticBeas, 15, 5, nullptr, StateId::HereticBeastXdie8, 0, 0},                      // HERETIC_S_BEAST_XDIE7
	{SpriteId::HereticBeas, 16, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_BEAST_XDIE8
	{SpriteId::HereticFrb1, 0, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball2, 0, 0},                 // HERETIC_S_BEASTBALL1
	{SpriteId::HereticFrb1, 0, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball3, 0, 0},                 // HERETIC_S_BEASTBALL2
	{SpriteId::HereticFrb1, 1, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball4, 0, 0},                 // HERETIC_S_BEASTBALL3
	{SpriteId::HereticFrb1, 1, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball5, 0, 0},                 // HERETIC_S_BEASTBALL4
	{SpriteId::HereticFrb1, 2, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball6, 0, 0},                 // HERETIC_S_BEASTBALL5
	{SpriteId::HereticFrb1, 2, 2, DOOM_ACTION(A_BeastPuff), StateId::HereticBeastball1, 0, 0},                 // HERETIC_S_BEASTBALL6
	{SpriteId::HereticFrb1, 3, 4, nullptr, StateId::HereticBeastballx2, 0, 0},                       // HERETIC_S_BEASTBALLX1
	{SpriteId::HereticFrb1, 4, 4, nullptr, StateId::HereticBeastballx3, 0, 0},                       // HERETIC_S_BEASTBALLX2
	{SpriteId::HereticFrb1, 5, 4, nullptr, StateId::HereticBeastballx4, 0, 0},                       // HERETIC_S_BEASTBALLX3
	{SpriteId::HereticFrb1, 6, 4, nullptr, StateId::HereticBeastballx5, 0, 0},                       // HERETIC_S_BEASTBALLX4
	{SpriteId::HereticFrb1, 7, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_BEASTBALLX5
	{SpriteId::HereticFrb1, 0, 4, nullptr, StateId::HereticBurnball2, 0, 0},                         // HERETIC_S_BURNBALL1
	{SpriteId::HereticFrb1, 1, 4, nullptr, StateId::HereticBurnball3, 0, 0},                         // HERETIC_S_BURNBALL2
	{SpriteId::HereticFrb1, 2, 4, nullptr, StateId::HereticBurnball4, 0, 0},                         // HERETIC_S_BURNBALL3
	{SpriteId::HereticFrb1, 3, 4, nullptr, StateId::HereticBurnball5, 0, 0},                         // HERETIC_S_BURNBALL4
	{SpriteId::HereticFrb1, 4, 4, nullptr, StateId::HereticBurnball6, 0, 0},                         // HERETIC_S_BURNBALL5
	{SpriteId::HereticFrb1, 5, 4, nullptr, StateId::HereticBurnball7, 0, 0},                         // HERETIC_S_BURNBALL6
	{SpriteId::HereticFrb1, 6, 4, nullptr, StateId::HereticBurnball8, 0, 0},                         // HERETIC_S_BURNBALL7
	{SpriteId::HereticFrb1, 7, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_BURNBALL8
	{SpriteId::HereticFrb1, 32768, 4, nullptr, StateId::HereticBurnballfb2, 0, 0},                   // HERETIC_S_BURNBALLFB1
	{SpriteId::HereticFrb1, 32769, 4, nullptr, StateId::HereticBurnballfb3, 0, 0},                   // HERETIC_S_BURNBALLFB2
	{SpriteId::HereticFrb1, 32770, 4, nullptr, StateId::HereticBurnballfb4, 0, 0},                   // HERETIC_S_BURNBALLFB3
	{SpriteId::HereticFrb1, 32771, 4, nullptr, StateId::HereticBurnballfb5, 0, 0},                   // HERETIC_S_BURNBALLFB4
	{SpriteId::HereticFrb1, 32772, 4, nullptr, StateId::HereticBurnballfb6, 0, 0},                   // HERETIC_S_BURNBALLFB5
	{SpriteId::HereticFrb1, 32773, 4, nullptr, StateId::HereticBurnballfb7, 0, 0},                   // HERETIC_S_BURNBALLFB6
	{SpriteId::HereticFrb1, 32774, 4, nullptr, StateId::HereticBurnballfb8, 0, 0},                   // HERETIC_S_BURNBALLFB7
	{SpriteId::HereticFrb1, 32775, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_BURNBALLFB8
	{SpriteId::HereticFrb1, 3, 4, nullptr, StateId::HereticPuffy2, 0, 0},                            // HERETIC_S_PUFFY1
	{SpriteId::HereticFrb1, 4, 4, nullptr, StateId::HereticPuffy3, 0, 0},                            // HERETIC_S_PUFFY2
	{SpriteId::HereticFrb1, 5, 4, nullptr, StateId::HereticPuffy4, 0, 0},                            // HERETIC_S_PUFFY3
	{SpriteId::HereticFrb1, 6, 4, nullptr, StateId::HereticPuffy5, 0, 0},                            // HERETIC_S_PUFFY4
	{SpriteId::HereticFrb1, 7, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_PUFFY5
	{SpriteId::HereticSnke, 0, 10, DOOM_ACTION(A_Look), StateId::HereticSnakeLook2, 0, 0},                    // HERETIC_S_SNAKE_LOOK1
	{SpriteId::HereticSnke, 1, 10, DOOM_ACTION(A_Look), StateId::HereticSnakeLook1, 0, 0},                    // HERETIC_S_SNAKE_LOOK2
	{SpriteId::HereticSnke, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticSnakeWalk2, 0, 0},                    // HERETIC_S_SNAKE_WALK1
	{SpriteId::HereticSnke, 1, 4, DOOM_ACTION(A_Chase), StateId::HereticSnakeWalk3, 0, 0},                    // HERETIC_S_SNAKE_WALK2
	{SpriteId::HereticSnke, 2, 4, DOOM_ACTION(A_Chase), StateId::HereticSnakeWalk4, 0, 0},                    // HERETIC_S_SNAKE_WALK3
	{SpriteId::HereticSnke, 3, 4, DOOM_ACTION(A_Chase), StateId::HereticSnakeWalk1, 0, 0},                    // HERETIC_S_SNAKE_WALK4
	{SpriteId::HereticSnke, 5, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticSnakeAtk2, 0, 0},                // HERETIC_S_SNAKE_ATK1
	{SpriteId::HereticSnke, 5, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticSnakeAtk3, 0, 0},                // HERETIC_S_SNAKE_ATK2
	{SpriteId::HereticSnke, 5, 4, DOOM_ACTION(A_SnakeAttack), StateId::HereticSnakeAtk4, 0, 0},               // HERETIC_S_SNAKE_ATK3
	{SpriteId::HereticSnke, 5, 4, DOOM_ACTION(A_SnakeAttack), StateId::HereticSnakeAtk5, 0, 0},               // HERETIC_S_SNAKE_ATK4
	{SpriteId::HereticSnke, 5, 4, DOOM_ACTION(A_SnakeAttack), StateId::HereticSnakeAtk6, 0, 0},               // HERETIC_S_SNAKE_ATK5
	{SpriteId::HereticSnke, 5, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticSnakeAtk7, 0, 0},                // HERETIC_S_SNAKE_ATK6
	{SpriteId::HereticSnke, 5, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticSnakeAtk8, 0, 0},                // HERETIC_S_SNAKE_ATK7
	{SpriteId::HereticSnke, 5, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticSnakeAtk9, 0, 0},                // HERETIC_S_SNAKE_ATK8
	{SpriteId::HereticSnke, 5, 4, DOOM_ACTION(A_SnakeAttack2), StateId::HereticSnakeWalk1, 0, 0},             // HERETIC_S_SNAKE_ATK9
	{SpriteId::HereticSnke, 4, 3, nullptr, StateId::HereticSnakePain2, 0, 0},                       // HERETIC_S_SNAKE_PAIN1
	{SpriteId::HereticSnke, 4, 3, DOOM_ACTION(A_Pain), StateId::HereticSnakeWalk1, 0, 0},                     // HERETIC_S_SNAKE_PAIN2
	{SpriteId::HereticSnke, 6, 5, nullptr, StateId::HereticSnakeDie2, 0, 0},                        // HERETIC_S_SNAKE_DIE1
	{SpriteId::HereticSnke, 7, 5, DOOM_ACTION(A_Scream), StateId::HereticSnakeDie3, 0, 0},                    // HERETIC_S_SNAKE_DIE2
	{SpriteId::HereticSnke, 8, 5, nullptr, StateId::HereticSnakeDie4, 0, 0},                        // HERETIC_S_SNAKE_DIE3
	{SpriteId::HereticSnke, 9, 5, nullptr, StateId::HereticSnakeDie5, 0, 0},                        // HERETIC_S_SNAKE_DIE4
	{SpriteId::HereticSnke, 10, 5, nullptr, StateId::HereticSnakeDie6, 0, 0},                       // HERETIC_S_SNAKE_DIE5
	{SpriteId::HereticSnke, 11, 5, nullptr, StateId::HereticSnakeDie7, 0, 0},                       // HERETIC_S_SNAKE_DIE6
	{SpriteId::HereticSnke, 12, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticSnakeDie8, 0, 0},               // HERETIC_S_SNAKE_DIE7
	{SpriteId::HereticSnke, 13, 5, nullptr, StateId::HereticSnakeDie9, 0, 0},                       // HERETIC_S_SNAKE_DIE8
	{SpriteId::HereticSnke, 14, 5, nullptr, StateId::HereticSnakeDie10, 0, 0},                      // HERETIC_S_SNAKE_DIE9
	{SpriteId::HereticSnke, 15, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_SNAKE_DIE10
	{SpriteId::HereticSnfx, 32768, 5, nullptr, StateId::HereticSnakeproA2, 0, 0},                   // HERETIC_S_SNAKEPRO_A1
	{SpriteId::HereticSnfx, 32769, 5, nullptr, StateId::HereticSnakeproA3, 0, 0},                   // HERETIC_S_SNAKEPRO_A2
	{SpriteId::HereticSnfx, 32770, 5, nullptr, StateId::HereticSnakeproA4, 0, 0},                   // HERETIC_S_SNAKEPRO_A3
	{SpriteId::HereticSnfx, 32771, 5, nullptr, StateId::HereticSnakeproA1, 0, 0},                   // HERETIC_S_SNAKEPRO_A4
	{SpriteId::HereticSnfx, 32772, 5, nullptr, StateId::HereticSnakeproAx2, 0, 0},                  // HERETIC_S_SNAKEPRO_AX1
	{SpriteId::HereticSnfx, 32773, 5, nullptr, StateId::HereticSnakeproAx3, 0, 0},                  // HERETIC_S_SNAKEPRO_AX2
	{SpriteId::HereticSnfx, 32774, 4, nullptr, StateId::HereticSnakeproAx4, 0, 0},                  // HERETIC_S_SNAKEPRO_AX3
	{SpriteId::HereticSnfx, 32775, 3, nullptr, StateId::HereticSnakeproAx5, 0, 0},                  // HERETIC_S_SNAKEPRO_AX4
	{SpriteId::HereticSnfx, 32776, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SNAKEPRO_AX5
	{SpriteId::HereticSnfx, 32777, 6, nullptr, StateId::HereticSnakeproB2, 0, 0},                   // HERETIC_S_SNAKEPRO_B1
	{SpriteId::HereticSnfx, 32778, 6, nullptr, StateId::HereticSnakeproB1, 0, 0},                   // HERETIC_S_SNAKEPRO_B2
	{SpriteId::HereticSnfx, 32779, 5, nullptr, StateId::HereticSnakeproBx2, 0, 0},                  // HERETIC_S_SNAKEPRO_BX1
	{SpriteId::HereticSnfx, 32780, 5, nullptr, StateId::HereticSnakeproBx3, 0, 0},                  // HERETIC_S_SNAKEPRO_BX2
	{SpriteId::HereticSnfx, 32781, 4, nullptr, StateId::HereticSnakeproBx4, 0, 0},                  // HERETIC_S_SNAKEPRO_BX3
	{SpriteId::HereticSnfx, 32782, 3, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SNAKEPRO_BX4
	{SpriteId::HereticHead, 0, 10, DOOM_ACTION(A_Look), StateId::HereticHeadLook, 0, 0},                      // HERETIC_S_HEAD_LOOK
	{SpriteId::HereticHead, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticHeadFloat, 0, 0},                     // HERETIC_S_HEAD_FLOAT
	{SpriteId::HereticHead, 0, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticHeadAtk2, 0, 0},                 // HERETIC_S_HEAD_ATK1
	{SpriteId::HereticHead, 1, 20, DOOM_ACTION(A_HeadAttack), StateId::HereticHeadFloat, 0, 0},               // HERETIC_S_HEAD_ATK2
	{SpriteId::HereticHead, 0, 4, nullptr, StateId::HereticHeadPain2, 0, 0},                        // HERETIC_S_HEAD_PAIN1
	{SpriteId::HereticHead, 0, 4, DOOM_ACTION(A_Pain), StateId::HereticHeadFloat, 0, 0},                      // HERETIC_S_HEAD_PAIN2
	{SpriteId::HereticHead, 2, 7, nullptr, StateId::HereticHeadDie2, 0, 0},                         // HERETIC_S_HEAD_DIE1
	{SpriteId::HereticHead, 3, 7, DOOM_ACTION(A_Scream), StateId::HereticHeadDie3, 0, 0},                     // HERETIC_S_HEAD_DIE2
	{SpriteId::HereticHead, 4, 7, nullptr, StateId::HereticHeadDie4, 0, 0},                         // HERETIC_S_HEAD_DIE3
	{SpriteId::HereticHead, 5, 7, nullptr, StateId::HereticHeadDie5, 0, 0},                         // HERETIC_S_HEAD_DIE4
	{SpriteId::HereticHead, 6, 7, DOOM_ACTION(A_NoBlocking), StateId::HereticHeadDie6, 0, 0},                 // HERETIC_S_HEAD_DIE5
	{SpriteId::HereticHead, 7, 7, nullptr, StateId::HereticHeadDie7, 0, 0},                         // HERETIC_S_HEAD_DIE6
	{SpriteId::HereticHead, 8, -1, DOOM_ACTION(A_BossDeath), StateId::HereticNull, 0, 0},                      // HERETIC_S_HEAD_DIE7
	{SpriteId::HereticFx05, 0, 6, nullptr, StateId::HereticHeadfx12, 0, 0},                         // HERETIC_S_HEADFX1_1
	{SpriteId::HereticFx05, 1, 6, nullptr, StateId::HereticHeadfx13, 0, 0},                         // HERETIC_S_HEADFX1_2
	{SpriteId::HereticFx05, 2, 6, nullptr, StateId::HereticHeadfx11, 0, 0},                         // HERETIC_S_HEADFX1_3
	{SpriteId::HereticFx05, 3, 5, DOOM_ACTION(A_HeadIceImpact), StateId::HereticHeadfxi12, 0, 0},             // HERETIC_S_HEADFXI1_1
	{SpriteId::HereticFx05, 4, 5, nullptr, StateId::HereticHeadfxi13, 0, 0},                        // HERETIC_S_HEADFXI1_2
	{SpriteId::HereticFx05, 5, 5, nullptr, StateId::HereticHeadfxi14, 0, 0},                        // HERETIC_S_HEADFXI1_3
	{SpriteId::HereticFx05, 6, 5, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_HEADFXI1_4
	{SpriteId::HereticFx05, 7, 6, nullptr, StateId::HereticHeadfx22, 0, 0},                         // HERETIC_S_HEADFX2_1
	{SpriteId::HereticFx05, 8, 6, nullptr, StateId::HereticHeadfx23, 0, 0},                         // HERETIC_S_HEADFX2_2
	{SpriteId::HereticFx05, 9, 6, nullptr, StateId::HereticHeadfx21, 0, 0},                         // HERETIC_S_HEADFX2_3
	{SpriteId::HereticFx05, 3, 5, nullptr, StateId::HereticHeadfxi22, 0, 0},                        // HERETIC_S_HEADFXI2_1
	{SpriteId::HereticFx05, 4, 5, nullptr, StateId::HereticHeadfxi23, 0, 0},                        // HERETIC_S_HEADFXI2_2
	{SpriteId::HereticFx05, 5, 5, nullptr, StateId::HereticHeadfxi24, 0, 0},                        // HERETIC_S_HEADFXI2_3
	{SpriteId::HereticFx05, 6, 5, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_HEADFXI2_4
	{SpriteId::HereticFx06, 0, 4, DOOM_ACTION(A_HeadFireGrow), StateId::HereticHeadfx32, 0, 0},               // HERETIC_S_HEADFX3_1
	{SpriteId::HereticFx06, 1, 4, DOOM_ACTION(A_HeadFireGrow), StateId::HereticHeadfx33, 0, 0},               // HERETIC_S_HEADFX3_2
	{SpriteId::HereticFx06, 2, 4, DOOM_ACTION(A_HeadFireGrow), StateId::HereticHeadfx31, 0, 0},               // HERETIC_S_HEADFX3_3
	{SpriteId::HereticFx06, 0, 5, nullptr, StateId::HereticHeadfx35, 0, 0},                         // HERETIC_S_HEADFX3_4
	{SpriteId::HereticFx06, 1, 5, nullptr, StateId::HereticHeadfx36, 0, 0},                         // HERETIC_S_HEADFX3_5
	{SpriteId::HereticFx06, 2, 5, nullptr, StateId::HereticHeadfx34, 0, 0},                         // HERETIC_S_HEADFX3_6
	{SpriteId::HereticFx06, 3, 5, nullptr, StateId::HereticHeadfxi32, 0, 0},                        // HERETIC_S_HEADFXI3_1
	{SpriteId::HereticFx06, 4, 5, nullptr, StateId::HereticHeadfxi33, 0, 0},                        // HERETIC_S_HEADFXI3_2
	{SpriteId::HereticFx06, 5, 5, nullptr, StateId::HereticHeadfxi34, 0, 0},                        // HERETIC_S_HEADFXI3_3
	{SpriteId::HereticFx06, 6, 5, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_HEADFXI3_4
	{SpriteId::HereticFx07, 3, 3, nullptr, StateId::HereticHeadfx42, 0, 0},                         // HERETIC_S_HEADFX4_1
	{SpriteId::HereticFx07, 4, 3, nullptr, StateId::HereticHeadfx43, 0, 0},                         // HERETIC_S_HEADFX4_2
	{SpriteId::HereticFx07, 5, 3, nullptr, StateId::HereticHeadfx44, 0, 0},                         // HERETIC_S_HEADFX4_3
	{SpriteId::HereticFx07, 6, 3, nullptr, StateId::HereticHeadfx45, 0, 0},                         // HERETIC_S_HEADFX4_4
	{SpriteId::HereticFx07, 0, 3, DOOM_ACTION(A_WhirlwindSeek), StateId::HereticHeadfx46, 0, 0},              // HERETIC_S_HEADFX4_5
	{SpriteId::HereticFx07, 1, 3, DOOM_ACTION(A_WhirlwindSeek), StateId::HereticHeadfx47, 0, 0},              // HERETIC_S_HEADFX4_6
	{SpriteId::HereticFx07, 2, 3, DOOM_ACTION(A_WhirlwindSeek), StateId::HereticHeadfx45, 0, 0},              // HERETIC_S_HEADFX4_7
	{SpriteId::HereticFx07, 6, 4, nullptr, StateId::HereticHeadfxi42, 0, 0},                        // HERETIC_S_HEADFXI4_1
	{SpriteId::HereticFx07, 5, 4, nullptr, StateId::HereticHeadfxi43, 0, 0},                        // HERETIC_S_HEADFXI4_2
	{SpriteId::HereticFx07, 4, 4, nullptr, StateId::HereticHeadfxi44, 0, 0},                        // HERETIC_S_HEADFXI4_3
	{SpriteId::HereticFx07, 3, 4, nullptr, StateId::HereticNull, 0, 0},                              // HERETIC_S_HEADFXI4_4
	{SpriteId::HereticClnk, 0, 10, DOOM_ACTION(A_Look), StateId::HereticClinkLook2, 0, 0},                    // HERETIC_S_CLINK_LOOK1
	{SpriteId::HereticClnk, 1, 10, DOOM_ACTION(A_Look), StateId::HereticClinkLook1, 0, 0},                    // HERETIC_S_CLINK_LOOK2
	{SpriteId::HereticClnk, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticClinkWalk2, 0, 0},                    // HERETIC_S_CLINK_WALK1
	{SpriteId::HereticClnk, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticClinkWalk3, 0, 0},                    // HERETIC_S_CLINK_WALK2
	{SpriteId::HereticClnk, 2, 3, DOOM_ACTION(A_Chase), StateId::HereticClinkWalk4, 0, 0},                    // HERETIC_S_CLINK_WALK3
	{SpriteId::HereticClnk, 3, 3, DOOM_ACTION(A_Chase), StateId::HereticClinkWalk1, 0, 0},                    // HERETIC_S_CLINK_WALK4
	{SpriteId::HereticClnk, 4, 5, DOOM_ACTION(A_FaceTarget), StateId::HereticClinkAtk2, 0, 0},                // HERETIC_S_CLINK_ATK1
	{SpriteId::HereticClnk, 5, 4, DOOM_ACTION(A_FaceTarget), StateId::HereticClinkAtk3, 0, 0},                // HERETIC_S_CLINK_ATK2
	{SpriteId::HereticClnk, 6, 7, DOOM_ACTION(A_ClinkAttack), StateId::HereticClinkWalk1, 0, 0},              // HERETIC_S_CLINK_ATK3
	{SpriteId::HereticClnk, 7, 3, nullptr, StateId::HereticClinkPain2, 0, 0},                       // HERETIC_S_CLINK_PAIN1
	{SpriteId::HereticClnk, 7, 3, DOOM_ACTION(A_Pain), StateId::HereticClinkWalk1, 0, 0},                     // HERETIC_S_CLINK_PAIN2
	{SpriteId::HereticClnk, 8, 6, nullptr, StateId::HereticClinkDie2, 0, 0},                        // HERETIC_S_CLINK_DIE1
	{SpriteId::HereticClnk, 9, 6, nullptr, StateId::HereticClinkDie3, 0, 0},                        // HERETIC_S_CLINK_DIE2
	{SpriteId::HereticClnk, 10, 5, DOOM_ACTION(A_Scream), StateId::HereticClinkDie4, 0, 0},                   // HERETIC_S_CLINK_DIE3
	{SpriteId::HereticClnk, 11, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticClinkDie5, 0, 0},               // HERETIC_S_CLINK_DIE4
	{SpriteId::HereticClnk, 12, 5, nullptr, StateId::HereticClinkDie6, 0, 0},                       // HERETIC_S_CLINK_DIE5
	{SpriteId::HereticClnk, 13, 5, nullptr, StateId::HereticClinkDie7, 0, 0},                       // HERETIC_S_CLINK_DIE6
	{SpriteId::HereticClnk, 14, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_CLINK_DIE7
	{SpriteId::HereticWzrd, 0, 10, DOOM_ACTION(A_Look), StateId::HereticWizardLook2, 0, 0},                   // HERETIC_S_WIZARD_LOOK1
	{SpriteId::HereticWzrd, 1, 10, DOOM_ACTION(A_Look), StateId::HereticWizardLook1, 0, 0},                   // HERETIC_S_WIZARD_LOOK2
	{SpriteId::HereticWzrd, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk2, 0, 0},                   // HERETIC_S_WIZARD_WALK1
	{SpriteId::HereticWzrd, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk3, 0, 0},                   // HERETIC_S_WIZARD_WALK2
	{SpriteId::HereticWzrd, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk4, 0, 0},                   // HERETIC_S_WIZARD_WALK3
	{SpriteId::HereticWzrd, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk5, 0, 0},                   // HERETIC_S_WIZARD_WALK4
	{SpriteId::HereticWzrd, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk6, 0, 0},                   // HERETIC_S_WIZARD_WALK5
	{SpriteId::HereticWzrd, 1, 4, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk7, 0, 0},                   // HERETIC_S_WIZARD_WALK6
	{SpriteId::HereticWzrd, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk8, 0, 0},                   // HERETIC_S_WIZARD_WALK7
	{SpriteId::HereticWzrd, 1, 4, DOOM_ACTION(A_Chase), StateId::HereticWizardWalk1, 0, 0},                   // HERETIC_S_WIZARD_WALK8
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk1), StateId::HereticWizardAtk2, 0, 0},                  // HERETIC_S_WIZARD_ATK1
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk2), StateId::HereticWizardAtk3, 0, 0},                  // HERETIC_S_WIZARD_ATK2
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk1), StateId::HereticWizardAtk4, 0, 0},                  // HERETIC_S_WIZARD_ATK3
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk2), StateId::HereticWizardAtk5, 0, 0},                  // HERETIC_S_WIZARD_ATK4
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk1), StateId::HereticWizardAtk6, 0, 0},                  // HERETIC_S_WIZARD_ATK5
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk2), StateId::HereticWizardAtk7, 0, 0},                  // HERETIC_S_WIZARD_ATK6
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk1), StateId::HereticWizardAtk8, 0, 0},                  // HERETIC_S_WIZARD_ATK7
	{SpriteId::HereticWzrd, 2, 4, DOOM_ACTION(A_WizAtk2), StateId::HereticWizardAtk9, 0, 0},                  // HERETIC_S_WIZARD_ATK8
	{SpriteId::HereticWzrd, 3, 12, DOOM_ACTION(A_WizAtk3), StateId::HereticWizardWalk1, 0, 0},                // HERETIC_S_WIZARD_ATK9
	{SpriteId::HereticWzrd, 4, 3, DOOM_ACTION(A_GhostOff), StateId::HereticWizardPain2, 0, 0},                // HERETIC_S_WIZARD_PAIN1
	{SpriteId::HereticWzrd, 4, 3, DOOM_ACTION(A_Pain), StateId::HereticWizardWalk1, 0, 0},                    // HERETIC_S_WIZARD_PAIN2
	{SpriteId::HereticWzrd, 5, 6, DOOM_ACTION(A_GhostOff), StateId::HereticWizardDie2, 0, 0},                 // HERETIC_S_WIZARD_DIE1
	{SpriteId::HereticWzrd, 6, 6, DOOM_ACTION(A_Scream), StateId::HereticWizardDie3, 0, 0},                   // HERETIC_S_WIZARD_DIE2
	{SpriteId::HereticWzrd, 7, 6, nullptr, StateId::HereticWizardDie4, 0, 0},                       // HERETIC_S_WIZARD_DIE3
	{SpriteId::HereticWzrd, 8, 6, nullptr, StateId::HereticWizardDie5, 0, 0},                       // HERETIC_S_WIZARD_DIE4
	{SpriteId::HereticWzrd, 9, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticWizardDie6, 0, 0},               // HERETIC_S_WIZARD_DIE5
	{SpriteId::HereticWzrd, 10, 6, nullptr, StateId::HereticWizardDie7, 0, 0},                      // HERETIC_S_WIZARD_DIE6
	{SpriteId::HereticWzrd, 11, 6, nullptr, StateId::HereticWizardDie8, 0, 0},                      // HERETIC_S_WIZARD_DIE7
	{SpriteId::HereticWzrd, 12, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_WIZARD_DIE8
	{SpriteId::HereticFx11, 32768, 6, nullptr, StateId::HereticWizfx12, 0, 0},                      // HERETIC_S_WIZFX1_1
	{SpriteId::HereticFx11, 32769, 6, nullptr, StateId::HereticWizfx11, 0, 0},                      // HERETIC_S_WIZFX1_2
	{SpriteId::HereticFx11, 32770, 5, nullptr, StateId::HereticWizfxi12, 0, 0},                     // HERETIC_S_WIZFXI1_1
	{SpriteId::HereticFx11, 32771, 5, nullptr, StateId::HereticWizfxi13, 0, 0},                     // HERETIC_S_WIZFXI1_2
	{SpriteId::HereticFx11, 32772, 5, nullptr, StateId::HereticWizfxi14, 0, 0},                     // HERETIC_S_WIZFXI1_3
	{SpriteId::HereticFx11, 32773, 5, nullptr, StateId::HereticWizfxi15, 0, 0},                     // HERETIC_S_WIZFXI1_4
	{SpriteId::HereticFx11, 32774, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_WIZFXI1_5
	{SpriteId::HereticImpx, 0, 10, DOOM_ACTION(A_Look), StateId::HereticImpLook2, 0, 0},                      // HERETIC_S_IMP_LOOK1
	{SpriteId::HereticImpx, 1, 10, DOOM_ACTION(A_Look), StateId::HereticImpLook3, 0, 0},                      // HERETIC_S_IMP_LOOK2
	{SpriteId::HereticImpx, 2, 10, DOOM_ACTION(A_Look), StateId::HereticImpLook4, 0, 0},                      // HERETIC_S_IMP_LOOK3
	{SpriteId::HereticImpx, 1, 10, DOOM_ACTION(A_Look), StateId::HereticImpLook1, 0, 0},                      // HERETIC_S_IMP_LOOK4
	{SpriteId::HereticImpx, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly2, 0, 0},                       // HERETIC_S_IMP_FLY1
	{SpriteId::HereticImpx, 0, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly3, 0, 0},                       // HERETIC_S_IMP_FLY2
	{SpriteId::HereticImpx, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly4, 0, 0},                       // HERETIC_S_IMP_FLY3
	{SpriteId::HereticImpx, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly5, 0, 0},                       // HERETIC_S_IMP_FLY4
	{SpriteId::HereticImpx, 2, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly6, 0, 0},                       // HERETIC_S_IMP_FLY5
	{SpriteId::HereticImpx, 2, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly7, 0, 0},                       // HERETIC_S_IMP_FLY6
	{SpriteId::HereticImpx, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly8, 0, 0},                       // HERETIC_S_IMP_FLY7
	{SpriteId::HereticImpx, 1, 3, DOOM_ACTION(A_Chase), StateId::HereticImpFly1, 0, 0},                       // HERETIC_S_IMP_FLY8
	{SpriteId::HereticImpx, 3, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticImpMeatk2, 0, 0},                // HERETIC_S_IMP_MEATK1
	{SpriteId::HereticImpx, 4, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticImpMeatk3, 0, 0},                // HERETIC_S_IMP_MEATK2
	{SpriteId::HereticImpx, 5, 6, DOOM_ACTION(A_ImpMeAttack), StateId::HereticImpFly1, 0, 0},                 // HERETIC_S_IMP_MEATK3
	{SpriteId::HereticImpx, 0, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticImpMsatk12, 0, 0},             // HERETIC_S_IMP_MSATK1_1
	{SpriteId::HereticImpx, 1, 6, DOOM_ACTION(A_ImpMsAttack), StateId::HereticImpMsatk13, 0, 0},             // HERETIC_S_IMP_MSATK1_2
	{SpriteId::HereticImpx, 2, 6, nullptr, StateId::HereticImpMsatk14, 0, 0},                      // HERETIC_S_IMP_MSATK1_3
	{SpriteId::HereticImpx, 1, 6, nullptr, StateId::HereticImpMsatk15, 0, 0},                      // HERETIC_S_IMP_MSATK1_4
	{SpriteId::HereticImpx, 0, 6, nullptr, StateId::HereticImpMsatk16, 0, 0},                      // HERETIC_S_IMP_MSATK1_5
	{SpriteId::HereticImpx, 1, 6, nullptr, StateId::HereticImpMsatk13, 0, 0},                      // HERETIC_S_IMP_MSATK1_6
	{SpriteId::HereticImpx, 3, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticImpMsatk22, 0, 0},              // HERETIC_S_IMP_MSATK2_1
	{SpriteId::HereticImpx, 4, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticImpMsatk23, 0, 0},              // HERETIC_S_IMP_MSATK2_2
	{SpriteId::HereticImpx, 5, 6, DOOM_ACTION(A_ImpMsAttack2), StateId::HereticImpFly1, 0, 0},                // HERETIC_S_IMP_MSATK2_3
	{SpriteId::HereticImpx, 6, 3, nullptr, StateId::HereticImpPain2, 0, 0},                         // HERETIC_S_IMP_PAIN1
	{SpriteId::HereticImpx, 6, 3, DOOM_ACTION(A_Pain), StateId::HereticImpFly1, 0, 0},                        // HERETIC_S_IMP_PAIN2
	{SpriteId::HereticImpx, 6, 4, DOOM_ACTION(A_ImpDeath), StateId::HereticImpDie2, 0, 0},                    // HERETIC_S_IMP_DIE1
	{SpriteId::HereticImpx, 7, 5, nullptr, StateId::HereticImpDie2, 0, 0},                          // HERETIC_S_IMP_DIE2
	{SpriteId::HereticImpx, 18, 5, DOOM_ACTION(A_ImpXDeath1), StateId::HereticImpXdie2, 0, 0},                // HERETIC_S_IMP_XDIE1
	{SpriteId::HereticImpx, 19, 5, nullptr, StateId::HereticImpXdie3, 0, 0},                        // HERETIC_S_IMP_XDIE2
	{SpriteId::HereticImpx, 20, 5, nullptr, StateId::HereticImpXdie4, 0, 0},                        // HERETIC_S_IMP_XDIE3
	{SpriteId::HereticImpx, 21, 5, DOOM_ACTION(A_ImpXDeath2), StateId::HereticImpXdie5, 0, 0},                // HERETIC_S_IMP_XDIE4
	{SpriteId::HereticImpx, 22, 5, nullptr, StateId::HereticImpXdie5, 0, 0},                        // HERETIC_S_IMP_XDIE5
	{SpriteId::HereticImpx, 8, 7, DOOM_ACTION(A_ImpExplode), StateId::HereticImpCrash2, 0, 0},                // HERETIC_S_IMP_CRASH1
	{SpriteId::HereticImpx, 9, 7, DOOM_ACTION(A_Scream), StateId::HereticImpCrash3, 0, 0},                    // HERETIC_S_IMP_CRASH2
	{SpriteId::HereticImpx, 10, 7, nullptr, StateId::HereticImpCrash4, 0, 0},                       // HERETIC_S_IMP_CRASH3
	{SpriteId::HereticImpx, 11, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_IMP_CRASH4
	{SpriteId::HereticImpx, 23, 7, nullptr, StateId::HereticImpXcrash2, 0, 0},                      // HERETIC_S_IMP_XCRASH1
	{SpriteId::HereticImpx, 24, 7, nullptr, StateId::HereticImpXcrash3, 0, 0},                      // HERETIC_S_IMP_XCRASH2
	{SpriteId::HereticImpx, 25, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_IMP_XCRASH3
	{SpriteId::HereticImpx, 12, 5, nullptr, StateId::HereticImpChunka2, 0, 0},                      // HERETIC_S_IMP_CHUNKA1
	{SpriteId::HereticImpx, 13, 700, nullptr, StateId::HereticImpChunka3, 0, 0},                    // HERETIC_S_IMP_CHUNKA2
	{SpriteId::HereticImpx, 14, 700, nullptr, StateId::HereticNull, 0, 0},                           // HERETIC_S_IMP_CHUNKA3
	{SpriteId::HereticImpx, 15, 5, nullptr, StateId::HereticImpChunkb2, 0, 0},                      // HERETIC_S_IMP_CHUNKB1
	{SpriteId::HereticImpx, 16, 700, nullptr, StateId::HereticImpChunkb3, 0, 0},                    // HERETIC_S_IMP_CHUNKB2
	{SpriteId::HereticImpx, 17, 700, nullptr, StateId::HereticNull, 0, 0},                           // HERETIC_S_IMP_CHUNKB3
	{SpriteId::HereticFx10, 32768, 6, nullptr, StateId::HereticImpfx2, 0, 0},                        // HERETIC_S_IMPFX1
	{SpriteId::HereticFx10, 32769, 6, nullptr, StateId::HereticImpfx3, 0, 0},                        // HERETIC_S_IMPFX2
	{SpriteId::HereticFx10, 32770, 6, nullptr, StateId::HereticImpfx1, 0, 0},                        // HERETIC_S_IMPFX3
	{SpriteId::HereticFx10, 32771, 5, nullptr, StateId::HereticImpfxi2, 0, 0},                       // HERETIC_S_IMPFXI1
	{SpriteId::HereticFx10, 32772, 5, nullptr, StateId::HereticImpfxi3, 0, 0},                       // HERETIC_S_IMPFXI2
	{SpriteId::HereticFx10, 32773, 5, nullptr, StateId::HereticImpfxi4, 0, 0},                       // HERETIC_S_IMPFXI3
	{SpriteId::HereticFx10, 32774, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_IMPFXI4
	{SpriteId::HereticKnig, 0, 10, DOOM_ACTION(A_Look), StateId::HereticKnightStnd2, 0, 0},                   // HERETIC_S_KNIGHT_STND1
	{SpriteId::HereticKnig, 1, 10, DOOM_ACTION(A_Look), StateId::HereticKnightStnd1, 0, 0},                   // HERETIC_S_KNIGHT_STND2
	{SpriteId::HereticKnig, 0, 4, DOOM_ACTION(A_Chase), StateId::HereticKnightWalk2, 0, 0},                   // HERETIC_S_KNIGHT_WALK1
	{SpriteId::HereticKnig, 1, 4, DOOM_ACTION(A_Chase), StateId::HereticKnightWalk3, 0, 0},                   // HERETIC_S_KNIGHT_WALK2
	{SpriteId::HereticKnig, 2, 4, DOOM_ACTION(A_Chase), StateId::HereticKnightWalk4, 0, 0},                   // HERETIC_S_KNIGHT_WALK3
	{SpriteId::HereticKnig, 3, 4, DOOM_ACTION(A_Chase), StateId::HereticKnightWalk1, 0, 0},                   // HERETIC_S_KNIGHT_WALK4
	{SpriteId::HereticKnig, 4, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticKnightAtk2, 0, 0},              // HERETIC_S_KNIGHT_ATK1
	{SpriteId::HereticKnig, 5, 8, DOOM_ACTION(A_FaceTarget), StateId::HereticKnightAtk3, 0, 0},               // HERETIC_S_KNIGHT_ATK2
	{SpriteId::HereticKnig, 6, 8, DOOM_ACTION(A_KnightAttack), StateId::HereticKnightAtk4, 0, 0},             // HERETIC_S_KNIGHT_ATK3
	{SpriteId::HereticKnig, 4, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticKnightAtk5, 0, 0},              // HERETIC_S_KNIGHT_ATK4
	{SpriteId::HereticKnig, 5, 8, DOOM_ACTION(A_FaceTarget), StateId::HereticKnightAtk6, 0, 0},               // HERETIC_S_KNIGHT_ATK5
	{SpriteId::HereticKnig, 6, 8, DOOM_ACTION(A_KnightAttack), StateId::HereticKnightWalk1, 0, 0},            // HERETIC_S_KNIGHT_ATK6
	{SpriteId::HereticKnig, 7, 3, nullptr, StateId::HereticKnightPain2, 0, 0},                      // HERETIC_S_KNIGHT_PAIN1
	{SpriteId::HereticKnig, 7, 3, DOOM_ACTION(A_Pain), StateId::HereticKnightWalk1, 0, 0},                    // HERETIC_S_KNIGHT_PAIN2
	{SpriteId::HereticKnig, 8, 6, nullptr, StateId::HereticKnightDie2, 0, 0},                       // HERETIC_S_KNIGHT_DIE1
	{SpriteId::HereticKnig, 9, 6, DOOM_ACTION(A_Scream), StateId::HereticKnightDie3, 0, 0},                   // HERETIC_S_KNIGHT_DIE2
	{SpriteId::HereticKnig, 10, 6, nullptr, StateId::HereticKnightDie4, 0, 0},                      // HERETIC_S_KNIGHT_DIE3
	{SpriteId::HereticKnig, 11, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticKnightDie5, 0, 0},              // HERETIC_S_KNIGHT_DIE4
	{SpriteId::HereticKnig, 12, 6, nullptr, StateId::HereticKnightDie6, 0, 0},                      // HERETIC_S_KNIGHT_DIE5
	{SpriteId::HereticKnig, 13, 6, nullptr, StateId::HereticKnightDie7, 0, 0},                      // HERETIC_S_KNIGHT_DIE6
	{SpriteId::HereticKnig, 14, -1, nullptr, StateId::HereticNull, 0, 0},                            // HERETIC_S_KNIGHT_DIE7
	{SpriteId::HereticSpax, 32768, 3, DOOM_ACTION(A_ContMobjSound), StateId::HereticSpinaxe2, 0, 0},           // HERETIC_S_SPINAXE1
	{SpriteId::HereticSpax, 32769, 3, nullptr, StateId::HereticSpinaxe3, 0, 0},                      // HERETIC_S_SPINAXE2
	{SpriteId::HereticSpax, 32770, 3, nullptr, StateId::HereticSpinaxe1, 0, 0},                      // HERETIC_S_SPINAXE3
	{SpriteId::HereticSpax, 32771, 6, nullptr, StateId::HereticSpinaxex2, 0, 0},                     // HERETIC_S_SPINAXEX1
	{SpriteId::HereticSpax, 32772, 6, nullptr, StateId::HereticSpinaxex3, 0, 0},                     // HERETIC_S_SPINAXEX2
	{SpriteId::HereticSpax, 32773, 6, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SPINAXEX3
	{SpriteId::HereticRaxe, 32768, 5, DOOM_ACTION(A_DripBlood), StateId::HereticRedaxe2, 0, 0},                // HERETIC_S_REDAXE1
	{SpriteId::HereticRaxe, 32769, 5, DOOM_ACTION(A_DripBlood), StateId::HereticRedaxe1, 0, 0},                // HERETIC_S_REDAXE2
	{SpriteId::HereticRaxe, 32770, 6, nullptr, StateId::HereticRedaxex2, 0, 0},                      // HERETIC_S_REDAXEX1
	{SpriteId::HereticRaxe, 32771, 6, nullptr, StateId::HereticRedaxex3, 0, 0},                      // HERETIC_S_REDAXEX2
	{SpriteId::HereticRaxe, 32772, 6, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_REDAXEX3
	{SpriteId::HereticSrcr, 0, 10, DOOM_ACTION(A_Look), StateId::HereticSrcr1Look2, 0, 0},                    // HERETIC_S_SRCR1_LOOK1
	{SpriteId::HereticSrcr, 1, 10, DOOM_ACTION(A_Look), StateId::HereticSrcr1Look1, 0, 0},                    // HERETIC_S_SRCR1_LOOK2
	{SpriteId::HereticSrcr, 0, 5, DOOM_ACTION(A_Sor1Chase), StateId::HereticSrcr1Walk2, 0, 0},                // HERETIC_S_SRCR1_WALK1
	{SpriteId::HereticSrcr, 1, 5, DOOM_ACTION(A_Sor1Chase), StateId::HereticSrcr1Walk3, 0, 0},                // HERETIC_S_SRCR1_WALK2
	{SpriteId::HereticSrcr, 2, 5, DOOM_ACTION(A_Sor1Chase), StateId::HereticSrcr1Walk4, 0, 0},                // HERETIC_S_SRCR1_WALK3
	{SpriteId::HereticSrcr, 3, 5, DOOM_ACTION(A_Sor1Chase), StateId::HereticSrcr1Walk1, 0, 0},                // HERETIC_S_SRCR1_WALK4
	{SpriteId::HereticSrcr, 16, 6, DOOM_ACTION(A_Sor1Pain), StateId::HereticSrcr1Walk1, 0, 0},                // HERETIC_S_SRCR1_PAIN1
	{SpriteId::HereticSrcr, 16, 7, DOOM_ACTION(A_FaceTarget), StateId::HereticSrcr1Atk2, 0, 0},               // HERETIC_S_SRCR1_ATK1
	{SpriteId::HereticSrcr, 17, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticSrcr1Atk3, 0, 0},               // HERETIC_S_SRCR1_ATK2
	{SpriteId::HereticSrcr, 18, 10, DOOM_ACTION(A_Srcr1Attack), StateId::HereticSrcr1Walk1, 0, 0},            // HERETIC_S_SRCR1_ATK3
	{SpriteId::HereticSrcr, 18, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticSrcr1Atk5, 0, 0},              // HERETIC_S_SRCR1_ATK4
	{SpriteId::HereticSrcr, 16, 7, DOOM_ACTION(A_FaceTarget), StateId::HereticSrcr1Atk6, 0, 0},               // HERETIC_S_SRCR1_ATK5
	{SpriteId::HereticSrcr, 17, 6, DOOM_ACTION(A_FaceTarget), StateId::HereticSrcr1Atk7, 0, 0},               // HERETIC_S_SRCR1_ATK6
	{SpriteId::HereticSrcr, 18, 10, DOOM_ACTION(A_Srcr1Attack), StateId::HereticSrcr1Walk1, 0, 0},            // HERETIC_S_SRCR1_ATK7
	{SpriteId::HereticSrcr, 4, 7, nullptr, StateId::HereticSrcr1Die2, 0, 0},                        // HERETIC_S_SRCR1_DIE1
	{SpriteId::HereticSrcr, 5, 7, DOOM_ACTION(A_Scream), StateId::HereticSrcr1Die3, 0, 0},                    // HERETIC_S_SRCR1_DIE2
	{SpriteId::HereticSrcr, 6, 7, nullptr, StateId::HereticSrcr1Die4, 0, 0},                        // HERETIC_S_SRCR1_DIE3
	{SpriteId::HereticSrcr, 7, 6, nullptr, StateId::HereticSrcr1Die5, 0, 0},                        // HERETIC_S_SRCR1_DIE4
	{SpriteId::HereticSrcr, 8, 6, nullptr, StateId::HereticSrcr1Die6, 0, 0},                        // HERETIC_S_SRCR1_DIE5
	{SpriteId::HereticSrcr, 9, 6, nullptr, StateId::HereticSrcr1Die7, 0, 0},                        // HERETIC_S_SRCR1_DIE6
	{SpriteId::HereticSrcr, 10, 6, nullptr, StateId::HereticSrcr1Die8, 0, 0},                       // HERETIC_S_SRCR1_DIE7
	{SpriteId::HereticSrcr, 11, 25, DOOM_ACTION(A_SorZap), StateId::HereticSrcr1Die9, 0, 0},                  // HERETIC_S_SRCR1_DIE8
	{SpriteId::HereticSrcr, 12, 5, nullptr, StateId::HereticSrcr1Die10, 0, 0},                      // HERETIC_S_SRCR1_DIE9
	{SpriteId::HereticSrcr, 13, 5, nullptr, StateId::HereticSrcr1Die11, 0, 0},                      // HERETIC_S_SRCR1_DIE10
	{SpriteId::HereticSrcr, 14, 4, nullptr, StateId::HereticSrcr1Die12, 0, 0},                      // HERETIC_S_SRCR1_DIE11
	{SpriteId::HereticSrcr, 11, 20, DOOM_ACTION(A_SorZap), StateId::HereticSrcr1Die13, 0, 0},                 // HERETIC_S_SRCR1_DIE12
	{SpriteId::HereticSrcr, 12, 5, nullptr, StateId::HereticSrcr1Die14, 0, 0},                      // HERETIC_S_SRCR1_DIE13
	{SpriteId::HereticSrcr, 13, 5, nullptr, StateId::HereticSrcr1Die15, 0, 0},                      // HERETIC_S_SRCR1_DIE14
	{SpriteId::HereticSrcr, 14, 4, nullptr, StateId::HereticSrcr1Die16, 0, 0},                      // HERETIC_S_SRCR1_DIE15
	{SpriteId::HereticSrcr, 11, 12, nullptr, StateId::HereticSrcr1Die17, 0, 0},                     // HERETIC_S_SRCR1_DIE16
	{SpriteId::HereticSrcr, 15, -1, DOOM_ACTION(A_SorcererRise), StateId::HereticNull, 0, 0},                  // HERETIC_S_SRCR1_DIE17
	{SpriteId::HereticFx14, 32768, 6, nullptr, StateId::HereticSrcrfx12, 0, 0},                     // HERETIC_S_SRCRFX1_1
	{SpriteId::HereticFx14, 32769, 6, nullptr, StateId::HereticSrcrfx13, 0, 0},                     // HERETIC_S_SRCRFX1_2
	{SpriteId::HereticFx14, 32770, 6, nullptr, StateId::HereticSrcrfx11, 0, 0},                     // HERETIC_S_SRCRFX1_3
	{SpriteId::HereticFx14, 32771, 5, nullptr, StateId::HereticSrcrfxi12, 0, 0},                    // HERETIC_S_SRCRFXI1_1
	{SpriteId::HereticFx14, 32772, 5, nullptr, StateId::HereticSrcrfxi13, 0, 0},                    // HERETIC_S_SRCRFXI1_2
	{SpriteId::HereticFx14, 32773, 5, nullptr, StateId::HereticSrcrfxi14, 0, 0},                    // HERETIC_S_SRCRFXI1_3
	{SpriteId::HereticFx14, 32774, 5, nullptr, StateId::HereticSrcrfxi15, 0, 0},                    // HERETIC_S_SRCRFXI1_4
	{SpriteId::HereticFx14, 32775, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SRCRFXI1_5
	{SpriteId::HereticSor2, 0, 4, nullptr, StateId::HereticSor2Rise2, 0, 0},                        // HERETIC_S_SOR2_RISE1
	{SpriteId::HereticSor2, 1, 4, nullptr, StateId::HereticSor2Rise3, 0, 0},                        // HERETIC_S_SOR2_RISE2
	{SpriteId::HereticSor2, 2, 4, DOOM_ACTION(A_SorRise), StateId::HereticSor2Rise4, 0, 0},                   // HERETIC_S_SOR2_RISE3
	{SpriteId::HereticSor2, 3, 4, nullptr, StateId::HereticSor2Rise5, 0, 0},                        // HERETIC_S_SOR2_RISE4
	{SpriteId::HereticSor2, 4, 4, nullptr, StateId::HereticSor2Rise6, 0, 0},                        // HERETIC_S_SOR2_RISE5
	{SpriteId::HereticSor2, 5, 4, nullptr, StateId::HereticSor2Rise7, 0, 0},                        // HERETIC_S_SOR2_RISE6
	{SpriteId::HereticSor2, 6, 12, DOOM_ACTION(A_SorSightSnd), StateId::HereticSor2Walk1, 0, 0},              // HERETIC_S_SOR2_RISE7
	{SpriteId::HereticSor2, 12, 10, DOOM_ACTION(A_Look), StateId::HereticSor2Look2, 0, 0},                    // HERETIC_S_SOR2_LOOK1
	{SpriteId::HereticSor2, 13, 10, DOOM_ACTION(A_Look), StateId::HereticSor2Look1, 0, 0},                    // HERETIC_S_SOR2_LOOK2
	{SpriteId::HereticSor2, 12, 4, DOOM_ACTION(A_Chase), StateId::HereticSor2Walk2, 0, 0},                    // HERETIC_S_SOR2_WALK1
	{SpriteId::HereticSor2, 13, 4, DOOM_ACTION(A_Chase), StateId::HereticSor2Walk3, 0, 0},                    // HERETIC_S_SOR2_WALK2
	{SpriteId::HereticSor2, 14, 4, DOOM_ACTION(A_Chase), StateId::HereticSor2Walk4, 0, 0},                    // HERETIC_S_SOR2_WALK3
	{SpriteId::HereticSor2, 15, 4, DOOM_ACTION(A_Chase), StateId::HereticSor2Walk1, 0, 0},                    // HERETIC_S_SOR2_WALK4
	{SpriteId::HereticSor2, 16, 3, nullptr, StateId::HereticSor2Pain2, 0, 0},                       // HERETIC_S_SOR2_PAIN1
	{SpriteId::HereticSor2, 16, 6, DOOM_ACTION(A_Pain), StateId::HereticSor2Walk1, 0, 0},                     // HERETIC_S_SOR2_PAIN2
	{SpriteId::HereticSor2, 17, 9, DOOM_ACTION(A_Srcr2Decide), StateId::HereticSor2Atk2, 0, 0},               // HERETIC_S_SOR2_ATK1
	{SpriteId::HereticSor2, 18, 9, DOOM_ACTION(A_FaceTarget), StateId::HereticSor2Atk3, 0, 0},                // HERETIC_S_SOR2_ATK2
	{SpriteId::HereticSor2, 19, 20, DOOM_ACTION(A_Srcr2Attack), StateId::HereticSor2Walk1, 0, 0},             // HERETIC_S_SOR2_ATK3
	{SpriteId::HereticSor2, 11, 6, nullptr, StateId::HereticSor2Tele2, 0, 0},                       // HERETIC_S_SOR2_TELE1
	{SpriteId::HereticSor2, 10, 6, nullptr, StateId::HereticSor2Tele3, 0, 0},                       // HERETIC_S_SOR2_TELE2
	{SpriteId::HereticSor2, 9, 6, nullptr, StateId::HereticSor2Tele4, 0, 0},                        // HERETIC_S_SOR2_TELE3
	{SpriteId::HereticSor2, 8, 6, nullptr, StateId::HereticSor2Tele5, 0, 0},                        // HERETIC_S_SOR2_TELE4
	{SpriteId::HereticSor2, 7, 6, nullptr, StateId::HereticSor2Tele6, 0, 0},                        // HERETIC_S_SOR2_TELE5
	{SpriteId::HereticSor2, 6, 6, nullptr, StateId::HereticSor2Walk1, 0, 0},                        // HERETIC_S_SOR2_TELE6
	{SpriteId::HereticSdth, 0, 8, DOOM_ACTION(A_Sor2DthInit), StateId::HereticSor2Die2, 0, 0},                // HERETIC_S_SOR2_DIE1
	{SpriteId::HereticSdth, 1, 8, nullptr, StateId::HereticSor2Die3, 0, 0},                         // HERETIC_S_SOR2_DIE2
	{SpriteId::HereticSdth, 2, 8, DOOM_ACTION(A_SorDSph), StateId::HereticSor2Die4, 0, 0},                    // HERETIC_S_SOR2_DIE3
	{SpriteId::HereticSdth, 3, 7, nullptr, StateId::HereticSor2Die5, 0, 0},                         // HERETIC_S_SOR2_DIE4
	{SpriteId::HereticSdth, 4, 7, nullptr, StateId::HereticSor2Die6, 0, 0},                         // HERETIC_S_SOR2_DIE5
	{SpriteId::HereticSdth, 5, 7, DOOM_ACTION(A_Sor2DthLoop), StateId::HereticSor2Die7, 0, 0},                // HERETIC_S_SOR2_DIE6
	{SpriteId::HereticSdth, 6, 6, DOOM_ACTION(A_SorDExp), StateId::HereticSor2Die8, 0, 0},                    // HERETIC_S_SOR2_DIE7
	{SpriteId::HereticSdth, 7, 6, nullptr, StateId::HereticSor2Die9, 0, 0},                         // HERETIC_S_SOR2_DIE8
	{SpriteId::HereticSdth, 8, 18, nullptr, StateId::HereticSor2Die10, 0, 0},                       // HERETIC_S_SOR2_DIE9
	{SpriteId::HereticSdth, 9, 6, DOOM_ACTION(A_NoBlocking), StateId::HereticSor2Die11, 0, 0},                // HERETIC_S_SOR2_DIE10
	{SpriteId::HereticSdth, 10, 6, DOOM_ACTION(A_SorDBon), StateId::HereticSor2Die12, 0, 0},                  // HERETIC_S_SOR2_DIE11
	{SpriteId::HereticSdth, 11, 6, nullptr, StateId::HereticSor2Die13, 0, 0},                       // HERETIC_S_SOR2_DIE12
	{SpriteId::HereticSdth, 12, 6, nullptr, StateId::HereticSor2Die14, 0, 0},                       // HERETIC_S_SOR2_DIE13
	{SpriteId::HereticSdth, 13, 6, nullptr, StateId::HereticSor2Die15, 0, 0},                       // HERETIC_S_SOR2_DIE14
	{SpriteId::HereticSdth, 14, -1, DOOM_ACTION(A_BossDeath), StateId::HereticNull, 0, 0},                     // HERETIC_S_SOR2_DIE15
	{SpriteId::HereticFx16, 32768, 3, DOOM_ACTION(A_BlueSpark), StateId::HereticSor2fx12, 0, 0},              // HERETIC_S_SOR2FX1_1
	{SpriteId::HereticFx16, 32769, 3, DOOM_ACTION(A_BlueSpark), StateId::HereticSor2fx13, 0, 0},              // HERETIC_S_SOR2FX1_2
	{SpriteId::HereticFx16, 32770, 3, DOOM_ACTION(A_BlueSpark), StateId::HereticSor2fx11, 0, 0},              // HERETIC_S_SOR2FX1_3
	{SpriteId::HereticFx16, 32774, 5, DOOM_ACTION(A_Explode), StateId::HereticSor2fxi12, 0, 0},               // HERETIC_S_SOR2FXI1_1
	{SpriteId::HereticFx16, 32775, 5, nullptr, StateId::HereticSor2fxi13, 0, 0},                    // HERETIC_S_SOR2FXI1_2
	{SpriteId::HereticFx16, 32776, 5, nullptr, StateId::HereticSor2fxi14, 0, 0},                    // HERETIC_S_SOR2FXI1_3
	{SpriteId::HereticFx16, 32777, 5, nullptr, StateId::HereticSor2fxi15, 0, 0},                    // HERETIC_S_SOR2FXI1_4
	{SpriteId::HereticFx16, 32778, 5, nullptr, StateId::HereticSor2fxi16, 0, 0},                    // HERETIC_S_SOR2FXI1_5
	{SpriteId::HereticFx16, 32779, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SOR2FXI1_6
	{SpriteId::HereticFx16, 32771, 12, nullptr, StateId::HereticSor2fxspark2, 0, 0},                 // HERETIC_S_SOR2FXSPARK1
	{SpriteId::HereticFx16, 32772, 12, nullptr, StateId::HereticSor2fxspark3, 0, 0},                 // HERETIC_S_SOR2FXSPARK2
	{SpriteId::HereticFx16, 32773, 12, nullptr, StateId::HereticNull, 0, 0},                         // HERETIC_S_SOR2FXSPARK3
	{SpriteId::HereticFx11, 32768, 35, nullptr, StateId::HereticSor2fx22, 0, 0},                    // HERETIC_S_SOR2FX2_1
	{SpriteId::HereticFx11, 32768, 5, DOOM_ACTION(A_GenWizard), StateId::HereticSor2fx23, 0, 0},              // HERETIC_S_SOR2FX2_2
	{SpriteId::HereticFx11, 32769, 5, nullptr, StateId::HereticSor2fx22, 0, 0},                     // HERETIC_S_SOR2FX2_3
	{SpriteId::HereticFx11, 32770, 5, nullptr, StateId::HereticSor2fxi22, 0, 0},                    // HERETIC_S_SOR2FXI2_1
	{SpriteId::HereticFx11, 32771, 5, nullptr, StateId::HereticSor2fxi23, 0, 0},                    // HERETIC_S_SOR2FXI2_2
	{SpriteId::HereticFx11, 32772, 5, nullptr, StateId::HereticSor2fxi24, 0, 0},                    // HERETIC_S_SOR2FXI2_3
	{SpriteId::HereticFx11, 32773, 5, nullptr, StateId::HereticSor2fxi25, 0, 0},                    // HERETIC_S_SOR2FXI2_4
	{SpriteId::HereticFx11, 32774, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_SOR2FXI2_5
	{SpriteId::HereticSor2, 6, 8, nullptr, StateId::HereticSor2telefade2, 0, 0},                     // HERETIC_S_SOR2TELEFADE1
	{SpriteId::HereticSor2, 7, 6, nullptr, StateId::HereticSor2telefade3, 0, 0},                     // HERETIC_S_SOR2TELEFADE2
	{SpriteId::HereticSor2, 8, 6, nullptr, StateId::HereticSor2telefade4, 0, 0},                     // HERETIC_S_SOR2TELEFADE3
	{SpriteId::HereticSor2, 9, 6, nullptr, StateId::HereticSor2telefade5, 0, 0},                     // HERETIC_S_SOR2TELEFADE4
	{SpriteId::HereticSor2, 10, 6, nullptr, StateId::HereticSor2telefade6, 0, 0},                    // HERETIC_S_SOR2TELEFADE5
	{SpriteId::HereticSor2, 11, 6, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_SOR2TELEFADE6
	{SpriteId::HereticMntr, 0, 10, DOOM_ACTION(A_Look), StateId::HereticMntrLook2, 0, 0},                     // HERETIC_S_MNTR_LOOK1
	{SpriteId::HereticMntr, 1, 10, DOOM_ACTION(A_Look), StateId::HereticMntrLook1, 0, 0},                     // HERETIC_S_MNTR_LOOK2
	{SpriteId::HereticMntr, 0, 5, DOOM_ACTION(A_Chase), StateId::HereticMntrWalk2, 0, 0},                     // HERETIC_S_MNTR_WALK1
	{SpriteId::HereticMntr, 1, 5, DOOM_ACTION(A_Chase), StateId::HereticMntrWalk3, 0, 0},                     // HERETIC_S_MNTR_WALK2
	{SpriteId::HereticMntr, 2, 5, DOOM_ACTION(A_Chase), StateId::HereticMntrWalk4, 0, 0},                     // HERETIC_S_MNTR_WALK3
	{SpriteId::HereticMntr, 3, 5, DOOM_ACTION(A_Chase), StateId::HereticMntrWalk1, 0, 0},                     // HERETIC_S_MNTR_WALK4
	{SpriteId::HereticMntr, 21, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticMntrAtk12, 0, 0},             // HERETIC_S_MNTR_ATK1_1
	{SpriteId::HereticMntr, 22, 7, DOOM_ACTION(A_FaceTarget), StateId::HereticMntrAtk13, 0, 0},              // HERETIC_S_MNTR_ATK1_2
	{SpriteId::HereticMntr, 23, 12, DOOM_ACTION(A_MinotaurAtk1), StateId::HereticMntrWalk1, 0, 0},            // HERETIC_S_MNTR_ATK1_3
	{SpriteId::HereticMntr, 21, 10, DOOM_ACTION(A_MinotaurDecide), StateId::HereticMntrAtk22, 0, 0},         // HERETIC_S_MNTR_ATK2_1
	{SpriteId::HereticMntr, 24, 4, DOOM_ACTION(A_FaceTarget), StateId::HereticMntrAtk23, 0, 0},              // HERETIC_S_MNTR_ATK2_2
	{SpriteId::HereticMntr, 25, 9, DOOM_ACTION(A_MinotaurAtk2), StateId::HereticMntrWalk1, 0, 0},             // HERETIC_S_MNTR_ATK2_3
	{SpriteId::HereticMntr, 21, 10, DOOM_ACTION(A_FaceTarget), StateId::HereticMntrAtk32, 0, 0},             // HERETIC_S_MNTR_ATK3_1
	{SpriteId::HereticMntr, 22, 7, DOOM_ACTION(A_FaceTarget), StateId::HereticMntrAtk33, 0, 0},              // HERETIC_S_MNTR_ATK3_2
	{SpriteId::HereticMntr, 23, 12, DOOM_ACTION(A_MinotaurAtk3), StateId::HereticMntrWalk1, 0, 0},            // HERETIC_S_MNTR_ATK3_3
	{SpriteId::HereticMntr, 23, 12, nullptr, StateId::HereticMntrAtk31, 0, 0},                     // HERETIC_S_MNTR_ATK3_4
	{SpriteId::HereticMntr, 20, 2, DOOM_ACTION(A_MinotaurCharge), StateId::HereticMntrAtk41, 0, 0},          // HERETIC_S_MNTR_ATK4_1
	{SpriteId::HereticMntr, 4, 3, nullptr, StateId::HereticMntrPain2, 0, 0},                        // HERETIC_S_MNTR_PAIN1
	{SpriteId::HereticMntr, 4, 6, DOOM_ACTION(A_Pain), StateId::HereticMntrWalk1, 0, 0},                      // HERETIC_S_MNTR_PAIN2
	{SpriteId::HereticMntr, 5, 6, nullptr, StateId::HereticMntrDie2, 0, 0},                         // HERETIC_S_MNTR_DIE1
	{SpriteId::HereticMntr, 6, 5, nullptr, StateId::HereticMntrDie3, 0, 0},                         // HERETIC_S_MNTR_DIE2
	{SpriteId::HereticMntr, 7, 6, DOOM_ACTION(A_Scream), StateId::HereticMntrDie4, 0, 0},                     // HERETIC_S_MNTR_DIE3
	{SpriteId::HereticMntr, 8, 5, nullptr, StateId::HereticMntrDie5, 0, 0},                         // HERETIC_S_MNTR_DIE4
	{SpriteId::HereticMntr, 9, 6, nullptr, StateId::HereticMntrDie6, 0, 0},                         // HERETIC_S_MNTR_DIE5
	{SpriteId::HereticMntr, 10, 5, nullptr, StateId::HereticMntrDie7, 0, 0},                        // HERETIC_S_MNTR_DIE6
	{SpriteId::HereticMntr, 11, 6, nullptr, StateId::HereticMntrDie8, 0, 0},                        // HERETIC_S_MNTR_DIE7
	{SpriteId::HereticMntr, 12, 5, DOOM_ACTION(A_NoBlocking), StateId::HereticMntrDie9, 0, 0},                // HERETIC_S_MNTR_DIE8
	{SpriteId::HereticMntr, 13, 6, nullptr, StateId::HereticMntrDie10, 0, 0},                       // HERETIC_S_MNTR_DIE9
	{SpriteId::HereticMntr, 14, 5, nullptr, StateId::HereticMntrDie11, 0, 0},                       // HERETIC_S_MNTR_DIE10
	{SpriteId::HereticMntr, 15, 6, nullptr, StateId::HereticMntrDie12, 0, 0},                       // HERETIC_S_MNTR_DIE11
	{SpriteId::HereticMntr, 16, 5, nullptr, StateId::HereticMntrDie13, 0, 0},                       // HERETIC_S_MNTR_DIE12
	{SpriteId::HereticMntr, 17, 6, nullptr, StateId::HereticMntrDie14, 0, 0},                       // HERETIC_S_MNTR_DIE13
	{SpriteId::HereticMntr, 18, 5, nullptr, StateId::HereticMntrDie15, 0, 0},                       // HERETIC_S_MNTR_DIE14
	{SpriteId::HereticMntr, 19, -1, DOOM_ACTION(A_BossDeath), StateId::HereticNull, 0, 0},                     // HERETIC_S_MNTR_DIE15
	{SpriteId::HereticFx12, 32768, 6, nullptr, StateId::HereticMntrfx12, 0, 0},                     // HERETIC_S_MNTRFX1_1
	{SpriteId::HereticFx12, 32769, 6, nullptr, StateId::HereticMntrfx11, 0, 0},                     // HERETIC_S_MNTRFX1_2
	{SpriteId::HereticFx12, 32770, 5, nullptr, StateId::HereticMntrfxi12, 0, 0},                    // HERETIC_S_MNTRFXI1_1
	{SpriteId::HereticFx12, 32771, 5, nullptr, StateId::HereticMntrfxi13, 0, 0},                    // HERETIC_S_MNTRFXI1_2
	{SpriteId::HereticFx12, 32772, 5, nullptr, StateId::HereticMntrfxi14, 0, 0},                    // HERETIC_S_MNTRFXI1_3
	{SpriteId::HereticFx12, 32773, 5, nullptr, StateId::HereticMntrfxi15, 0, 0},                    // HERETIC_S_MNTRFXI1_4
	{SpriteId::HereticFx12, 32774, 5, nullptr, StateId::HereticMntrfxi16, 0, 0},                    // HERETIC_S_MNTRFXI1_5
	{SpriteId::HereticFx12, 32775, 5, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_MNTRFXI1_6
	{SpriteId::HereticFx13, 0, 2, DOOM_ACTION(A_MntrFloorFire), StateId::HereticMntrfx21, 0, 0},              // HERETIC_S_MNTRFX2_1
	{SpriteId::HereticFx13, 32776, 4, DOOM_ACTION(A_Explode), StateId::HereticMntrfxi22, 0, 0},               // HERETIC_S_MNTRFXI2_1
	{SpriteId::HereticFx13, 32777, 4, nullptr, StateId::HereticMntrfxi23, 0, 0},                    // HERETIC_S_MNTRFXI2_2
	{SpriteId::HereticFx13, 32778, 4, nullptr, StateId::HereticMntrfxi24, 0, 0},                    // HERETIC_S_MNTRFXI2_3
	{SpriteId::HereticFx13, 32779, 4, nullptr, StateId::HereticMntrfxi25, 0, 0},                    // HERETIC_S_MNTRFXI2_4
	{SpriteId::HereticFx13, 32780, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_MNTRFXI2_5
	{SpriteId::HereticFx13, 32771, 4, nullptr, StateId::HereticMntrfx32, 0, 0},                     // HERETIC_S_MNTRFX3_1
	{SpriteId::HereticFx13, 32770, 4, nullptr, StateId::HereticMntrfx33, 0, 0},                     // HERETIC_S_MNTRFX3_2
	{SpriteId::HereticFx13, 32769, 5, nullptr, StateId::HereticMntrfx34, 0, 0},                     // HERETIC_S_MNTRFX3_3
	{SpriteId::HereticFx13, 32770, 5, nullptr, StateId::HereticMntrfx35, 0, 0},                     // HERETIC_S_MNTRFX3_4
	{SpriteId::HereticFx13, 32771, 5, nullptr, StateId::HereticMntrfx36, 0, 0},                     // HERETIC_S_MNTRFX3_5
	{SpriteId::HereticFx13, 32772, 5, nullptr, StateId::HereticMntrfx37, 0, 0},                     // HERETIC_S_MNTRFX3_6
	{SpriteId::HereticFx13, 32773, 4, nullptr, StateId::HereticMntrfx38, 0, 0},                     // HERETIC_S_MNTRFX3_7
	{SpriteId::HereticFx13, 32774, 4, nullptr, StateId::HereticMntrfx39, 0, 0},                     // HERETIC_S_MNTRFX3_8
	{SpriteId::HereticFx13, 32775, 4, nullptr, StateId::HereticNull, 0, 0},                          // HERETIC_S_MNTRFX3_9
	{SpriteId::HereticAkyy, 32768, 3, nullptr, StateId::HereticAkyy2, 0, 0},                         // HERETIC_S_AKYY1
	{SpriteId::HereticAkyy, 32769, 3, nullptr, StateId::HereticAkyy3, 0, 0},                         // HERETIC_S_AKYY2
	{SpriteId::HereticAkyy, 32770, 3, nullptr, StateId::HereticAkyy4, 0, 0},                         // HERETIC_S_AKYY3
	{SpriteId::HereticAkyy, 32771, 3, nullptr, StateId::HereticAkyy5, 0, 0},                         // HERETIC_S_AKYY4
	{SpriteId::HereticAkyy, 32772, 3, nullptr, StateId::HereticAkyy6, 0, 0},                         // HERETIC_S_AKYY5
	{SpriteId::HereticAkyy, 32773, 3, nullptr, StateId::HereticAkyy7, 0, 0},                         // HERETIC_S_AKYY6
	{SpriteId::HereticAkyy, 32774, 3, nullptr, StateId::HereticAkyy8, 0, 0},                         // HERETIC_S_AKYY7
	{SpriteId::HereticAkyy, 32775, 3, nullptr, StateId::HereticAkyy9, 0, 0},                         // HERETIC_S_AKYY8
	{SpriteId::HereticAkyy, 32776, 3, nullptr, StateId::HereticAkyy10, 0, 0},                        // HERETIC_S_AKYY9
	{SpriteId::HereticAkyy, 32777, 3, nullptr, StateId::HereticAkyy1, 0, 0},                         // HERETIC_S_AKYY10
	{SpriteId::HereticBkyy, 32768, 3, nullptr, StateId::HereticBkyy2, 0, 0},                         // HERETIC_S_BKYY1
	{SpriteId::HereticBkyy, 32769, 3, nullptr, StateId::HereticBkyy3, 0, 0},                         // HERETIC_S_BKYY2
	{SpriteId::HereticBkyy, 32770, 3, nullptr, StateId::HereticBkyy4, 0, 0},                         // HERETIC_S_BKYY3
	{SpriteId::HereticBkyy, 32771, 3, nullptr, StateId::HereticBkyy5, 0, 0},                         // HERETIC_S_BKYY4
	{SpriteId::HereticBkyy, 32772, 3, nullptr, StateId::HereticBkyy6, 0, 0},                         // HERETIC_S_BKYY5
	{SpriteId::HereticBkyy, 32773, 3, nullptr, StateId::HereticBkyy7, 0, 0},                         // HERETIC_S_BKYY6
	{SpriteId::HereticBkyy, 32774, 3, nullptr, StateId::HereticBkyy8, 0, 0},                         // HERETIC_S_BKYY7
	{SpriteId::HereticBkyy, 32775, 3, nullptr, StateId::HereticBkyy9, 0, 0},                         // HERETIC_S_BKYY8
	{SpriteId::HereticBkyy, 32776, 3, nullptr, StateId::HereticBkyy10, 0, 0},                        // HERETIC_S_BKYY9
	{SpriteId::HereticBkyy, 32777, 3, nullptr, StateId::HereticBkyy1, 0, 0},                         // HERETIC_S_BKYY10
	{SpriteId::HereticCkyy, 32768, 3, nullptr, StateId::HereticCkyy2, 0, 0},                         // HERETIC_S_CKYY1
	{SpriteId::HereticCkyy, 32769, 3, nullptr, StateId::HereticCkyy3, 0, 0},                         // HERETIC_S_CKYY2
	{SpriteId::HereticCkyy, 32770, 3, nullptr, StateId::HereticCkyy4, 0, 0},                         // HERETIC_S_CKYY3
	{SpriteId::HereticCkyy, 32771, 3, nullptr, StateId::HereticCkyy5, 0, 0},                         // HERETIC_S_CKYY4
	{SpriteId::HereticCkyy, 32772, 3, nullptr, StateId::HereticCkyy6, 0, 0},                         // HERETIC_S_CKYY5
	{SpriteId::HereticCkyy, 32773, 3, nullptr, StateId::HereticCkyy7, 0, 0},                         // HERETIC_S_CKYY6
	{SpriteId::HereticCkyy, 32774, 3, nullptr, StateId::HereticCkyy8, 0, 0},                         // HERETIC_S_CKYY7
	{SpriteId::HereticCkyy, 32775, 3, nullptr, StateId::HereticCkyy9, 0, 0},                         // HERETIC_S_CKYY8
	{SpriteId::HereticCkyy, 32776, 3, nullptr, StateId::HereticCkyy1, 0, 0},                         // HERETIC_S_CKYY9
	{SpriteId::HereticAmg1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_AMG1
	{SpriteId::HereticAmg2, 0, 4, nullptr, StateId::HereticAmg22, 0, 0},                            // HERETIC_S_AMG2_1
	{SpriteId::HereticAmg2, 1, 4, nullptr, StateId::HereticAmg23, 0, 0},                            // HERETIC_S_AMG2_2
	{SpriteId::HereticAmg2, 2, 4, nullptr, StateId::HereticAmg21, 0, 0},                            // HERETIC_S_AMG2_3
	{SpriteId::HereticAmm1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_AMM1
	{SpriteId::HereticAmm2, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_AMM2
	{SpriteId::HereticAmc1, 0, -1, nullptr, StateId::HereticNull, 0, 0},                             // HERETIC_S_AMC1
	{SpriteId::HereticAmc2, 0, 5, nullptr, StateId::HereticAmc22, 0, 0},                            // HERETIC_S_AMC2_1
	{SpriteId::HereticAmc2, 1, 5, nullptr, StateId::HereticAmc23, 0, 0},                            // HERETIC_S_AMC2_2
	{SpriteId::HereticAmc2, 2, 5, nullptr, StateId::HereticAmc21, 0, 0},                            // HERETIC_S_AMC2_3
	{SpriteId::HereticAms1, 0, 5, nullptr, StateId::HereticAms12, 0, 0},                            // HERETIC_S_AMS1_1
	{SpriteId::HereticAms1, 1, 5, nullptr, StateId::HereticAms11, 0, 0},                            // HERETIC_S_AMS1_2
	{SpriteId::HereticAms2, 0, 5, nullptr, StateId::HereticAms22, 0, 0},                            // HERETIC_S_AMS2_1
	{SpriteId::HereticAms2, 1, 5, nullptr, StateId::HereticAms21, 0, 0},                            // HERETIC_S_AMS2_2
	{SpriteId::HereticAmp1, 0, 4, nullptr, StateId::HereticAmp12, 0, 0},                            // HERETIC_S_AMP1_1
	{SpriteId::HereticAmp1, 1, 4, nullptr, StateId::HereticAmp13, 0, 0},                            // HERETIC_S_AMP1_2
	{SpriteId::HereticAmp1, 2, 4, nullptr, StateId::HereticAmp11, 0, 0},                            // HERETIC_S_AMP1_3
	{SpriteId::HereticAmp2, 0, 4, nullptr, StateId::HereticAmp22, 0, 0},                            // HERETIC_S_AMP2_1
	{SpriteId::HereticAmp2, 1, 4, nullptr, StateId::HereticAmp23, 0, 0},                            // HERETIC_S_AMP2_2
	{SpriteId::HereticAmp2, 2, 4, nullptr, StateId::HereticAmp21, 0, 0},                            // HERETIC_S_AMP2_3
	{SpriteId::HereticAmb1, 0, 4, nullptr, StateId::HereticAmb12, 0, 0},                            // HERETIC_S_AMB1_1
	{SpriteId::HereticAmb1, 1, 4, nullptr, StateId::HereticAmb13, 0, 0},                            // HERETIC_S_AMB1_2
	{SpriteId::HereticAmb1, 2, 4, nullptr, StateId::HereticAmb11, 0, 0},                            // HERETIC_S_AMB1_3
	{SpriteId::HereticAmb2, 0, 4, nullptr, StateId::HereticAmb22, 0, 0},                            // HERETIC_S_AMB2_1
	{SpriteId::HereticAmb2, 1, 4, nullptr, StateId::HereticAmb23, 0, 0},                            // HERETIC_S_AMB2_2
	{SpriteId::HereticAmb2, 2, 4, nullptr, StateId::HereticAmb21, 0, 0},                            // HERETIC_S_AMB2_3
	{SpriteId::HereticAmg1, 0, 100, DOOM_ACTION(A_ESound), StateId::HereticSndWind, 0, 0},                    // HERETIC_S_SND_WIND
	{SpriteId::HereticAmg1, 0, 85, DOOM_ACTION(A_ESound), StateId::HereticSndWaterfall, 0, 0}                 // HERETIC_S_SND_WATERFALL
};

#undef DOOM_ACTION


raven_mobjinfo_t heretic_mobjinfo[std::to_underlying(MobjType::HereticCount)] = {

	{
		// MT_MISC0
		81,                    // doomednum
		StateId::HereticItemPtn11, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_SPECIAL,            // flags
		MobjFlag2::FloatBob           // flags2
	},

	{
		// MT_ITEMSHIELD1
		85,                   // doomednum
		StateId::HereticItemShld1, // spawnstate
		1000,                 // spawnhealth
		StateId::HereticNull,       // seestate
		SfxId::None,             // seesound
		8,                    // reactiontime
		SfxId::None,             // attacksound
		StateId::HereticNull,       // painstate
		0,                    // painchance
		SfxId::None,             // painsound
		StateId::HereticNull,       // meleestate
		StateId::HereticNull,       // missilestate
		StateId::HereticNull,       // crashstate
		StateId::HereticNull,       // deathstate
		StateId::HereticNull,       // xdeathstate
		SfxId::None,             // deathsound
		0,                    // speed
		20 * FRACUNIT,        // radius
		16 * FRACUNIT,        // height
		100,                  // mass
		0,                    // damage
		SfxId::None,             // activesound
		MF_SPECIAL,           // flags
		MobjFlag2::FloatBob          // flags2
	},

	{
		// MT_ITEMSHIELD2
		31,                    // doomednum
		StateId::HereticItemShd21, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_SPECIAL,            // flags
		MobjFlag2::FloatBob           // flags2
	},

	{
		// MT_MISC1
		8,                         // doomednum
		StateId::HereticItemBagh1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_MISC2
		35,                        // doomednum
		StateId::HereticItemSpmp1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_ARTIINVISIBILITY
		75,                                    // doomednum
		StateId::HereticArtiInvs1,                  // spawnstate
		1000,                                  // spawnhealth
		StateId::HereticNull,                        // seestate
		SfxId::None,                              // seesound
		8,                                     // reactiontime
		SfxId::None,                              // attacksound
		StateId::HereticNull,                        // painstate
		0,                                     // painchance
		SfxId::None,                              // painsound
		StateId::HereticNull,                        // meleestate
		StateId::HereticNull,                        // missilestate
		StateId::HereticNull,                        // crashstate
		StateId::HereticNull,                        // deathstate
		StateId::HereticNull,                        // xdeathstate
		SfxId::None,                              // deathsound
		0,                                     // speed
		20 * FRACUNIT,                         // radius
		16 * FRACUNIT,                         // height
		100,                                   // mass
		0,                                     // damage
		SfxId::None,                              // activesound
		MF_SPECIAL | MF_SHADOW | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob                           // flags2
	},

	{
		// MT_MISC3
		82,                        // doomednum
		StateId::HereticArtiPtn21,     // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_ARTIFLY
		83,                        // doomednum
		StateId::HereticArtiSoar1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_ARTIINVULNERABILITY
		84,                        // doomednum
		StateId::HereticArtiInvu1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_ARTITOMEOFPOWER
		86,                        // doomednum
		StateId::HereticArtiPwbk1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_ARTIEGG
		30,                        // doomednum
		StateId::HereticArtiEggc1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_EGGFX
		-1,                                                     // doomednum
		StateId::HereticEggfx1,                                       // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticEggfxi11,                                    // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		18 * FRACUNIT,                                          // speed
		8 * FRACUNIT,                                           // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_ARTISUPERHEAL
		32,                        // doomednum
		StateId::HereticArtiSphl1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_MISC4
		33,                        // doomednum
		StateId::HereticArtiTrch1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_MISC5
		34,                        // doomednum
		StateId::HereticArtiFbmb1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_FIREBOMB
		-1,                       // doomednum
		StateId::HereticFirebomb1,      // spawnstate
		1000,                     // spawnhealth
		StateId::HereticNull,           // seestate
		SfxId::None,                 // seesound
		8,                        // reactiontime
		SfxId::None,                 // attacksound
		StateId::HereticNull,           // painstate
		0,                        // painchance
		SfxId::None,                 // painsound
		StateId::HereticNull,           // meleestate
		StateId::HereticNull,           // missilestate
		StateId::HereticNull,           // crashstate
		StateId::HereticNull,           // deathstate
		StateId::HereticNull,           // xdeathstate
		SfxId::HereticPhohit,       // deathsound
		0,                        // speed
		20 * FRACUNIT,            // radius
		16 * FRACUNIT,            // height
		100,                      // mass
		0,                        // damage
		SfxId::None,                 // activesound
		MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2{}                         // flags2
	},

	{
		// MT_ARTITELEPORT
		36,                        // doomednum
		StateId::HereticArtiAtlp1,      // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_COUNTITEM, // flags
		MobjFlag2::FloatBob               // flags2
	},

	{
		// MT_POD
		2035,                                                                    // doomednum
		StateId::HereticPodWait1,                                                     // spawnstate
		45,                                                                      // spawnhealth
		StateId::HereticNull,                                                          // seestate
		SfxId::None,                                                                // seesound
		8,                                                                       // reactiontime
		SfxId::None,                                                                // attacksound
		StateId::HereticPodPain1,                                                     // painstate
		255,                                                                     // painchance
		SfxId::None,                                                                // painsound
		StateId::HereticNull,                                                          // meleestate
		StateId::HereticNull,                                                          // missilestate
		StateId::HereticNull,                                                          // crashstate
		StateId::HereticPodDie1,                                                      // deathstate
		StateId::HereticNull,                                                          // xdeathstate
		SfxId::HereticPodexp,                                                      // deathsound
		0,                                                                       // speed
		16 * FRACUNIT,                                                           // radius
		54 * FRACUNIT,                                                           // height
		100,                                                                     // mass
		0,                                                                       // damage
		SfxId::None,                                                                // activesound
		MF_SOLID | MF_NOBLOOD | MF_SHOOTABLE | MF_DROPOFF,                       // flags
		MobjFlag2::WindThrust | MobjFlag2::Pushable | MobjFlag2::Slide | MobjFlag2::PassMobj | MobjFlag2::TeleStomp // flags2
	},

	{
		// MT_PODGOO
		-1,                                          // doomednum
		StateId::HereticPodgoo1,                           // spawnstate
		1000,                                        // spawnhealth
		StateId::HereticNull,                              // seestate
		SfxId::None,                                    // seesound
		8,                                           // reactiontime
		SfxId::None,                                    // attacksound
		StateId::HereticNull,                              // painstate
		0,                                           // painchance
		SfxId::None,                                    // painsound
		StateId::HereticNull,                              // meleestate
		StateId::HereticNull,                              // missilestate
		StateId::HereticNull,                              // crashstate
		StateId::HereticPodgoox,                           // deathstate
		StateId::HereticNull,                              // xdeathstate
		SfxId::None,                                    // deathsound
		0,                                           // speed
		2 * FRACUNIT,                                // radius
		4 * FRACUNIT,                                // height
		100,                                         // mass
		0,                                           // damage
		SfxId::None,                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,     // flags
		MobjFlag2::NoTeleport | MobjFlag2::LoGrav | MobjFlag2::CannotPush // flags2
	},

	{
		// MT_PODGENERATOR
		43,                          // doomednum
		StateId::HereticPodgenerator,      // spawnstate
		1000,                        // spawnhealth
		StateId::HereticNull,              // seestate
		SfxId::None,                    // seesound
		8,                           // reactiontime
		SfxId::None,                    // attacksound
		StateId::HereticNull,              // painstate
		0,                           // painchance
		SfxId::None,                    // painsound
		StateId::HereticNull,              // meleestate
		StateId::HereticNull,              // missilestate
		StateId::HereticNull,              // crashstate
		StateId::HereticNull,              // deathstate
		StateId::HereticNull,              // xdeathstate
		SfxId::None,                    // deathsound
		0,                           // speed
		20 * FRACUNIT,               // radius
		16 * FRACUNIT,               // height
		100,                         // mass
		0,                           // damage
		SfxId::None,                    // activesound
		MF_NOBLOCKMAP | MF_NOSECTOR, // flags
		MobjFlag2{}                            // flags2
	},

	{
		// MT_SPLASH
		-1,                                          // doomednum
		StateId::HereticSplash1,                           // spawnstate
		1000,                                        // spawnhealth
		StateId::HereticNull,                              // seestate
		SfxId::None,                                    // seesound
		8,                                           // reactiontime
		SfxId::None,                                    // attacksound
		StateId::HereticNull,                              // painstate
		0,                                           // painchance
		SfxId::None,                                    // painsound
		StateId::HereticNull,                              // meleestate
		StateId::HereticNull,                              // missilestate
		StateId::HereticNull,                              // crashstate
		StateId::HereticSplashx,                           // deathstate
		StateId::HereticNull,                              // xdeathstate
		SfxId::None,                                    // deathsound
		0,                                           // speed
		2 * FRACUNIT,                                // radius
		4 * FRACUNIT,                                // height
		100,                                         // mass
		0,                                           // damage
		SfxId::None,                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,     // flags
		MobjFlag2::NoTeleport | MobjFlag2::LoGrav | MobjFlag2::CannotPush // flags2
	},

	{
		// MT_SPLASHBASE
		-1,                    // doomednum
		StateId::HereticSplashbase1, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_NOBLOCKMAP,         // flags
		MobjFlag2{}                      // flags2
	},

	{
		// MT_LAVASPLASH
		-1,                    // doomednum
		StateId::HereticLavasplash1, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_NOBLOCKMAP,         // flags
		MobjFlag2{}                      // flags2
	},

	{
		// MT_LAVASMOKE
		-1,                                       // doomednum
		StateId::HereticLavasmoke1,                     // spawnstate
		1000,                                     // spawnhealth
		StateId::HereticNull,                           // seestate
		SfxId::None,                                 // seesound
		8,                                        // reactiontime
		SfxId::None,                                 // attacksound
		StateId::HereticNull,                           // painstate
		0,                                        // painchance
		SfxId::None,                                 // painsound
		StateId::HereticNull,                           // meleestate
		StateId::HereticNull,                           // missilestate
		StateId::HereticNull,                           // crashstate
		StateId::HereticNull,                           // deathstate
		StateId::HereticNull,                           // xdeathstate
		SfxId::None,                                 // deathsound
		0,                                        // speed
		20 * FRACUNIT,                            // radius
		16 * FRACUNIT,                            // height
		100,                                      // mass
		0,                                        // damage
		SfxId::None,                                 // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2{}                                         // flags2
	},

	{
		// MT_SLUDGECHUNK
		-1,                                          // doomednum
		StateId::HereticSludgechunk1,                      // spawnstate
		1000,                                        // spawnhealth
		StateId::HereticNull,                              // seestate
		SfxId::None,                                    // seesound
		8,                                           // reactiontime
		SfxId::None,                                    // attacksound
		StateId::HereticNull,                              // painstate
		0,                                           // painchance
		SfxId::None,                                    // painsound
		StateId::HereticNull,                              // meleestate
		StateId::HereticNull,                              // missilestate
		StateId::HereticNull,                              // crashstate
		StateId::HereticSludgechunkx,                      // deathstate
		StateId::HereticNull,                              // xdeathstate
		SfxId::None,                                    // deathsound
		0,                                           // speed
		2 * FRACUNIT,                                // radius
		4 * FRACUNIT,                                // height
		100,                                         // mass
		0,                                           // damage
		SfxId::None,                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,     // flags
		MobjFlag2::NoTeleport | MobjFlag2::LoGrav | MobjFlag2::CannotPush // flags2
	},

	{
		// MT_SLUDGESPLASH
		-1,                      // doomednum
		StateId::HereticSludgesplash1, // spawnstate
		1000,                    // spawnhealth
		StateId::HereticNull,          // seestate
		SfxId::None,                // seesound
		8,                       // reactiontime
		SfxId::None,                // attacksound
		StateId::HereticNull,          // painstate
		0,                       // painchance
		SfxId::None,                // painsound
		StateId::HereticNull,          // meleestate
		StateId::HereticNull,          // missilestate
		StateId::HereticNull,          // crashstate
		StateId::HereticNull,          // deathstate
		StateId::HereticNull,          // xdeathstate
		SfxId::None,                // deathsound
		0,                       // speed
		20 * FRACUNIT,           // radius
		16 * FRACUNIT,           // height
		100,                     // mass
		0,                       // damage
		SfxId::None,                // activesound
		MF_NOBLOCKMAP,           // flags
		MobjFlag2{}                        // flags2
	},

	{
		// MT_SKULLHANG70
		17,                             // doomednum
		StateId::HereticSkullhang701,        // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		70 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_SKULLHANG60
		24,                             // doomednum
		StateId::HereticSkullhang601,        // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		60 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_SKULLHANG45
		25,                             // doomednum
		StateId::HereticSkullhang451,        // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		45 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_SKULLHANG35
		26,                             // doomednum
		StateId::HereticSkullhang351,        // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		35 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_CHANDELIER
		28,                             // doomednum
		StateId::HereticChandelier1,          // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		60 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_SERPTORCH
		27,                   // doomednum
		StateId::HereticSerptorch1, // spawnstate
		1000,                 // spawnhealth
		StateId::HereticNull,       // seestate
		SfxId::None,             // seesound
		8,                    // reactiontime
		SfxId::None,             // attacksound
		StateId::HereticNull,       // painstate
		0,                    // painchance
		SfxId::None,             // painsound
		StateId::HereticNull,       // meleestate
		StateId::HereticNull,       // missilestate
		StateId::HereticNull,       // crashstate
		StateId::HereticNull,       // deathstate
		StateId::HereticNull,       // xdeathstate
		SfxId::None,             // deathsound
		0,                    // speed
		12 * FRACUNIT,        // radius
		54 * FRACUNIT,        // height
		100,                  // mass
		0,                    // damage
		SfxId::None,             // activesound
		MF_SOLID,             // flags
		MobjFlag2{}                     // flags2
	},

	{
		// MT_SMALLPILLAR
		29,                    // doomednum
		StateId::HereticSmallpillar, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		16 * FRACUNIT,         // radius
		34 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_SOLID,              // flags
		MobjFlag2{}                      // flags2
	},

	{
		// MT_STALAGMITESMALL
		37,                        // doomednum
		StateId::HereticStalagmitesmall, // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		8 * FRACUNIT,              // radius
		32 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SOLID,                  // flags
		MobjFlag2{}                          // flags2
	},

	{
		// MT_STALAGMITELARGE
		38,                        // doomednum
		StateId::HereticStalagmitelarge, // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		12 * FRACUNIT,             // radius
		64 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SOLID,                  // flags
		MobjFlag2{}                          // flags2
	},

	{
		// MT_STALACTITESMALL
		39,                                        // doomednum
		StateId::HereticStalactitesmall,                 // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                  // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticNull,                            // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                  // deathsound
		0,                                         // speed
		8 * FRACUNIT,                              // radius
		36 * FRACUNIT,                             // height
		100,                                       // mass
		0,                                         // damage
		SfxId::None,                                  // activesound
		MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                                          // flags2
	},

	{
		// MT_STALACTITELARGE
		40,                                        // doomednum
		StateId::HereticStalactitelarge,                 // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                  // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticNull,                            // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                  // deathsound
		0,                                         // speed
		12 * FRACUNIT,                             // radius
		68 * FRACUNIT,                             // height
		100,                                       // mass
		0,                                         // damage
		SfxId::None,                                  // activesound
		MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                                          // flags2
	},

	{
		// MT_MISC6
		76,                     // doomednum
		StateId::HereticFirebrazier1, // spawnstate
		1000,                   // spawnhealth
		StateId::HereticNull,         // seestate
		SfxId::None,               // seesound
		8,                      // reactiontime
		SfxId::None,               // attacksound
		StateId::HereticNull,         // painstate
		0,                      // painchance
		SfxId::None,               // painsound
		StateId::HereticNull,         // meleestate
		StateId::HereticNull,         // missilestate
		StateId::HereticNull,         // crashstate
		StateId::HereticNull,         // deathstate
		StateId::HereticNull,         // xdeathstate
		SfxId::None,               // deathsound
		0,                      // speed
		16 * FRACUNIT,          // radius
		44 * FRACUNIT,          // height
		100,                    // mass
		0,                      // damage
		SfxId::None,               // activesound
		MF_SOLID,               // flags
		MobjFlag2{}                       // flags2
	},

	{
		// MT_BARREL
		44,               // doomednum
		StateId::HereticBarrel, // spawnstate
		1000,             // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		12 * FRACUNIT,    // radius
		32 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SOLID,         // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_MISC7
		47,                 // doomednum
		StateId::HereticBrpillar, // spawnstate
		1000,               // spawnhealth
		StateId::HereticNull,     // seestate
		SfxId::None,           // seesound
		8,                  // reactiontime
		SfxId::None,           // attacksound
		StateId::HereticNull,     // painstate
		0,                  // painchance
		SfxId::None,           // painsound
		StateId::HereticNull,     // meleestate
		StateId::HereticNull,     // missilestate
		StateId::HereticNull,     // crashstate
		StateId::HereticNull,     // deathstate
		StateId::HereticNull,     // xdeathstate
		SfxId::None,           // deathsound
		0,                  // speed
		14 * FRACUNIT,      // radius
		128 * FRACUNIT,     // height
		100,                // mass
		0,                  // damage
		SfxId::None,           // activesound
		MF_SOLID,           // flags
		MobjFlag2{}                   // flags2
	},

	{
		// MT_MISC8
		48,                             // doomednum
		StateId::HereticMoss1,                // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		23 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_MISC9
		49,                             // doomednum
		StateId::HereticMoss2,                // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		27 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                               // flags2
	},

	{
		// MT_MISC10
		50,                   // doomednum
		StateId::HereticWalltorch1, // spawnstate
		1000,                 // spawnhealth
		StateId::HereticNull,       // seestate
		SfxId::None,             // seesound
		8,                    // reactiontime
		SfxId::None,             // attacksound
		StateId::HereticNull,       // painstate
		0,                    // painchance
		SfxId::None,             // painsound
		StateId::HereticNull,       // meleestate
		StateId::HereticNull,       // missilestate
		StateId::HereticNull,       // crashstate
		StateId::HereticNull,       // deathstate
		StateId::HereticNull,       // xdeathstate
		SfxId::None,             // deathsound
		0,                    // speed
		20 * FRACUNIT,        // radius
		16 * FRACUNIT,        // height
		100,                  // mass
		0,                    // damage
		SfxId::None,             // activesound
		MF_NOGRAVITY,         // flags
		MobjFlag2{}                     // flags2
	},

	{
		// MT_MISC11
		51,                                        // doomednum
		StateId::HereticHangingcorpse,                   // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                  // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticNull,                            // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                  // deathsound
		0,                                         // speed
		8 * FRACUNIT,                              // radius
		104 * FRACUNIT,                            // height
		100,                                       // mass
		0,                                         // damage
		SfxId::None,                                  // activesound
		MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
		MobjFlag2{}                                          // flags2
	},

	{
		// MT_KEYGIZMOBLUE
		94,                  // doomednum
		StateId::HereticKeygizmo1, // spawnstate
		1000,                // spawnhealth
		StateId::HereticNull,      // seestate
		SfxId::None,            // seesound
		8,                   // reactiontime
		SfxId::None,            // attacksound
		StateId::HereticNull,      // painstate
		0,                   // painchance
		SfxId::None,            // painsound
		StateId::HereticNull,      // meleestate
		StateId::HereticNull,      // missilestate
		StateId::HereticNull,      // crashstate
		StateId::HereticNull,      // deathstate
		StateId::HereticNull,      // xdeathstate
		SfxId::None,            // deathsound
		0,                   // speed
		16 * FRACUNIT,       // radius
		50 * FRACUNIT,       // height
		100,                 // mass
		0,                   // damage
		SfxId::None,            // activesound
		MF_SOLID,            // flags
		MobjFlag2{}                    // flags2
	},

	{
		// MT_KEYGIZMOGREEN
		95,                  // doomednum
		StateId::HereticKeygizmo1, // spawnstate
		1000,                // spawnhealth
		StateId::HereticNull,      // seestate
		SfxId::None,            // seesound
		8,                   // reactiontime
		SfxId::None,            // attacksound
		StateId::HereticNull,      // painstate
		0,                   // painchance
		SfxId::None,            // painsound
		StateId::HereticNull,      // meleestate
		StateId::HereticNull,      // missilestate
		StateId::HereticNull,      // crashstate
		StateId::HereticNull,      // deathstate
		StateId::HereticNull,      // xdeathstate
		SfxId::None,            // deathsound
		0,                   // speed
		16 * FRACUNIT,       // radius
		50 * FRACUNIT,       // height
		100,                 // mass
		0,                   // damage
		SfxId::None,            // activesound
		MF_SOLID,            // flags
		MobjFlag2{}                    // flags2
	},

	{
		// MT_KEYGIZMOYELLOW
		96,                  // doomednum
		StateId::HereticKeygizmo1, // spawnstate
		1000,                // spawnhealth
		StateId::HereticNull,      // seestate
		SfxId::None,            // seesound
		8,                   // reactiontime
		SfxId::None,            // attacksound
		StateId::HereticNull,      // painstate
		0,                   // painchance
		SfxId::None,            // painsound
		StateId::HereticNull,      // meleestate
		StateId::HereticNull,      // missilestate
		StateId::HereticNull,      // crashstate
		StateId::HereticNull,      // deathstate
		StateId::HereticNull,      // xdeathstate
		SfxId::None,            // deathsound
		0,                   // speed
		16 * FRACUNIT,       // radius
		50 * FRACUNIT,       // height
		100,                 // mass
		0,                   // damage
		SfxId::None,            // activesound
		MF_SOLID,            // flags
		MobjFlag2{}                    // flags2
	},

	{
		// MT_KEYGIZMOFLOAT
		-1,                      // doomednum
		StateId::HereticKgzStart,     // spawnstate
		1000,                    // spawnhealth
		StateId::HereticNull,          // seestate
		SfxId::None,                // seesound
		8,                       // reactiontime
		SfxId::None,                // attacksound
		StateId::HereticNull,          // painstate
		0,                       // painchance
		SfxId::None,                // painsound
		StateId::HereticNull,          // meleestate
		StateId::HereticNull,          // missilestate
		StateId::HereticNull,          // crashstate
		StateId::HereticNull,          // deathstate
		StateId::HereticNull,          // xdeathstate
		SfxId::None,                // deathsound
		0,                       // speed
		16 * FRACUNIT,           // radius
		16 * FRACUNIT,           // height
		100,                     // mass
		0,                       // damage
		SfxId::None,                // activesound
		MF_SOLID | MF_NOGRAVITY, // flags
		MobjFlag2{}                        // flags2
	},

	{
		// MT_MISC12
		87,                 // doomednum
		StateId::HereticVolcano1, // spawnstate
		1000,               // spawnhealth
		StateId::HereticNull,     // seestate
		SfxId::None,           // seesound
		8,                  // reactiontime
		SfxId::None,           // attacksound
		StateId::HereticNull,     // painstate
		0,                  // painchance
		SfxId::None,           // painsound
		StateId::HereticNull,     // meleestate
		StateId::HereticNull,     // missilestate
		StateId::HereticNull,     // crashstate
		StateId::HereticNull,     // deathstate
		StateId::HereticNull,     // xdeathstate
		SfxId::None,           // deathsound
		0,                  // speed
		12 * FRACUNIT,      // radius
		20 * FRACUNIT,      // height
		100,                // mass
		0,                  // damage
		SfxId::None,           // activesound
		MF_SOLID,           // flags
		MobjFlag2{}                   // flags2
	},

	{
		// MT_VOLCANOBLAST
		-1,                                          // doomednum
		StateId::HereticVolcanoball1,                      // spawnstate
		1000,                                        // spawnhealth
		StateId::HereticNull,                              // seestate
		SfxId::None,                                    // seesound
		8,                                           // reactiontime
		SfxId::None,                                    // attacksound
		StateId::HereticNull,                              // painstate
		0,                                           // painchance
		SfxId::None,                                    // painsound
		StateId::HereticNull,                              // meleestate
		StateId::HereticNull,                              // missilestate
		StateId::HereticNull,                              // crashstate
		StateId::HereticVolcanoballx1,                     // deathstate
		StateId::HereticNull,                              // xdeathstate
		SfxId::HereticVolhit,                          // deathsound
		2 * FRACUNIT,                                // speed
		8 * FRACUNIT,                                // radius
		8 * FRACUNIT,                                // height
		100,                                         // mass
		2,                                           // damage
		SfxId::None,                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,     // flags
		MobjFlag2::LoGrav | MobjFlag2::NoTeleport | MobjFlag2::FireDamage // flags2
	},

	{
		// MT_VOLCANOTBLAST
		-1,                                          // doomednum
		StateId::HereticVolcanotball1,                     // spawnstate
		1000,                                        // spawnhealth
		StateId::HereticNull,                              // seestate
		SfxId::None,                                    // seesound
		8,                                           // reactiontime
		SfxId::None,                                    // attacksound
		StateId::HereticNull,                              // painstate
		0,                                           // painchance
		SfxId::None,                                    // painsound
		StateId::HereticNull,                              // meleestate
		StateId::HereticNull,                              // missilestate
		StateId::HereticNull,                              // crashstate
		StateId::HereticVolcanotballx1,                    // deathstate
		StateId::HereticNull,                              // xdeathstate
		SfxId::None,                                    // deathsound
		2 * FRACUNIT,                                // speed
		8 * FRACUNIT,                                // radius
		6 * FRACUNIT,                                // height
		100,                                         // mass
		1,                                           // damage
		SfxId::None,                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,     // flags
		MobjFlag2::LoGrav | MobjFlag2::NoTeleport | MobjFlag2::FireDamage // flags2
	},

	{
		// MT_TELEGLITGEN
		74,                                         // doomednum
		StateId::HereticTeleglitgen1,                     // spawnstate
		1000,                                       // spawnhealth
		StateId::HereticNull,                             // seestate
		SfxId::None,                                   // seesound
		8,                                          // reactiontime
		SfxId::None,                                   // attacksound
		StateId::HereticNull,                             // painstate
		0,                                          // painchance
		SfxId::None,                                   // painsound
		StateId::HereticNull,                             // meleestate
		StateId::HereticNull,                             // missilestate
		StateId::HereticNull,                             // crashstate
		StateId::HereticNull,                             // deathstate
		StateId::HereticNull,                             // xdeathstate
		SfxId::None,                                   // deathsound
		0,                                          // speed
		20 * FRACUNIT,                              // radius
		16 * FRACUNIT,                              // height
		100,                                        // mass
		0,                                          // damage
		SfxId::None,                                   // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR, // flags
		MobjFlag2{}                                           // flags2
	},

	{
		// MT_TELEGLITGEN2
		52,                                         // doomednum
		StateId::HereticTeleglitgen2,                     // spawnstate
		1000,                                       // spawnhealth
		StateId::HereticNull,                             // seestate
		SfxId::None,                                   // seesound
		8,                                          // reactiontime
		SfxId::None,                                   // attacksound
		StateId::HereticNull,                             // painstate
		0,                                          // painchance
		SfxId::None,                                   // painsound
		StateId::HereticNull,                             // meleestate
		StateId::HereticNull,                             // missilestate
		StateId::HereticNull,                             // crashstate
		StateId::HereticNull,                             // deathstate
		StateId::HereticNull,                             // xdeathstate
		SfxId::None,                                   // deathsound
		0,                                          // speed
		20 * FRACUNIT,                              // radius
		16 * FRACUNIT,                              // height
		100,                                        // mass
		0,                                          // damage
		SfxId::None,                                   // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR, // flags
		MobjFlag2{}                                           // flags2
	},

	{
		// MT_TELEGLITTER
		-1,                                        // doomednum
		StateId::HereticTeleglitter11,                  // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                  // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticNull,                            // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                  // deathsound
		0,                                         // speed
		20 * FRACUNIT,                             // radius
		16 * FRACUNIT,                             // height
		100,                                       // mass
		0,                                         // damage
		SfxId::None,                                  // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
		MobjFlag2{}                                          // flags2
	},

	{
		// MT_TELEGLITTER2
		-1,                                        // doomednum
		StateId::HereticTeleglitter21,                  // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                  // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticNull,                            // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                  // deathsound
		0,                                         // speed
		20 * FRACUNIT,                             // radius
		16 * FRACUNIT,                             // height
		100,                                       // mass
		0,                                         // damage
		SfxId::None,                                  // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
		MobjFlag2{}                                          // flags2
	},

	{
		// MT_TFOG
		-1,                           // doomednum
		StateId::HereticTfog1,              // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_TELEPORTMAN
		14,                          // doomednum
		StateId::HereticNull,              // spawnstate
		1000,                        // spawnhealth
		StateId::HereticNull,              // seestate
		SfxId::None,                    // seesound
		8,                           // reactiontime
		SfxId::None,                    // attacksound
		StateId::HereticNull,              // painstate
		0,                           // painchance
		SfxId::None,                    // painsound
		StateId::HereticNull,              // meleestate
		StateId::HereticNull,              // missilestate
		StateId::HereticNull,              // crashstate
		StateId::HereticNull,              // deathstate
		StateId::HereticNull,              // xdeathstate
		SfxId::None,                    // deathsound
		0,                           // speed
		20 * FRACUNIT,               // radius
		16 * FRACUNIT,               // height
		100,                         // mass
		0,                           // damage
		SfxId::None,                    // activesound
		MF_NOBLOCKMAP | MF_NOSECTOR, // flags
		MobjFlag2{}                            // flags2
	},

	{
		// MT_STAFFPUFF
		-1,                           // doomednum
		StateId::HereticStaffpuff1,         // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::HereticStfhit,           // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_STAFFPUFF2
		-1,                           // doomednum
		StateId::HereticStaffpuff21,       // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::HereticStfpow,           // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_BEAKPUFF
		-1,                           // doomednum
		StateId::HereticStaffpuff1,         // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::HereticChicatk,          // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_MISC13
		2005,           // doomednum
		StateId::HereticWgnt, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_GAUNTLETPUFF1
		-1,                                       // doomednum
		StateId::HereticGauntletpuff11,                // spawnstate
		1000,                                     // spawnhealth
		StateId::HereticNull,                           // seestate
		SfxId::None,                                 // seesound
		8,                                        // reactiontime
		SfxId::None,                                 // attacksound
		StateId::HereticNull,                           // painstate
		0,                                        // painchance
		SfxId::None,                                 // painsound
		StateId::HereticNull,                           // meleestate
		StateId::HereticNull,                           // missilestate
		StateId::HereticNull,                           // crashstate
		StateId::HereticNull,                           // deathstate
		StateId::HereticNull,                           // xdeathstate
		SfxId::None,                                 // deathsound
		0,                                        // speed
		20 * FRACUNIT,                            // radius
		16 * FRACUNIT,                            // height
		100,                                      // mass
		0,                                        // damage
		SfxId::None,                                 // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2{}                                         // flags2
	},

	{
		// MT_GAUNTLETPUFF2
		-1,                                       // doomednum
		StateId::HereticGauntletpuff21,                // spawnstate
		1000,                                     // spawnhealth
		StateId::HereticNull,                           // seestate
		SfxId::None,                                 // seesound
		8,                                        // reactiontime
		SfxId::None,                                 // attacksound
		StateId::HereticNull,                           // painstate
		0,                                        // painchance
		SfxId::None,                                 // painsound
		StateId::HereticNull,                           // meleestate
		StateId::HereticNull,                           // missilestate
		StateId::HereticNull,                           // crashstate
		StateId::HereticNull,                           // deathstate
		StateId::HereticNull,                           // xdeathstate
		SfxId::None,                                 // deathsound
		0,                                        // speed
		20 * FRACUNIT,                            // radius
		16 * FRACUNIT,                            // height
		100,                                      // mass
		0,                                        // damage
		SfxId::None,                                 // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2{}                                         // flags2
	},

	{
		// MT_MISC14
		53,             // doomednum
		StateId::HereticBlsr, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_BLASTERFX1
		-1,                                                     // doomednum
		StateId::HereticBlasterfx11,                                 // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                               // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticBlasterfxi11,                                // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticBlshit,                                     // deathsound
		184 * FRACUNIT,                                         // speed
		12 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_BLASTERSMOKE
		-1,                                       // doomednum
		StateId::HereticBlastersmoke1,                  // spawnstate
		1000,                                     // spawnhealth
		StateId::HereticNull,                           // seestate
		SfxId::None,                                 // seesound
		8,                                        // reactiontime
		SfxId::None,                                 // attacksound
		StateId::HereticNull,                           // painstate
		0,                                        // painchance
		SfxId::None,                                 // painsound
		StateId::HereticNull,                           // meleestate
		StateId::HereticNull,                           // missilestate
		StateId::HereticNull,                           // crashstate
		StateId::HereticNull,                           // deathstate
		StateId::HereticNull,                           // xdeathstate
		SfxId::None,                                 // deathsound
		0,                                        // speed
		20 * FRACUNIT,                            // radius
		16 * FRACUNIT,                            // height
		100,                                      // mass
		0,                                        // damage
		SfxId::None,                                 // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2::NoTeleport | MobjFlag2::CannotPush           // flags2
	},

	{
		// MT_RIPPER
		-1,                                                     // doomednum
		StateId::HereticRipper1,                                      // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRipperx1,                                     // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		14 * FRACUNIT,                                          // speed
		8 * FRACUNIT,                                           // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::Rip                                // flags2
	},

	{
		// MT_BLASTERPUFF1
		-1,                           // doomednum
		StateId::HereticBlasterpuff11,     // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_BLASTERPUFF2
		-1,                           // doomednum
		StateId::HereticBlasterpuff21,     // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_WMACE
		2002,           // doomednum
		StateId::HereticWmce, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_MACEFX1
		-1,                                                     // doomednum
		StateId::HereticMacefx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticLobsht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticMacefxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		20 * FRACUNIT,                                          // speed
		8 * FRACUNIT,                                           // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::FloorBounce | MobjFlag2::ThruGhost | MobjFlag2::NoTeleport        // flags2
	},

	{
		// MT_MACEFX2
		-1,                                                           // doomednum
		StateId::HereticMacefx21,                                          // spawnstate
		1000,                                                         // spawnhealth
		StateId::HereticNull,                                               // seestate
		SfxId::None,                                                            // seesound
		8,                                                            // reactiontime
		SfxId::None,                                                     // attacksound
		StateId::HereticNull,                                               // painstate
		0,                                                            // painchance
		SfxId::None,                                                     // painsound
		StateId::HereticNull,                                               // meleestate
		StateId::HereticNull,                                               // missilestate
		StateId::HereticNull,                                               // crashstate
		StateId::HereticMacefxi21,                                         // deathstate
		StateId::HereticNull,                                               // xdeathstate
		SfxId::None,                                                            // deathsound
		10 * FRACUNIT,                                                // speed
		8 * FRACUNIT,                                                 // radius
		6 * FRACUNIT,                                                 // height
		100,                                                          // mass
		6,                                                            // damage
		SfxId::None,                                                     // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,                      // flags
		MobjFlag2::LoGrav | MobjFlag2::FloorBounce | MobjFlag2::ThruGhost | MobjFlag2::NoTeleport // flags2
	},

	{
		// MT_MACEFX3
		-1,                                                           // doomednum
		StateId::HereticMacefx31,                                          // spawnstate
		1000,                                                         // spawnhealth
		StateId::HereticNull,                                               // seestate
		SfxId::None,                                                            // seesound
		8,                                                            // reactiontime
		SfxId::None,                                                     // attacksound
		StateId::HereticNull,                                               // painstate
		0,                                                            // painchance
		SfxId::None,                                                     // painsound
		StateId::HereticNull,                                               // meleestate
		StateId::HereticNull,                                               // missilestate
		StateId::HereticNull,                                               // crashstate
		StateId::HereticMacefxi11,                                         // deathstate
		StateId::HereticNull,                                               // xdeathstate
		SfxId::None,                                                            // deathsound
		7 * FRACUNIT,                                                 // speed
		8 * FRACUNIT,                                                 // radius
		6 * FRACUNIT,                                                 // height
		100,                                                          // mass
		4,                                                            // damage
		SfxId::None,                                                     // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,                      // flags
		MobjFlag2::LoGrav | MobjFlag2::FloorBounce | MobjFlag2::ThruGhost | MobjFlag2::NoTeleport // flags2
	},

	{
		// MT_MACEFX4
		-1,                                                          // doomednum
		StateId::HereticMacefx41,                                         // spawnstate
		1000,                                                        // spawnhealth
		StateId::HereticNull,                                              // seestate
		SfxId::None,                                                           // seesound
		8,                                                           // reactiontime
		SfxId::None,                                                    // attacksound
		StateId::HereticNull,                                              // painstate
		0,                                                           // painchance
		SfxId::None,                                                    // painsound
		StateId::HereticNull,                                              // meleestate
		StateId::HereticNull,                                              // missilestate
		StateId::HereticNull,                                              // crashstate
		StateId::HereticMacefxi41,                                        // deathstate
		StateId::HereticNull,                                              // xdeathstate
		SfxId::None,                                                           // deathsound
		7 * FRACUNIT,                                                // speed
		8 * FRACUNIT,                                                // radius
		6 * FRACUNIT,                                                // height
		100,                                                         // mass
		18,                                                          // damage
		SfxId::None,                                                    // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,                     // flags
		MobjFlag2::LoGrav | MobjFlag2::FloorBounce | MobjFlag2::ThruGhost | MobjFlag2::TeleStomp // flags2
	},

	{
		// MT_WSKULLROD
		2004,           // doomednum
		StateId::HereticWskl, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_HORNRODFX1
		-1,                                                     // doomednum
		StateId::HereticHrodfx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticHrnsht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticHrodfxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		22 * FRACUNIT,                                          // speed
		12 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		3,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport                         // flags2
	},

	{
		// MT_HORNRODFX2
		-1,                                                     // doomednum
		StateId::HereticHrodfx21,                                    // spawnstate
		4 * 35,                                                 // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticHrnsht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticHrodfxi21,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticRamphit,                                    // deathsound
		22 * FRACUNIT,                                          // speed
		12 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		10,                                                     // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_RAINPLR1
		-1,                                                     // doomednum
		StateId::HereticRainplr11,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRainplr1x1,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		12 * FRACUNIT,                                          // speed
		5 * FRACUNIT,                                           // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		5,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_RAINPLR2
		-1,                                                     // doomednum
		StateId::HereticRainplr21,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRainplr2x1,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		12 * FRACUNIT,                                          // speed
		5 * FRACUNIT,                                           // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		5,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_RAINPLR3
		-1,                                                     // doomednum
		StateId::HereticRainplr31,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRainplr3x1,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		12 * FRACUNIT,                                          // speed
		5 * FRACUNIT,                                           // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		5,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_RAINPLR4
		-1,                                                     // doomednum
		StateId::HereticRainplr41,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRainplr4x1,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		12 * FRACUNIT,                                          // speed
		5 * FRACUNIT,                                           // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		5,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_GOLDWANDFX1
		-1,                                                     // doomednum
		StateId::HereticGwandfx11,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticGwandfxi11,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticGldhit,                                     // deathsound
		22 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_GOLDWANDFX2
		-1,                                                     // doomednum
		StateId::HereticGwandfx21,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticGwandfxi11,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		18 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_GOLDWANDPUFF1
		-1,                           // doomednum
		StateId::HereticGwandpuff11,       // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_GOLDWANDPUFF2
		-1,                           // doomednum
		StateId::HereticGwandfxi11,        // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_WPHOENIXROD
		2003,           // doomednum
		StateId::HereticWphx, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_PHOENIXFX1
		-1,                                                     // doomednum
		StateId::HereticPhoenixfx11,                                 // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticPhosht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticPhoenixfxi11,                                // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticPhohit,                                     // deathsound
		20 * FRACUNIT,                                          // speed
		11 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		20,                                                     // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::ThruGhost | MobjFlag2::NoTeleport                          // flags2
	},

	// The following thing is present in the mobjinfo table from Heretic 1.0,
	// but not in Heretic 1.3 (ie. it was removed).  It has been re-inserted
	// here to support HHE patches.

	{
		// MT_PHOENIXFX_REMOVED
		-1,                                                     // doomednum
		StateId::HereticPhoenixfxix1,                                // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                               // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticPhoenixfxix3,                                // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                               // deathsound
		0,                                                      // speed
		2 * FRACUNIT,                                           // radius
		4 * FRACUNIT,                                           // height
		100,                                                    // mass
		0,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_PHOENIXPUFF
		-1,                                       // doomednum
		StateId::HereticPhoenixpuff1,                   // spawnstate
		1000,                                     // spawnhealth
		StateId::HereticNull,                           // seestate
		SfxId::None,                                 // seesound
		8,                                        // reactiontime
		SfxId::None,                                 // attacksound
		StateId::HereticNull,                           // painstate
		0,                                        // painchance
		SfxId::None,                                 // painsound
		StateId::HereticNull,                           // meleestate
		StateId::HereticNull,                           // missilestate
		StateId::HereticNull,                           // crashstate
		StateId::HereticNull,                           // deathstate
		StateId::HereticNull,                           // xdeathstate
		SfxId::None,                                 // deathsound
		0,                                        // speed
		20 * FRACUNIT,                            // radius
		16 * FRACUNIT,                            // height
		100,                                      // mass
		0,                                        // damage
		SfxId::None,                                 // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2::NoTeleport | MobjFlag2::CannotPush           // flags2
	},

	{
		// MT_PHOENIXFX2
		-1,                                                     // doomednum
		StateId::HereticPhoenixfx21,                                 // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticPhoenixfxi21,                                // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		10 * FRACUNIT,                                          // speed
		6 * FRACUNIT,                                           // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::FireDamage                         // flags2
	},

	{
		// MT_MISC15
		2001,           // doomednum
		StateId::HereticWbow, // spawnstate
		1000,           // spawnhealth
		StateId::HereticNull, // seestate
		SfxId::None,       // seesound
		8,              // reactiontime
		SfxId::None,       // attacksound
		StateId::HereticNull, // painstate
		0,              // painchance
		SfxId::None,       // painsound
		StateId::HereticNull, // meleestate
		StateId::HereticNull, // missilestate
		StateId::HereticNull, // crashstate
		StateId::HereticNull, // deathstate
		StateId::HereticNull, // xdeathstate
		SfxId::None,       // deathsound
		0,              // speed
		20 * FRACUNIT,  // radius
		16 * FRACUNIT,  // height
		100,            // mass
		0,              // damage
		SfxId::None,       // activesound
		MF_SPECIAL,     // flags
		MobjFlag2{}               // flags2
	},

	{
		// MT_CRBOWFX1
		-1,                                                     // doomednum
		StateId::HereticCrbowfx1,                                     // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticBowsht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticCrbowfxi11,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		30 * FRACUNIT,                                          // speed
		11 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		10,                                                     // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_CRBOWFX2
		-1,                                                     // doomednum
		StateId::HereticCrbowfx2,                                     // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::HereticBowsht,                                     // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticCrbowfxi11,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		32 * FRACUNIT,                                          // speed
		11 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		6,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_CRBOWFX3
		-1,                                                     // doomednum
		StateId::HereticCrbowfx3,                                     // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticCrbowfxi31,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		20 * FRACUNIT,                                          // speed
		11 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::ThruGhost | MobjFlag2::NoTeleport         // flags2
	},

	{
		// MT_CRBOWFX4
		-1,                   // doomednum
		StateId::HereticCrbowfx41, // spawnstate
		1000,                 // spawnhealth
		StateId::HereticNull,       // seestate
		SfxId::None,             // seesound
		8,                    // reactiontime
		SfxId::None,             // attacksound
		StateId::HereticNull,       // painstate
		0,                    // painchance
		SfxId::None,             // painsound
		StateId::HereticNull,       // meleestate
		StateId::HereticNull,       // missilestate
		StateId::HereticNull,       // crashstate
		StateId::HereticNull,       // deathstate
		StateId::HereticNull,       // xdeathstate
		SfxId::None,             // deathsound
		0,                    // speed
		20 * FRACUNIT,        // radius
		16 * FRACUNIT,        // height
		100,                  // mass
		0,                    // damage
		SfxId::None,             // activesound
		MF_NOBLOCKMAP,        // flags
		MobjFlag2::LoGrav            // flags2
	},

	{
		// MT_BLOOD
		-1,               // doomednum
		StateId::HereticBlood1, // spawnstate
		1000,             // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_NOBLOCKMAP,    // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_BLOODSPLATTER
		-1,                                      // doomednum
		StateId::HereticBloodsplatter1,                // spawnstate
		1000,                                    // spawnhealth
		StateId::HereticNull,                          // seestate
		SfxId::None,                                // seesound
		8,                                       // reactiontime
		SfxId::None,                                // attacksound
		StateId::HereticNull,                          // painstate
		0,                                       // painchance
		SfxId::None,                                // painsound
		StateId::HereticNull,                          // meleestate
		StateId::HereticNull,                          // missilestate
		StateId::HereticNull,                          // crashstate
		StateId::HereticBloodsplatterx,                // deathstate
		StateId::HereticNull,                          // xdeathstate
		SfxId::None,                                // deathsound
		0,                                       // speed
		2 * FRACUNIT,                            // radius
		4 * FRACUNIT,                            // height
		100,                                     // mass
		0,                                       // damage
		SfxId::None,                                // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF, // flags
		MobjFlag2::NoTeleport | MobjFlag2::CannotPush          // flags2
	},

	{
		// MT_PLAYER
		-1,                                                                      // doomednum
		StateId::HereticPlay,                                                          // spawnstate
		100,                                                                     // spawnhealth
		StateId::HereticPlayRun1,                                                     // seestate
		SfxId::None,                                                                // seesound
		0,                                                                       // reactiontime
		SfxId::None,                                                                // attacksound
		StateId::HereticPlayPain,                                                     // painstate
		255,                                                                     // painchance
		SfxId::HereticPlrpai,                                                      // painsound
		StateId::HereticNull,                                                          // meleestate
		StateId::HereticPlayAtk1,                                                     // missilestate
		StateId::HereticNull,                                                          // crashstate
		StateId::HereticPlayDie1,                                                     // deathstate
		StateId::HereticPlayXdie1,                                                    // xdeathstate
		SfxId::HereticPlrdth,                                                      // deathsound
		0,                                                                       // speed
		16 * FRACUNIT,                                                           // radius
		56 * FRACUNIT,                                                           // height
		100,                                                                     // mass
		0,                                                                       // damage
		SfxId::None,                                                                // activesound
		MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_PICKUP | MF_NOTDMATCH,         // flags
		MobjFlag2::WindThrust | MobjFlag2::FootClip | MobjFlag2::Slide | MobjFlag2::PassMobj | MobjFlag2::TeleStomp // flags2
	},

	{
		// MT_BLOODYSKULL
		-1,                         // doomednum
		StateId::HereticBloodyskull1,     // spawnstate
		1000,                       // spawnhealth
		StateId::HereticNull,             // seestate
		SfxId::None,                   // seesound
		8,                          // reactiontime
		SfxId::None,                   // attacksound
		StateId::HereticNull,             // painstate
		0,                          // painchance
		SfxId::None,                   // painsound
		StateId::HereticNull,             // meleestate
		StateId::HereticNull,             // missilestate
		StateId::HereticNull,             // crashstate
		StateId::HereticNull,             // deathstate
		StateId::HereticNull,             // xdeathstate
		SfxId::None,                   // deathsound
		0,                          // speed
		4 * FRACUNIT,               // radius
		4 * FRACUNIT,               // height
		100,                        // mass
		0,                          // damage
		SfxId::None,                   // activesound
		MF_NOBLOCKMAP | MF_DROPOFF, // flags
		MobjFlag2::LoGrav | MobjFlag2::CannotPush // flags2
	},

	{
		// MT_CHICPLAYER
		-1,                                                                                   // doomednum
		StateId::HereticChicplay,                                                                   // spawnstate
		100,                                                                                  // spawnhealth
		StateId::HereticChicplayRun1,                                                              // seestate
		SfxId::None,                                                                             // seesound
		0,                                                                                    // reactiontime
		SfxId::None,                                                                             // attacksound
		StateId::HereticChicplayPain,                                                              // painstate
		255,                                                                                  // painchance
		SfxId::HereticChicpai,                                                                  // painsound
		StateId::HereticNull,                                                                       // meleestate
		StateId::HereticChicplayAtk1,                                                              // missilestate
		StateId::HereticNull,                                                                       // crashstate
		StateId::HereticChickenDie1,                                                               // deathstate
		StateId::HereticNull,                                                                       // xdeathstate
		SfxId::HereticChicdth,                                                                  // deathsound
		0,                                                                                    // speed
		16 * FRACUNIT,                                                                        // radius
		24 * FRACUNIT,                                                                        // height
		100,                                                                                  // mass
		0,                                                                                    // damage
		SfxId::None,                                                                             // activesound
		MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_NOTDMATCH,                                  // flags
		MobjFlag2::WindThrust | MobjFlag2::Slide | MobjFlag2::PassMobj | MobjFlag2::FootClip | MobjFlag2::LoGrav | MobjFlag2::TeleStomp // flags2
	},

	{
		// MT_CHICKEN
		-1,                                                  // doomednum
		StateId::HereticChickenLook1,                             // spawnstate
		10,                                                  // spawnhealth
		StateId::HereticChickenWalk1,                             // seestate
		SfxId::HereticChicpai,                                 // seesound
		8,                                                   // reactiontime
		SfxId::HereticChicatk,                                 // attacksound
		StateId::HereticChickenPain1,                             // painstate
		200,                                                 // painchance
		SfxId::HereticChicpai,                                 // painsound
		StateId::HereticChickenAtk1,                              // meleestate
		StateId::Null,                                                   // missilestate
		StateId::HereticNull,                                      // crashstate
		StateId::HereticChickenDie1,                              // deathstate
		StateId::HereticNull,                                      // xdeathstate
		SfxId::HereticChicdth,                                 // deathsound
		4,                                                   // speed
		9 * FRACUNIT,                                        // radius
		22 * FRACUNIT,                                       // height
		40,                                                  // mass
		0,                                                   // damage
		SfxId::HereticChicact,                                 // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF, // flags
		MobjFlag2::WindThrust | MobjFlag2::FootClip | MobjFlag2::PassMobj         // flags2
	},

	{
		// MT_FEATHER
		-1,                                                           // doomednum
		StateId::HereticFeather1,                                           // spawnstate
		1000,                                                         // spawnhealth
		StateId::HereticNull,                                               // seestate
		SfxId::None,                                                     // seesound
		8,                                                            // reactiontime
		SfxId::None,                                                     // attacksound
		StateId::HereticNull,                                               // painstate
		0,                                                            // painchance
		SfxId::None,                                                     // painsound
		StateId::HereticNull,                                               // meleestate
		StateId::HereticNull,                                               // missilestate
		StateId::HereticNull,                                               // crashstate
		StateId::HereticFeatherx,                                           // deathstate
		StateId::HereticNull,                                               // xdeathstate
		SfxId::None,                                                     // deathsound
		0,                                                            // speed
		2 * FRACUNIT,                                                 // radius
		4 * FRACUNIT,                                                 // height
		100,                                                          // mass
		0,                                                            // damage
		SfxId::None,                                                     // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,                      // flags
		MobjFlag2::NoTeleport | MobjFlag2::LoGrav | MobjFlag2::CannotPush | MobjFlag2::WindThrust // flags2
	},

	{
		// MT_MUMMY
		68,                                     // doomednum
		StateId::HereticMummyLook1,                  // spawnstate
		80,                                     // spawnhealth
		StateId::HereticMummyWalk1,                  // seestate
		SfxId::HereticMumsit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticMumat1,                     // attacksound
		StateId::HereticMummyPain1,                  // painstate
		128,                                    // painchance
		SfxId::HereticMumpai,                     // painsound
		StateId::HereticMummyAtk1,                   // meleestate
		StateId::Null,                                      // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticMummyDie1,                   // deathstate
		StateId::HereticNull,                         // xdeathstate
		SfxId::HereticMumdth,                     // deathsound
		12,                                     // speed
		22 * FRACUNIT,                          // radius
		62 * FRACUNIT,                          // height
		75,                                     // mass
		0,                                      // damage
		SfxId::HereticMumact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj             // flags2
	},

	{
		// MT_MUMMYLEADER
		45,                                     // doomednum
		StateId::HereticMummyLook1,                  // spawnstate
		100,                                    // spawnhealth
		StateId::HereticMummyWalk1,                  // seestate
		SfxId::HereticMumsit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticMumat1,                     // attacksound
		StateId::HereticMummyPain1,                  // painstate
		64,                                     // painchance
		SfxId::HereticMumpai,                     // painsound
		StateId::HereticMummyAtk1,                   // meleestate
		StateId::HereticMummylAtk1,                  // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticMummyDie1,                   // deathstate
		StateId::HereticNull,                         // xdeathstate
		SfxId::HereticMumdth,                     // deathsound
		12,                                     // speed
		22 * FRACUNIT,                          // radius
		62 * FRACUNIT,                          // height
		75,                                     // mass
		0,                                      // damage
		SfxId::HereticMumact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj             // flags2
	},

	{
		// MT_MUMMYGHOST
		69,                                                 // doomednum
		StateId::HereticMummyLook1,                              // spawnstate
		80,                                                 // spawnhealth
		StateId::HereticMummyWalk1,                              // seestate
		SfxId::HereticMumsit,                                 // seesound
		8,                                                  // reactiontime
		SfxId::HereticMumat1,                                 // attacksound
		StateId::HereticMummyPain1,                              // painstate
		128,                                                // painchance
		SfxId::HereticMumpai,                                 // painsound
		StateId::HereticMummyAtk1,                               // meleestate
		StateId::Null,                                                  // missilestate
		StateId::HereticNull,                                     // crashstate
		StateId::HereticMummyDie1,                               // deathstate
		StateId::HereticNull,                                     // xdeathstate
		SfxId::HereticMumdth,                                 // deathsound
		12,                                                 // speed
		22 * FRACUNIT,                                      // radius
		62 * FRACUNIT,                                      // height
		75,                                                 // mass
		0,                                                  // damage
		SfxId::HereticMumact,                                 // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj                         // flags2
	},

	{
		// MT_MUMMYLEADERGHOST
		46,                                                 // doomednum
		StateId::HereticMummyLook1,                              // spawnstate
		100,                                                // spawnhealth
		StateId::HereticMummyWalk1,                              // seestate
		SfxId::HereticMumsit,                                 // seesound
		8,                                                  // reactiontime
		SfxId::HereticMumat1,                                 // attacksound
		StateId::HereticMummyPain1,                              // painstate
		64,                                                 // painchance
		SfxId::HereticMumpai,                                 // painsound
		StateId::HereticMummyAtk1,                               // meleestate
		StateId::HereticMummylAtk1,                              // missilestate
		StateId::HereticNull,                                     // crashstate
		StateId::HereticMummyDie1,                               // deathstate
		StateId::HereticNull,                                     // xdeathstate
		SfxId::HereticMumdth,                                 // deathsound
		12,                                                 // speed
		22 * FRACUNIT,                                      // radius
		62 * FRACUNIT,                                      // height
		75,                                                 // mass
		0,                                                  // damage
		SfxId::HereticMumact,                                 // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj                         // flags2
	},

	{
		// MT_MUMMYSOUL
		-1,                           // doomednum
		StateId::HereticMummySoul1,        // spawnstate
		1000,                         // spawnhealth
		StateId::HereticNull,               // seestate
		SfxId::None,                     // seesound
		8,                            // reactiontime
		SfxId::None,                     // attacksound
		StateId::HereticNull,               // painstate
		0,                            // painchance
		SfxId::None,                     // painsound
		StateId::HereticNull,               // meleestate
		StateId::HereticNull,               // missilestate
		StateId::HereticNull,               // crashstate
		StateId::HereticNull,               // deathstate
		StateId::HereticNull,               // xdeathstate
		SfxId::None,                     // deathsound
		0,                            // speed
		20 * FRACUNIT,                // radius
		16 * FRACUNIT,                // height
		100,                          // mass
		0,                            // damage
		SfxId::None,                     // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY, // flags
		MobjFlag2{}                             // flags2
	},

	{
		// MT_MUMMYFX1
		-1,                                                     // doomednum
		StateId::HereticMummyfx11,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticMummyfxi11,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		9 * FRACUNIT,                                           // speed
		8 * FRACUNIT,                                           // radius
		14 * FRACUNIT,                                          // height
		100,                                                    // mass
		4,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_BEAST
		70,                                     // doomednum
		StateId::HereticBeastLook1,                  // spawnstate
		220,                                    // spawnhealth
		StateId::HereticBeastWalk1,                  // seestate
		SfxId::HereticBstsit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticBstatk,                     // attacksound
		StateId::HereticBeastPain1,                  // painstate
		100,                                    // painchance
		SfxId::HereticBstpai,                     // painsound
		StateId::Null,                                      // meleestate
		StateId::HereticBeastAtk1,                   // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticBeastDie1,                   // deathstate
		StateId::HereticBeastXdie1,                  // xdeathstate
		SfxId::HereticBstdth,                     // deathsound
		14,                                     // speed
		32 * FRACUNIT,                          // radius
		74 * FRACUNIT,                          // height
		200,                                    // mass
		0,                                      // damage
		SfxId::HereticBstact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj             // flags2
	},

	{
		// MT_BEASTBALL
		-1,                                                     // doomednum
		StateId::HereticBeastball1,                                   // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticBeastballx1,                                  // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		12 * FRACUNIT,                                          // speed
		9 * FRACUNIT,                                           // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		4,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport                         // flags2
	},

	{
		// MT_BURNBALL
		-1,                                        // doomednum
		StateId::HereticBurnball1,                       // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                         // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticBeastballx1,                     // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                         // deathsound
		10 * FRACUNIT,                             // speed
		6 * FRACUNIT,                              // radius
		8 * FRACUNIT,                              // height
		100,                                       // mass
		2,                                         // damage
		SfxId::None,                                  // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
		MobjFlag2::NoTeleport                             // flags2
	},

	{
		// MT_BURNBALLFB
		-1,                                        // doomednum
		StateId::HereticBurnballfb1,                     // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                         // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticBeastballx1,                     // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                         // deathsound
		10 * FRACUNIT,                             // speed
		6 * FRACUNIT,                              // radius
		8 * FRACUNIT,                              // height
		100,                                       // mass
		2,                                         // damage
		SfxId::None,                                  // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
		MobjFlag2::NoTeleport                             // flags2
	},

	{
		// MT_PUFFY
		-1,                                        // doomednum
		StateId::HereticPuffy1,                          // spawnstate
		1000,                                      // spawnhealth
		StateId::HereticNull,                            // seestate
		SfxId::None,                                         // seesound
		8,                                         // reactiontime
		SfxId::None,                                  // attacksound
		StateId::HereticNull,                            // painstate
		0,                                         // painchance
		SfxId::None,                                  // painsound
		StateId::HereticNull,                            // meleestate
		StateId::HereticNull,                            // missilestate
		StateId::HereticNull,                            // crashstate
		StateId::HereticPuffy1,                          // deathstate
		StateId::HereticNull,                            // xdeathstate
		SfxId::None,                                         // deathsound
		10 * FRACUNIT,                             // speed
		6 * FRACUNIT,                              // radius
		8 * FRACUNIT,                              // height
		100,                                       // mass
		2,                                         // damage
		SfxId::None,                                  // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
		MobjFlag2::NoTeleport                             // flags2
	},

	{
		// MT_SNAKE
		92,                                     // doomednum
		StateId::HereticSnakeLook1,                  // spawnstate
		280,                                    // spawnhealth
		StateId::HereticSnakeWalk1,                  // seestate
		SfxId::HereticSnksit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticSnkatk,                     // attacksound
		StateId::HereticSnakePain1,                  // painstate
		48,                                     // painchance
		SfxId::HereticSnkpai,                     // painsound
		StateId::Null,                                      // meleestate
		StateId::HereticSnakeAtk1,                   // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticSnakeDie1,                   // deathstate
		StateId::HereticNull,                         // xdeathstate
		SfxId::HereticSnkdth,                     // deathsound
		10,                                     // speed
		22 * FRACUNIT,                          // radius
		70 * FRACUNIT,                          // height
		100,                                    // mass
		0,                                      // damage
		SfxId::HereticSnkact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj             // flags2
	},

	{
		// MT_SNAKEPRO_A
		-1,                                                     // doomednum
		StateId::HereticSnakeproA1,                                  // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSnakeproAx1,                                 // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		14 * FRACUNIT,                                          // speed
		12 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport                         // flags2
	},

	{
		// MT_SNAKEPRO_B
		-1,                                                     // doomednum
		StateId::HereticSnakeproB1,                                  // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSnakeproBx1,                                 // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		14 * FRACUNIT,                                          // speed
		12 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		3,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_HEAD
		6,                                                   // doomednum
		StateId::HereticHeadLook,                                 // spawnstate
		700,                                                 // spawnhealth
		StateId::HereticHeadFloat,                                // seestate
		SfxId::HereticHedsit,                                  // seesound
		8,                                                   // reactiontime
		SfxId::HereticHedat1,                                  // attacksound
		StateId::HereticHeadPain1,                                // painstate
		32,                                                  // painchance
		SfxId::HereticHedpai,                                  // painsound
		StateId::Null,                                                   // meleestate
		StateId::HereticHeadAtk1,                                 // missilestate
		StateId::HereticNull,                                      // crashstate
		StateId::HereticHeadDie1,                                 // deathstate
		StateId::HereticNull,                                      // xdeathstate
		SfxId::HereticHeddth,                                  // deathsound
		6,                                                   // speed
		40 * FRACUNIT,                                       // radius
		72 * FRACUNIT,                                       // height
		325,                                                 // mass
		0,                                                   // damage
		SfxId::HereticHedact,                                  // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD, // flags
		MobjFlag2::PassMobj                                         // flags2
	},

	{
		// MT_HEADFX1
		-1,                                                     // doomednum
		StateId::HereticHeadfx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticHeadfxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		13 * FRACUNIT,                                          // speed
		12 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::ThruGhost                          // flags2
	},

	{
		// MT_HEADFX2
		-1,                                                     // doomednum
		StateId::HereticHeadfx21,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticHeadfxi21,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		8 * FRACUNIT,                                           // speed
		12 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		3,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_HEADFX3
		-1,                                                     // doomednum
		StateId::HereticHeadfx31,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticHeadfxi31,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		10 * FRACUNIT,                                          // speed
		14 * FRACUNIT,                                          // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		5,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport                         // flags2
	},

	{
		// MT_WHIRLWIND
		-1,                                                                 // doomednum
		StateId::HereticHeadfx41,                                                // spawnstate
		1000,                                                               // spawnhealth
		StateId::HereticNull,                                                     // seestate
		SfxId::None,                                                                  // seesound
		8,                                                                  // reactiontime
		SfxId::None,                                                           // attacksound
		StateId::HereticNull,                                                     // painstate
		0,                                                                  // painchance
		SfxId::None,                                                           // painsound
		StateId::HereticNull,                                                     // meleestate
		StateId::HereticNull,                                                     // missilestate
		StateId::HereticNull,                                                     // crashstate
		StateId::HereticHeadfxi41,                                               // deathstate
		StateId::HereticNull,                                                     // xdeathstate
		SfxId::None,                                                                  // deathsound
		10 * FRACUNIT,                                                      // speed
		16 * FRACUNIT,                                                      // radius
		74 * FRACUNIT,                                                      // height
		100,                                                                // mass
		1,                                                                  // damage
		SfxId::None,                                                           // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY | MF_SHADOW, // flags
		MobjFlag2::NoTeleport                                                      // flags2
	},

	{
		// MT_CLINK
		90,                                                  // doomednum
		StateId::HereticClinkLook1,                               // spawnstate
		150,                                                 // spawnhealth
		StateId::HereticClinkWalk1,                               // seestate
		SfxId::HereticClksit,                                  // seesound
		8,                                                   // reactiontime
		SfxId::HereticClkatk,                                  // attacksound
		StateId::HereticClinkPain1,                               // painstate
		32,                                                  // painchance
		SfxId::HereticClkpai,                                  // painsound
		StateId::HereticClinkAtk1,                                // meleestate
		StateId::Null,                                                   // missilestate
		StateId::HereticNull,                                      // crashstate
		StateId::HereticClinkDie1,                                // deathstate
		StateId::HereticNull,                                      // xdeathstate
		SfxId::HereticClkdth,                                  // deathsound
		14,                                                  // speed
		20 * FRACUNIT,                                       // radius
		64 * FRACUNIT,                                       // height
		75,                                                  // mass
		0,                                                   // damage
		SfxId::HereticClkact,                                  // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj                          // flags2
	},

	{
		// MT_WIZARD
		15,                                                               // doomednum
		StateId::HereticWizardLook1,                                           // spawnstate
		180,                                                              // spawnhealth
		StateId::HereticWizardWalk1,                                           // seestate
		SfxId::HereticWizsit,                                               // seesound
		8,                                                                // reactiontime
		SfxId::HereticWizatk,                                               // attacksound
		StateId::HereticWizardPain1,                                           // painstate
		64,                                                               // painchance
		SfxId::HereticWizpai,                                               // painsound
		StateId::Null,                                                                // meleestate
		StateId::HereticWizardAtk1,                                            // missilestate
		StateId::HereticNull,                                                   // crashstate
		StateId::HereticWizardDie1,                                            // deathstate
		StateId::HereticNull,                                                   // xdeathstate
		SfxId::HereticWizdth,                                               // deathsound
		12,                                                               // speed
		16 * FRACUNIT,                                                    // radius
		68 * FRACUNIT,                                                    // height
		100,                                                              // mass
		0,                                                                // damage
		SfxId::HereticWizact,                                               // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_FLOAT | MF_NOGRAVITY, // flags
		MobjFlag2::PassMobj                                                      // flags2
	},

	{
		// MT_WIZFX1
		-1,                                                     // doomednum
		StateId::HereticWizfx11,                                     // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticWizfxi11,                                    // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		18 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		3,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_IMP
		66,                                                               // doomednum
		StateId::HereticImpLook1,                                              // spawnstate
		40,                                                               // spawnhealth
		StateId::HereticImpFly1,                                               // seestate
		SfxId::HereticImpsit,                                               // seesound
		8,                                                                // reactiontime
		SfxId::HereticImpat1,                                               // attacksound
		StateId::HereticImpPain1,                                              // painstate
		200,                                                              // painchance
		SfxId::HereticImppai,                                               // painsound
		StateId::HereticImpMeatk1,                                             // meleestate
		StateId::HereticImpMsatk11,                                           // missilestate
		StateId::HereticImpCrash1,                                             // crashstate
		StateId::HereticImpDie1,                                               // deathstate
		StateId::HereticImpXdie1,                                              // xdeathstate
		SfxId::HereticImpdth,                                               // deathsound
		10,                                                               // speed
		16 * FRACUNIT,                                                    // radius
		36 * FRACUNIT,                                                    // height
		50,                                                               // mass
		0,                                                                // damage
		SfxId::HereticImpact,                                               // activesound
		MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL, // flags
		MobjFlag2::SpawnFloat | MobjFlag2::PassMobj | MobjFlag2::RangeHalf                     // flags2
	},

	{
		// MT_IMPLEADER
		5,                                                                // doomednum
		StateId::HereticImpLook1,                                              // spawnstate
		80,                                                               // spawnhealth
		StateId::HereticImpFly1,                                               // seestate
		SfxId::HereticImpsit,                                               // seesound
		8,                                                                // reactiontime
		SfxId::HereticImpat2,                                               // attacksound
		StateId::HereticImpPain1,                                              // painstate
		200,                                                              // painchance
		SfxId::HereticImppai,                                               // painsound
		StateId::Null,                                                                // meleestate
		StateId::HereticImpMsatk21,                                           // missilestate
		StateId::HereticImpCrash1,                                             // crashstate
		StateId::HereticImpDie1,                                               // deathstate
		StateId::HereticImpXdie1,                                              // xdeathstate
		SfxId::HereticImpdth,                                               // deathsound
		10,                                                               // speed
		16 * FRACUNIT,                                                    // radius
		36 * FRACUNIT,                                                    // height
		50,                                                               // mass
		0,                                                                // damage
		SfxId::HereticImpact,                                               // activesound
		MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL, // flags
		MobjFlag2::SpawnFloat | MobjFlag2::PassMobj                                     // flags2
	},

	{
		// MT_IMPCHUNK1
		-1,                    // doomednum
		StateId::HereticImpChunka1, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_NOBLOCKMAP,         // flags
		MobjFlag2{}                      // flags2
	},

	{
		// MT_IMPCHUNK2
		-1,                    // doomednum
		StateId::HereticImpChunkb1, // spawnstate
		1000,                  // spawnhealth
		StateId::HereticNull,        // seestate
		SfxId::None,              // seesound
		8,                     // reactiontime
		SfxId::None,              // attacksound
		StateId::HereticNull,        // painstate
		0,                     // painchance
		SfxId::None,              // painsound
		StateId::HereticNull,        // meleestate
		StateId::HereticNull,        // missilestate
		StateId::HereticNull,        // crashstate
		StateId::HereticNull,        // deathstate
		StateId::HereticNull,        // xdeathstate
		SfxId::None,              // deathsound
		0,                     // speed
		20 * FRACUNIT,         // radius
		16 * FRACUNIT,         // height
		100,                   // mass
		0,                     // damage
		SfxId::None,              // activesound
		MF_NOBLOCKMAP,         // flags
		MobjFlag2{}                      // flags2
	},

	{
		// MT_IMPBALL
		-1,                                                     // doomednum
		StateId::HereticImpfx1,                                       // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticImpfxi1,                                      // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		10 * FRACUNIT,                                          // speed
		8 * FRACUNIT,                                           // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport                         // flags2
	},

	{
		// MT_KNIGHT
		64,                                     // doomednum
		StateId::HereticKnightStnd1,                 // spawnstate
		200,                                    // spawnhealth
		StateId::HereticKnightWalk1,                 // seestate
		SfxId::HereticKgtsit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticKgtatk,                     // attacksound
		StateId::HereticKnightPain1,                 // painstate
		100,                                    // painchance
		SfxId::HereticKgtpai,                     // painsound
		StateId::HereticKnightAtk1,                  // meleestate
		StateId::HereticKnightAtk1,                  // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticKnightDie1,                  // deathstate
		StateId::HereticNull,                         // xdeathstate
		SfxId::HereticKgtdth,                     // deathsound
		12,                                     // speed
		24 * FRACUNIT,                          // radius
		78 * FRACUNIT,                          // height
		150,                                    // mass
		0,                                      // damage
		SfxId::HereticKgtact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj             // flags2
	},

	{
		// MT_KNIGHTGHOST
		65,                                                 // doomednum
		StateId::HereticKnightStnd1,                             // spawnstate
		200,                                                // spawnhealth
		StateId::HereticKnightWalk1,                             // seestate
		SfxId::HereticKgtsit,                                 // seesound
		8,                                                  // reactiontime
		SfxId::HereticKgtatk,                                 // attacksound
		StateId::HereticKnightPain1,                             // painstate
		100,                                                // painchance
		SfxId::HereticKgtpai,                                 // painsound
		StateId::HereticKnightAtk1,                              // meleestate
		StateId::HereticKnightAtk1,                              // missilestate
		StateId::HereticNull,                                     // crashstate
		StateId::HereticKnightDie1,                              // deathstate
		StateId::HereticNull,                                     // xdeathstate
		SfxId::HereticKgtdth,                                 // deathsound
		12,                                                 // speed
		24 * FRACUNIT,                                      // radius
		78 * FRACUNIT,                                      // height
		150,                                                // mass
		0,                                                  // damage
		SfxId::HereticKgtact,                                 // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_SHADOW, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj                         // flags2
	},

	{
		// MT_KNIGHTAXE
		-1,                                                     // doomednum
		StateId::HereticSpinaxe1,                                     // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSpinaxex1,                                    // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		9 * FRACUNIT,                                           // speed
		10 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		2,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::WindThrust | MobjFlag2::NoTeleport | MobjFlag2::ThruGhost         // flags2
	},

	{
		// MT_REDAXE
		-1,                                                     // doomednum
		StateId::HereticRedaxe1,                                      // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticRedaxex1,                                     // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticHrnhit,                                     // deathsound
		9 * FRACUNIT,                                           // speed
		10 * FRACUNIT,                                          // radius
		8 * FRACUNIT,                                           // height
		100,                                                    // mass
		7,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::ThruGhost                          // flags2
	},

	{
		// MT_SORCERER1
		7,                                      // doomednum
		StateId::HereticSrcr1Look1,                  // spawnstate
		2000,                                   // spawnhealth
		StateId::HereticSrcr1Walk1,                  // seestate
		SfxId::HereticSbtsit,                     // seesound
		8,                                      // reactiontime
		SfxId::HereticSbtatk,                     // attacksound
		StateId::HereticSrcr1Pain1,                  // painstate
		56,                                     // painchance
		SfxId::HereticSbtpai,                     // painsound
		StateId::Null,                                      // meleestate
		StateId::HereticSrcr1Atk1,                   // missilestate
		StateId::HereticNull,                         // crashstate
		StateId::HereticSrcr1Die1,                   // deathstate
		StateId::HereticNull,                         // xdeathstate
		SfxId::HereticSbtdth,                     // deathsound
		16,                                     // speed
		28 * FRACUNIT,                          // radius
		100 * FRACUNIT,                         // height
		800,                                    // mass
		0,                                      // damage
		SfxId::HereticSbtact,                     // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj | MobjFlag2::Boss  // flags2
	},

	{
		// MT_SRCRFX1
		-1,                                                     // doomednum
		StateId::HereticSrcrfx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSrcrfxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		20 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		10 * FRACUNIT,                                          // height
		100,                                                    // mass
		10,                                                     // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::FireDamage                         // flags2
	},

	{
		// MT_SORCERER2
		-1,                                                  // doomednum
		StateId::HereticSor2Look1,                                // spawnstate
		3500,                                                // spawnhealth
		StateId::HereticSor2Walk1,                                // seestate
		SfxId::HereticSorsit,                                  // seesound
		8,                                                   // reactiontime
		SfxId::HereticSoratk,                                  // attacksound
		StateId::HereticSor2Pain1,                                // painstate
		32,                                                  // painchance
		SfxId::HereticSorpai,                                  // painsound
		StateId::Null,                                                   // meleestate
		StateId::HereticSor2Atk1,                                 // missilestate
		StateId::HereticNull,                                      // crashstate
		StateId::HereticSor2Die1,                                 // deathstate
		StateId::HereticNull,                                      // xdeathstate
		SfxId::None,                                                   // deathsound
		14,                                                  // speed
		16 * FRACUNIT,                                       // radius
		70 * FRACUNIT,                                       // height
		300,                                                 // mass
		0,                                                   // damage
		SfxId::HereticSoract,                                  // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj | MobjFlag2::Boss               // flags2
	},

	{
		// MT_SOR2FX1
		-1,                                                     // doomednum
		StateId::HereticSor2fx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSor2fxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		20 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		1,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_SOR2FXSPARK
		-1,                             // doomednum
		StateId::HereticSor2fxspark1,         // spawnstate
		1000,                           // spawnhealth
		StateId::HereticNull,                 // seestate
		SfxId::None,                       // seesound
		8,                              // reactiontime
		SfxId::None,                       // attacksound
		StateId::HereticNull,                 // painstate
		0,                              // painchance
		SfxId::None,                       // painsound
		StateId::HereticNull,                 // meleestate
		StateId::HereticNull,                 // missilestate
		StateId::HereticNull,                 // crashstate
		StateId::HereticNull,                 // deathstate
		StateId::HereticNull,                 // xdeathstate
		SfxId::None,                       // deathsound
		0,                              // speed
		20 * FRACUNIT,                  // radius
		16 * FRACUNIT,                  // height
		100,                            // mass
		0,                              // damage
		SfxId::None,                       // activesound
		MF_NOBLOCKMAP | MF_NOGRAVITY,   // flags
		MobjFlag2::NoTeleport | MobjFlag2::CannotPush // flags2
	},

	{
		// MT_SOR2FX2
		-1,                                                     // doomednum
		StateId::HereticSor2fx21,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticSor2fxi21,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		6 * FRACUNIT,                                           // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		10,                                                     // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport                                          // flags2
	},

	{
		// MT_SOR2TELEFADE
		-1,                      // doomednum
		StateId::HereticSor2telefade1, // spawnstate
		1000,                    // spawnhealth
		StateId::HereticNull,          // seestate
		SfxId::None,                // seesound
		8,                       // reactiontime
		SfxId::None,                // attacksound
		StateId::HereticNull,          // painstate
		0,                       // painchance
		SfxId::None,                // painsound
		StateId::HereticNull,          // meleestate
		StateId::HereticNull,          // missilestate
		StateId::HereticNull,          // crashstate
		StateId::HereticNull,          // deathstate
		StateId::HereticNull,          // xdeathstate
		SfxId::None,                // deathsound
		0,                       // speed
		20 * FRACUNIT,           // radius
		16 * FRACUNIT,           // height
		100,                     // mass
		0,                       // damage
		SfxId::None,                // activesound
		MF_NOBLOCKMAP,           // flags
		MobjFlag2{}                        // flags2
	},

	{
		// MT_MINOTAUR
		9,                                                   // doomednum
		StateId::HereticMntrLook1,                                // spawnstate
		3000,                                                // spawnhealth
		StateId::HereticMntrWalk1,                                // seestate
		SfxId::HereticMinsit,                                  // seesound
		8,                                                   // reactiontime
		SfxId::HereticMinat1,                                  // attacksound
		StateId::HereticMntrPain1,                                // painstate
		25,                                                  // painchance
		SfxId::HereticMinpai,                                  // painsound
		StateId::HereticMntrAtk11,                               // meleestate
		StateId::HereticMntrAtk21,                               // missilestate
		StateId::HereticNull,                                      // crashstate
		StateId::HereticMntrDie1,                                 // deathstate
		StateId::HereticNull,                                      // xdeathstate
		SfxId::HereticMindth,                                  // deathsound
		16,                                                  // speed
		28 * FRACUNIT,                                       // radius
		100 * FRACUNIT,                                      // height
		800,                                                 // mass
		7,                                                   // damage
		SfxId::HereticMinact,                                  // activesound
		MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF, // flags
		MobjFlag2::FootClip | MobjFlag2::PassMobj | MobjFlag2::Boss               // flags2
	},

	{
		// MT_MNTRFX1
		-1,                                                     // doomednum
		StateId::HereticMntrfx11,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticMntrfxi11,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::None,                                                      // deathsound
		20 * FRACUNIT,                                          // speed
		10 * FRACUNIT,                                          // radius
		6 * FRACUNIT,                                           // height
		100,                                                    // mass
		3,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::FireDamage                         // flags2
	},

	{
		// MT_MNTRFX2
		-1,                                                     // doomednum
		StateId::HereticMntrfx21,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticMntrfxi21,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticPhohit,                                     // deathsound
		14 * FRACUNIT,                                          // speed
		5 * FRACUNIT,                                           // radius
		12 * FRACUNIT,                                          // height
		100,                                                    // mass
		4,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::FireDamage                         // flags2
	},

	{
		// MT_MNTRFX3
		-1,                                                     // doomednum
		StateId::HereticMntrfx31,                                    // spawnstate
		1000,                                                   // spawnhealth
		StateId::HereticNull,                                         // seestate
		SfxId::None,                                                      // seesound
		8,                                                      // reactiontime
		SfxId::None,                                               // attacksound
		StateId::HereticNull,                                         // painstate
		0,                                                      // painchance
		SfxId::None,                                               // painsound
		StateId::HereticNull,                                         // meleestate
		StateId::HereticNull,                                         // missilestate
		StateId::HereticNull,                                         // crashstate
		StateId::HereticMntrfxi21,                                   // deathstate
		StateId::HereticNull,                                         // xdeathstate
		SfxId::HereticPhohit,                                     // deathsound
		0,                                                      // speed
		8 * FRACUNIT,                                           // radius
		16 * FRACUNIT,                                          // height
		100,                                                    // mass
		4,                                                      // damage
		SfxId::None,                                               // activesound
		MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY, // flags
		MobjFlag2::NoTeleport | MobjFlag2::FireDamage                         // flags2
	},

	{
		// MT_AKYY
		73,                        // doomednum
		StateId::HereticAkyy1,           // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_NOTDMATCH, // flags
		MobjFlag2{}                          // flags2
	},

	{
		// MT_BKYY
		79,                        // doomednum
		StateId::HereticBkyy1,           // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_NOTDMATCH, // flags
		MobjFlag2{}                          // flags2
	},

	{
		// MT_CKEY
		80,                        // doomednum
		StateId::HereticCkyy1,           // spawnstate
		1000,                      // spawnhealth
		StateId::HereticNull,            // seestate
		SfxId::None,                  // seesound
		8,                         // reactiontime
		SfxId::None,                  // attacksound
		StateId::HereticNull,            // painstate
		0,                         // painchance
		SfxId::None,                  // painsound
		StateId::HereticNull,            // meleestate
		StateId::HereticNull,            // missilestate
		StateId::HereticNull,            // crashstate
		StateId::HereticNull,            // deathstate
		StateId::HereticNull,            // xdeathstate
		SfxId::None,                  // deathsound
		0,                         // speed
		20 * FRACUNIT,             // radius
		16 * FRACUNIT,             // height
		100,                       // mass
		0,                         // damage
		SfxId::None,                  // activesound
		MF_SPECIAL | MF_NOTDMATCH, // flags
		MobjFlag2{}                          // flags2
	},

	{
		// MT_AMGWNDWIMPY
		10,              // doomednum
		StateId::HereticAmg1,  // spawnstate
		AMMO_GWND_WIMPY, // spawnhealth
		StateId::HereticNull,  // seestate
		SfxId::None,        // seesound
		8,               // reactiontime
		SfxId::None,        // attacksound
		StateId::HereticNull,  // painstate
		0,               // painchance
		SfxId::None,        // painsound
		StateId::HereticNull,  // meleestate
		StateId::HereticNull,  // missilestate
		StateId::HereticNull,  // crashstate
		StateId::HereticNull,  // deathstate
		StateId::HereticNull,  // xdeathstate
		SfxId::None,        // deathsound
		0,               // speed
		20 * FRACUNIT,   // radius
		16 * FRACUNIT,   // height
		100,             // mass
		0,               // damage
		SfxId::None,        // activesound
		MF_SPECIAL,      // flags
		MobjFlag2{}                // flags2
	},

	{
		// MT_AMGWNDHEFTY
		12,               // doomednum
		StateId::HereticAmg21, // spawnstate
		AMMO_GWND_HEFTY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMMACEWIMPY
		13,              // doomednum
		StateId::HereticAmm1,  // spawnstate
		AMMO_MACE_WIMPY, // spawnhealth
		StateId::HereticNull,  // seestate
		SfxId::None,        // seesound
		8,               // reactiontime
		SfxId::None,        // attacksound
		StateId::HereticNull,  // painstate
		0,               // painchance
		SfxId::None,        // painsound
		StateId::HereticNull,  // meleestate
		StateId::HereticNull,  // missilestate
		StateId::HereticNull,  // crashstate
		StateId::HereticNull,  // deathstate
		StateId::HereticNull,  // xdeathstate
		SfxId::None,        // deathsound
		0,               // speed
		20 * FRACUNIT,   // radius
		16 * FRACUNIT,   // height
		100,             // mass
		0,               // damage
		SfxId::None,        // activesound
		MF_SPECIAL,      // flags
		MobjFlag2{}                // flags2
	},

	{
		// MT_AMMACEHEFTY
		16,              // doomednum
		StateId::HereticAmm2,  // spawnstate
		AMMO_MACE_HEFTY, // spawnhealth
		StateId::HereticNull,  // seestate
		SfxId::None,        // seesound
		8,               // reactiontime
		SfxId::None,        // attacksound
		StateId::HereticNull,  // painstate
		0,               // painchance
		SfxId::None,        // painsound
		StateId::HereticNull,  // meleestate
		StateId::HereticNull,  // missilestate
		StateId::HereticNull,  // crashstate
		StateId::HereticNull,  // deathstate
		StateId::HereticNull,  // xdeathstate
		SfxId::None,        // deathsound
		0,               // speed
		20 * FRACUNIT,   // radius
		16 * FRACUNIT,   // height
		100,             // mass
		0,               // damage
		SfxId::None,        // activesound
		MF_SPECIAL,      // flags
		MobjFlag2{}                // flags2
	},

	{
		// MT_AMCBOWWIMPY
		18,              // doomednum
		StateId::HereticAmc1,  // spawnstate
		AMMO_CBOW_WIMPY, // spawnhealth
		StateId::HereticNull,  // seestate
		SfxId::None,        // seesound
		8,               // reactiontime
		SfxId::None,        // attacksound
		StateId::HereticNull,  // painstate
		0,               // painchance
		SfxId::None,        // painsound
		StateId::HereticNull,  // meleestate
		StateId::HereticNull,  // missilestate
		StateId::HereticNull,  // crashstate
		StateId::HereticNull,  // deathstate
		StateId::HereticNull,  // xdeathstate
		SfxId::None,        // deathsound
		0,               // speed
		20 * FRACUNIT,   // radius
		16 * FRACUNIT,   // height
		100,             // mass
		0,               // damage
		SfxId::None,        // activesound
		MF_SPECIAL,      // flags
		MobjFlag2{}                // flags2
	},

	{
		// MT_AMCBOWHEFTY
		19,               // doomednum
		StateId::HereticAmc21, // spawnstate
		AMMO_CBOW_HEFTY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMSKRDWIMPY
		20,               // doomednum
		StateId::HereticAms11, // spawnstate
		AMMO_SKRD_WIMPY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMSKRDHEFTY
		21,               // doomednum
		StateId::HereticAms21, // spawnstate
		AMMO_SKRD_HEFTY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMPHRDWIMPY
		22,               // doomednum
		StateId::HereticAmp11, // spawnstate
		AMMO_PHRD_WIMPY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMPHRDHEFTY
		23,               // doomednum
		StateId::HereticAmp21, // spawnstate
		AMMO_PHRD_HEFTY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMBLSRWIMPY
		54,               // doomednum
		StateId::HereticAmb11, // spawnstate
		AMMO_BLSR_WIMPY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_AMBLSRHEFTY
		55,               // doomednum
		StateId::HereticAmb21, // spawnstate
		AMMO_BLSR_HEFTY,  // spawnhealth
		StateId::HereticNull,   // seestate
		SfxId::None,         // seesound
		8,                // reactiontime
		SfxId::None,         // attacksound
		StateId::HereticNull,   // painstate
		0,                // painchance
		SfxId::None,         // painsound
		StateId::HereticNull,   // meleestate
		StateId::HereticNull,   // missilestate
		StateId::HereticNull,   // crashstate
		StateId::HereticNull,   // deathstate
		StateId::HereticNull,   // xdeathstate
		SfxId::None,         // deathsound
		0,                // speed
		20 * FRACUNIT,    // radius
		16 * FRACUNIT,    // height
		100,              // mass
		0,                // damage
		SfxId::None,         // activesound
		MF_SPECIAL,       // flags
		MobjFlag2{}                 // flags2
	},

	{
		// MT_SOUNDWIND
		42,                          // doomednum
		StateId::HereticSndWind,          // spawnstate
		1000,                        // spawnhealth
		StateId::HereticNull,              // seestate
		SfxId::None,                    // seesound
		8,                           // reactiontime
		SfxId::None,                    // attacksound
		StateId::HereticNull,              // painstate
		0,                           // painchance
		SfxId::None,                    // painsound
		StateId::HereticNull,              // meleestate
		StateId::HereticNull,              // missilestate
		StateId::HereticNull,              // crashstate
		StateId::HereticNull,              // deathstate
		StateId::HereticNull,              // xdeathstate
		SfxId::None,                    // deathsound
		0,                           // speed
		20 * FRACUNIT,               // radius
		16 * FRACUNIT,               // height
		100,                         // mass
		0,                           // damage
		SfxId::None,                    // activesound
		MF_NOBLOCKMAP | MF_NOSECTOR, // flags
		MobjFlag2{}                            // flags2
	},

	{
		// MT_SOUNDWATERFALL
		41,                          // doomednum
		StateId::HereticSndWaterfall,     // spawnstate
		1000,                        // spawnhealth
		StateId::HereticNull,              // seestate
		SfxId::None,                    // seesound
		8,                           // reactiontime
		SfxId::None,                    // attacksound
		StateId::HereticNull,              // painstate
		0,                           // painchance
		SfxId::None,                    // painsound
		StateId::HereticNull,              // meleestate
		StateId::HereticNull,              // missilestate
		StateId::HereticNull,              // crashstate
		StateId::HereticNull,              // deathstate
		StateId::HereticNull,              // xdeathstate
		SfxId::None,                    // deathsound
		0,                           // speed
		20 * FRACUNIT,               // radius
		16 * FRACUNIT,               // height
		100,                         // mass
		0,                           // damage
		SfxId::None,                    // activesound
		MF_NOBLOCKMAP | MF_NOSECTOR, // flags
		MobjFlag2{}                            // flags2
	}
};
