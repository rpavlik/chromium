/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_glstate.h"
#include "packspu.h"
#include "cr_packfunctions.h"
#include <stdio.h>

extern SPUNamedFunctionTable pack_table[];
PackSPU pack_spu;

SPUFunctions pack_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	pack_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *packSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;

	pack_spu.id = id;
	packspuGatherConfiguration( child );
	packspuConnectToServer();

	crPackInit( pack_spu.server.conn->swap );
	crPackInitBuffer( &(pack_spu.buffer), crNetAlloc( pack_spu.server.conn ), pack_spu.server.buffer_size, 0 );
	crPackSetBuffer( &pack_spu.buffer );
	crPackFlushFunc( packspuFlush );
	crPackSendHugeFunc( packspuHuge );

	pack_spu.swap = pack_spu.server.conn->swap;
	packspuCreateFunctions();
	crStateInit();

	/* GL Limits were computed in packspuGatherConfiguration() above */
	pack_spu.ctx = crStateCreateContext( &pack_spu.limits );
	crStateMakeCurrent( pack_spu.ctx );
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

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "pack";
	*super = NULL;
	*init = packSPUInit;
	*self = packSPUSelfDispatch;
	*cleanup = packSPUCleanup;
	
	return 1;
}
