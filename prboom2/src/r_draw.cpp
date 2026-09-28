// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      The actual span/column drawing functions.
 *      Here find the main potential for optimization,
 *       e.g. inline assembly, different algorithms.
 */

#include <utility>

#include <stdint.h>

#include "doomstat.hpp"
#include "w_wad.hpp"
#include "r_main.hpp"
#include "r_draw.hpp"
#include "v_video.hpp"
#include "st_stuff.hpp"
#include "g_game.hpp"
#include "am_map.hpp"
#include "lprintf.hpp"

#include "dsda/stretch.hpp"

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

byte* viewimage;
int viewwidth;
int viewheight;

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

// CPhipps - made const*'s
const byte* tranmap;      // translucency filter maps 256x256   // phares
const byte* main_tranmap; // killough 4/11/98

//
// R_DrawColumn
// Source is the top of the column to scale.
//

// SoM: OPTIMIZE for ANYRES
enum struct ColumnType : int32_t
{
	None,
	Opaque,
	Trans,
	FlexTrans,
	Fuzz,
	FlexAdd
};

static int temp_x = 0;
static int tempyl[4], tempyh[4];

// e6y: resolution limitation is removed
static byte* tempbuf;

static int startx = 0;
static ColumnType temptype = ColumnType::None;
static int commontop, commonbot;
static const byte* temptranmap = nullptr;
// SoM 7-28-04: Fix the fuzz problem.
static const byte* tempfuzzmap;

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50
// proff 08/17/98: Changed for high-res
//#define FUZZOFF (SCREENWIDTH)
#define FUZZOFF 1

static const int fuzzoffset_org[FUZZTABLE] = {
	FUZZOFF, -FUZZOFF,FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF,
	FUZZOFF,FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF,
	FUZZOFF, -FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF,
	FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF, -FUZZOFF,FUZZOFF,
	FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF, -FUZZOFF,FUZZOFF,FUZZOFF, -FUZZOFF,FUZZOFF
};

static int fuzzoffset[FUZZTABLE];

static int fuzzpos = 0;

// Fuzz cell size for scaled software fuzz
static int fuzzcellsize;
int fuzz_cutoff = false;

// render pipelines
#define RDC_STANDARD      1
#define RDC_TRANSLUCENT   2
#define RDC_TRANSLATED    4
#define RDC_FUZZ          8
// no color mapping
#define RDC_NOCOLMAP     16

draw_vars_t drawvars = {
	nullptr, // topleft
	0,    // pitch
};

dboolean R_FullView()
{
	return viewheight == SCREENHEIGHT;
}

dboolean R_PartialView()
{
	return viewheight != SCREENHEIGHT;
}

dboolean R_StatusBarVisible()
{
	return R_PartialView() || automap_solid;
}

//
// Error functions that will abort if R_FlushColumns tries to flush
// columns without a column type.
//

static void R_FlushWholeError()
{
	Log::Fatal("R_FlushWholeColumns called without being initialized.\n");
}

static void R_FlushHTError()
{
	Log::Fatal("R_FlushHTColumns called without being initialized.\n");
}

static void R_QuadFlushError()
{
	Log::Fatal("R_FlushQuadColumn called without being initialized.\n");
}

static void (*R_FlushWholeColumns)() = R_FlushWholeError;
static void (*R_FlushHTColumns)() = R_FlushHTError;
static void (*R_FlushQuadColumn)() = R_QuadFlushError;

static void R_FlushColumns()
{
	if(temp_x != 4 || commontop >= commonbot)
		R_FlushWholeColumns();
	else
	{
		R_FlushHTColumns();
		R_FlushQuadColumn();
	}
	temp_x = 0;
}

//
// R_ResetColumnBuffer
//
// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
//
void R_ResetColumnBuffer()
{
	// haleyjd 10/06/05: this must not be done if temp_x == 0!
	if(temp_x)
		R_FlushColumns();
	temptype = ColumnType::None;
	R_FlushWholeColumns = R_FlushWholeError;
	R_FlushHTColumns = R_FlushHTError;
	R_FlushQuadColumn = R_QuadFlushError;
}

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz
#include "r_drawflush.inl"

//
// R_DrawColumn
//

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

byte* translationtables;

#define R_DRAWCOLUMN_PIPELINE_TYPE ColumnPipeline::Standard
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.

#define R_DRAWCOLUMN_PIPELINE_TYPE ColumnPipeline::Translucent
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLUCENT

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE ColumnPipeline::Translated
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE ColumnPipeline::Fuzz
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[std::to_underlying(DrawFilterType::Count)][std::to_underlying(ColumnPipeline::Count)] = {
	{
		R_DrawColumn_PointUV,
		R_DrawTLColumn_PointUV,
		R_DrawTranslatedColumn_PointUV,
		R_DrawFuzzColumn_PointUV,
	},
	{
		R_DrawColumn_PointUV_PointZ,
		R_DrawTLColumn_PointUV_PointZ,
		R_DrawTranslatedColumn_PointUV_PointZ,
		R_DrawFuzzColumn_PointUV_PointZ,
	},
};

