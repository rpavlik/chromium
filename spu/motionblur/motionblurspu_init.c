/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "motionblurspu.h"


MotionblurSPU motionblur_spu;

SPUFunctions motionblur_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_motionblur_table /* THE ACTUAL FUNCTIONS */
};


static SPUFunctions *
motionblurSPUInit( int id, SPU *child, SPU *self,
									 unsigned int context_id,
									 unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

	motionblur_spu.id = id;
	motionblur_spu.has_child = 0;
	motionblur_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(motionblur_spu.child) );
		crSPUCopyDispatchTable( &(motionblur_spu.child), &(child->dispatch_table) );
		motionblur_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(motionblur_spu.super) );
	crSPUCopyDispatchTable( &(motionblur_spu.super), &(self->superSPU->dispatch_table) );
	motionblurspuGatherConfiguration();

	return &motionblur_functions;
}


static void
motionblurSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(motionblur_spu.self) );
	crSPUCopyDispatchTable( &(motionblur_spu.self), self );

	motionblur_spu.server = (CRServer *)(self->server);
}


static int
motionblurSPUCleanup(void)
{
	return 1;
}


int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "motionblur";
	*super = "passthrough";
	*init = motionblurSPUInit;
	*self = motionblurSPUSelfDispatch;
	*cleanup = motionblurSPUCleanup;
	*options = motionblurSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
