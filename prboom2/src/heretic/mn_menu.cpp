// SPDX-License-Identifier: GPL-2.0-or-later

// MN_menu.c

#include "doomstat.hpp"
#include "w_wad.hpp"
#include "v_video.hpp"
#include "m_menu.hpp"
#include "g_game.hpp"
#include "dsda/settings.hpp"
#include "heretic/dstrings.hpp"
#include "heretic/mn_menu.hpp"

#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-4)
#define SFX_VOL_INDEX 1
#define MUS_VOL_INDEX 3

extern int g_menu_save_page_size;

static int FontABaseLump;
static int FontAYellowBaseLump;
static int FontBBaseLump;
static int SkullBaseLump;
static int MenuTime;

dboolean mn_SuicideConsole;

static int MN_SafeFontALump(int offset)
{
	if(offset > 58)
		return FontABaseLump;

	return FontABaseLump + offset;
}

static void MN_InitFonts()
{
	FontABaseLump = W_GetNumForName("FONTA_S") + 1;
	if(hexen) FontAYellowBaseLump = W_GetNumForName("FONTAY_S") + 1;
	FontBBaseLump = W_GetNumForName("FONTB_S") + 1;
}

extern menu_t MainDef;
extern menu_t EpiDef;
extern menu_t SkillDef;
extern menu_t OptionsDef;
extern menu_t SoundDef;
extern menu_t LoadDef;
extern menu_t SaveDef;
extern menuitem_t SoundMenu[];

/////////////////////////////
//
// Raven Prototypes
//
/////////////////////////////

extern "C" void MN_GameFiles(int choice);
extern "C" void MN_Info(int choice);
extern "C" void MN_Info2(int choice);
extern "C" void MN_Info3(int choice);
extern "C" void MN_Info4(int choice);
extern "C" void MN_FinishInfo(int choice);

extern "C" void MN_DrawInfoAd();
extern "C" void MN_DrawInfoHelp1();
extern "C" void MN_DrawInfoHelp2();
extern "C" void MN_DrawInfoCredits();

extern "C" void MN_DrawAd();
extern "C" void MN_DrawCredits();
extern "C" void MN_DrawHelp1();
extern "C" void MN_DrawHelp2();

static void MN_DrTextAColor(const char* text, int x, int y, int cm);
static void MN_DrTextBColor(const char* text, int x, int y, int cm);

extern "C" void M_ChangeMenu(menu_t* menu, menuactive_t mnact);
extern dboolean inhelpscreens;
extern menu_t ExtHelpDef;
extern "C" void M_NewGame(int choice);
extern "C" void M_Options(int choice);
extern "C" void M_QuitDOOM(int choice);
extern "C" void M_LoadGame(int choice);
extern "C" void M_SaveGame(int choice);


/////////////////////////////
//
// Raven Info Screens
//
/////////////////////////////

enum { infoempty1, info1_end } info_e1;

enum { infoempty2, info2_end } info_e2;

enum { infoempty3, info3_end } info_e3;

enum { infoempty4, info4_end } info_e4;

menuitem_t InfoMenu1[] = {{M_ITEM_ACTION, "", MN_Info2, 0}};
menuitem_t InfoMenu2[] = {{M_ITEM_ACTION, "", MN_Info3, 0}};
menuitem_t InfoMenu3[] = {{M_ITEM_ACTION, "", MN_Info4, 0}};
menuitem_t InfoMenu4[] = {{M_ITEM_ACTION, "", MN_FinishInfo, 0}};

menu_t InfoDef1 =
{
	info1_end,
	&MainDef,
	InfoMenu1,
	MN_DrawInfoAd,
	330, 175,
	0
};

menu_t InfoDef2 =
{
	info2_end,
	&InfoDef1,
	InfoMenu2,
	MN_DrawInfoHelp1,
	330, 175,
	0
};

menu_t InfoDef3 =
{
	info3_end,
	&InfoDef2,
	InfoMenu3,
	MN_DrawInfoHelp2,
	330, 175,
	0
};