R_DrawColumn_f R_GetDrawColumnFunc(ColumnPipeline type, DrawFilterType filterz)
{
	R_DrawColumn_f result = drawcolumnfuncs[std::to_underlying(filterz)][std::to_underlying(type)];
	if(result == nullptr)
		Log::Fatal("R_GetDrawColumnFunc: undefined function ({}, {})", std::to_underlying(type), std::to_underlying(filterz));
	return result;
}

void R_SetDefaultDrawColumnVars(draw_column_vars_t* dcvars)
{
	dcvars->x = dcvars->yl = dcvars->yh = 0;
	dcvars->iscale = dcvars->texturemid = dcvars->texheight = 0;
	dcvars->source = dcvars->prevsource = dcvars->nextsource = nullptr;
	dcvars->colormap = colormaps[0];
	dcvars->translation = nullptr;
	dcvars->edgeslope = static_cast<EdgeSlope>(0);
	dcvars->drawingmasked = 0;
	dcvars->flags = static_cast<DrawColumnFlag>(0);

	// [AR] mark weapon sprite
	dcvars->isplayersprite = false;
	dcvars->pspritepostheight = 0;

	// heretic
	dcvars->baseclip = -1;
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//

byte playernumtotrans[MAX_MAXPLAYERS];

// HERETIC_TODO: player colors
const byte player_colors[] = {0x70, 0x60, 0x40, 0x20};

void R_InitTranslationTables()
{
	int i, j;
#define MAXTRANS 3
	byte transtocolour[MAXTRANS];

	if(hexen)
	{
		int lumpnum = W_GetNumForName("trantbl0");
		translationtables = static_cast<byte*>(Z_Malloc(256 * 3 * (g_maxplayers - 1)));

		for(i = 0; i < g_maxplayers; i++)
			playernumtotrans[i] = i;

		for(i = 0; i < 3 * (g_maxplayers - 1); i++)
		{
			const byte* transLump = static_cast<const byte*>(W_LumpByNum(lumpnum + i));
			memcpy(translationtables + i * 256, transLump, 256);
		}

		return;
	}

	// killough 5/2/98:
	// Remove dependency of colormaps aligned on 256-byte boundary

	if(translationtables == nullptr) // CPhipps - allow multiple calls
		translationtables = static_cast<byte*>(Z_Malloc(256 * MAXTRANS));

	for(i = 0; i < MAXTRANS; i++) transtocolour[i] = 255;

	for(i = 0; i < g_maxplayers; i++)
	{
		byte wantcolour = player_colors[i];
		playernumtotrans[i] = 0;
		if(wantcolour != 0x70) // Not green, would like translation
			for(j = 0; j < MAXTRANS; j++)
				if(transtocolour[j] == 255)
				{
					transtocolour[j] = wantcolour;
					playernumtotrans[i] = j + 1;
					break;
				}
	}

	// translate just the 16 green colors
	for(i = 0; i < 256; i++)
		if(i >= 0x70 && i <= 0x7f)
		{
			// CPhipps - configurable player colours
			translationtables[i] = colormaps[0][(i & 0xf) + transtocolour[0]];
			translationtables[i + 256] = colormaps[0][(i & 0xf) + transtocolour[1]];
			translationtables[i + 512] = colormaps[0][(i & 0xf) + transtocolour[2]];
		}
		else // Keep all other colors as is.
			translationtables[i] = translationtables[i + 256] = translationtables[i + 512] = i;
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

void R_DrawSpan(draw_span_vars_t* dsvars)
{
	unsigned count = dsvars->x2 - dsvars->x1 + 1;
	fixed_t xfrac = dsvars->xfrac;
	fixed_t yfrac = dsvars->yfrac;
	const fixed_t xstep = dsvars->xstep;
	const fixed_t ystep = dsvars->ystep;
	const byte* source = dsvars->source;
	const byte* colormap = dsvars->colormap;
	byte* dest = drawvars.topleft + dsvars->y * drawvars.pitch + dsvars->x1;

	while(count)
	{
		const fixed_t xtemp = (xfrac >> 16) & 63;
		const fixed_t ytemp = (yfrac >> 10) & 4032;
		const fixed_t spot = xtemp | ytemp;
		xfrac += xstep;
		yfrac += ystep;
		*dest++ = colormap[source[spot]];
		count--;
	}
}

void R_InitBuffersRes()
{
	extern byte* solidcol;

	if(solidcol) Z_Free(solidcol);
	if(tempbuf) Z_Free(tempbuf);

	solidcol = static_cast<byte*>(Z_Calloc(1, SCREENWIDTH * sizeof(*solidcol)));
	tempbuf = static_cast<byte*>(Z_Calloc(1, (SCREENHEIGHT * 4) * sizeof(*tempbuf)));

	temp_x = 0;
}

//
// R_InitBuffer
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_InitBuffer(int width, int height)
{
	int i;

	drawvars.topleft = screens[0].data;
	drawvars.pitch = screens[0].pitch;

	for(i = 0; i < FUZZTABLE; i++)
		fuzzoffset[i] = fuzzoffset_org[i] * screens[0].pitch;

	if(!tallscreen)
		fuzzcellsize = (SCREENHEIGHT + 100) / 200;
	else
		fuzzcellsize = (SCREENWIDTH + 160) / 320;
}

//
// R_FillBackColor
// Fills the statusbar widescreen area
// with a color
//

void R_FillBackColor()
{
	extern patchnum_t stbarbg;
	static byte col;
	static byte col_top;
	static int prevlump = -1;
	const int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;
	const int ST_SCALED_BORDER = brdr_b.height * patches_scaley / 2;
	int lump = stbarbg.lumpnum;

	if(prevlump != lump)
	{
		const unsigned char* playpal = V_GetPlaypal();
		SDL_Color stbar_color = V_GetPatchColor(lump);
		int r = stbar_color.r;
		int g = stbar_color.g;
		int b = stbar_color.b;

		// Convert to palette and tune down saturation
		col = V_BestColor(playpal, r / 3, g / 3, b / 3);
		col_top = V_BestColor(playpal, r / 2, g / 2, b / 2);

		// If colors are the same, brighten top
		if(col_top == col)
			col_top = V_BestColor(playpal, r, g, b);

		prevlump = lump;
	}

	V_BeginMenuDraw();
	V_FillRect(1, 0, stbar_top, SCREENWIDTH, ST_SCALED_BORDER, col_top);
	V_FillRect(1, 0, stbar_top + ST_SCALED_BORDER, SCREENWIDTH, ST_SCALED_HEIGHT - ST_SCALED_BORDER, col);
	V_EndMenuDraw();
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// CPhipps - patch drawing updated

void R_FillBackScreen()
{
	int automap = automap_solid;

	if(grnrock.lumpnum == 0)
		return;

	V_BeginUIDraw();

	// e6y: wide-res
	if(ratio_multiplier != ratio_scale || wide_offsety)
	{
		int only_stbar;

		only_stbar = V_IsSoftwareMode() || automap || R_PartialView();

		if(only_stbar && ST_SCALED_OFFSETX > 0)
		{
			int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;
			int stbar_solid_bg = dsda_IntConfig(ConfigId::StsSolidBgColor);

			if(stbar_solid_bg)
			{
				R_FillBackColor();
				V_EndUIDraw();
				return;
			}

			if(V_IsOpenGLMode()) // OpenGL has no way to adjust y-offset independent from height
				V_FillFlat(grnrock.lumpnum, 1, 0, 0, SCREENWIDTH, SCREENHEIGHT, PatchTranslation::Stretch);
			else
				V_FillFlat(grnrock.lumpnum, 1, 0, stbar_top, SCREENWIDTH, ST_SCALED_HEIGHT, PatchTranslation::Stretch);

			// heretic_note: I think this looks bad, so I'm skipping it...
			if(!heretic)
			{
				// line between view and status bar
				V_FillPatch(brdr_b.lumpnum, 1, 0, stbar_top, ST_SCALED_OFFSETX, brdr_b.height, PatchTranslation::None);
				V_FillPatch(brdr_b.lumpnum, 1, SCREENWIDTH - ST_SCALED_OFFSETX, stbar_top, ST_SCALED_OFFSETX, brdr_b.height, PatchTranslation::None);
			}
		}
	}

	V_EndUIDraw();
}

//
// Copy a screen buffer.
//

static void R_CopyScreenBufferSection(int x, int y, int count)
{
	if(V_IsSoftwareMode())
		memcpy(screens[0].data + y * screens[0].pitch + x,
			screens[1].data + y * screens[1].pitch + x,
			count); // LFB copy.
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//

void R_DrawViewBorder()
{
	int i;

	if(V_IsOpenGLMode())
	{
		// proff 11/99: we don't have a backscreen in OpenGL from where we can copy this
		R_FillBackScreen();
		return;
	}

	// e6y: wide-res
	if((ratio_multiplier != ratio_scale || wide_offsety) && R_StatusBarVisible())
	{
		for(i = SCREENHEIGHT - ST_SCALED_HEIGHT; i < SCREENHEIGHT; i++)
		{
			R_CopyScreenBufferSection(0, i, ST_SCALED_OFFSETX);
			R_CopyScreenBufferSection(SCREENWIDTH - ST_SCALED_OFFSETX, i, ST_SCALED_OFFSETX);
		}
	}
}

void R_SetFuzzPos(int fp)
{
	fuzzpos = fp;
}

int R_GetFuzzPos()
{
	return fuzzpos;
}

void R_ResetFuzzCol(int height)
{
	R_ResetColumnBuffer();

	fuzzpos = (fuzzpos + (height / fuzzcellsize)) % FUZZTABLE;
}

void R_CheckFuzzCol(int x, int height)
{
	if(!(x % fuzzcellsize))
		R_ResetFuzzCol(height);
}
