/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_glstate.h"
#include "simplequeryspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable simplequery_table[];
SimplequerySPU simplequery_spu;

SPUFunctions simplequery_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	simplequery_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *simplequerySPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	simplequery_spu.id = id;
	simplequery_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(simplequery_spu.child) );
		crSPUCopyDispatchTable( &(simplequery_spu.child), &(child->dispatch_table) );
		simplequery_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(simplequery_spu.super) );
	crSPUCopyDispatchTable( &(simplequery_spu.super), &(super->dispatch_table) );
	simplequeryspuGatherConfiguration();

	simplequery_spu.ctx = crStateCreateContext( NULL );
	crStateMakeCurrent( simplequery_spu.ctx );

	return &simplequery_functions;
}

void simplequerySPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(simplequery_spu.self) );
	crSPUCopyDispatchTable( &(simplequery_spu.self), self );
}

int simplequerySPUCleanup(void)
{
	return 1;
}

extern SPUOptions simplequerySPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "simplequery";
	*super = "sqpassthrough";
	*init = simplequerySPUInit;
	*self = simplequerySPUSelfDispatch;
	*cleanup = simplequerySPUCleanup;
	*options = simplequerySPUOptions;

	return 1;
}
