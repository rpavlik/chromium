/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "apichangespu.h"
#include <stdio.h>

extern SPUNamedFunctionTable apichange_table[];

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	apichange_table // THE ACTUAL FUNCTIONS
};

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	apichange_spu.id = id;
	apichange_spu.has_child = 0;
	if (child)
	{
		crSPUCopyDispatchTable( &(apichange_spu.child), &(child->dispatch_table) );
		apichange_spu.has_child = 1;
	}
	crSPUCopyDispatchTable( &(apichange_spu.super), &(super->dispatch_table) );
	apichangespuGatherConfiguration();

	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUCopyDispatchTable( &(apichange_spu.self), self );
}

int SPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "apichange";
	*super = "passthroughspu";
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	
	return 1;
}
