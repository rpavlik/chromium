/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef WET_SPU_H
#define WET_SPU_H

#ifdef WINDOWS
#define WETSPU_APIENTRY __stdcall
#else
#define WETSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

void wetspuGatherConfiguration( void );

typedef struct _wsr {
	int x,y;
	int drop_frame;
	struct _wsr *next;
} WetSPURaindrop;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	int mesh_dice;
	int density, raininess;
	float ripple_freq, ripple_scale;
	float time_scale;
	int mesh_width, mesh_height;
	float ior;

	int tex_id;

	int frame_counter;

	float **mesh_displacement;
	WetSPURaindrop *drops;
} WetSPU;

extern WetSPU wet_spu;

#endif /* WET_SPU_H */
