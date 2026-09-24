// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// declared in v_video.hpp; the fixed underlying type makes this enough
enum struct ColorRange : int32_t;

#include <utility>

#include "v_video.hpp"
#include "xs_Float.hpp"

enum struct GLTextureFlag : uint32_t
{
	Sprite   = 0x00000002,
	HasHoles = 0x00000004,
	Sky      = 0x00000008,

	ClampX  = 0x00000040,
	ClampY  = 0x00000080,
	ClampXy = (ClampX | ClampY),
	Indexed = 0x00000100,
	SkyHack = 0x00000200,
};
ENUM_FLAGS_FUNC(GLTextureFlag)

enum struct GLFlatFlag : uint32_t
{
	Ceiling       = 0x00000001,
	HaveTransform = 0x00000002,
};
ENUM_FLAGS_FUNC(GLFlatFlag)

#ifdef __cplusplus
extern "C"
{
#endif

#define MAXCOORD (32767.0f / MAP_COEFF)

enum struct GLTexType : int32_t
{
	Unregistered,
	Broken,
	Patch,
	Texture,
	Flat,
	Colormap
};

typedef struct gl_strip_coords_s
{
	GLfloat v[4][3];

	GLfloat t[4][2];
} gl_strip_coords_t;

#define PLAYERCOLORMAP_COUNT (9)

typedef struct detail_s
{
	GLuint texid;
	int texture_num;
	float width, height;
	float offsetx, offsety;
} detail_t;

typedef struct color_rgb_s
{
	byte r;
	byte g;
	byte b;
} color_rgb_t;

typedef struct
{
	int index;
	int patch_index;
	int width, height;
	int leftoffset, topoffset;
	int tex_width, tex_height;
	int realtexwidth, realtexheight;
	int buffer_width, buffer_height;
	int buffer_size;

	//e6y: support for Boom colormaps
	GLuint*** glTexExID;
	unsigned int texflags[std::to_underlying(ColorRange::Limit) + MAX_MAXPLAYERS][PLAYERCOLORMAP_COUNT];
	GLuint* texid_p;
	unsigned int* texflags_p;

	ColorRange cm;
	int player_cm;

	GLTexType textype;
	GLTextureFlag flags;
	float scalexfac, scaleyfac; //e6y: right/bottom UV coordinates for patch drawing
} GLTexture;

typedef struct
{
	float x1, x2;
	float z1, z2;
	dboolean fracleft, fracright; //e6y
} GLSeg;

typedef struct
{
	GLSeg* glseg;
	float ytop, ybottom;
	float ul, ur, vt, vb;
	float light;
	float alpha;
	float skypitch;
	float skyyaw;
	float skyoffset;
	float xscale;
	float yscale;
	dboolean anchor_vb;
	GLTexture* gltexture;
	byte flag;
	seg_t* seg;
} GLWall;


typedef struct
{
	int sectornum;
	float light; // the lightlevel of the flat
	float fogdensity;
	float uoffs, voffs; // the texture coordinates
	float rotation;
	float xscale;
	float yscale;
	float z; // the z position of the flat (height)
	GLTexture* gltexture;
	GLFlatFlag flags;
	float alpha;
} GLFlat;

/* GLLoopDef is the struct for one loop. A loop is a list of vertexes
 * for triangles, which is calculated by the gluTesselator in gld_PrecalculateSector
 * and in gld_PreprocessCarvedFlat
 */
typedef struct
{
	int index;       // subsector index
	GLenum mode;     // GL_TRIANGLES, GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN
	int vertexcount; // number of vertexes in this loop
	int vertexindex; // index into vertex list
} GLLoopDef;

// GLSector is the struct for a sector with a list of loops.

#define SECTOR_CLAMPXY   0x00000001

typedef struct
{
	int loopcount;    // number of loops for this sector
	GLLoopDef* loops; // the loops itself
	unsigned int flags;
} GLSector;

typedef struct
{
	int loopcount;    // number of loops for this sector
	GLLoopDef* loops; // the loops itself
} GLMapSubsector;

typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} GLVertex;

typedef struct
{
	GLfloat u;
	GLfloat v;
} GLTexcoord;

typedef struct
{
	GLLoopDef loop; // the loops itself
} GLSubSector;

typedef struct
{
	float x, y, z;
	float radius;
	float light;
} GLShadow;

enum struct HealthBarColor : int32_t
{
	Null,
	Red,
	Yellow,
};

typedef struct
{
	HealthBarColor color;

	float x1, x2, x3;
	float z1, z2, z3;
	float y;
} GLHealthBar;

