#include "cr_spu.h"
#include <stdio.h>

extern SPUNamedFunctionTable passthrough_table[];
extern void BuildPassthroughTable( SPU *child );

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	passthrough_table // THE ACTUAL FUNCTIONS
};

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) super;
	(void) context_id;
	(void) num_contexts;

	BuildPassthroughTable( child );
	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

int SPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "passthrough";
	*super = NULL;
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	
	return 1;
}
