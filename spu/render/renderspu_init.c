#include "cr_spu.h"
#include <stdio.h>

extern SPUNamedFunctionTable render_table[];
extern void LoadSystemGL( SPUNamedFunctionTable * );

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	render_table // THE ACTUAL FUNCTIONS
};

SPUFunctions *SPUInit( SPUDispatchTable **child,
		unsigned int num_children,
		unsigned int context_id,
		unsigned int num_contexts,
		unsigned int num_args,
		SPUArgs *args,
		void *data)
{
	printf ("Render SPU being initialized: %d %d %d %d!\n", num_children, context_id, num_contexts, num_args);
	LoadSystemGL( render_table );
	return &the_functions;
}

void SPUParent(SPUDispatchTable *parent,
		SPUDispatchTable *super,
		void *data)
{
	printf ("render SPU getting its parent information!\n");
	(void)parent;
	(void)super;
	(void)data;
}

int SPUCleanup(void)
{
	printf ("render SPU being cleaned up!\n");
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUParentFuncPtr *parent, SPUCleanupFuncPtr *cleanup, 
	int *nargs, SPUArgs **args )
{
	*name = "render";
	*super = NULL;
	*init = SPUInit;
	*parent = SPUParent;
	*cleanup = SPUCleanup;
	*nargs = 0;
	*args = NULL;
	
	printf ("render SPU being loaded!\n");
	return 1;
}
