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
#include "cr_error.h"

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	CRTimer *timer;

	unsigned long total_frames;
	float first_swap_time;
	int report_frames;	/* Give a report at least every N frames */
	float report_seconds;	/* Give a report at least every N seconds */
	int report_at_end;	/* Give an average report at the end of the run */
} FpsSPU;

extern FpsSPU fps_spu;

extern SPUNamedFunctionTable _cr_fps_table[];

extern SPUOptions fpsSPUOptions[];

extern void fpsspuGatherConfiguration( void );


#endif /* FPS_SPU_H */
