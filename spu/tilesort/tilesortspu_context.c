/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "cr_string.h"
#ifdef USE_DMX
#include <X11/Xlib.h>
#include <X11/extensions/dmxext.h>
#endif


/* This function is registered with the DLM and will get called when
 * the DLM detects any OpeNGL errors.
 */
static void
ErrorCallback(int line, const char *file, GLenum error, const char *info)
{
	crStateError(line, file, error, info);
}


/*
 * Initialize per-thread data.
 */
void tilesortspuInitThreadPacking( ThreadInfo *thread )
{
	int i;

	thread->pinchState.numRestore = 0;
	thread->pinchState.wind = 0;
	thread->pinchState.isLoop = 0;

	thread->packer = crPackNewContext( tilesort_spu.swap );
	if (!thread->packer)
		crError("tilesortspuInitThread failed!");

	crPackErrorFunction(thread->packer, ErrorCallback);

	crPackSetContext( thread->packer ); /* sets the packer's per-thread context */
	crPackInitBuffer( &(thread->geometry_buffer),
										crAlloc( tilesort_spu.geom_buffer_size ),
										tilesort_spu.geom_buffer_size,
										tilesort_spu.geom_buffer_mtu );

	thread->geometry_buffer.geometry_only = GL_TRUE;
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
	crPackFlushFunc( thread->packer, tilesortspuFlush_callback );
	crPackFlushArg( thread->packer, (void *) thread );
	crPackSendHugeFunc( thread->packer, tilesortspuHuge );
	crPackResetBBOX( thread->packer );

	CRASSERT(thread->net[0].conn);
	CRASSERT(tilesort_spu.num_servers > 0);

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		crPackInitBuffer( &(thread->buffer[i]),
											crNetAlloc( thread->net[i].conn ),
											thread->net[i].conn->buffer_size,
											thread->net[i].conn->mtu );
		if (thread->net[i].conn->Barf)
		{
			thread->buffer[i].canBarf = GL_TRUE;
			thread->packer->buffer.canBarf = GL_TRUE;
			thread->geometry_buffer.canBarf = GL_TRUE;
		}
	}


	thread->currentContext = NULL;
}

/*
 * Allocate a new ThreadInfo structure, setup a connection to the
 * server, allocate/init a packer context, bind this ThreadInfo to
 * the calling thread with crSetTSD().
 */
#ifdef CHROMIUM_THREADSAFE
static ThreadInfo *tilesortspuNewThread(void)
{
	ThreadInfo *thread;
	int i;

	crLockMutex(&_TileSortMutex);

	CRASSERT(tilesort_spu.numThreads > 0);
	CRASSERT(tilesort_spu.numThreads < MAX_THREADS);
	thread = &(tilesort_spu.thread[tilesort_spu.numThreads]);
	tilesort_spu.numThreads++;

	thread->state_server_index = -1;

	thread->net = (CRNetServer *) crCalloc( tilesort_spu.num_servers * sizeof(CRNetServer) );
	thread->buffer = (CRPackBuffer *) crCalloc( tilesort_spu.num_servers * sizeof(CRPackBuffer) );

	for (i = 0; i < tilesort_spu.num_servers; i++) {
		thread->net[i].name = crStrdup( tilesort_spu.thread[0].net[i].name );
		thread->net[i].buffer_size = tilesort_spu.thread[0].net[i].buffer_size;
		/* Establish new connection to server[i] */
		crNetNewClient( tilesort_spu.thread[0].net[i].conn, &(thread->net[i]));
	}

	tilesortspuInitThreadPacking( thread );

	crSetTSD(&_ThreadTSD, thread);

	crUnlockMutex(&_TileSortMutex);

	return thread;
}
#endif


GLint TILESORTSPU_APIENTRY tilesortspu_CreateContext( const char *dpyName, GLint visBits )
{
	static GLint freeContextID = 200;
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	ContextInfo *contextInfo;
	int i;

	crDebug( "Tilesort SPU: CreateContext(visBits=0x%x)", visBits );

	/* release geometry buffer */
	crPackReleaseBuffer( thread0->packer );

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_TileSortMutex);
#endif

	contextInfo = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!contextInfo)
		return -1;


