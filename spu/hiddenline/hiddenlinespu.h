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

#include "cr_bufpool.h"
#include "cr_spu.h"
#include "cr_glstate.h"
#include "cr_hash.h"
#include "cr_pack.h"
#include "cr_threads.h"

#include "state/cr_limits.h"


typedef struct _buflist {
	void *buf;
	void *data, *opcodes;
	unsigned int num_opcodes;
	int can_reclaim;
	struct _buflist *next;
} BufList;


/*
 * Per-context state.
 * Note: in principle, we could share one bufpool among all contexts.
 * However, the bufpool functions aren't reentrant (thread-safe) so
 * we'd have to put mutexes around them.  Having a per-context bufpool
 * is just simpler.
 */
typedef struct {
	CRPackContext *packer;
	CRPackBuffer pack_buffer;
	CRContext *ctx;  /* state tracker */
	int super_context;  /* returned by super.CreateContext() */
	GLcolorf clear_color;
	BufList *frame_head, *frame_tail;
	CRBufferPool bufpool;
} ContextInfo;

typedef struct {
	int id;
	int has_child;

	SPUDispatchTable self, child, super;

	/* config options */
	int buffer_size;
	float poly_r, poly_g, poly_b;
	float line_r, line_g, line_b;
	float line_width;
	int single_clear;

	CRLimitsState limits;

	CRHashTable *contextTable;
#ifndef CHROMIUM_THREADSAFE
	ContextInfo *currentContext;
#endif
	CRmutex mutex;
} HiddenlineSPU;

extern HiddenlineSPU hiddenline_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _HiddenlineTSD;
#define GET_CONTEXT(C)  ContextInfo *C = (ContextInfo *) crGetTSD(&_HiddenlineTSD)
#else
#define GET_CONTEXT(C)  ContextInfo *C = (hiddenline_spu.currentContext)
#endif


void hiddenlinespuCreateFunctions( void );
void hiddenlinespuGatherConfiguration( SPU *child );

void hiddenlineProvidePackBuffer( void );
void hiddenlineReclaimPackBuffer( BufList *bl );
void hiddenlineFlush( void *arg );
void hiddenlineHuge( CROpcode opcode, void *buf );

#endif /* HIDDENLINE_SPU_H */
