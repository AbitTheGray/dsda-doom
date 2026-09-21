// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA HUD Component Base

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <math.h>

#include "am_map.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp"
#include "hu_lib.hpp"
#include "hu_stuff.hpp"
#include "m_menu.hpp"
#include "p_mobj.hpp"
#include "p_pspr.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "r_data.hpp"
#include "r_main.hpp"
#include "r_state.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"

#include "dsda.hpp"
#include "dsda/demo.hpp"
#include "dsda/exhud.hpp"
#include "dsda/font.hpp"
#include "dsda/global.hpp"
#include "dsda/settings.hpp"
#include "dsda/text_color.hpp"
#include "dsda/utility.hpp"

#define DSDA_TEXT_SIZE 200
#define DSDA_CHAR_HEIGHT 8
#define DSDA_CHAR_WIDTH 6

typedef struct
{
	hu_textline_t text;
	char msg[DSDA_TEXT_SIZE];
} dsda_text_t;

typedef struct
{
	int x;
	int y;
	int vpt;
} dsda_patch_component_t;

int dsda_HudComponentY(int y_offset, int vpt, double ratio);
void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);
void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);
void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt);
fixed_t dsda_HexenArmor(player_t* player);
void dsda_DrawBigNumber(int x, int y, int delta_x, int delta_y, int cm, int vpt, int count, int n);
void dsda_DrawBasicText(dsda_text_t* component);
void dsda_RefreshHudText(dsda_text_t* component);

#ifdef __cplusplus
}
#endif
