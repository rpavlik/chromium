#include "cr_spu.h"
#include "packspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable pack_table[];
PackSPU pack_spu;

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	pack_table // THE ACTUAL FUNCTIONS
};

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;

	pack_spu.id = id;
	packspuCreateFunctions();
	packspuGatherConfiguration();
	packspuConnectToServer();
	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *self)
{
	(void)self;
}

int SPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "pack";
	*super = NULL;
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	
	return 1;
}
