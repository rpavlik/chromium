/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACKER_H
#define CR_PACKER_H

#ifdef WINDOWS
#ifdef DLLDATA
#undef DLLDATA
#endif
#define DLLDATA __declspec(dllexport)
#endif

#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "packer_extensions.h"

#define SWAP8(x) x
#define SWAP16(x) (short) ((x & 0x00FF << 8) | (x & 0xFF00 >> 8))
#define SWAP32(x) ((x & 0x000000FF << 24) | \
		               (x & 0x0000FF00 << 8)  | \
									 (x & 0x00FF0000 >> 8)  | \
									 (x & 0xFF000000 >> 24))
#define SWAPFLOAT(x) (float) ( (*((int *) &x) & 0x000000FF << 24) | \
                       (*((int *) &x) & 0x0000FF00 << 8) | \
                       (*((int *) &x) & 0x00FF0000 >> 8) | \
                       (*((int *) &x) & 0xFF000000 >> 24))

#endif /* CR_PACKER_H */
