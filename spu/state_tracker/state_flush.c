#include "cr_glstate.h"
#include "cr_spu.h"

void crStateFlushFunc( CRStateFlushFunc func )
{
	CRContext *g = GetCurrentContext();

	g->flush_func = func;
}

void crStateFlushArg( void *arg )
{
	CRContext *g = GetCurrentContext();

	g->flush_arg = arg;
}

void crStateDiffAPI( SPUDispatchTable *api )
{
	CRContext *g = GetCurrentContext();

	crSPUCopyDispatchTable( &(g->diff_api), api );
}
