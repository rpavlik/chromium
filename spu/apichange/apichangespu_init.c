/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "apichangespu.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_apichange_table[];

SPUFunctions apichange_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_apichange_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *apichangeSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

	apichange_spu.id = id;
	apichange_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(apichange_spu.child) );
		crSPUCopyDispatchTable( &(apichange_spu.child), &(child->dispatch_table) );
		apichange_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(apichange_spu.super) );
	crSPUCopyDispatchTable( &(apichange_spu.super), &(self->superSPU->dispatch_table) );
	apichangespuGatherConfiguration();

	return &apichange_functions;
}

void apichangeSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(apichange_spu.self) );
	crSPUCopyDispatchTable( &(apichange_spu.self), self );
}

int apichangeSPUCleanup(void)
{
	return 1;
}

extern SPUOptions apichangeSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "apichange";
	*super = "passthrough";
	*init = apichangeSPUInit;
	*self = apichangeSPUSelfDispatch;
	*cleanup = apichangeSPUCleanup;
	*options = apichangeSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
