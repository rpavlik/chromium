/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "fpsspu.h"
#include <stdio.h>

FpsSPU fps_spu;

static SPUFunctions fps_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_fps_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *fpsSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

	fps_spu.id = id;
	fps_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(fps_spu.child) );
		crSPUCopyDispatchTable( &(fps_spu.child), &(child->dispatch_table) );
		fps_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(fps_spu.super) );
	crSPUCopyDispatchTable( &(fps_spu.super), &(self->superSPU->dispatch_table) );
	fpsspuGatherConfiguration();

	fps_spu.timer = crTimerNewTimer();
	crStartTimer( fps_spu.timer );

	return &fps_functions;
}

static void fpsSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(fps_spu.self) );
	crSPUCopyDispatchTable( &(fps_spu.self), self );
}

static int fpsSPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "fps";
	*super = "passthrough";
	*init = fpsSPUInit;
	*self = fpsSPUSelfDispatch;
	*cleanup = fpsSPUCleanup;
	*options = fpsSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);

	return 1;
}