#ifdef WINDOWS
	crDebug("Tilesort SPU: HDC = %s", dpyName);
	if (!dpyName)
		contextInfo->client_hdc = GetDC(NULL);
	else
		contextInfo->client_hdc = (HDC) crStrToInt(dpyName);
#else
	crDebug("Tilesort SPU: Displayname = %s", (dpyName ? dpyName : "(null)"));
	
	contextInfo->dpy = XOpenDisplay(dpyName);
#endif

	crPackSetContext( thread0->packer );
	/*
	 * Allocate the state tracker state for this context.
	 * The GL limits were computed in tilesortspuGatherConfiguration().
	 */
	contextInfo->State = crStateCreateContext( &tilesort_spu.limits, visBits );
	if (!contextInfo->State) {
		crWarning( "tilesortspuCreateContext: crStateCreateContext() failed");
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&_TileSortMutex);
#endif
		return 0;  /* out of memory? */
	}

#ifdef CR_ARB_vertex_buffer_object
	contextInfo->State->bufferobject.retainBufferData = GL_TRUE;
#endif

	/* Initialize viewport & scissor */
	/* Set to -1 then set them for the first time in MakeCurrent */
	contextInfo->State->viewport.viewportW = -1;
	contextInfo->State->viewport.viewportH = -1;
	contextInfo->State->viewport.scissorW = -1;
	contextInfo->State->viewport.scissorH = -1;

	contextInfo->providedBBOX = GL_DEFAULT_BBOX_CR;

	/* Set the Current pointers now...., then reset vtx_count below... */
	crStateSetCurrentPointers( contextInfo->State, &(thread0->packer->current) );

	/* Set the vtx_count to nil, this MUST come after the
	 * crStateSetCurrentPointers above... */
	contextInfo->State->current.current->vtx_count = 0;

	/*
	 * Per-server context stuff.
	 */
	contextInfo->server = (ServerContextInfo *) crCalloc(tilesort_spu.num_servers * sizeof(ServerContextInfo));

	/*
	 * Allocate a CRContext for each server and initialize its buffer.
	 * This was originally in tilesortSPUInit()
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		contextInfo->server[i].State = crStateCreateContext( &tilesort_spu.limits,
																												 visBits );
		crStateSetCurrentPointers( contextInfo->server[i].State,
															 &(thread0->packer->current) );
	}

	/*
	 * Send a CreateContext msg to each server.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = 1;
		GLint return_val = 0;

		crPackSetBuffer( thread0->packer, &(thread0->buffer[i]) );

		if (tilesort_spu.swap)
			crPackCreateContextSWAP( dpyName, visBits, &return_val, &writeback);
		else
			crPackCreateContext( dpyName, visBits, &return_val, &writeback );

		/* release server buffer */
		crPackReleaseBuffer( thread0->packer );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBufferThread( i, thread0 );

		if (!thread0->net[0].conn->actual_network)
		{
			/* HUMUNGOUS HACK TO MATCH SERVER NUMBERING (from packspu_context.c)
			 *
			 * The hack exists solely to make file networking work for now.  This
			 * is totally gross, but since the server expects the numbers to start
			 * from 5000, we need to write them out this way.  This would be
			 * marginally less gross if the numbers (500 and 5000) were maybe
			 * some sort of #define'd constants somewhere so the client and the
			 * server could be aware of how each other were numbering things in
			 * cases like file networking where they actually
			 * care. 
			 *
			 * 	-Humper 
			 *
			 */
			return_val = 5000;
		}
		else
		{
			/* Get return value */
			while (writeback) {
				crNetRecv();
			}

			if (tilesort_spu.swap)
				return_val = (GLint) SWAP32(return_val);

			if (!return_val)
				return 0;  /* something went wrong on the server */
		}

		contextInfo->server[i].serverContext = return_val;
	}

	/* The default pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_buffer) );

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_TileSortMutex);
#endif

	contextInfo->id = freeContextID;
	crHashtableAdd(tilesort_spu.contextTable, freeContextID, contextInfo);

#if 11 /* Display list manager */
	{
		/* Our configuration for the DLM we're going to create.
		 * It says that we want the DLM to handle everything
		 * itself, making display lists transparent to 
		 * the host SPU.
		 */
		CRDLMConfig dlmConfig = {
			CRDLM_DEFAULT_BUFFERSIZE,	/* bufferSize */
			CRDLM_HANDLE_DLM,		/* handleCreation */
			CRDLM_HANDLE_SPU		/* handleReference */
		};

		/* Supplement that with our DLM.  In a more correct situation, we should
		 * see if we've been called through glXCreateContext, which has a parameter
		 * for sharing DLMs.  We don't currently get that information, so for now
		 * give each context its own DLM.
		 */
		if (!tilesort_spu.dlm) {
			tilesort_spu.dlm = crDLMNewDLM(sizeof(dlmConfig), &dlmConfig);
			if (!tilesort_spu.dlm) {
				crDebug("expando: couldn't get DLM!");
			}
			crDLMErrorFunction(ErrorCallback);
		}

		contextInfo->dlmContext = crDLMNewContext(tilesort_spu.dlm,
																							&contextInfo->State->client);
		if (!contextInfo->dlmContext) {
			crDebug("expando: couldn't get dlmContext");
			/* XXX need graceful error handling here */
		}

		/* We're not going to hold onto the dlm ourselves, so we can
		 * free it.  It won't be actually freed until all the structures
		 * that refer to it (like our dlmContext) are freed.
		 */
		/*
		crDLMFreeDLM(dlm);
		*/
	}
