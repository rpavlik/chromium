/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "readbackspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_readback_table[];

SPUFunctions readback_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_readback_table /* THE ACTUAL FUNCTIONS */
};

ReadbackSPU readback_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _ReadbackTSD;
#endif

static SPUFunctions *
readbackSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{
	WindowInfo *window;
	(void) context_id;
	(void) num_contexts;

#ifdef CHROMIUM_THREADSAFE
	crDebug("Readback SPU: thread-safe");
#endif

	crMemZero(&readback_spu, sizeof(readback_spu));
	readback_spu.id = id;
	readback_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(readback_spu.child) );
		crSPUCopyDispatchTable( &(readback_spu.child), &(child->dispatch_table) );
		readback_spu.has_child = 1;
	}
	else
	{
	   /* don't override any API functions - use the Render SPU functions */
	   static SPUNamedFunctionTable empty_table[] = {
		  { NULL, NULL }
	   };
	   readback_functions.table = empty_table;
	}
	crSPUInitDispatchTable( &(readback_spu.super) );
	crSPUCopyDispatchTable( &(readback_spu.super), &(self->superSPU->dispatch_table) );
	readbackspuGatherConfiguration( &readback_spu );

	readback_spu.contextTable = crAllocHashtable();
	readback_spu.windowTable = crAllocHashtable();

	/* create my default window (window number 0) */
	window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	CRASSERT(window);
	window->index = 0;
	window->renderWindow = 0; /* default render SPU window */
	window->childWindow = 0;  /* default child SPU window */
	crHashtableAdd(readback_spu.windowTable, 0, window);

	/*crStateInit();*/
	readback_spu.gather_conn = NULL;

	return &readback_functions;
}

static void
readbackSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(readback_spu.self) );
	crSPUCopyDispatchTable( &(readback_spu.self), self );

	readback_spu.server = (CRServer *)(self->server);
}

static int
readbackSPUCleanup(void)
{
	return 1;
}

extern SPUOptions readbackSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "readback";
	*super = "render";
	*init = readbackSPUInit;
	*self = readbackSPUSelfDispatch;
	*cleanup = readbackSPUCleanup;
	*options = readbackSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}




/*
 * This is a function used to test copies of the state
 * tracker.  See the progs/statecopytest/statecopytest.c program for
 * more information.
 *
 * NOTE: if building on Windows remove commented out function in
 *       readbackspu.def
 */
#if 0
void * READBACKSPU_APIENTRY readbackspu_state_test(void)
{
		extern CRStateBits *__currentBits;
		return &__currentBits;
}
#endif
