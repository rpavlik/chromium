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
#include "cr_string.h"

#define MAGIC_OFFSET 8000

/*
 * Initialize per-thread data.
 */
void tilesortspuInitThreadPacking( ThreadInfo *thread )
{
	int i;

	thread->geom_pack_size = tilesort_spu.buffer_size;

	thread->pinchState.numRestore = 0;
	thread->pinchState.wind = 0;
	thread->pinchState.isLoop = 0;

	thread->packer = crPackNewContext( tilesort_spu.swap );
	if (!thread->packer)
		crError("tilesortspuInitThread failed!");

	crPackSetContext( thread->packer ); /* sets the packer's per-thread context */
	crPackInitBuffer( &(thread->geometry_pack),
										crAlloc( thread->geom_pack_size ), 
										thread->geom_pack_size, tilesort_spu.MTU-(24+END_FLUFF+4+4));
	/* 24 is the size of the bounds info packet */
	/* END_FLUFF is the size of data of End */
	/* 4 since BoundsInfo opcode may take a whole 4 bytes */
	/* and 4 to let room for extra End's opcode, if needed */
	thread->geometry_pack.geometry_only = GL_TRUE;
	crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
	crPackFlushFunc( thread->packer, tilesortspuFlush_callback );
	crPackFlushArg( thread->packer, (void *) thread );
	crPackSendHugeFunc( thread->packer, tilesortspuHuge );
	crPackResetBBOX( thread->packer );

	CRASSERT(thread->net[0].conn);
	CRASSERT(tilesort_spu.num_servers > 0);

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		crPackInitBuffer( &(thread->pack[i]), crNetAlloc( thread->net[i].conn ),
											thread->net[i].conn->buffer_size, thread->net[i].conn->mtu );
		if (thread->net[i].conn->Barf)
		{
			thread->pack[i].canBarf = GL_TRUE;
			thread->packer->buffer.canBarf = GL_TRUE;
			thread->geometry_pack.canBarf = GL_TRUE;
		}
	}


	thread->currentContext = NULL;
	thread->currentContextIndex = -1;
}

/*
 * Allocate a new ThreadInfo structure, setup a connection to the
 * server, allocate/init a packer context, bind this ThreadInfo to
 * the calling thread with crSetTSD().
 */
#ifdef CHROMIUM_THREADSAFE
ThreadInfo *tilesortspuNewThread( GLint slot )
{
	ThreadInfo *thread;
	int i;

	crLockMutex(&_TileSortMutex);

	CRASSERT(tilesort_spu.numThreads > 0);
	CRASSERT(tilesort_spu.numThreads < MAX_THREADS);
	thread = &(tilesort_spu.thread[tilesort_spu.numThreads]);
	tilesort_spu.numThreads++;

	thread->net = (CRNetServer *) crCalloc( tilesort_spu.num_servers * sizeof(CRNetServer) );
	thread->pack = (CRPackBuffer *) crCalloc( tilesort_spu.num_servers * sizeof(CRPackBuffer) );

	for (i = 0; i < tilesort_spu.num_servers; i++) {
		thread->net[i].name = crStrdup( tilesort_spu.thread[0].net[i].name );
		thread->net[i].buffer_size = tilesort_spu.thread[0].net[i].buffer_size;
		/* Establish new connection to server[i] */
		crNetNewClient( tilesort_spu.thread[0].net[i].conn, &(thread->net[i]));
		if (tilesort_spu.MTU > thread->net[i].conn->mtu)
			tilesort_spu.MTU = thread->net[i].conn->mtu;
	}

	tilesortspuInitThreadPacking( thread );

	crSetTSD(&_ThreadTSD, thread);

	crUnlockMutex(&_TileSortMutex);

	return thread;
}
#endif


GLint TILESORTSPU_APIENTRY tilesortspu_CreateContext( const char *dpyName, GLint visual )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	int i, slot;

	crDebug( "In tilesortspu_CreateContext" );

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_TileSortMutex);
#endif

