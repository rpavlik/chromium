/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef READBACK_SPU_H
#define READBACK_SPU_H

#ifdef WINDOWS
#define READBACKSPU_APIENTRY __stdcall
#else
#define READBACKSPU_APIENTRY
#endif

#include "cr_spu.h"

void readbackspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	int extract_depth;
	int local_visualization;
} ReadbackSPU;

#define READBACK_BARRIER 1

extern ReadbackSPU readback_spu;

#endif /* READBACK_SPU_H */
