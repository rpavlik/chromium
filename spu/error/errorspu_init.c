/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include <stdio.h>

extern SPUNamedFunctionTable error_table[];

SPUFunctions error_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	error_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *errorSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;
	return &error_functions;
}

void errorSPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

int errorSPUCleanup(void)
{
	return 1;
}

SPUOptions errorSPUOptions[] = {
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "error";
	*super = NULL;
	*init = errorSPUInit;
	*self = errorSPUSelfDispatch;
	*cleanup = errorSPUCleanup;
	*options = errorSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
