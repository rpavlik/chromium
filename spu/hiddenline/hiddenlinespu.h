/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef HIDDENLINE_SPU_H
#define HIDDENLINE_SPU_H

#ifdef WINDOWS
#define HIDDENLINESPU_APIENTRY __stdcall
#else
#define HIDDENLINESPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_glstate.h"
#include "cr_pack.h"

#include "state/cr_limits.h"

void hiddenlinespuCreateFunctions( void );
void hiddenlinespuGatherConfiguration( SPU *child );

typedef struct _buflist {
	void *buf;
	void *data, *opcodes;
	unsigned int num_opcodes;
	int can_reclaim;
	struct _buflist *next;
} BufList;

typedef struct {
	int id;
	int has_child;

	SPUDispatchTable self, child, super;

	CRPackBuffer pack_buffer;
	int buffer_size;
	CRContext *ctx;

	float clear_r, clear_g, clear_b;
	float poly_r, poly_g, poly_b;
	float line_r, line_g, line_b;
	float line_width;

	CRLimitsState limits;
	BufList *frame_head, *frame_tail;

	CRPackContext *packer;
} HiddenlineSPU;

extern HiddenlineSPU hiddenline_spu;

void hiddenlineProvidePackBuffer( void );
void hiddenlineReclaimPackBuffer( BufList *bl );
void hiddenlineFlush( void *arg );
void hiddenlineHuge( CROpcode opcode, void *buf );

#endif /* HIDDENLINE_SPU_H */
