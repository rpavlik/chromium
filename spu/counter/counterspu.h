/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef COUNTER_SPU_H
#define COUNTER_SPU_H

#ifdef WINDOWS
#define COUNTERSPU_APIENTRY __stdcall
#else
#define COUNTERSPU_APIENTRY
#endif

#include "cr_spu.h"

void counterspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	GLuint v3f;
	GLuint v3fv;
	GLuint v4f;
	GLuint v4fv;
} counterSPU;

extern counterSPU counter_spu;

#endif /* COUNTER_SPU_H */
