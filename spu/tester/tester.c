#include "cr_spu.h"

#include <stdio.h>

SPUFunctions *SPUInit( SPUDispatchTable **child,
		unsigned int num_children,
		unsigned int context_id,
		unsigned int num_contexts,
		unsigned int num_args,
		SPUArgs *args,
		void *data)
{
	printf ("tester SPU being initialized: %d %d %d %d!\n", num_children, context_id, num_contexts, num_args);
	return NULL;
}

void SPUParent(SPUDispatchTable *parent,
		SPUDispatchTable *super,
		void *data)
{
	printf ("tester SPU getting its parent information!\n");
}

int SPUCleanup(void)
{
	printf ("tester SPU being cleaned up!\n");
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUParentFuncPtr *parent, SPUCleanupFuncPtr *cleanup, 
	int *nargs, SPUArgs **args )
{
	*name = "tester";
	*super = NULL;
	*init = SPUInit;
	*parent = SPUParent;
	*cleanup = SPUCleanup;
	*nargs = 0;
	*args = NULL;
	
	printf ("Tester SPU being loaded!\n");
	return 1;
}
