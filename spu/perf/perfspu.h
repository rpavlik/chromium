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
#include "cr_perf.h"

/* Check cr_applications.h for structures */

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	CRConnection *conn;
	int mothership_log;

	char hostname[100];
	char *log_filename;
	FILE *log_file;
	char token[100];
	char separator[100];

	int mode;
	
	int dump_on_swap_count;
	int dump_on_clear_count;
	int dump_on_finish;
	int dump_on_flush;

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

extern SPUNamedFunctionTable _cr_perf_table[];

extern CRConnection** crNetDump(int *num);

extern void perfspuDump( char *str );

extern void perfspuGatherConfiguration( void );

#endif /* PERF_SPU_H */
