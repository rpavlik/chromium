/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state.h"
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
	static int first_time = 1;
	if (first_time)
	{
		first_time = 0;
		crDebug( "CALLING INIT ON %p", (void *)&(diff_api) );
		crSPUInitDispatchTable( &(diff_api) );
	}
	crSPUCopyDispatchTable( &(diff_api), api );
}
