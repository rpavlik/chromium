/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_NOPSPU_H
#define CR_NOPSPU_H

#include "cr_glstate.h"
#include "cr_spu.h"

typedef struct {
	int id;

	unsigned int return_ids;

	CRContext *ctx;
} NOPSPU;

extern NOPSPU nop_spu;
extern void nopspuGatherConfiguration(void);
extern SPUOptions nopSPUOptions[];

#endif /* CR_NOPSPU_H */
