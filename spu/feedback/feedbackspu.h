/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef FEEDBACK_SPU_H
#define FEEDBACK_SPU_H

#ifdef WINDOWS
#define FEEDBACKSPU_APIENTRY __stdcall
#else
#define FEEDBACKSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_timer.h"
#include "cr_applications.h"

void feedbackspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	int render_mode;
} feedbackSPU;

extern feedbackSPU feedback_spu;

#endif /* FEEDBACK_SPU_H */
