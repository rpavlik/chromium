/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#define DEBUG_FP_EXCEPTIONS 0
#if DEBUG_FP_EXCEPTIONS
#include <fpu_control.h>
#include <math.h>
#endif
#include "cr_rand.h"
#include "cr_spu.h"
#include "cr_mem.h"
#include "tilesortspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_tilesort_table[];
TileSortSPU tilesort_spu;

static SPUFunctions tilesort_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_tilesort_table /* THE ACTUAL FUNCTIONS */
};

#ifdef CHROMIUM_THREADSAFE
CRmutex _TileSortMutex;
CRtsd _ThreadTSD;
#endif

static SPUFunctions *
tilesortSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
#if 0
	GLint vpdims[2];
	GLint totalDims[2];
	int i, j;
#endif

	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) self;

#if DEBUG_FP_EXCEPTIONS
	{
            fpu_control_t mask;
            _FPU_GETCW(mask);
            mask &= ~(_FPU_MASK_IM | _FPU_MASK_DM | _FPU_MASK_ZM
                      | _FPU_MASK_OM | _FPU_MASK_UM);
            _FPU_SETCW(mask);
	}
#endif

	crRandSeed(id);

	crMemZero( &tilesort_spu, sizeof(TileSortSPU) );

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&_ThreadTSD, thread0);
	crInitMutex(&_TileSortMutex);
#endif

	_math_init_eval();

	tilesort_spu.id = id;
	tilesortspuGatherConfiguration( child );
	tilesortspuConnectToServers(); /* set up thread0's server connection */

	if (tilesort_spu.MTU > thread0->net[0].conn->mtu) {
		tilesort_spu.MTU = thread0->net[0].conn->mtu;

	/* get a buffer which can hold one big big opcode (counter computing
	 * against packer/pack_buffer.c) */
		tilesort_spu.buffer_size = ((((tilesort_spu.MTU - sizeof(CRMessageOpcodes) ) * 5 + 3) / 4 + 0x3) & ~0x3) + sizeof(CRMessageOpcodes);
	}
	tilesort_spu.swap = thread0->net[0].conn->swap;

	tilesortspuInitThreadPacking( thread0 );

	tilesortspuCreateFunctions();

	crStateInit();
	tilesortspuCreateDiffAPI();
	tilesortspuBucketingInit();

	tilesortspuComputeMaxViewport();

	return &tilesort_functions;
}

static void
tilesortSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(tilesort_spu.self) );
	crSPUCopyDispatchTable( &(tilesort_spu.self), self );
}

static int
tilesortSPUCleanup(void)
{
	return 1;
}

extern SPUOptions tilesortSPUOptions[];

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "tilesort";
	*super = NULL;
	*init = tilesortSPUInit;
	*self = tilesortSPUSelfDispatch;
	*cleanup = tilesortSPUCleanup;
	*options = tilesortSPUOptions;
	*flags = (SPU_HAS_PACKER|
		  SPU_IS_TERMINAL|
		  SPU_MAX_SERVERS_UNLIMITED);
	
	return 1;
}