menu_t InfoDef4 =
{
	info4_end,
	&InfoDef3,
	InfoMenu4,
	MN_DrawInfoCredits,
	330, 175,
	0
};

void MN_Info(int choice) { M_SetupNextMenu(&InfoDef1); }
void MN_Info2(int choice) { M_SetupNextMenu(&InfoDef2); }
void MN_Info3(int choice) { M_SetupNextMenu(&InfoDef3); }
void MN_Info4(int choice) { M_SetupNextMenu(&InfoDef4); }
void MN_FinishInfo(int choice) { M_SetupNextMenu(&MainDef); }

extern "C" void MN_DrawInfoAd()
{
	inhelpscreens = true;
	MN_DrawAd();
}

extern "C" void MN_DrawInfoHelp1()
{
	inhelpscreens = true;
	MN_DrawHelp1();
}

extern "C" void MN_DrawInfoHelp2()
{
	inhelpscreens = true;
	MN_DrawHelp2();
}

extern "C" void MN_DrawInfoCredits()
{
	inhelpscreens = true;
	MN_DrawCredits();
}

extern "C" void MN_DrawAd()
{
	const char* ravenlump;
	ravenlump = (heretic && (gamemode == shareware)) ? "ORDER" : "CREDIT";
	M_ChangeMenu(nullptr, static_cast<menuactive_t>(mnact_full));
	V_DrawRawScreen(ravenlump);
	return;
}

extern "C" void MN_DrawHelp1()
{
	M_ChangeMenu(nullptr, static_cast<menuactive_t>(mnact_full));
	V_DrawRawScreen("HELP1");
	return;
}

extern "C" void MN_DrawHelp2()
{
	M_ChangeMenu(nullptr, static_cast<menuactive_t>(mnact_full));
	V_DrawRawScreen("HELP2");
	return;
}

extern "C" void MN_DrawCredits()
{
	M_ChangeMenu(nullptr, static_cast<menuactive_t>(mnact_full));
	V_DrawRawScreen("CREDIT");
	return;
}

/////////////////////////////
//
// RavenMainMenu (Heretic and Hexen use this)
//
/////////////////////////////

enum
{
	rnewgame = 0,
	roptions,
	rgamefiles,
	rinfo,
	rquitdoom,
	rmain_end
} rmain_e;

menuitem_t RavenMainMenu[] =
{
	{static_cast<menuitem_type_t>(1), "", M_NewGame, 'n', "NEW GAME"},
	{static_cast<menuitem_type_t>(1), "", M_Options, 'o', "OPTIONS"},
	{static_cast<menuitem_type_t>(1), "", MN_GameFiles, 'g', "GAME FILES"},
	{static_cast<menuitem_type_t>(1), "", MN_Info, 'i', "INFO"},
	{static_cast<menuitem_type_t>(1), "", M_QuitDOOM, 'q', "QUIT GAME"}
};


/////////////////////////////
//
// Raven GameFiles Menu
//
/////////////////////////////

enum
{
	rloadgame,
	rsavegame,
	rsaveload_end
} saveload_e;

menuitem_t SaveLoadMenu[] =
{
	{static_cast<menuitem_type_t>(1), "", M_LoadGame, 'l', "LOAD GAME"},
	{static_cast<menuitem_type_t>(1), "", M_SaveGame, 's', "SAVE GAME"},
};

menu_t SaveLoadDef =
{
	rsaveload_end, // number of menu items
	&MainDef,      // previous menu screen
	SaveLoadMenu,  // table that defines menu items
	nullptr,              // drawing routine
	97, 64,        // initial cursor position
	0              // last menu item the user was on
};

extern "C" void MN_GameFiles(int choice)
{
	M_SetupNextMenu(&SaveLoadDef);
}


/////////////////////////////
//
// Raven MN_Init
//
/////////////////////////////

