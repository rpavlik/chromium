/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "saveframespu.h"
#include <stdio.h>

extern SPUNamedFunctionTable saveframe_table[];

SPUFunctions the_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	saveframe_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	saveframe_spu.id = id;
	saveframe_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(saveframe_spu.child) );
		crSPUCopyDispatchTable( &(saveframe_spu.child), &(child->dispatch_table) );
		saveframe_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(saveframe_spu.super) );
	crSPUCopyDispatchTable( &(saveframe_spu.super), &(super->dispatch_table) );
	saveframespuGatherConfiguration();

	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(saveframe_spu.self) );
	crSPUCopyDispatchTable( &(saveframe_spu.self), self );
}

int SPUCleanup(void)
{
	return 1;
}

extern SPUOptions saveframeSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "saveframe";
	*super = "passthrough";
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	*options = saveframeSPUOptions;
	
	return 1;
}
