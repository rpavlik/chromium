/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "templatespu.h"

TemplateSPU template_spu;

static SPUFunctions template_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_template_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *
templateSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{

	(void) self;
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
	crSPUCopyDispatchTable( &(template_spu.super), &(self->superSPU->dispatch_table) );
	templatespuGatherConfiguration();

	return &template_functions;
}

static void
templateSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(template_spu.self) );
	crSPUCopyDispatchTable( &(template_spu.self), self );

	template_spu.server = (CRServer *)(self->server);
}

static int
templateSPUCleanup(void)
{
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "template";
	*super = NULL;
	*init = templateSPUInit;
	*self = templateSPUSelfDispatch;
	*cleanup = templateSPUCleanup;
	*options = templateSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