void MN_Init()
{
	MN_InitFonts();

	if(heretic)
	{
		SkullBaseLump = W_GetNumForName("M_SKL00");
	}
	else
	{
		SkullBaseLump = W_CheckNumForName2("FBULA0", ns_sprites);
	}

	// override doom menu parameters

	MainDef.x = 110;
	MainDef.y = 56;

	OptionsDef.x = 88;
	OptionsDef.y = 16;

	SoundDef.x = OptionsDef.x;
	SoundDef.y = OptionsDef.y;

	LoadDef.x = 70;
	LoadDef.y = 30;
	LoadDef.numitems = g_menu_save_page_size;

	SaveDef.x = 70;
	SaveDef.y = 30;
	SaveDef.numitems = g_menu_save_page_size;

	if(heretic)
	{
		EpiDef.x = 80;
		EpiDef.y = 50;

		SkillDef.x = 38;
		SkillDef.y = 30;

		if(gamemode == retail)
		{
			EpiDef.y -= ITEM_HEIGHT;
		}
	}
	else
	{
		EpiDef.x = 66;
		EpiDef.y = 66;

		SkillDef.x = 120;
		SkillDef.y = 44;
	}

	SoundMenu[0].alttext = "SFX VOLUME";
	SoundMenu[2].alttext = "MUSIC VOLUME";

	// Use exclusive Raven MainMenu.
	MainDef.menuitems = RavenMainMenu;
	MainDef.numitems = rmain_end;
	SaveDef.prevMenu = &SaveLoadDef;
	LoadDef.prevMenu = &SaveLoadDef;

	// remove "ORDER" screen if not shareware
	if(gamemode != shareware)
	{
		InfoDef1.routine = MN_DrawInfoHelp1;
		InfoMenu1[0].routine = MN_Info3;
		InfoDef2.prevMenu = &MainDef;
	}
}

void MN_UpdateClass(int choice)
{
	PlayerClass[consoleplayer] = static_cast<pclass_t>(choice + 1);

	switch(PlayerClass[consoleplayer])
	{
		case PCLASS_FIGHTER:
			SkillDef.x = 120;
			SkillDef.menuitems[0].alttext = hexen_skill_fighter[0];
			SkillDef.menuitems[1].alttext = hexen_skill_fighter[1];
			SkillDef.menuitems[2].alttext = hexen_skill_fighter[2];
			SkillDef.menuitems[3].alttext = hexen_skill_fighter[3];
			SkillDef.menuitems[4].alttext = hexen_skill_fighter[4];
			break;
		case PCLASS_CLERIC:
			SkillDef.x = 116;
			SkillDef.menuitems[0].alttext = hexen_skill_cleric[0];
			SkillDef.menuitems[1].alttext = hexen_skill_cleric[1];
			SkillDef.menuitems[2].alttext = hexen_skill_cleric[2];
			SkillDef.menuitems[3].alttext = hexen_skill_cleric[3];
			SkillDef.menuitems[4].alttext = hexen_skill_cleric[4];
			break;
		case PCLASS_MAGE:
			SkillDef.x = 112;
			SkillDef.menuitems[0].alttext = hexen_skill_mage[0];
			SkillDef.menuitems[1].alttext = hexen_skill_mage[1];
			SkillDef.menuitems[2].alttext = hexen_skill_mage[2];
			SkillDef.menuitems[3].alttext = hexen_skill_mage[3];
			SkillDef.menuitems[4].alttext = hexen_skill_mage[4];
			break;
		default:
			break;
	}
}

void MN_Ticker()
{
	if(!menuactive)
	{
		return;
	}
	MenuTime++;
}

extern menu_t* currentMenu;
extern short itemOn;

void MN_DrawMessage(const char* messageString)
{
	char* msg;
	char* p;
	int y;

	msg = Z_Strdup(messageString);
	p = msg;
	y = 100 - MN_TextAHeight(msg) / 2;

	while(*p)
	{
		char* text;
		char c;

		text = p;
		while((c = *p) && *p != '\n')
			p++;
		*p = 0;

		MN_DrTextA(text, 160 - MN_TextAWidth(text) / 2, y);
		y += R_NumPatchHeight(FontABaseLump);

		if((*p = c))
			p++;
	}
	Z_Free(msg);
}

