/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef APICHANGE_SPU_H
#define APICHANGE_SPU_H

#ifdef WINDOWS
#define APICHANGESPU_APIENTRY __stdcall
#else
#define APICHANGESPU_APIENTRY
#endif

#include "cr_spu.h"

void apichangespuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	int changeFrequency;
} ApichangeSPU;

extern ApichangeSPU apichange_spu;

#endif /* APICHANGE_SPU_H */
