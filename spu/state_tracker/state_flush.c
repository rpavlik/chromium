#include "cr_glstate.h"
#include "cr_spu.h"

void crStateFlushFunc( CRStateFlushFunc func )
{
	CRContext *g = GetCurrentContext();

	g->flush_func = func;
}

void crStateFlushAPI( SPUDispatchTable *api )
{
	CRContext *g = GetCurrentContext();

	crSPUCopyDispatchTable( &(g->flush_api), api );
}
