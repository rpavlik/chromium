/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "mattespu.h"

/** Matte SPU descriptor */ 
MatteSPU matte_spu;

// Thse are used for storing the current context, in threadsafe
// and non-threadsafe environments.
#ifdef CHROMIUM_THREADSAFE
CRtsd matteTSD;
#else
ContextInfo *matteCurrentContext = NULL;
#endif

/** SPU functions */
static SPUFunctions matte_functions = {
	NULL, /**< CHILD COPY */
	NULL, /**< DATA */
	_cr_matte_table /**< THE ACTUAL FUNCTIONS - pointer to NamedFunction table */
};

/**
 * Matte spu init function
 * \param id
 * \param child
 * \param self
 * \param context_id
 * \param num_contexts
 */
static SPUFunctions *
matteSPUInit( int id, SPU *child, SPU *self, unsigned int context_id, unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

#ifdef CHROMIUM_THREADSAFE
	crInitTSD(&matteTSD);
#endif

	matte_spu.id = id;
	matte_spu.has_child = 0;
	matte_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(matte_spu.child) );
		crSPUCopyDispatchTable( &(matte_spu.child), &(child->dispatch_table) );
		matte_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(matte_spu.super) );
	crSPUCopyDispatchTable( &(matte_spu.super), &(self->superSPU->dispatch_table) );
	mattespuGatherConfiguration();

	matte_spu.contextTable = crAllocHashtable();

	return &matte_functions;
}

static void
matteSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(matte_spu.self) );
	crSPUCopyDispatchTable( &(matte_spu.self), self );

	matte_spu.server = (CRServer *)(self->server);
}

static int
matteSPUCleanup(void)
{
	crFreeHashtable(matte_spu.contextTable, matteFreeContextInfo);
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "matte";
	*super = "passthrough";
	*init = matteSPUInit;
	*self = matteSPUSelfDispatch;
	*cleanup = matteSPUCleanup;
	*options = matteSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
