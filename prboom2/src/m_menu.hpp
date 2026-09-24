// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Menu widget stuff, episode selection and such.
 */

#pragma once

#include <stdint.h>

// declared in dsda/input.hpp; the fixed underlying type makes this enough
enum struct InputId : int32_t;

#include <stdint.h>

// declared in v_video.hpp; the fixed underlying type makes this enough
enum struct ColorRange : int32_t;

#include "d_event.hpp"
#include "dsda/configuration.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.

dboolean M_Responder(event_t* ev);

dboolean fadeBG();
dboolean M_MenuIsShaded();
void M_ShadedScreen(int scrn);

#define FULLSHADE 20

// Called by main loop,
// only used for menu (skull cursor) animation.

void M_Ticker();

// Called by main loop,
// draws the menus directly into the screen buffer.

void M_Drawer();

// Called by D_DoomMain,
// loads the config file.

void M_Init();

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.

void M_StartControlPanel();

void M_ForcedLoadGame(const char* msg); // killough 5/15/98: forced loadgames

void M_ResetMenu(); // killough 11/98: reset main menu ordering

void M_DrawCredits();
void M_DrawCreditsDynamic(); // killough 11/98

void M_DrawTabs(const char** pages, int m, int y);

// Big Thermo (for Raven)
void M_DrawThermoBig(int x, int y, int thermWidth, int thermRange, int thermDot, int menu_item);

enum struct SaveOrLoadMenu : int32_t
{
	Load,
	Save,
};

// Save / Load Highlights
dboolean M_FileBoxSelected(SaveOrLoadMenu menu, int item);
ColorRange M_FileTextColor(SaveOrLoadMenu menu, int item);

// Menu Highlights
dboolean M_MouseHovered(int index);
ColorRange M_HighlightColor(dboolean highlight, ColorRange color);
PatchTranslation M_AddColorFlag(ColorRange color);

/****************************
 *
 * The setup_group enum is used to show which 'groups' keys fall into so
 * that you can bind a key differently in each 'group'.
 * It also applies behaviour to other types of settings.
 */

enum struct SetupGroup : int32_t
{
	Null, // Has no meaning; not applicable
	Screen, // A key can not be assigned to more than one action
	Map,  // in the same group. A key can be assigned to one
	Menu, // action in one group, and another action in another.
	Build,

	Conf, // migrate to new config process
};

/****************************
 *
 * phares 4/17/98:
 * State definition for each item.
 * This is the definition of the structure for each setup item. Not all
 * fields are used by all items.
 *
 * A setup screen is defined by an array of these items specific to
 * that screen.
 *
 * killough 11/98:
 *
 * Restructured to allow simpler table entries,
 * and to Xref with defaults[] array in m_misc.c.
 * Moved from m_menu.c to m_menu.h so that m_misc.c can use it.
 */

typedef struct setup_menu_s
{
	const char* m_text;  /* text to display */
	int m_flags;         /* phares 4/17/98: flag bits S_* (defined above) */
	SetupGroup m_group; /* Group */
	short m_x;           /* screen x position (left is 0) */
	ConfigId config_id;
	InputId input;              // composite input identifier
	const char** selectstrings; /* list of strings for choice value */
	struct setup_menu_s* menu;  /* next or prev menu */
} setup_menu_t;

//
// MENU TYPEDEFS
//

enum struct MenuItemType : int32_t
{
	Skip = -1,
	Inactive,
	Action,
	Thermo,
};

typedef struct
{
	MenuItemType status;
	char name[10];

	// choice = menu item #.
	// if status = M_ITEM_THERMO,
	//   choice=0:leftarrow,1:rightarrow
	void (*routine)(int choice);
	char alphaKey; // hotkey in menu
	const char* alttext;
	ColorRange color;
	byte flags;
} menuitem_t;

#define MENUF_TEXTINPUT 0x01
#define MENUF_OPTLUMP   0x02 // [Nugget] Optional graphic lump

typedef struct menu_s
{
	short numitems;          // # of menu items
	struct menu_s* prevMenu; // previous menu
	menuitem_t* menuitems;   // menu items
	void (*routine)();       // draw routine
	short x;
	short y;      // x,y of menu
	short lastOn; // last item user was on in menu
	byte flags;
} menu_t;

#define SAVESTRINGSIZE 24

#define MENU_NULL      -1
#define MENU_LEFT      -2
#define MENU_RIGHT     -3
#define MENU_UP        -4
#define MENU_DOWN      -5
#define MENU_BACKSPACE -6
#define MENU_ENTER     -7
#define MENU_ESCAPE    -8
#define MENU_CLEAR     -9

void M_SetupNextMenu(menu_t* menudef);
void M_DrawDelVerify();
void M_ChangeMessages();
void M_LeaveSetupMenu();
void M_ClearMenus();

extern dboolean delete_verify;

dboolean M_ConsoleOpen();

#ifdef __cplusplus
}
#endif
