/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_spu.h"
#include "cr_glstate.h"
#include "packspu.h"
#include "cr_packfunctions.h"
#include <stdio.h>

extern SPUNamedFunctionTable pack_table[];

SPUFunctions pack_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	pack_table /* THE ACTUAL FUNCTIONS */
};

PackSPU pack_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _PackTSD;
CRmutex _PackMutex;
#endif

SPUFunctions *packSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	ThreadInfo *thread;

	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;

#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&_PackMutex);
#endif

	pack_spu.id = id;

	packspuGatherConfiguration( child );

	/* This connects to the server, sets up the packer, etc. */
	thread = packspuNewThread( crThreadID() );
	CRASSERT( thread == &(pack_spu.thread[0]) );

	packspuCreateFunctions();
	crStateInit();

	return &pack_functions;
}

void packSPUSelfDispatch(SPUDispatchTable *self)
{
	(void)self;
}

int packSPUCleanup(void)
{
	return 1;
}

extern SPUOptions packSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "pack";
	*super = NULL;
	*init = packSPUInit;
	*self = packSPUSelfDispatch;
	*cleanup = packSPUCleanup;
	*options = packSPUOptions;
	*flags = (SPU_HAS_PACKER|SPU_IS_TERMINAL|SPU_MAX_SERVERS_ONE);

	return 1;
}
