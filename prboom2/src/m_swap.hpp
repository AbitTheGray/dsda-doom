// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Endianess handling, swapping 16bit and 32bit.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* CPhipps - now the endianness handling, converting input or output to/from
 * the machine's endianness to that wanted for this type of I/O
 *
 * To find our own endianness, use config.h
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Endianess handling. */

/* cph - First the macros to do the actual byte swapping */

/* leban
 * rather than continue the confusing tradition of redefining the
 * stardard macro, we now present the doom_ntoh and doom_hton macros....
 * might as well use the xdoom macros.
 */

/* Try to use superfast macros on systems that support them */
#ifdef HAVE_ASM_BYTEORDER_H
#include <asm/byteorder.h>
#ifdef __arch__swab16
#define doom_swap_s  (signed short)__arch__swab16
#endif
#ifdef __arch__swab32
#define doom_swap_l  (signed long)__arch__swab32
#endif
#endif /* HAVE_ASM_BYTEORDER_H */

#ifdef HAVE_LIBKERN_OSBYTEORDER_H
#include <libkern/OSByteOrder.h>

#define doom_swap_s (short)OSSwapInt16
#define doom_swap_l (long)OSSwapInt32
#endif

#ifndef doom_swap_l
#define doom_swap_l(x) \
        ((long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                             (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                             (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                             (((unsigned long int)(x) & 0xff000000U) >> 24)))
#endif

#ifndef doom_swap_s
#define doom_swap_s(x) \
        ((short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                              (((unsigned short int)(x) & 0xff00) >> 8)))
#endif

/* Macros are named doom_XtoYT, where
 * X is thing to convert from, Y is thing to convert to, chosen from
 * n for network, h for host (i.e our machine's), w for WAD (Doom data files)
 * and T is the type, l or s for long or short
 *
 * CPhipps - all WADs and network packets will be little endian for now
 * Use separate macros so network could be converted to big-endian later.
 */

#ifdef WORDS_BIGENDIAN

#define doom_wtohl(x) doom_swap_l(x)
#define doom_htowl(x) doom_swap_l(x)
#define doom_wtohs(x) doom_swap_s(x)
#define doom_htows(x) doom_swap_s(x)

#define doom_ntohl(x) doom_swap_l(x)
#define doom_htonl(x) doom_swap_l(x)
#define doom_ntohs(x) doom_swap_s(x)
#define doom_htons(x) doom_swap_s(x)

#else

#define doom_wtohl(x) (long int)(x)
#define doom_htowl(x) (long int)(x)
#define doom_wtohs(x) (short int)(x)
#define doom_htows(x) (short int)(x)

#define doom_ntohl(x) (long int)(x)
#define doom_htonl(x) (long int)(x)
#define doom_ntohs(x) (short int)(x)
#define doom_htons(x) (short int)(x)

#endif

/* CPhipps - Boom's old LONG and SHORT endianness macros are for WAD stuff */

#define LittleLong(x) doom_wtohl(x)
#define LittleShort(x) doom_htows(x)

#ifdef __cplusplus
}
#endif
