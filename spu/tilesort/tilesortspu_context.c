/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "tilesortspu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"



GLint TILESORTSPU_APIENTRY tilesortspu_CreateContext( void *display, GLint visual )
{
	static GLboolean firstCall = GL_TRUE;
	int i, ctxPos = -1;

	crDebug( "In tilesortspu_CreateContext" );
	/*
	printf("***** %s @ %p(%p, %d)\n", __FUNCTION__, &tilesortspu_CreateContext, display, visual);
	*/

#ifdef WINDOWS
	tilesort_spu.client_hdc = (HDC) display;
	tilesort_spu.client_hwnd = NULL;
#else
	tilesort_spu.glx_display = (Display *) display;
        /*
	tilesort_spu.glx_drawable = (Drawable) visual;
	tilesort_spu.glx_drawable = 0;
        */
#endif


	/*
	 * Find empty position in context[] array
	 */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (tilesort_spu.context[i] == NULL) {
			ctxPos = i;
			break;
		}
	}
	if (ctxPos < 0) {
		crWarning( "tilesortspuCreateContext: No free contexts\n");
		return 0;  /* Ran out of contexts */
	}


	/* The GL limits were computed in tilesortspuGatherConfiguration() */
	tilesort_spu.context[ctxPos] = crStateCreateContext(&tilesort_spu.limits);

	if (!tilesort_spu.context[ctxPos]) {
		crWarning( "tilesortspuCreateContext: crStateCreateContext() failed\n");
		return 0;  /* out of memory? */
	}

	/* Fix up viewport & scissor */
	tilesort_spu.context[ctxPos]->viewport.viewportW = tilesort_spu.muralWidth;
	tilesort_spu.context[ctxPos]->viewport.viewportH = tilesort_spu.muralHeight;
	tilesort_spu.context[ctxPos]->viewport.scissorW = tilesort_spu.muralWidth;
	tilesort_spu.context[ctxPos]->viewport.scissorH = tilesort_spu.muralHeight;

	if (firstCall) {
		crPackInit( tilesort_spu.swap );
		crPackInitBuffer( &(tilesort_spu.geometry_pack),
						  crAlloc( tilesort_spu.geom_pack_size ), 
						  tilesort_spu.geom_pack_size, END_FLUFF );
	}
	crPackSetBuffer( &(tilesort_spu.geometry_pack) );
	crPackFlushFunc( tilesortspuFlush );
	crPackFlushArg( tilesort_spu.context[ctxPos] );
	crPackSendHugeFunc( tilesortspuHuge );
	crPackResetBBOX();  /* XXX move into MakeCurrent() ? */

	crStateSetCurrentPointers( tilesort_spu.context[ctxPos],
							   &(cr_packer_globals.current) );
	if (tilesort_spu.context[ctxPos]->current.current)
		tilesort_spu.context[ctxPos]->current.current->vtx_count = 0;


	/*
	 * Allocate a crStateContext for each server and initialize its buffer.
	 * This was originally in tilesortSPUInit()
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		server->context[ctxPos] = crStateCreateContext( &tilesort_spu.limits );
		server->context[ctxPos]->current.rasterPos.x =
			server->context[ctxPos]->current.rasterPosPre.x =
			(float) server->x1[0];
		server->context[ctxPos]->current.rasterPos.y =
			server->context[ctxPos]->current.rasterPosPre.y =
			(float) server->y1[0];
		if (firstCall)
			crPackInitBuffer( &(server->pack), crNetAlloc( server->net.conn ),
						  server->net.buffer_size, 0 );
	}

	/*
	 * Send a CreateContext msg to each server
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int writeback = tilesort_spu.num_servers ? 1 : 0;
		GLint return_val = 0;

		crPackSetBuffer( &(server->pack) );

		if (tilesort_spu.swap)
			crPackCreateContextSWAP( display, visual, &return_val, &writeback);
		else
			crPackCreateContext( display, visual, &return_val, &writeback );

		crPackGetBuffer( &(server->pack) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( server );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		if (tilesort_spu.swap)
			return_val = (GLint) SWAP32(return_val);

		if (!return_val)
			return 0;  /* something went wrong on the server */

		server->serverCtx[ctxPos] = return_val;
	}

	/*
	printf("***** %s() returned %d\n", __FUNCTION__, 8000 + ctxPos);
	*/

	firstCall = GL_FALSE;

	/* The default pack buffer */
	crPackSetBuffer( &(tilesort_spu.geometry_pack) );

	return 8000 + ctxPos;   /* magic number */
}



