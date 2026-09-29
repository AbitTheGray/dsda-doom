// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh, visplane stuff (floor, ceilings).
 */

#pragma once

#include <cstdint>
#include <utility>

#include "r_data.hpp"

#include "cpp/Util.hpp"

// A flat number (visplane_t::picnum, sector_t::floorsky/ceilingsky) can carry a sky tag in its top two bits.
// The rest of the number is then what the tag refers to.
enum struct SkyFlatTag : uint32_t
{
	Line = Bit<uint32_t>(31u),   // a line whose first sidedef's upper texture is the sky; the rest is the line index
	Sector = Bit<uint32_t>(30u), // a sky texture given to the sector; the rest is the texture number
	Any = Line | Sector,
};
ENUM_FLAGS_FUNC(SkyFlatTag)

// Whether `picnum` carries `tag` (with `SkyFlatTag::Any`, either tag).
[[nodiscard]]
inline constexpr bool SkyFlatHasTag(const int32_t picnum, const SkyFlatTag tag)
{
	return (static_cast<uint32_t>(picnum) & std::to_underlying(tag)) != 0u;
}

// `picnum` with `tag` removed: the line index or texture number that was tagged.
[[nodiscard]]
inline constexpr int32_t SkyFlatUntagged(const int32_t picnum, const SkyFlatTag tag)
{
	return static_cast<int32_t>(static_cast<uint32_t>(picnum) & ~std::to_underlying(tag));
}

// `value` (a line index or texture number) with `tag` added.
[[nodiscard]]
inline constexpr int32_t SkyFlatTagged(const int32_t value, const SkyFlatTag tag)
{
	return static_cast<int32_t>(static_cast<uint32_t>(value) | std::to_underlying(tag));
}

#ifdef __cplusplus
extern "C"
{
#endif

/* Visplane related. */
extern int* lastopening; // dropoff overflow

// e6y: resolution limitation is removed
extern int *floorclip, *ceilingclip; // dropoff overflow
extern fixed_t *yslope, *distscale;

void R_InitVisplanesRes();
void R_InitPlanesRes();
void R_InitPlanes();
void R_ClearPlanes();
void R_DrawPlanes();

void dsda_RefreshLinearSky();

const rpatch_t* R_HackedSkyPatch(texture_t* texture);

visplane_t* R_FindPlane(
	fixed_t height,
	int picnum,
	int lightlevel,
	int special,
	fixed_t xoffs, /* killough 2/28/98: add x-y offsets */
	fixed_t yoffs,
	angle_t rotation,
	fixed_t xscale,
	fixed_t yscale
);

visplane_t* R_CheckPlane(visplane_t* pl, int start, int stop);
visplane_t* R_DupPlane(const visplane_t* pl, int start, int stop);

#ifdef __cplusplus
}
#endif
