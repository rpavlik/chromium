/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "templatespu.h"
#include <stdio.h>

extern SPUNamedFunctionTable template_table[];
TemplateSPU template_spu;

SPUFunctions template_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	template_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *templateSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	template_spu.id = id;
	template_spu.has_child = 0;
	template_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(template_spu.child) );
		crSPUCopyDispatchTable( &(template_spu.child), &(child->dispatch_table) );
		template_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(template_spu.super) );
	crSPUCopyDispatchTable( &(template_spu.super), &(super->dispatch_table) );
	templatespuGatherConfiguration();

	return &template_functions;
}

void templateSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(template_spu.self) );
	crSPUCopyDispatchTable( &(template_spu.self), self );

	template_spu.server = (CRServer *)(self->server);
}

int templateSPUCleanup(void)
{
	return 1;
}

extern SPUOptions templateSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "template";
	*super = NULL;
	*init = templateSPUInit;
	*self = templateSPUSelfDispatch;
	*cleanup = templateSPUCleanup;
	*options = templateSPUOptions;
	
	return 1;
}
