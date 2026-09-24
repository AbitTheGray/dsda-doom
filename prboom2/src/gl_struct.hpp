// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// declared in v_video.hpp; the fixed underlying type makes this enough
enum struct ColorRange : int32_t;

#include <SDL_opengl.h>

#include "cpp/Util.hpp"

enum struct BleedType : uint8_t
{
	None    = 0x0,
	Ceiling = 0x1,
	Occlude = 0x2
};
ENUM_FLAGS_FUNC(BleedType)

#ifdef __cplusplus
extern "C"
{
#endif

extern dboolean use_gl_nodes;

enum struct SkyType : int32_t
{
	Auto,
	None,
	Standard,
	Skydome,

	Count
};

#define MAX_GLGAMMA 32


extern SkyType gl_drawskys;
extern dboolean gl_ui_lightmode_indexed;
extern dboolean gl_automap_lightmode_indexed;
extern dboolean gl_menu_lightmode_indexed;
void gld_FlushTextures();

void gld_InitVertexData();
void gld_CleanVertexData();
void gld_UpdateSplitData(sector_t* sector);

void gld_Init(int width, int height);
void gld_InitCommandLine();

void gld_BeginUIDraw();
void gld_EndUIDraw();
void gld_BeginAutomapDraw();
void gld_EndAutomapDraw();
void gld_BeginMenuDraw();
void gld_EndMenuDraw();

void gld_DrawNumPatch(int x, int y, int lump, dboolean center, ColorRange cm, PatchTranslation flags);
void gld_DrawNumPatch_f(float x, float y, int lump, dboolean center, ColorRange cm, PatchTranslation flags);

void gld_FillRaw(int lump, int x, int y, int src_width, int src_height, int dst_width, int dst_height, PatchTranslation flags);
#define gld_FillRawName(name, x, y, src_width, src_height, dst_width, dst_height, flags) \
  gld_FillRaw(W_GetNumForName(name), (x), (y), (src_width), (src_height), (dst_width), (dst_height), (flags))

#define gld_FillFlat(lump, x, y, width, height, flags) \
  gld_FillRaw((firstflat+lump), (x), (y), 64, 64, (width), (height), (flags))
#define gld_FillFlatName(flatname, x, y, width, height, flags) \
  gld_FillFlat(R_FlatNumForName(flatname), (x), (y), (width), (height), (flags))

void gld_FillPatch(int lump, int x, int y, int width, int height, PatchTranslation flags);
#define gld_FillPatchName(name, x, y, width, height, flags) \
  gld_FillPatch(W_GetNumForName(name), (x), (y), (width), (height), (flags))

void gld_DrawLine(int x0, int y0, int x1, int y1, int BaseColor);
void gld_DrawLine_f(float x0, float y0, float x1, float y1, int BaseColor);
void gld_DrawWeapon(int weaponlump, vissprite_t* vis, int lightlevel);
void gld_FillBlock(int x, int y, int width, int height, int col);
void gld_DrawShaded(int x, int y, int width, int height, int shade);
void gld_SetPalette(int palette);
unsigned char* gld_ReadScreen();

void gld_CleanMemory();
void gld_CleanStaticMemory();
void gld_PreprocessLevel();

void gld_Set2DMode();
void gld_InitDrawScene();
void gld_StartDrawScene();
void gld_AddPlane(int subsectornum, visplane_t* floor, visplane_t* ceiling);
void gld_AddWall(seg_t* seg);
void gld_ProjectSprite(mobj_t* thing, int lightlevel);
void gld_DrawScene(player_t* player);
void gld_EndDrawScene();
void gld_Finish();

// wipe
int gld_wipe_doMelt(int ticks, int* y_lookup);
int gld_wipe_exitMelt(int ticks);
int gld_wipe_StartScreen();
int gld_wipe_EndScreen();

//clipper
dboolean gld_clipper_SafeCheckRange(angle_t startAngle, angle_t endAngle);
void gld_clipper_SafeAddClipRange(angle_t startangle, angle_t endangle);
void gld_FrustumSetup();
dboolean gld_SphereInFrustum(float x, float y, float z, float radius);

//missing flats (fake floors and ceilings)
extern dboolean gl_use_stencil;
sector_t* GetBestFake(sector_t* sector, int ceiling, int validcount);
sector_t* GetBestBleedSector(sector_t* source, BleedType type);

void gld_DrawMapLines();

//multisampling
void gld_MultisamplingInit();
void gld_MultisamplingSet();

void gld_ProcessTexturedMap();
void gld_ResetTexturedAutomap();
void gld_MapDrawSubsectors(player_t* plr, int fx, int fy, fixed_t mx, fixed_t my, int fw, int fh, fixed_t scale);

void gld_Init8InGLMode();
void gld_Draw8InGL();

// Nice map
enum struct AutomapIcon : int32_t
{
	Shadow,

	Corpse,
	Normal,
	Health,
	Armor,
	Ammo,
	Key,
	Power,
	Weap,

	Arrow,
	Monster,
	Player,
	Mark,
	Bullet,

	Count
};

typedef struct am_icon_s
{
	GLuint tex_id;
	const char* name;
	int lumpnum;
} am_icon_t;

extern am_icon_t am_icons[];

void gld_InitMapPics();
void gld_AddNiceThing(AutomapIcon type, float x, float y, float radius, float angle,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void gld_DrawNiceThings(int fx, int fy, int fw, int fh);
void gld_ClearNiceThings();

extern int gl_render_fov;

#ifdef __cplusplus
}
#endif
