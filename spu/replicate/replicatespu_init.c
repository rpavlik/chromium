/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_glstate.h"
#include "replicatespu.h"


extern SPUNamedFunctionTable _cr_replicate_table[];

SPUFunctions replicate_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_replicate_table /* THE ACTUAL FUNCTIONS */
};

ReplicateSPU replicate_spu;

#ifdef CHROMIUM_THREADSAFE_notyet
CRtsd _ReplicateTSD;
CRmutex _ReplicateMutex;
#endif

static SPUFunctions *
replicateSPUInit( int id, SPU *child, SPU *self,
						 unsigned int context_id,
						 unsigned int num_contexts )
{
	ThreadInfo *thread;

	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) self;

#ifdef CHROMIUM_THREADSAFE_notyet
	crInitMutex(&_ReplicateMutex);
#endif

	replicate_spu.id = id;
	replicate_spu.glx_display = NULL;
	replicate_spu.NOP = 1; /* a nice hack to save CPU!! */
	replicate_spu.StartedVnc = 0;

	replicatespuGatherConfiguration( child );

	/* Init window/context hash tables */
	replicate_spu.windowTable = crAllocHashtable();
	replicate_spu.contextTable = crAllocHashtable();
	replicate_spu.contextList = crAllocList();

	/* This connects to the server, sets up the packer, etc. */
	thread = replicatespuNewThread( crThreadID() );

	CRASSERT( thread == &(replicate_spu.thread[0]) );

	replicate_spu.rserver[0].conn = thread->server.conn;
	replicate_spu.rserver[0].buffer_size = thread->server.buffer_size;
	replicate_spu.rserver[0].name = thread->server.name;

	replicatespuCreateFunctions();
	crStateInit();
	replicatespuCreateDiffAPI();
	replicatespuCreateStateAPI();

	return &replicate_functions;
}

static void
replicateSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(replicate_spu.self) );
	crSPUCopyDispatchTable( &(replicate_spu.self), self );
}


static int
replicateSPUCleanup(void)
{
	int i;

	replicatespuDestroyAllWindowsAndContexts();

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {
		if (IS_CONNECTED(replicate_spu.rserver[i].conn)) {
			crNetDisconnect(replicate_spu.rserver[i].conn);
		}
		replicate_spu.rserver[i].conn = NULL;
	}

	return 1;
}


extern SPUOptions replicateSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "replicate";
	*super = NULL;
	*init = replicateSPUInit;
	*self = replicateSPUSelfDispatch;
	*cleanup = replicateSPUCleanup;
	*options = replicateSPUOptions;
	*flags = (SPU_HAS_PACKER|SPU_IS_TERMINAL|SPU_MAX_SERVERS_ONE);

	return 1;
}
