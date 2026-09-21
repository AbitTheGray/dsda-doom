// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Signal Context

typedef enum {
  sf_display             = 0x0001,
  sf_player_view         = 0x0002,
  sf_setup_frame         = 0x0004,
  sf_clear               = 0x0008,
  sf_init_scene          = 0x0010,
  sf_gl_frustum          = 0x0020,
  sf_bsp_nodes           = 0x0040,
  sf_draw_planes         = 0x0080,
  sf_reset_column_buffer = 0x0100,
  sf_draw_masked         = 0x0200,
  sf_draw_scene          = 0x0400,
  sf_status_bar          = 0x0800,
  sf_hud                 = 0x1000,
} signal_context_t;

extern int signal_context;

#define DSDA_ADD_CONTEXT(x) signal_context |= x
#define DSDA_REMOVE_CONTEXT(x) signal_context &= ~x;
