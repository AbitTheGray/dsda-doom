// SPDX-License-Identifier: GPL-2.0-or-later

#include "s_sound.hpp"
#include "g_game.hpp"
#include "d_main.hpp"

static void Hexen_D_DrawTitle(const char* _x)
{
	D_SetPage("TITLE", 280, MusicId::None);
	S_StartSongName("hexen", true);
}

static void Hexen_D_DrawTitle2(const char* _x)
{
	D_SetPage("TITLE", 210, MusicId::None);
}

static void Hexen_D_DrawCredits(const char* _x)
{
	D_SetPage("CREDIT", 200, MusicId::None);
}

extern const demostate_t hexen_demostates[][4] =
{
	{
		{Hexen_D_DrawTitle, nullptr},
		{Hexen_D_DrawTitle, nullptr},
		{Hexen_D_DrawTitle, nullptr},
		{Hexen_D_DrawTitle, nullptr},
	},

	{
		{Hexen_D_DrawTitle2, nullptr},
		{Hexen_D_DrawTitle2, nullptr},
		{Hexen_D_DrawTitle2, nullptr},
		{Hexen_D_DrawTitle2, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
	},

	{
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
	},

	{
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
		{Hexen_D_DrawCredits, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
		{G_DeferedPlayDemo, "demo3"},
	},

	{
		{nullptr},
		{nullptr},
		{nullptr},
		{nullptr},
	}
};
