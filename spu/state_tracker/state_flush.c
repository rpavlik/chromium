#include "cr_glstate.h"
#include "cr_spu.h"

SPUDispatchTable diff_api;

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
	crSPUCopyDispatchTable( &(diff_api), api );
}
