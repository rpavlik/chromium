#include "cr_spu.h"
#include "renderspu.h"

#include <stdio.h>

extern SPUNamedFunctionTable render_table[];
extern void LoadSystemGL( SPUNamedFunctionTable * );

SPUFunctions the_functions = {
	NULL, // CHILD COPY
	NULL, // DATA
	render_table // THE ACTUAL FUNCTIONS
};

RenderSPU render_spu;

SPUFunctions *SPUInit( int id, SPU *child,
		SPU *super,
		unsigned int num_children,
		unsigned int context_id,
		unsigned int num_contexts,
		unsigned int num_args,
		SPUArgs *args,
		void *data)
{
	printf ("Render SPU %d being initialized: %d %d %d %d!\n", id, num_children, context_id, num_contexts, num_args);
	LoadSystemGL( render_table );
	render_spu.id = id;
	renderspuGatherConfiguration();
	renderspuCreateWindow();
	return &the_functions;
}

void SPUSelfDispatch(SPUDispatchTable *self)
{
	printf ("render SPU getting its dispatch information!\n");
	render_spu.dispatch = self;
}

int SPUCleanup(void)
{
	printf ("render SPU being cleaned up!\n");
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup, 
	int *nargs, SPUArgs **args )
{
	*name = "render";
	*super = NULL;
	*init = SPUInit;
	*self = SPUSelfDispatch;
	*cleanup = SPUCleanup;
	*nargs = 0;
	*args = NULL;
	
	printf ("render SPU being loaded!\n");
	return 1;
}
