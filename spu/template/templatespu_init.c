/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "templatespu.h"
#include <stdio.h>

extern SPUNamedFunctionTable template_table[];

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	template_table // THE ACTUAL FUNCTIONS
};

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	template_spu.id = id;
	template_spu.has_child = 0;
	if (child)
	{
		crSPUCopyDispatchTable( &(template_spu.child), &(child->dispatch_table) );
		template_spu.has_child = 1;
	}
	crSPUCopyDispatchTable( &(template_spu.super), &(super->dispatch_table) );
	templatespuGatherConfiguration();

	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

int SPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "template";
	*super = NULL;
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	
	return 1;
}
