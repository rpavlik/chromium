/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "apihistogramspu.h"

/** Apihistogram SPU descriptor */ 
ApihistogramSPU apihistogram_spu;

/** SPU functions */
static SPUFunctions apihistogram_functions = {
	NULL, /**< CHILD COPY */
	NULL, /**< DATA */
	_cr_apihistogram_table /**< THE ACTUAL FUNCTIONS - pointer to NamedFunction table */
};

/**
 * Apihistogram spu init function
 * \param id
 * \param child
 * \param self
 * \param context_id
 * \param num_contexts
 */
static SPUFunctions *
apihistogramSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

	apihistogram_spu.id = id;
	apihistogram_spu.has_child = 0;
	apihistogram_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(apihistogram_spu.child) );
		crSPUCopyDispatchTable( &(apihistogram_spu.child), &(child->dispatch_table) );
		apihistogram_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(apihistogram_spu.super) );
	crSPUCopyDispatchTable( &(apihistogram_spu.super), &(self->superSPU->dispatch_table) );
	apihistogramspuGatherConfiguration();

	apihistogramspuInitCounts();

	return &apihistogram_functions;
}

static void
apihistogramSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(apihistogram_spu.self) );
	crSPUCopyDispatchTable( &(apihistogram_spu.self), self );

	apihistogram_spu.server = (CRServer *)(self->server);
}

static int
apihistogramSPUCleanup(void)
{
	apihistogramspuPrintReport(apihistogram_spu.fp);
	if (apihistogram_spu.fp != stderr &&
			apihistogram_spu.fp != stdout) {
		fclose(apihistogram_spu.fp);
	}
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "apihistogram";
	*super = "passthrough";
	*init = apihistogramSPUInit;
	*self = apihistogramSPUSelfDispatch;
	*cleanup = apihistogramSPUCleanup;
	*options = apihistogramSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
