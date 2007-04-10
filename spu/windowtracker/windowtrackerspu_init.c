/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "windowtrackerspu.h"

/** Windowtracker SPU descriptor */ 
WindowtrackerSPU windowtracker_spu;

/** SPU functions */
static SPUFunctions windowtracker_functions = {
	NULL, /**< CHILD COPY */
	NULL, /**< DATA */
	_cr_windowtracker_table /**< THE ACTUAL FUNCTIONS - pointer to NamedFunction table */
};

/**
 * Windowtracker spu init function
 * \param id
 * \param child
 * \param self
 * \param context_id
 * \param num_contexts
 */
static SPUFunctions *
windowtrackerSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

	windowtracker_spu.id = id;
	windowtracker_spu.has_child = 0;
	windowtracker_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(windowtracker_spu.child) );
		crSPUCopyDispatchTable( &(windowtracker_spu.child), &(child->dispatch_table) );
		windowtracker_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(windowtracker_spu.super) );
	crSPUCopyDispatchTable( &(windowtracker_spu.super), &(self->superSPU->dispatch_table) );
	windowtrackerspuGatherConfiguration();

	return &windowtracker_functions;
}

static void
windowtrackerSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(windowtracker_spu.self) );
	crSPUCopyDispatchTable( &(windowtracker_spu.self), self );

	windowtracker_spu.server = (CRServer *)(self->server);
}

static int
windowtrackerSPUCleanup(void)
{
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "windowtracker";
	*super = "passthrough";
	*init = windowtrackerSPUInit;
	*self = windowtrackerSPUSelfDispatch;
	*cleanup = windowtrackerSPUCleanup;
	*options = windowtrackerSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
