/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "nopspu.h"
#include "cr_spu.h"
#include "cr_glstate.h"

extern SPUNamedFunctionTable _cr_nop_table[];

NOPSPU nop_spu;

static SPUFunctions nop_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_nop_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *nopSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) self;

	nopspuGatherConfiguration();

	crStateInit();
	nop_spu.ctx = crStateCreateContext( NULL, CR_RGB_BIT );
	crStateSetCurrent( nop_spu.ctx );

	return &nop_functions;
}

static void nopSPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

static int nopSPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "nop";
	*super = NULL;
	*init = nopSPUInit;
	*self = nopSPUSelfDispatch;
	*cleanup = nopSPUCleanup;
	*options = nopSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