#endif

	return freeContextID++;
}



void TILESORTSPU_APIENTRY tilesortspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GET_THREAD(thread);
	ContextInfo *newCtx;
	int i;
	WindowInfo *winInfo;

#ifdef CHROMIUM_THREADSAFE
	if (!thread)
		thread = tilesortspuNewThread();
#endif
	CRASSERT(thread);

	if (thread->currentContext)
		tilesortspuFlush( thread );

	if (ctx) {
		newCtx = (ContextInfo *) crHashtableSearch(tilesort_spu.contextTable, ctx);
		CRASSERT(newCtx);

		winInfo = tilesortspuGetWindowInfo(window, nativeWindow);
		CRASSERT(winInfo);
#ifdef WINDOWS
		winInfo->client_hwnd = WindowFromDC( newCtx->client_hdc );
#else
		winInfo->dpy = newCtx->dpy;
		winInfo->isDMXWindow = GL_FALSE;
		if (tilesort_spu.useDMX) {
#if USE_DMX
			int event_base, error_base;
			if (DMXQueryExtension(winInfo->dpy, &event_base, &error_base))
				winInfo->isDMXWindow = GL_TRUE;
			else
				crWarning("tilesort SPU: use_dmx is set, but %s doesn't support DMX",
									DisplayString(winInfo->dpy));
#else /* USE_DMX */
			crWarning("tilesort SPU: use_dmx is set, but Chromium wasn't compiled with DMX support");
#endif /* USE_DMX */
		}
#endif /* WINDOWS */

		thread->currentContext = newCtx;
		thread->currentContext->currentWindow = winInfo;

		crStateSetCurrentPointers( newCtx->State, &(thread->packer->current) );

#ifndef WINDOWS
		if (nativeWindow) {
			CRASSERT(winInfo->xwin == nativeWindow);
		}
#endif

		/* XXX this might be excessive to do here */
		/* have to do it at least once for new windows to get back-end info */
		tilesortspuUpdateWindowInfo(winInfo);
	}
	else {
		winInfo = NULL;
		thread->currentContext = NULL;
		newCtx = NULL;
	}

	/* release geometry buffer */
	crPackReleaseBuffer( thread->packer );

	if (newCtx) {
		crPackSetContext( thread->packer );
		crStateSetCurrent( newCtx->State );
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
		/* Now send MakeCurrent to the i-th server */
		crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

		if (ctx) {
			const int serverCtx = newCtx ? newCtx->server[i].serverContext : 0;
			int serverWindow;

			if (winInfo) {
#ifdef USE_DMX
				if (tilesort_spu.useDMX) {
					/* translate nativeWindow to a back-end child window ID */
					nativeWindow = (GLint) winInfo->backendWindows[i].xsubwin;
				}
#endif
				/* translate Cr window number to server window number */
				serverWindow = winInfo->server[i].window;
			}
			else {
				serverWindow = 0;
			}

			if (!newCtx->validRasterOrigin || !winInfo->validRasterOrigin) {
				/* set raster origin */
				if (winInfo->server[i].num_extents > 0) {
					newCtx->server[i].State->current.rasterOrigin.x
						= (GLfloat) winInfo->server[i].extents[0].x1;
					newCtx->server[i].State->current.rasterOrigin.y
						= (GLfloat) winInfo->server[i].extents[0].y1;
				}
				else {
					newCtx->server[i].State->current.rasterOrigin.x = 0;
					newCtx->server[i].State->current.rasterOrigin.y = 0;
				}
			}

			if (tilesort_spu.swap)
				crPackMakeCurrentSWAP( serverWindow, nativeWindow, serverCtx );
			else
				crPackMakeCurrent( serverWindow, nativeWindow, serverCtx );
		}
		else {
			if (tilesort_spu.swap)
				crPackMakeCurrentSWAP( -1, 0, -1 );
			else
				crPackMakeCurrent( -1, 0, -1 );
		}

		/* release server buffer */
		crPackReleaseBuffer( thread->packer );
	}

	/* Both the context and window have these flags.  If either one is
	 * invalid, we have to reset the contexts' raster origins.
	 */
	newCtx->validRasterOrigin = GL_TRUE;
	winInfo->validRasterOrigin = GL_TRUE;

	/* Do one-time initializations */
	if (newCtx) {
		if (newCtx->State->viewport.viewportW == -1 ||
				newCtx->State->viewport.viewportH == -1) {
			/* set initial viewport and scissor bounds */
			tilesortspu_Viewport(0, 0, winInfo->lastWidth, winInfo->lastHeight);
			tilesortspu_Scissor(0, 0, winInfo->lastWidth, winInfo->lastHeight);
		}
	}

	/* Restore the default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );

	crDLMSetCurrentState(newCtx->dlmContext);

	if (newCtx && !newCtx->everCurrent) {
		/* This is the first time the context has been made current.  Query
		 * the servers' extension strings and update our notion of which
		 * extensions we have and don't have (for this context and the servers'
		 * contexts).  Omit for file network.
		 */
		if (thread->net->conn->actual_network) {
			int i;
			const GLubyte *ext = tilesortspuGetExtensionsString();
			crStateSetExtensionString( newCtx->State, ext );
			for (i = 0; i < tilesort_spu.num_servers; i++)
				crStateSetExtensionString(newCtx->server[i].State, ext);
			crFree((void *) ext);
		}
		newCtx->everCurrent = GL_TRUE;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_DestroyContext( GLint ctx )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	ContextInfo *contextInfo;
	GET_THREAD(thread);
	int i;

	contextInfo = (ContextInfo *) crHashtableSearch(tilesort_spu.contextTable, ctx);
	if (!contextInfo)
		return;

	/* release geometry buffer */
	crPackReleaseBuffer( thread0->packer );

	if (thread->currentContext == contextInfo) {
		/* flush the current context */
		tilesortspuFlush(thread);
		/* unbind */
		crStateSetCurrent(NULL);
	}

	/* release again, just in case */
	crPackReleaseBuffer( thread0->packer );

	/*
	 * Send DestroyCurrent msg to each server using zero-th thread's connection.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int serverCtx;

		crPackSetBuffer( thread0->packer, &(thread0->buffer[i]) );

		serverCtx = contextInfo->server[i].serverContext;

		if (tilesort_spu.swap)
			crPackDestroyContextSWAP( serverCtx );
		else
			crPackDestroyContext( serverCtx );

		/* release server buffer */
		crPackReleaseBuffer( thread0->packer );

		contextInfo->server[i].serverContext = 0;
		crStateDestroyContext(contextInfo->server[i].State);
		contextInfo->server[i].State = NULL;
	}

	/* Check if we're deleting the currently bound context */
	if (thread->currentContext == contextInfo) {
		/* unbind */
		thread->currentContext = NULL;
	}

	/* Destroy the tilesort state context */
	crFree(contextInfo->server);
	crStateDestroyContext(contextInfo->State);
	crHashtableDelete(tilesort_spu.contextTable, ctx, crFree);

	/* The default buffer */
	crPackSetBuffer( thread0->packer, &(thread->geometry_buffer) );

	crDLMFreeContext(contextInfo->dlmContext);
}


