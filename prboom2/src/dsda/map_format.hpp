// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Format

#pragma once

#include "doomtype.hpp"
#include "r_defs.hpp"
#include "p_maputl.hpp"
#include "p_spec.hpp"
#include "info.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	dboolean zdoom;
	dboolean hexen;
	dboolean polyobjs;
	dboolean acs;
	dboolean thing_id;
	dboolean sndseq;
	dboolean animdefs;
	dboolean doublesky;
	dboolean map99;
	short generalized_mask;
	line_activation_t switch_activation;
	void (*init_sector_special)(sector_t*, int);
	void (*player_in_special_sector)(player_t*, sector_t*);
	dboolean (*mobj_in_special_sector)(mobj_t*);
	void (*spawn_scroller)(line_t*, int);
	void (*spawn_friction)(line_t*);
	void (*spawn_pusher)(line_t*);
	void (*spawn_extra)(line_t*, int);
	void (*cross_special_line)(line_t*, int, mobj_t*, dboolean);
	void (*shoot_special_line)(mobj_t*, line_t*);
	dboolean (*test_activate_line)(line_t*, mobj_t*, int, line_activation_t);
	dboolean (*execute_line_special)(int, int*, line_t*, int, mobj_t*);
	void (*post_process_line_special)(line_t*);
	void (*post_process_sidedef_special)(side_t*, const char*, const char*, const char*, sector_t*, int);
	void (*animate_surfaces)();
	void (*check_impact)(mobj_t*);
	LineFlag (*translate_line_flags)(uint32_t raw_flags, line_activation_t* activation);
	void (*apply_sector_movement_special)(mobj_t*, int);
	void (*t_vertical_door)(vldoor_t*);
	void (*t_move_floor)(floormove_t*);
	void (*t_move_ceiling)(ceiling_t*);
	void (*t_build_pillar)(pillar_t*);
	void (*t_plat_raise)(plat_t*);
	int (*ev_teleport)(short, int, line_t*, int, mobj_t*, TeleportFlag);
	void (*player_thrust)(player_t* player, angle_t angle, fixed_t move);
	void (*build_mobj_thing_id_list)();
	void (*add_mobj_thing_id)(mobj_t*, short);
	void (*remove_mobj_thing_id)(mobj_t*);
	void (*iterate_spechit)(mobj_t*, fixed_t, fixed_t);
	size_t mapthing_size;
	size_t maplinedef_size;
	MobjType mt_push;
	MobjType mt_pull;
	int dn_polyanchor;
	int dn_polyspawn_start;
	int dn_polyspawn_hurt;
	int dn_polyspawn_end;
	ThingVisibility visibility;
} map_format_t;

extern map_format_t map_format;

enum struct DoorType : int32_t
{
	None = -1,
	Red,
	Blue,
	Yellow,
	Unknown = Yellow,
	Multiple
};

DoorType dsda_DoorType(int index);
dboolean dsda_IsExitLine(int index);
dboolean dsda_IsSecretExitLine(int index);
dboolean dsda_IsDeathExitLine(int index);
dboolean dsda_IsDeathSecretExitLine(int index);
dboolean dsda_IsTeleportLine(int index);
void dsda_ApplyZDoomMapFormat();
void dsda_ApplyUDMF();
void dsda_ApplyBinaryMapFormat();

#ifdef __cplusplus
}
#endif
