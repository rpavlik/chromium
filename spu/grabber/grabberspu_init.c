/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "grabberspu.h"
#include "cr_mothership.h"
#include "cr_mem.h"

GrabberSPU grabber_spu;

static SPUFunctions grabber_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_grabber_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *
grabberSPUInit( int id, SPU *child, SPU *self,
     unsigned int context_id, unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

	grabber_spu.id = id;
	grabber_spu.has_child = 0;
	grabber_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(grabber_spu.child) );
		crSPUCopyDispatchTable( &(grabber_spu.child), &(child->dispatch_table) );
		grabber_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(grabber_spu.super) );
	crSPUCopyDispatchTable( &(grabber_spu.super), &(self->superSPU->dispatch_table) );
	grabberGatherConfiguration(&grabber_spu);

	return &grabber_functions;
}

static void
grabberSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(grabber_spu.self) );
	crSPUCopyDispatchTable( &(grabber_spu.self), self );

	grabber_spu.server = (CRServer *)(self->server);
}

static int
grabberSPUCleanup(void)
{
	if (grabber_spu.mothershipConnection != NULL) {
	    crMothershipDisconnect(grabber_spu.mothershipConnection);
	}
	if (grabber_spu.currentWindowAttributeName != NULL) {
	    crFree(grabber_spu.currentWindowAttributeName);
	}
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "grabber";
	*super = "passthrough";
	*init = grabberSPUInit;
	*self = grabberSPUSelfDispatch;
	*cleanup = grabberSPUCleanup;
	*options = grabberSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