#ifdef WINDOWS
	crDebug("Tilesort SPU: HDC = %s\n", dpyName);
	if (!dpyName)
		tilesort_spu.client_hdc = GetDC(NULL);
	else
		tilesort_spu.client_hdc = (HDC) atoi(dpyName);
	tilesort_spu.client_hwnd = NULL;
#else
	crDebug("Tilesort SPU: Displayname = %s\n", (dpyName ? dpyName : "(null)"));
	tilesort_spu.glx_display = XOpenDisplay(dpyName);
#endif

	/*
	 * Find empty position in context[] array
	 */
	for (slot = 0; slot < CR_MAX_CONTEXTS; slot++) {
		if (tilesort_spu.context[slot].State == NULL) {
			break;
		}
	}
	if (slot == CR_MAX_CONTEXTS) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&_TileSortMutex);
#endif
		return 0;  /* too many contexts */
	}
	if (slot == tilesort_spu.numContexts) {
		tilesort_spu.numContexts++;
	}

	crPackSetContext( thread0->packer );
	/*
	 * Allocate the state tracker state for this context.
	 * The GL limits were computed in tilesortspuGatherConfiguration().
	 */
	tilesort_spu.context[slot].State = crStateCreateContext( &tilesort_spu.limits );
	if (!tilesort_spu.context[slot].State) {
		crWarning( "tilesortspuCreateContext: crStateCreateContext() failed\n");
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&_TileSortMutex);
#endif
		return 0;  /* out of memory? */
	}

	/* Initialize viewport & scissor */
	tilesort_spu.context[slot].State->viewport.viewportW = tilesort_spu.muralWidth;
	tilesort_spu.context[slot].State->viewport.viewportH = tilesort_spu.muralHeight;
	tilesort_spu.context[slot].State->viewport.scissorW = tilesort_spu.muralWidth;
	tilesort_spu.context[slot].State->viewport.scissorH = tilesort_spu.muralHeight;

	/* Set the Current pointers now...., then reset vtx_count below... */
	crStateSetCurrentPointers( tilesort_spu.context[slot].State,
						 &(thread0->packer->current) );

	/* Set the vtx_count to nil, this MUST come after the
	 * crStateSetCurrentPointers above... */
	tilesort_spu.context[slot].State->current.current->vtx_count = 0;

	/*
	 * Allocate a CRContext for each server and initialize its buffer.
	 * This was originally in tilesortSPUInit()
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		server->context[slot] = crStateCreateContext( &tilesort_spu.limits );
		server->context[slot]->current.rasterPos.x =
			server->context[slot]->current.rasterPosPre.x =
			(float) server->extents[0].x1;
		server->context[slot]->current.rasterPos.y =
			server->context[slot]->current.rasterPosPre.y =
			(float) server->extents[0].y1;
	}

	/*
	 * Send a CreateContext msg to each server.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int writeback = tilesort_spu.num_servers ? 1 : 0;
		GLint return_val = 0;

		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackCreateContextSWAP( dpyName, visual, &return_val, &writeback);
		else
			crPackCreateContext( dpyName, visual, &return_val, &writeback );

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		if (tilesort_spu.swap)
			return_val = (GLint) SWAP32(return_val);

		if (!return_val)
			return 0;  /* something went wrong on the server */

		server->serverCtx[slot] = return_val;
	}

	/*
	printf("***** %s() returned %d\n", __FUNCTION__, MAGIC_OFFSET + ctxPos);
	*/

	/* The default pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_pack) );

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_TileSortMutex);
#endif

	return MAGIC_OFFSET + slot;
}



void TILESORTSPU_APIENTRY tilesortspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GET_THREAD(thread);
	ContextInfo *newCtx;
	int i, ctxPos = ctx - MAGIC_OFFSET;

	/*
	printf("***** %s @ %p(%p, %d, %d)  ctxPos=%d\n", __FUNCTION__, &tilesortspu_MakeCurrent, display, drawable, ctx, ctxPos);
	*/