void TILESORTSPU_APIENTRY tilesortspu_MakeCurrent( void *display, GLint drawable, GLint ctx )
{
	int i, ctxPos = ctx - 8000;   /* see magic number above */

	CRASSERT(ctxPos >= 0);
	CRASSERT(ctxPos < CR_MAX_CONTEXTS);
	CRASSERT(tilesort_spu.context[ctxPos]);

	/*
	printf("***** %s @ %p(%p, %d, %d)  ctxPos=%d\n", __FUNCTION__, &tilesortspu_MakeCurrent, display, drawable, ctx, ctxPos);
	*/

#ifdef WINDOWS
	/* XXX FIX */
#else
	tilesort_spu.glx_drawable = (Drawable) drawable;
#endif

	if (tilesort_spu.currentContext)
		tilesortspuFlush( tilesort_spu.currentContext );

	/* The default buffer */
	crPackGetBuffer( &(tilesort_spu.geometry_pack) );


	crStateMakeCurrent( tilesort_spu.context[ctxPos] );
	crStateFlushArg( tilesort_spu.context[ctxPos] );

	/* set tilesort spu current context */
	tilesort_spu.currentContext = tilesort_spu.context[ctxPos];

	/*
	 * Send MakeCurrent msg to each server
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int serverCtx;

		/* set local currentContext pointer */
		CRASSERT(server->context[ctxPos]);
		CRASSERT(server->serverCtx[ctxPos]);
		server->currentContext = server->context[ctxPos];

		/* Now send MakeCurrent to the i-th server */
		crPackSetBuffer( &(server->pack) );

		serverCtx = server->serverCtx[ctxPos];
		if (tilesort_spu.swap)
			crPackMakeCurrentSWAP( display, drawable, serverCtx );
		else
			crPackMakeCurrent( display, drawable, serverCtx );

		crPackGetBuffer( &(server->pack) );

	}

	/* The default buffer */
	crPackSetBuffer( &(tilesort_spu.geometry_pack) );
}



void TILESORTSPU_APIENTRY tilesortspu_DestroyContext( void *display, GLint ctx )
{
	int i, ctxPos = ctx - 8000;  /* see magic number above */

	CRASSERT(ctxPos >= 0);
	CRASSERT(ctxPos < CR_MAX_CONTEXTS);


	if (!tilesort_spu.context[ctxPos])
		return;

	/* The default buffer */
	crPackGetBuffer( &(tilesort_spu.geometry_pack) );

	/* set tilesort spu current context */
	if (tilesort_spu.currentContext == tilesort_spu.context[ctxPos])
		tilesort_spu.currentContext = NULL;

	/*
	 * Send DestroyCurrent msg to each server
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int serverCtx;

		crPackSetBuffer( &(server->pack) );

		serverCtx = server->serverCtx[ctxPos];
		if (tilesort_spu.swap)
			crPackDestroyContextSWAP( display, serverCtx );
		else
			crPackDestroyContext( display, serverCtx );

		crPackGetBuffer( &(server->pack) );

		server->serverCtx[ctxPos] = 0;
		crStateDestroyContext(server->context[ctxPos]);
		server->context[ctxPos] = NULL;
	}

	/* Destroy the tilesort state context */
	crStateDestroyContext( tilesort_spu.context[ctxPos] );
	tilesort_spu.context[ctxPos] = NULL;

	/* The default buffer */
	crPackSetBuffer( &(tilesort_spu.geometry_pack) );
}
