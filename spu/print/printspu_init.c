#include "cr_spu.h"
#include "printspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable print_table[];

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	print_table // THE ACTUAL FUNCTIONS
};

PrintSpu print_spu;

SPUFunctions *SPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	(void) id;
	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;

	crSPUCopyDispatchTable( &(print_spu.passthrough), &(super->dispatch_table) );
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
	*name = "print";
	*super = "passthroughspu";
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	
	return 1;
}
