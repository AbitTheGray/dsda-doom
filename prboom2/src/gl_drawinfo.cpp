// SPDX-License-Identifier: GPL-2.0-or-later

/* memory manager for GL data
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include "doomtype.hpp"
#include "gl_intern.hpp"
#include "lprintf.hpp"

GLDrawInfo gld_drawinfo;

//
// gld_FreeDrawInfo
//
void gld_FreeDrawInfo()
{
	int i;

	for(i = 0; i < gld_drawinfo.maxsize; i++)
	{
		if(gld_drawinfo.data[i].data)
		{
			Z_Free(gld_drawinfo.data[i].data);
			gld_drawinfo.data[i].data = nullptr;
		}
	}
	Z_Free(gld_drawinfo.data);
	gld_drawinfo.data = nullptr;

	for(i = 0; i < std::to_underlying(GLDrawItemType::Types); i++)
	{
		if(gld_drawinfo.items[i])
		{
			Z_Free(gld_drawinfo.items[i]);
			gld_drawinfo.items[i] = nullptr;
		}
	}

	memset(&gld_drawinfo, 0, sizeof(GLDrawInfo));
}

//
// gld_ResetDrawInfo
//
// Should be used between frames (in gld_StartDrawScene)
//
void gld_ResetDrawInfo()
{
	int i;

	for(i = 0; i < gld_drawinfo.maxsize; i++)
	{
		gld_drawinfo.data[i].size = 0;
	}
	gld_drawinfo.size = 0;

	for(i = 0; i < std::to_underlying(GLDrawItemType::Types); i++)
	{
		gld_drawinfo.num_items[i] = 0;
	}
}

//
// gld_AddDrawRange
//
static void gld_AddDrawRange(int size)
{
	gld_drawinfo.maxsize++;
	gld_drawinfo.data = static_cast<decltype(gld_drawinfo.data)>(Z_Realloc(gld_drawinfo.data,
		gld_drawinfo.maxsize * sizeof(gld_drawinfo.data[0])));

	gld_drawinfo.data[gld_drawinfo.size].maxsize = size;
	gld_drawinfo.data[gld_drawinfo.size].data = static_cast<decltype(gld_drawinfo.data[gld_drawinfo.size].data)>(Z_Malloc(size));
	gld_drawinfo.data[gld_drawinfo.size].size = 0;
}

//
// gld_AddDrawItem
//
#define NEWSIZE (MAX(64 * 1024, itemsize))
#define SIZEOF8(type) ((sizeof(type)+7)&~7)
void gld_AddDrawItem(GLDrawItemType itemtype, void* itemdata)
{
	int itemsize = 0;
	byte* item_p = nullptr;

	static int itemsizes[std::to_underlying(GLDrawItemType::Types)] = {
		0,
		SIZEOF8(GLWall), SIZEOF8(GLWall), SIZEOF8(GLWall), SIZEOF8(GLWall), SIZEOF8(GLWall),
		SIZEOF8(GLWall), SIZEOF8(GLWall),
		SIZEOF8(GLFlat), SIZEOF8(GLFlat),
		SIZEOF8(GLFlat), SIZEOF8(GLFlat),
		SIZEOF8(GLSprite), SIZEOF8(GLSprite), SIZEOF8(GLSprite),
		SIZEOF8(GLShadow),
		SIZEOF8(GLHealthBar)
	};

	itemsize = itemsizes[std::to_underlying(itemtype)];
	if(itemsize == 0)
	{
		I_Error("gld_AddDrawItem: unknown GLDrawItemType %d", itemtype);
	}

	if(gld_drawinfo.maxsize == 0)
	{
		gld_AddDrawRange(NEWSIZE);
	}

	if(gld_drawinfo.data[gld_drawinfo.size].size + itemsize >=
		gld_drawinfo.data[gld_drawinfo.size].maxsize)
	{
		gld_drawinfo.size++;
		if(gld_drawinfo.size >= gld_drawinfo.maxsize)
		{
			gld_AddDrawRange(NEWSIZE);
		}
	}

	item_p = gld_drawinfo.data[gld_drawinfo.size].data +
		gld_drawinfo.data[gld_drawinfo.size].size;

	memcpy(item_p, itemdata, itemsize);

	gld_drawinfo.data[gld_drawinfo.size].size += itemsize;

	if(gld_drawinfo.num_items[std::to_underlying(itemtype)] >= gld_drawinfo.max_items[std::to_underlying(itemtype)])
	{
		gld_drawinfo.max_items[std::to_underlying(itemtype)] += 64;
		gld_drawinfo.items[std::to_underlying(itemtype)] = static_cast<GLDrawItem*>(Z_Realloc(
			gld_drawinfo.items[std::to_underlying(itemtype)],
			gld_drawinfo.max_items[std::to_underlying(itemtype)] * sizeof(gld_drawinfo.items[0][0])));
	}

	gld_drawinfo.items[std::to_underlying(itemtype)][gld_drawinfo.num_items[std::to_underlying(itemtype)]].item.item = item_p;
	gld_drawinfo.num_items[std::to_underlying(itemtype)]++;
}
#undef SIZEOF8
#undef NEWSIZE