extern GLSeg* gl_segs;
extern GLSeg* gl_lines;

#define GLDWF_TOP 1
#define GLDWF_M1S 2
#define GLDWF_M2S 3
#define GLDWF_BOT 4
#define GLDWF_TOPFLUD 5 //e6y: project the ceiling plane into the gap
#define GLDWF_BOTFLUD 6 //e6y: project the floor plane into the gap
#define GLDWF_SKY 7
#define GLDWF_SKYFLIP 8

typedef struct
{
	ColorRange cm;
	float x, y, z;
	float vt, vb;
	float ul, ur;
	float x1, y1;
	float x2, y2;
	float light;
	float alpha;
	fixed_t scale;
	GLTexture* gltexture;
	uint64_t flags;
	int index;
	int id;
	int xy;
	fixed_t fx, fy;
} GLSprite;

enum struct GLDrawItemType : int32_t
{
	None,

	Wall,  // opaque wall
	Mwall, // opaque mid wall
	Fwall, // projected wall
	Twall, // transparent walls
	Swall, // sky walls

	Awall,  // animated wall
	Fawall, // animated projected wall

	Ceiling, // ceiling
	Floor,   // floor

	Aceiling, // animated ceiling
	Afloor,   // animated floor

	Sprite,  // opaque sprite
	Tsprite, // transparent sprites
	Asprite,

	Shadow,

	Hbar,

	Types
};

typedef struct GLDrawItem_s
{
	union
	{
		void* item;
		GLWall* wall;
		GLFlat* flat;
		GLSprite* sprite;
		GLShadow* shadow;
		GLHealthBar* hbar;
	} item;
} GLDrawItem;

typedef struct GLDrawDataItem_s
{
	byte* data;
	int maxsize;
	int size;
} GLDrawDataItem_t;

typedef struct
{
	GLDrawDataItem_t* data;
	int maxsize;
	int size;

	GLDrawItem* items[std::to_underlying(GLDrawItemType::Types)];
	int num_items[std::to_underlying(GLDrawItemType::Types)];
	int max_items[std::to_underlying(GLDrawItemType::Types)];
} GLDrawInfo;

void gld_AddDrawItem(GLDrawItemType itemtype, void* itemdata);

void gld_DrawTriangleStrip(GLWall* wall, gl_strip_coords_t* c);

extern float roll;
extern float yaw;
extern float inv_yaw;
extern float pitch;

extern int gl_preprocessed; //e6y

extern GLDrawInfo gld_drawinfo;
void gld_FreeDrawInfo();
void gld_ResetDrawInfo();

extern GLSector* sectorloops;
extern GLMapSubsector* subsectorloops;

extern GLfloat gl_texture_filter_anisotropic;
void gld_SetTexFilters(GLTexture* gltexture);

extern float xCamera, yCamera, zCamera;

//
//detail
//

void gld_InitDetail();

void gld_PreprocessDetail();

extern GLuint* last_glTexID;
GLTexture* gld_RegisterTexture(int texture_num, dboolean mipmap, dboolean force, dboolean indexed, dboolean sky);
void gld_BindTexture(GLTexture* gltexture, GLTextureFlag flags, dboolean sky);
GLTexture* gld_RegisterPatch(int lump, ColorRange cm, dboolean is_sprite, dboolean indexed);
void gld_BindPatch(GLTexture* gltexture, ColorRange cm);
GLTexture* gld_RegisterRaw(int lump, int width, int height, dboolean mipmap, dboolean indexed);
void gld_BindRaw(GLTexture* gltexture, GLTextureFlag flags);
#define gld_RegisterFlat(lump, mipmap, indexed) \
  gld_RegisterRaw((firstflat+lump), 64, 64, (mipmap), (indexed))
#define gld_BindFlat(gltexture, flags) \
  gld_BindRaw((gltexture), (flags))
GLTexture* gld_RegisterSkyTexture(int texture_num, dboolean force);
void gld_BindSkyTexture(GLTexture* gltexture);
GLTexture* gld_RegisterColormapTexture(int palette_index, int gamma_level, dboolean fullbright);
void gld_BindColormapTexture(GLTexture* gltexture, int palette_index, int gamma_level, dboolean fullbright);
void gld_InitColormapTextures(dboolean fullbright);
void gld_InitFuzzTexture();
int gld_GetTexDimension(int value);
void gld_SetIndexedPalette(int palette_index);
void gld_Precache();

void SetFrameTextureMode();

//gl_vertex
void gld_SplitLeftEdge(const GLWall* wall);
void gld_SplitRightEdge(const GLWall* wall);
void gld_RecalcVertexHeights(const vertex_t* v);

