/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "tilesortspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable tilesort_table[];
TileSortSPU tilesort_spu;

SPUFunctions tilesort_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	tilesort_table /* THE ACTUAL FUNCTIONS */
};

#ifdef CHROMIUM_THREADSAFE
CRmutex _TileSortMutex;
CRtsd _ThreadTSD;
#endif

extern void _math_init_eval(void);

SPUFunctions *tilesortSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLint vpdims[2];
	GLint totalDims[2];
	int i, j;

	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) super;

	crMemZero( &tilesort_spu, sizeof(TileSortSPU) );

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&_ThreadTSD, thread0);
	crInitMutex(&_TileSortMutex);
#endif

	_math_init_eval();

	tilesort_spu.id = id;
	tilesortspuGatherConfiguration( child );
	tilesortspuConnectToServers(); /* set up thread0's server connection */

	tilesort_spu.swap = thread0->net[0].conn->swap;

	tilesortspuInitThreadPacking( thread0 );

	tilesortspuCreateFunctions();

	crStateInit();
	tilesortspuCreateDiffAPI();
	tilesortspuBucketingInit();

	/* 
	 * With the tilesort configuration we need to reset the 
	 * maximum viewport size by asking each node what it's
	 * capabilities are and setting our limits by totalling
	 * up the results.
	 */
	totalDims[0] = totalDims[1] = 0;
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = tilesort_spu.num_servers ? 1 : 0;

		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackGetIntegervSWAP( GL_MAX_VIEWPORT_DIMS, vpdims, &writeback );
		else
			crPackGetIntegerv( GL_MAX_VIEWPORT_DIMS, vpdims, &writeback );

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );

		while (writeback) {
			crNetRecv();
		}

		for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
		{
			if (tilesort_spu.servers[i].x1[j] == 0)
				totalDims[1] += vpdims[0];
			if (tilesort_spu.servers[i].y2[j] == tilesort_spu.muralHeight)
				totalDims[0] += vpdims[1];
		}
	}

	tilesort_spu.limits.maxViewportDims[0] = totalDims[0];
	tilesort_spu.limits.maxViewportDims[1] = totalDims[1];

	/* 
	 * Once we've computed the maximum viewport size, we send
	 * a message to each server with it's new viewport parameters.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);
		else
			crPackChromiumParametervCR(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );
	}

	return &tilesort_functions;
}

void tilesortSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(tilesort_spu.self) );
	crSPUCopyDispatchTable( &(tilesort_spu.self), self );
}

int tilesortSPUCleanup(void)
{
	return 1;
}

extern SPUOptions tilesortSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options )
{
	*name = "tilesort";
	*super = NULL;
	*init = tilesortSPUInit;
	*self = tilesortSPUSelfDispatch;
	*cleanup = tilesortSPUCleanup;
	*options = tilesortSPUOptions;
	
	return 1;
}
