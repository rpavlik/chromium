/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include <stdio.h>

extern SPUNamedFunctionTable nop_table[];

SPUFunctions the_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	nop_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *nopSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;
	return &the_functions;
}

void nopSPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

int nopSPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "nop";
	*super = NULL;
	*init = nopSPUInit;
	*self = nopSPUSelfDispatch;
	*cleanup = nopSPUCleanup;
	
	return 1;
}
