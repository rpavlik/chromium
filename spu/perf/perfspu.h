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
#include "cr_timer.h"
#include "cr_applications.h"

void perfspuGatherConfiguration( void );

/* Check cr_applications.h for structures */

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	char hostname[100];
	FILE *log_file;
	char token[100];
	char separator[100];

	int mode;
	
	int dump_on_swap_count;
	int dump_on_clear_count;

	int total_frames;
	int frame_counter;
	int clear_counter;

	PerfData framestats;
	PerfData old_framestats;
 
	PerfData timerstats;
	PerfData old_timerstats;
	CRTimer *timer;
	float timer_event;
} perfSPU;

extern perfSPU perf_spu;

#endif /* PERF_SPU_H */
