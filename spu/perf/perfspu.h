/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef PERF_SPU_H
#define PERF_SPU_H

#ifdef WINDOWS
#define PERFSPU_APIENTRY __stdcall
#else
#define PERFSPU_APIENTRY
#endif

#include "cr_spu.h"

void perfspuGatherConfiguration( void );

/* Per instance performance counters */
typedef struct {
	int 	begin, end;
	int	drawpix, readpix;
	int	swapbuf;
	int 	v2d, v2f, v2i, v2s;
	int	v3d, v3f, v3i, v3s;
	int	v4d, v4f, v4i, v4s;
} perf;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	int log_file;
} perfSPU;

extern perfSPU perf_spu;

#endif /* PERF_SPU_H */
