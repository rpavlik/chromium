/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "fpsspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable fps_table[];
FpsSPU fps_spu;

SPUFunctions fps_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	fps_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *fpsSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
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
	crSPUCopyDispatchTable( &(fps_spu.super), &(super->dispatch_table) );
	fpsspuGatherConfiguration();

	fps_spu.timer = crTimerNewTimer();
	crStartTimer( fps_spu.timer );

	return &fps_functions;
}

void fpsSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(fps_spu.self) );
	crSPUCopyDispatchTable( &(fps_spu.self), self );
}

int fpsSPUCleanup(void)
{
	return 1;
}

extern SPUOptions fpsSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "fps";
	*super = "passthrough";
	*init = fpsSPUInit;
	*self = fpsSPUSelfDispatch;
	*cleanup = fpsSPUCleanup;
	*options = fpsSPUOptions;

	return 1;
}
