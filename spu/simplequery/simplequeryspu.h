/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef SIMPLEQUERY_SPU_H
#define SIMPLEQUERY_SPU_H

#ifdef WINDOWS
#define SIMPLEQUERYSPU_APIENTRY __stdcall
#else
#define SIMPLEQUERYSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_glstate.h"

void simplequeryspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRContext *ctx;
} SimplequerySPU;

extern SimplequerySPU simplequery_spu;

#endif /* SIMPLEQUERY_SPU_H */
