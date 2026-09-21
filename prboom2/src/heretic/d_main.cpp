// SPDX-License-Identifier: GPL-2.0-or-later

// D_main.c

#include "sounds.hpp"
#include "g_game.hpp"
#include "d_main.hpp"

#include "heretic/dstrings.hpp"

static void Heretic_D_DrawTitle(const char* _x)
{
	D_SetPage("TITLE", 210, heretic_mus_titl);
}

static void Heretic_D_DrawTitle2(const char* _x)
{
	D_SetPage("TITLE", 140, 0);
}

static void Heretic_D_DrawCredits(const char* _x)
{
	D_SetPage("CREDIT", 200, 0);
}

static void Heretic_D_DrawOrder(const char* _x)
{
	D_SetPage("ORDER", 200, 0);
}

extern const demostate_t heretic_demostates[][4] =
{
	{
		{Heretic_D_DrawTitle, nullptr},
		{Heretic_D_DrawTitle, nullptr},
		{Heretic_D_DrawTitle, nullptr},
		{Heretic_D_DrawTitle, nullptr},
	},

	{
		{Heretic_D_DrawTitle2, nullptr},
		{Heretic_D_DrawTitle2, nullptr},
		{Heretic_D_DrawTitle2, nullptr},
		{Heretic_D_DrawTitle2, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
		{G_DeferedPlayDemo, "demo1"},
	},

	{
		{Heretic_D_DrawCredits, nullptr},
		{Heretic_D_DrawCredits, nullptr},
		{Heretic_D_DrawCredits, nullptr},
		{Heretic_D_DrawCredits, nullptr},
	},

	{
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
		{G_DeferedPlayDemo, "demo2"},
	},

	{
		{Heretic_D_DrawOrder, nullptr},
		{Heretic_D_DrawCredits, nullptr},
		{Heretic_D_DrawCredits, nullptr},
		{Heretic_D_DrawCredits, nullptr},
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
