/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_NOPSPU_H
#define CR_NOPSPU_H

#include "cr_glstate.h"

typedef struct {
	CRContext *ctx;
} NOPSPU;

extern NOPSPU nop_spu;

#endif /* CR_NOPSPU_H */
