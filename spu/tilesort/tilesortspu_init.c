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
	
fprintf(stderr,"tilesortSPUInit call crSetTSD\n");
	crSetTSD(&_ThreadTSD, thread0);
	crInitMutex(&_TileSortMutex);
#endif

	thread0->state_server_index = -1;	 /* one-time init for thread */

	tilesortspuInitEvaluators();

	/* Init window, context hash tables */
	tilesort_spu.windowTable = crAllocHashtable();
	tilesort_spu.contextTable = crAllocHashtable();
	tilesort_spu.listTable = crAllocHashtable();

	tilesort_spu.id = id;
	tilesort_spu.glassesType = RED_BLUE;
	tilesortspuSetAnaglyphMask(&tilesort_spu);

	tilesortspuGatherConfiguration( child );
	tilesortspuConnectToServers(); /* set up thread0's server connection */

	tilesort_spu.geom_buffer_size = tilesort_spu.buffer_size;

	/* geom_buffer_mtu must fit in data's part of our buffers */
	tilesort_spu.geom_buffer_mtu = crPackNumData(tilesort_spu.buffer_size)
	/* 24 is the size of the bounds info packet
	 * END_FLUFF is the size of data of the extra End opcode if needed
	 * 4 since BoundsInfo opcode may take a whole 4 bytes
	 * and 4 to let room for extra End's opcode, if needed
	 */
			- (24+END_FLUFF+4+4);

	/* the geometry must also fit in the mtu */
	if (tilesort_spu.geom_buffer_mtu >
		tilesort_spu.MTU - sizeof(CRMessageOpcodes) - (24+END_FLUFF+4+4))
		tilesort_spu.geom_buffer_mtu =
			tilesort_spu.MTU - sizeof(CRMessageOpcodes) - (24+END_FLUFF+4+4);

	tilesort_spu.swap = thread0->netServer[0].conn->swap;
	tilesort_spu.replay = 0;

	tilesortspuInitThreadPacking( thread0 );

	tilesortspuCreateFunctions();

	crStateInit();
	tilesortspuCreateDiffAPI();

        /* special dispatch tables for display lists */
	if (tilesort_spu.listTrack) {
		crMemZero((void *)&tilesort_spu.packerDispatch, sizeof tilesort_spu.packerDispatch);
        	crSPUInitDispatchTable(&tilesort_spu.packerDispatch);
        	tilesortspuLoadPackTable(&tilesort_spu.packerDispatch);

        	crSPUInitDispatchTable(&tilesort_spu.stateDispatch);
        	tilesortspuLoadStateTable(&tilesort_spu.stateDispatch);
	}

	if (tilesort_spu.useDMX) {
		/* load OpenGL */
		int n = crLoadOpenGL( &tilesort_spu.ws, NULL);
		if (!n) {
			crWarning("Tilesort SPU: Unable to load OpenGL, disabling DMX");
			tilesort_spu.useDMX = 0;
		}
	}

	return &tilesort_functions;
}

static void
tilesortSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(tilesort_spu.self) );
	crSPUCopyDispatchTable( &(tilesort_spu.self), self );
}

static void freeContextCallback(void *data)
{
	 ContextInfo *contextInfo = (ContextInfo *) data;
	 crFree(contextInfo->server);
	 crStateDestroyContext(contextInfo->State);
	 crFree(contextInfo);
}

static void freeWindowCallback(void *data)
{
	 WindowInfo *winInfo = (WindowInfo *) data;
	 tilesortspuFreeWindowInfo(winInfo);
}

static void freeListCallback(void *data)
{
	 int *listSent = (int *) data;
	 crFree(listSent);
}

static int
tilesortSPUCleanup(void)
{
	crFreeHashtable(tilesort_spu.windowTable, freeWindowCallback);
	tilesort_spu.windowTable = NULL;
	crFreeHashtable(tilesort_spu.contextTable, freeContextCallback);
	tilesort_spu.contextTable = NULL;
	crFreeHashtable(tilesort_spu.listTable, freeListCallback);
	tilesort_spu.listTable = NULL;

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
