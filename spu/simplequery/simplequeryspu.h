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

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRContext *ctx;
} SimplequerySPU;

extern SimplequerySPU simplequery_spu;

extern SPUNamedFunctionTable _cr_simplequery_table[];

extern SPUOptions simplequerySPUOptions[];

extern void simplequeryspuGatherConfiguration( void );

#endif /* SIMPLEQUERY_SPU_H */
