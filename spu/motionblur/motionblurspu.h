/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef MOTIONBLUR_SPU_H
#define MOTIONBLUR_SPU_H

#ifdef WINDOWS
#define MOTIONBLURSPU_APIENTRY __stdcall
#else
#define MOTIONBLURSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

void motionblurspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	float accumCoef;
	GLboolean beginBlurFlag;
} MotionblurSPU;

extern MotionblurSPU motionblur_spu;

#endif /* MOTIONBLUR_SPU_H */