void MN_Drawer()
{
	int i;
	int x;
	int y;
	int max;
	const char* selName;

	x = currentMenu->x;
	y = currentMenu->y;
	max = currentMenu->numitems;

	for(i = 0; i < max; i++)
	{
		const char* text = currentMenu->menuitems[i].alttext;
		int custom_skill_text = text && (currentMenu->menuitems[i].flags == MENUF_OPTLUMP);
		int color = M_HighlightColor(M_MouseHovered(i), CR_DEFAULT);

		if(custom_skill_text)
		{
			// use small font for custom skill
			y += 6; // add some padding (looks bad otherwise)
			MN_DrTextAColor(text, x, y, color);
		}
		else if(text)
			MN_DrTextBColor(text, x, y, color);
		y += ITEM_HEIGHT;
	}

	// Don't draw selector on INFO screens for Heretic / Hexen
	if(currentMenu == &InfoDef1 || currentMenu == &InfoDef2 ||
		currentMenu == &InfoDef3 || currentMenu == &InfoDef4 ||
		currentMenu == &ExtHelpDef)
		return;

	if(max)
	{
		y = currentMenu->y + (itemOn * ITEM_HEIGHT) + SELECTOR_YOFFSET;
		selName = (MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
		V_DrawNamePatch(x + SELECTOR_XOFFSET, y, 0, selName, CR_DEFAULT, VPT_STRETCH);
	}
	// MenuItem_t *item;
	// const char *message;
	// const char *selName;
	//
	// if (MenuActive == false)
	// {
	//   if (askforquit)
	//   {
	//     message = QuitEndMsg[typeofask - 1];
	//
	//     MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2, 80);
	//     if (typeofask == 3)
	//     {
	//       MN_DrTextA(SlotText[quicksave - 1], 160 -
	//                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
	//       MN_DrTextA("?", 160 +
	//                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
	//     }
	//     if (typeofask == 4)
	//     {
	//       MN_DrTextA(SlotText[quickload - 1], 160 -
	//                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
	//       MN_DrTextA("?", 160 +
	//                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
	//     }
	//   }
	//   return;
	// }
	// else
	// {
	//   if (InfoType)
	//   {
	//     MN_DrawInfo();
	//     return;
	//   }
	//   if (screenblocks < 10)
	//   {
	//     BorderNeedRefresh = true;
	//   }
	//   if (CurrentMenu->drawFunc != nullptr)
	//   {
	//     CurrentMenu->drawFunc();
	//   }
	//   x = CurrentMenu->x;
	//   y = CurrentMenu->y;
	//   item = CurrentMenu->items;
	//   for (i = 0; i < CurrentMenu->itemCount; i++)
	//   {
	//     if (item->type != ITT_EMPTY && item->text)
	//     {
	//       MN_DrTextB(item->text, x, y);
	//     }
	//
	//     y += ITEM_HEIGHT;
	//     item++;
	//   }
	//
	//   y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
	//   selName = (MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
	//   V_DrawPatch(x + SELECTOR_XOFFSET, y,
	//               W_LumpByName(selName));
	// }
}

static void Hexen_MN_DrawMainMenu();

void MN_DrawMainMenu()
{
	int frame;

	if(hexen) return Hexen_MN_DrawMainMenu();

	frame = (MenuTime / 3) % 18;
	V_DrawNamePatch(88, 0, 0, "M_HTIC", CR_DEFAULT, VPT_STRETCH);
	V_DrawNumPatch(40, 10, 0, SkullBaseLump + (17 - frame), CR_DEFAULT, VPT_STRETCH);
	V_DrawNumPatch(232, 10, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}

static void Hexen_MN_DrawMainMenu()
{
	int frame;

	frame = (MenuTime / 5) % 7;
	V_DrawNamePatch(88, 0, 0, "M_HTIC", CR_DEFAULT, VPT_STRETCH);
	V_DrawNumPatch(37, 80, 0, SkullBaseLump + (frame + 2) % 7, CR_DEFAULT, VPT_STRETCH);
	V_DrawNumPatch(278, 80, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}

// Class menu is in the episode slot for hexen
void MN_DrawEpisode()
{
	pclass_t class_;
	static const char* boxLumpName[3] = {
		"m_fbox",
		"m_cbox",
		"m_mbox"
	};
	static const char* walkLumpName[3] = {
		"m_fwalk1",
		"m_cwalk1",
		"m_mwalk1"
	};

	if(heretic) return;

	MN_DrTextB("CHOOSE CLASS:", 34, 24);
	class_ = (pclass_t)itemOn;
	V_DrawNamePatch(174, 8, 0, boxLumpName[class_], CR_DEFAULT, VPT_STRETCH);
	V_DrawNumPatch(174 + 24, 8 + 12, 0, W_GetNumForName(walkLumpName[class_]) + ((MenuTime >> 3) & 3),
		CR_DEFAULT, VPT_STRETCH);
}

void MN_DrawSkillMenu()
{
	if(heretic) return;

	MN_DrTextB("CHOOSE SKILL LEVEL:", 74, 16);
}

void MN_DrawOptions()
{
}

void MN_DrawSound()
{
	char num[4];

	M_DrawThermoBig(SoundDef.x - 8, SoundDef.y + ITEM_HEIGHT * SFX_VOL_INDEX, 16, 16, snd_SfxVolume, SFX_VOL_INDEX - 1);
	snprintf(num, sizeof(num), "%3d", snd_SfxVolume);
	MN_DrTextA(num, SoundDef.x + 130, SoundDef.y + ITEM_HEIGHT * SFX_VOL_INDEX + 3);

	M_DrawThermoBig(SoundDef.x - 8, SoundDef.y + ITEM_HEIGHT * MUS_VOL_INDEX, 16, 16, snd_MusicVolume, MUS_VOL_INDEX - 1);
	snprintf(num, sizeof(num), "%3d", snd_MusicVolume);
	MN_DrTextA(num, SoundDef.x + 130, SoundDef.y + ITEM_HEIGHT * MUS_VOL_INDEX + 3);
}

extern char savegamestrings[10][SAVESTRINGSIZE];

static void MN_DrawFileSlots(int x, int y, int menu)
{
	int i;
	extern const char* saves_pages[];

	for(i = 0; i < g_menu_save_page_size; i++)
	{
		dboolean selected = M_FileBoxSelected(menu, i);
		int textcolor = M_HighlightColor(selected, M_FileTextColor(menu, i));
		int boxcolor = M_HighlightColor(selected, CR_DEFAULT);
		int flags = VPT_STRETCH | M_AddColorFlag(boxcolor);

		V_DrawNamePatch(x, y, 0, "M_FSLOT", boxcolor, static_cast<enum patch_translation_e>(flags));
		MN_DrTextAColor(savegamestrings[i], x + 5, y + 5, textcolor);
		y += ITEM_HEIGHT;
	}

	M_DrawTabs(saves_pages, 5, 135);
}

void MN_DrawLoad()
{
	const char* title;

	title = "LOAD GAME";

	MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
	MN_DrawFileSlots(LoadDef.x, LoadDef.y, MN_LOAD);

	if(delete_verify)
		M_DrawDelVerify();
}

extern int saveStringEnter;
extern int saveSlot;

void MN_DrawSave()
{
	const char* title;

	title = "SAVE GAME";

	MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
	MN_DrawFileSlots(SaveDef.x, SaveDef.y, MN_SAVE);

	if(saveStringEnter)
	{
		int i;

		i = MN_TextAWidth(savegamestrings[saveSlot]);
		MN_DrTextA("[", SaveDef.x + 5 + i, SaveDef.y + 5 + saveSlot * ITEM_HEIGHT); // [ is _ in font A
	}

	if(delete_verify)
		M_DrawDelVerify();
}

void MN_DrawPause()
{
	const char* title;

	title = "PAUSE";
	MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
}

void MN_DrTextA(const char* text, int x, int y)
{
	MN_DrTextAColor(text, x, y, CR_DEFAULT);
}

static void MN_DrTextAColor(const char* text, int x, int y, int cm)
{
	char c;
	int lump;
	int flags;

	flags = VPT_STRETCH | M_AddColorFlag(cm);

	while((c = *text++) != 0)
	{
		c = toupper(c);
		if(c < 33)
		{
			x += 5;
		}
		else
		{
			lump = MN_SafeFontALump(c - 33);
			V_DrawNumPatch(x, y, 0, lump, cm, static_cast<enum patch_translation_e>(flags));
			x += R_NumPatchWidth(lump) - 1;
		}
	}
}

int MN_TextAHeight(const char* text)
{
	int i, line_height, total_height;

	line_height = R_NumPatchHeight(FontABaseLump);
	total_height = line_height;

	for(i = 0; text[i]; i++)
		if(text[i] == '\n')
			total_height += line_height;
	return total_height;
}

int MN_TextAWidth(const char* text)
{
	char c;
	int width;
	int lump;

	width = 0;
	while((c = *text++) != 0)
	{
		c = toupper(c);
		if(c < 33)
		{
			width += 5;
		}
		else
		{
			lump = MN_SafeFontALump(c - 33);
			width += R_NumPatchWidth(lump) - 1;
		}
	}
	return (width);
}

void MN_DrTextB(const char* text, int x, int y)
{
	MN_DrTextBColor(text, x, y, CR_DEFAULT);
}

static void MN_DrTextBColor(const char* text, int x, int y, int cm)
{
	char c;
	int lump;
	int flags;

	flags = VPT_STRETCH | M_AddColorFlag(cm);

	while((c = *text++) != 0)
	{
		c = toupper(c);
		if(c < 33)
		{
			x += 8;
		}
		else
		{
			lump = FontBBaseLump + c - 33;
			V_DrawNumPatch(x, y, 0, lump, cm, static_cast<enum patch_translation_e>(flags));
			x += R_NumPatchWidth(lump) - 1;
		}
	}
}

int MN_TextBWidth(const char* text)
{
	char c;
	int width;
	int lump;

	width = 0;
	while((c = *text++) != 0)
	{
		c = toupper(c);
		if(c < 33)
		{
			width += 5;
		}
		else
		{
			lump = FontBBaseLump + c - 33;
			width += R_NumPatchWidth(lump) - 1;
		}
	}
	return (width);
}

void MN_DrawTitle(int y, const char* text, int cm)
{
	MN_DrTextB(text, 160 - (MN_TextBWidth(text) / 2), y);
}

#define SLIDER_LIMIT 200
#define SLIDER_WIDTH (SLIDER_LIMIT - 64)
#define SLIDER_PATCH_COUNT (SLIDER_WIDTH / 8)

void MN_DrawSlider(int x, int y, int width, int range, int slot, int color)
{
	int xx;
	int i;
	int slot_offset;
	short slider_img = 0;

	// [AR] We check both if the item is selected and highlight
	// to include the label on the sound screen
	int flags = VPT_STRETCH | M_AddColorFlag(color);

	width -= 4;

	xx = x - 12;
	V_DrawNamePatch(xx, y, 0, "M_SLDLT", color, static_cast<enum patch_translation_e>(flags));
	xx += 32;
	for(i = 0; i < width; i++)
	{
		const char* name;
		name = (slider_img & 1 ? "M_SLDMD1" : "M_SLDMD2");
		slider_img ^= 1;
		V_DrawNamePatch(xx, y, 0, name, color, static_cast<enum patch_translation_e>(flags));

		xx += 8;
	}
	V_DrawNamePatch(xx, y, 0, "M_SLDRT", color, static_cast<enum patch_translation_e>(flags));

	if(slot >= range)
	{
		slot = range - 1;
	}

	width += 1;

	slot_offset = 8 * slot * width / range;
	slot_offset -= range / width;
	V_DrawNamePatch(x + 20 + slot_offset, y + 7, 0, "M_SLDKB", color, static_cast<enum patch_translation_e>(flags));
}

// hexen

void MN_DrTextAYellow(const char* text, int x, int y)
{
	char c;
	int lump;

	while((c = *text++) != 0)
	{
		c = toupper(c);
		if(c < 33)
		{
			x += 5;
		}
		else
		{
			lump = FontAYellowBaseLump + c - 33;
			V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
			x += R_NumPatchWidth(lump) - 1;
		}
	}
}
