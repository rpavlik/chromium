#include "cr_spu.h"
#include <stdio.h>

extern SPUNamedFunctionTable error_table[];

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	error_table // THE ACTUAL FUNCTIONS
};

static int SPUid;

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int num_children,
		unsigned int context_id,
		unsigned int num_contexts,
		unsigned int num_args,
		SPUArgs *args,
		void *data)
{
	printf ("error SPU %d being initialized: %d %d %d %d!\n", id, num_children, context_id, num_contexts, num_args);
	SPUid = id;
	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *parent)
{
	printf ("error SPU getting its self information!\n");
	(void)parent;
}

int SPUCleanup(void)
{
	printf ("error SPU being cleaned up!\n");
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup, 
	int *nargs, SPUArgs **args )
{
	*name = "error";
	*super = NULL;
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	*nargs = 0;
	*args = NULL;
	
	printf ("error SPU being loaded!\n");
	return 1;
}
