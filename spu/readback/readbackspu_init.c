/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "readbackspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable readback_table[];

SPUFunctions readback_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	readback_table /* THE ACTUAL FUNCTIONS */
};

ReadbackSPU readback_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _ReadbackTSD;
#endif

SPUFunctions *readbackSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{
#if 1
	/* XXX temporary */
	extern CRContext *__currentContext;
	crDebug("readback SPU: &__currentContext = %p\n", (void *) &__currentContext);
#endif

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
	crSPUInitDispatchTable( &(readback_spu.super) );
	crSPUCopyDispatchTable( &(readback_spu.super), &(self->superSPU->dispatch_table) );
	readbackspuGatherConfiguration( &readback_spu );

	crStateInit();

	return &readback_functions;
}

void readbackSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(readback_spu.self) );
	crSPUCopyDispatchTable( &(readback_spu.self), self );

	readback_spu.server = (CRServer *)(self->server);
}

int readbackSPUCleanup(void)
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
 * This is a (temporary?) function used to test copies of the state
 * tracker.  See the progs/statecopytest/statecopytest.c program for
 * more information.
 */
void * READBACKSPU_APIENTRY readbackspu_state_test(void)
{
		extern CRStateBits *__currentBits;
		return &__currentBits;
}
