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

/* This is very fine grained.... Don't know whether we want this
 * kind of detail yet. But we maintain a linked list of rendering
 * modes and the calls made inbetween the Begin/End calls.
 * 29/5/2002 - it's getting a little out of date this code....
 */
#define CR_PERF_PER_INSTANCE_COUNTERS 0

void perfspuGatherConfiguration( void );

typedef struct {
	int 	count;

	int 	v2d, v2f, v2i, v2s;
	int 	v2dv, v2fv, v2iv, v2sv;
	int 	v3d, v3f, v3i, v3s;
	int 	v3dv, v3fv, v3iv, v3sv;
	int 	v4d, v4f, v4i, v4s;
	int 	v4dv, v4fv, v4iv, v4sv;

	int	ipoints;	/* Interpreted points */
	int	ilines; 	/* Interpreted lines */
	int	itris;		/* Interpreted tris */
	int	iquads; 	/* Interpreted quads */
	int	ipolygons; 	/* Interpreted polygons */
} PerfVertex;

typedef struct __perf_prim {
	PerfVertex points;
	PerfVertex lines;
	PerfVertex lineloop;
	PerfVertex linestrip;
	PerfVertex triangles;
	PerfVertex tristrip;
	PerfVertex trifan;
	PerfVertex quads;
	PerfVertex quadstrip;
	PerfVertex polygon;
} PerfPrim;

typedef struct {
	/* performance counters */
	int 	beginend, mode;
	int	drawpix, readpix;
	int	swapbuffers;
	int 	v2d, v2f, v2i, v2s;
	int 	v3d, v3f, v3i, v3s;
	int 	v4d, v4f, v4i, v4s;
} PerfCounters;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	char hostname[100];
	FILE *log_file;
	char token[100];
	char separator[100];

	int mode;
	
	int swapdumpcount;
	int cleardumpcount;

	int frame_counter;
	int clear_counter;
	int swap_counter;

	int old_draw_pixels;
	int draw_pixels;
	int old_read_pixels;
	int read_pixels;

	PerfVertex snapshot;

	PerfPrim framestats;
	PerfPrim old_framestats;
 
	PerfPrim timerstats;
	PerfPrim old_timerstats;
	CRTimer *timer;
	float timer_event;

	PerfVertex *cur_framestats;
	PerfVertex *cur_timerstats;
} perfSPU;

extern perfSPU perf_spu;

#endif /* PERF_SPU_H */