//e6y
void gld_InitGLVersion();
void gld_ResetLastTexture();

unsigned char* gld_GetTextureBuffer(GLuint texid, int miplevel, int* width, int* height);

int gld_BuildTexture(GLTexture* gltexture, void* data, dboolean readonly, int width, int height);

GLuint CaptureScreenAsTexID();

//progress
void gld_ProgressUpdate(const char* text, int progress, int total);
int gld_ProgressStart();
int gld_ProgressEnd();

//FBO
extern GLuint glSceneImageFBOTexID;
extern GLuint glSceneImageTextureFBOTexID;

extern dboolean invul_cm;
extern float bw_red;
extern float bw_green;
extern float bw_blue;
extern int SceneInTexture;
void gld_InitFBO();
void gld_FreeScreenSizeFBO();

extern int imageformats[];

//missing flats (fake floors and ceilings)

void gld_PreprocessFakeSectors();

void gld_SetupFloodStencil(GLWall* wall);
void gld_ClearFloodStencil(GLWall* wall);

void gld_SetupFloodedPlaneCoords(GLWall* wall, gl_strip_coords_t* c);
void gld_SetupFloodedPlaneLight(GLWall* wall);

//light
void gld_StaticLightAlpha(float light, float alpha);
#define gld_StaticLight(light) gld_StaticLightAlpha(light, 1.0f)
void gld_InitLightTable();
int gld_GetGunFlashLight();

float gld_CalcLightLevel(int lightlevel);
float gld_Calc2DLightLevel(int lightlevel);

// SkyBox
#define SKY_NONE    0
#define SKY_CEILING 1
#define SKY_FLOOR   2

typedef struct PalEntry_s
{
	unsigned char r, g, b;
} PalEntry_t;

typedef struct SkyBoxParams_s
{
	int index;
	unsigned int type;
	GLWall wall;
	float x_offset, y_offset;
	// 0 - no colormap; 1 - INVUL inverse colormap
	PalEntry_t FloorSkyColor[2];
	PalEntry_t CeilingSkyColor[2];
} SkyBoxParams_t;

extern SkyBoxParams_t SkyBox;
extern GLfloat gl_whitecolor[];
void gld_InitSky();
void gld_AddSkyTexture(GLWall* wall, int sky1, int sky2, int skytype);
void gld_GetSkyCapColors();
void gld_InitFrameSky();
void gld_DrawStripsSky();
void gld_DrawScreenSkybox();
void gld_GetScreenSkyScale(GLWall* wall, float* scale_x, float* scale_y);
void gld_DrawDomeSkyBox();
void gld_DrawSkyCaps();

// VBO
typedef struct vbo_vertex_s
{
	float x, y, z;
	float u, v;
	unsigned char r, g, b, a;
}
	PACKEDATTR vbo_vertex_t;

#define NULL_VBO_VERTEX ((vbo_vertex_t*)nullptr)
#define sky_vbo_x (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->x : &vbo->data[0].x)
#define sky_vbo_u (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->u : &vbo->data[0].u)
#define sky_vbo_r (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_VERTEX->r : &vbo->data[0].r)

typedef struct vbo_xyz_uv_s
{
	float x, y, z;
	float u, v;
}
	PACKEDATTR vbo_xyz_uv_t;

extern vbo_xyz_uv_t* flats_vbo;
#define NULL_VBO_XYZ_UV ((vbo_xyz_uv_t*)nullptr)
#define flats_vbo_x (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_XYZ_UV->x : &flats_vbo[0].x)
#define flats_vbo_u (gl_ext_arb_vertex_buffer_object ? &NULL_VBO_XYZ_UV->u : &flats_vbo[0].u)

typedef struct vbo_xy_uv_rgba_s
{
	float x, y;
	float u, v;
	unsigned char r, g, b, a;
}
	PACKEDATTR vbo_xy_uv_rgba_t;

// preprocessing
extern byte* segrendered;    // true if sector rendered (only here for malloc)
extern int* linerendered[2]; // true if linedef rendered (only here for malloc)
extern int rendermarker;
extern GLuint flats_vbo_id;

void glsl_Init();
void glsl_SetTextureDims(int unit, unsigned int width, unsigned int height);
void glsl_PushNullShader();
void glsl_PopNullShader();
void glsl_PushMainShader();
void glsl_PopMainShader();
void glsl_PushFuzzShader(int tic, int sprite, float ratio);
void glsl_PopFuzzShader();
void glsl_SetLightLevel(float lightlevel);

#ifdef __cplusplus
}
#endif
