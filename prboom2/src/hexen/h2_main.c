// SPDX-License-Identifier: GPL-2.0-or-later

#include "s_sound.h"
#include "g_game.h"
#include "d_main.h"

static void Hexen_D_DrawTitle(const char* _x)
{
	D_SetPage("TITLE", 280, 0);
	S_StartSongName("hexen", true);
}

static void Hexen_D_DrawTitle2(const char* _x)
{
	D_SetPage("TITLE", 210, 0);
}

static void Hexen_D_DrawCredits(const char* _x)
{
	D_SetPage("CREDIT", 200, 0);
}

const demostate_t hexen_demostates[][4] =
{
	{
		{Hexen_D_DrawTitle, NULL},
		{Hexen_D_DrawTitle, NULL},
		{Hexen_D_DrawTitle, NULL},
		{Hexen_D_DrawTitle, NULL},
	},

	{
		{Hexen_D_DrawTitle2, NULL},
		{Hexen_D_DrawTitle2, NULL},
		{Hexen_D_DrawTitle2, NULL},
		{Hexen_D_DrawTitle2, NULL},
	},

	{
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
	},

	{
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
	},

	{
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
	},

	{
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
		{Hexen_D_DrawCredits, NULL},
	},

	{
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
	},

	{
		{NULL},
		{NULL},
		{NULL},
		{NULL},
	}
};
