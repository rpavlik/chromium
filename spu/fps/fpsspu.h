/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef FPS_SPU_H
#define FPS_SPU_H

#ifdef WINDOWS
#define FPSSPU_APIENTRY __stdcall
#else
#define FPSSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_timer.h"

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	CRTimer *timer;
} FpsSPU;

extern FpsSPU fps_spu;

extern SPUNamedFunctionTable _cr_fps_table[];

extern SPUOptions fpsSPUOptions[];

extern void fpsspuGatherConfiguration( void );

#endif /* FPS_SPU_H */