#ifdef CHROMIUM_THREADSAFE
	if (!thread)
		thread = tilesortspuNewThread( ctxPos );
#endif
	CRASSERT(thread);

#ifndef WINDOWS
	tilesort_spu.glx_drawable = (Drawable) nativeWindow;
#endif

	if (thread->currentContext)
		tilesortspuFlush( thread );

	if (ctx) {
		CRASSERT(ctxPos >= 0);
		CRASSERT(ctxPos < tilesort_spu.numContexts);
		CRASSERT(tilesort_spu.context[ctxPos].State);

		newCtx = &tilesort_spu.context[ctxPos];

		thread->currentContext = newCtx;
		thread->currentContextIndex = ctxPos;

		crStateSetCurrentPointers( tilesort_spu.context[ctxPos].State,
															 &(thread->packer->current) );
	}
	else {
		thread->currentContext = NULL;
		thread->currentContextIndex = -1;
	}

	/* Get the default buffer */
	crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

	if (ctx) {
		crPackSetContext( thread->packer );
		crStateSetCurrent( tilesort_spu.context[ctxPos].State );
		crStateFlushArg( (void *) thread );
	}
	else {
		crStateSetCurrent( NULL );
		crStateFlushArg( NULL );
	}

	/*
	 * Send MakeCurrent msg to each server using thread 0's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int serverCtx;

		/* Now send MakeCurrent to the i-th server */
		crPackSetBuffer( thread->packer, &(thread->pack[i]) );

		if (ctx) {
			 CRASSERT(server->context[ctxPos]);
			 CRASSERT(server->serverCtx[ctxPos]);

			 serverCtx = server->serverCtx[ctxPos];

			 if (tilesort_spu.swap)
				 crPackMakeCurrentSWAP( window, nativeWindow, serverCtx );
			 else
				 crPackMakeCurrent( window, nativeWindow, serverCtx );
		}
		else {
			if (tilesort_spu.swap)
				crPackMakeCurrentSWAP( -1, 0, -1 );
			else
				crPackMakeCurrent( -1, 0, -1 );
		}

		crPackGetBuffer( thread->packer, &(thread->pack[i]) );
	}

	/* Restore the default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
}



void TILESORTSPU_APIENTRY tilesortspu_DestroyContext( GLint ctx )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GET_THREAD(thread);
	int i, ctxPos = ctx - MAGIC_OFFSET;

	CRASSERT(ctxPos >= 0);
	CRASSERT(ctxPos < CR_MAX_CONTEXTS);

	if (!tilesort_spu.context[ctxPos].State)
		return;

	if (thread->currentContext == &(tilesort_spu.context[ctxPos])) {
		/* flush the current context */
		tilesortspuFlush(thread);
		/* unbind */
		crStateSetCurrent(NULL);
	}


	/* The default buffer */
	crPackGetBuffer( thread0->packer, &(thread0->geometry_pack) );

	/*
	 * Send DestroyCurrent msg to each server using zero-th thread's connection.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int serverCtx;

		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		serverCtx = server->serverCtx[ctxPos];
		if (tilesort_spu.swap)
			crPackDestroyContextSWAP( serverCtx );
		else
			crPackDestroyContext( serverCtx );

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		server->serverCtx[ctxPos] = 0;
		crStateDestroyContext(server->context[ctxPos]);
		server->context[ctxPos] = NULL;
	}

	/* Check if we're deleting the currently bound context */
	if (thread->currentContext == &(tilesort_spu.context[ctxPos])) {
		/* unbind */
		thread->currentContext = NULL;
		thread->currentContextIndex = -1;
	}

	/* Destroy the tilesort state context */
	crStateDestroyContext( tilesort_spu.context[ctxPos].State );
	tilesort_spu.context[ctxPos].State = NULL;

	/* The default buffer */
	crPackSetBuffer( thread0->packer, &(thread->geometry_pack) );
}
