/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h"
#include <stdio.h>

extern SPUNamedFunctionTable passthrough_table[];
extern void BuildPassthroughTable( SPU *child );

SPUFunctions passthrough_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	passthrough_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *passthroughSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) super;
	(void) context_id;
	(void) num_contexts;

	if (child == NULL)
	{
		crError( "You can't load the passthrough SPU as the last SPU in a chain!" );
	}
	BuildPassthroughTable( child );
	return &passthrough_functions;
}

void passthroughSPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

int passthroughSPUCleanup(void)
{
	return 1;
}

SPUOptions passthroughSPUOptions[] = {
   { NULL, BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "passthrough";
	*super = NULL;
	*init = passthroughSPUInit;
	*self = passthroughSPUSelfDispatch;
	*cleanup = passthroughSPUCleanup;
	*options = passthroughSPUOptions;
	
	return 1;
}
