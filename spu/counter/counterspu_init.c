/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "counterspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable counter_table[];
counterSPU counter_spu;

SPUFunctions counter_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	counter_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *counterSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	counter_spu.id = id;
	counter_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(counter_spu.child) );
		crSPUCopyDispatchTable( &(counter_spu.child), &(child->dispatch_table) );
		counter_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(counter_spu.super) );
	crSPUCopyDispatchTable( &(counter_spu.super), &(super->dispatch_table) );
	counterspuGatherConfiguration();

	counter_spu.v3fv = 0; /* Reset counter */

	return &counter_functions;
}

void counterSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(counter_spu.self) );
	crSPUCopyDispatchTable( &(counter_spu.self), self );
}

int counterSPUCleanup(void)
{
	return 1;
}

extern SPUOptions counterSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "counter";
	*super = "passthrough";
	*init = counterSPUInit;
	*self = counterSPUSelfDispatch;
	*cleanup = counterSPUCleanup;
	*options = counterSPUOptions;
	
	return 1;
}
