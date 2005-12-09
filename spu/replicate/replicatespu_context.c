/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "cr_string.h"
#include "cr_dlm.h"
#include "replicatespu_proto.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <X11/Xmd.h>
#include <X11/extensions/vnc.h>

#define MAGIC_OFFSET 3000

/*
 * Allocate a new ThreadInfo structure, setup a connection to the
 * server, allocate/init a packer context, bind this ThreadInfo to
 * the calling thread with crSetTSD().
 * We'll always call this function at least once even if we're not
 * using threads.
 */
ThreadInfo *replicatespuNewThread( unsigned long id )
{
	ThreadInfo *thread;

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#else
	CRASSERT(replicate_spu.numThreads == 0);
#endif

	CRASSERT(replicate_spu.numThreads < MAX_THREADS);
	thread = &(replicate_spu.thread[replicate_spu.numThreads]);

	thread->id = id;
	thread->currentContext = NULL;

	/* connect to the server */
	thread->server.name = crStrdup( replicate_spu.name );
	thread->server.buffer_size = replicate_spu.buffer_size;
	if (replicate_spu.numThreads == 0) {
		replicatespuConnectToServer( &(thread->server) );
		CRASSERT(thread->server.conn);
		replicate_spu.swap = thread->server.conn->swap;
	}
	else {
		/* a new pthread */
		replicatespuFlush( &(replicate_spu.thread[0]) );
		crNetNewClient( replicate_spu.thread[0].server.conn, &(thread->server));
		CRASSERT(thread->server.conn);
	}

	/* packer setup */
	CRASSERT(thread->packer == NULL);
	thread->packer = crPackNewContext( replicate_spu.swap );
	CRASSERT(thread->packer);
	crPackInitBuffer( &(thread->buffer), crNetAlloc(thread->server.conn),
				thread->server.conn->buffer_size, thread->server.conn->mtu );
	thread->buffer.canBarf = thread->server.conn->Barf ? GL_TRUE : GL_FALSE;
	crPackSetBuffer( thread->packer, &thread->buffer );
	crPackFlushFunc( thread->packer, replicatespuFlush );
	crPackFlushArg( thread->packer, (void *) thread );
	crPackSendHugeFunc( thread->packer, replicatespuHuge );
	crPackSetContext( thread->packer );

#ifdef CHROMIUM_THREADSAFE_notyet
	crSetTSD(&_ReplicateTSD, thread);
#endif

	replicate_spu.numThreads++;

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif
	return thread;
}


/**
 * Determine if the X VNC extension is available.
 * Get list of clients/viewers attached, and replicate to them.
 */
static void
replicatespuStartVnc( void )
{
	int maj, min;

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#endif
	if (replicate_spu.StartedVnc) {
#ifdef CHROMIUM_THREADSAFE_notyet
		crUnlockMutex(&_ReplicateMutex);
#endif
		return;
	}

	replicate_spu.StartedVnc = 1;

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif


	/* NOTE: should probably check the major/minor version too!! */
	CRASSERT(replicate_spu.glx_display);
	if (!XVncQueryExtension(replicate_spu.glx_display, &maj, &min)) {
		crWarning("Replicate SPU: VNC extension is not available.");
		replicate_spu.vncAvailable = GL_FALSE;
	} else {
		crWarning("Replicate SPU: VNC extension available.");
		replicate_spu.vncAvailable = GL_TRUE;
	}

	if (replicate_spu.vncAvailable) {
		VncConnectionList *vnclist;
		int num_conn;
		int i;

		replicate_spu.VncEventsBase = XVncGetEventBase(replicate_spu.glx_display);
		XVncSelectNotify(replicate_spu.glx_display, 1);

		vnclist = XVncListConnections(replicate_spu.glx_display, &num_conn);
		crDebug("Replicate SPU: Found %d open VNC connections", num_conn);
		for (i = 0; i < num_conn; i++) {
			if (vnclist[i].ipaddress) {
				replicatespuReplicate(vnclist[i].ipaddress);
			}
			else {
				crWarning("Replicate SPU: vnc client connection with no ipaddress!");
			}
		}
	}
}


GLint REPLICATESPU_APIENTRY
replicatespu_CreateContext( const char *dpyName, GLint visual )
{
	static GLint freeCtxID = MAGIC_OFFSET;
	static int done = 0;
	int writeback;
	GLint serverCtx = (GLint) -1;
	char headspuname[10];
	ContextInfo *context;

	replicatespuFlush( &(replicate_spu.thread[0]) );

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#endif
	if (!replicate_spu.glx_display) {
		replicate_spu.glx_display = XOpenDisplay(dpyName);
		if (!replicate_spu.glx_display) {
			/* Try local display */
			replicate_spu.glx_display = XOpenDisplay(":0");
		}
		CRASSERT(replicate_spu.glx_display);
	}

	crPackSetContext( replicate_spu.thread[0].packer );

	if (replicate_spu.swap)
	{
		crPackGetChromiumParametervCRSWAP( GL_HEAD_SPU_NAME_CR, 0, GL_BYTE, 6, headspuname, &writeback );
	}
	else
	{
		crPackGetChromiumParametervCR( GL_HEAD_SPU_NAME_CR, 0, GL_BYTE, 6, headspuname, &writeback );
	}
	replicatespuFlushOne( &(replicate_spu.thread[0]), 0);
	writeback = 1;
	while (writeback)
		crNetRecv();


	/* Pack the CreateContext command */
	if (replicate_spu.swap)
		crPackCreateContextSWAP( dpyName, visual, &serverCtx, &writeback );
	else
		crPackCreateContext( dpyName, visual, &serverCtx, &writeback );

	/* Flush buffer and get return value */
	replicatespuFlushOne( &(replicate_spu.thread[0]), 0);
	writeback = 1;
	while (writeback)
		crNetRecv();
	if (replicate_spu.swap) {
		serverCtx = (GLint) SWAP32(serverCtx);
	}

	if (serverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE_notyet
		crUnlockMutex(&_ReplicateMutex);
#endif
		crWarning("Replicate SPU: Failure in CreateContext");
		return -1;  /* failed */
	}

	context = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!context) {
		crWarning("Replicate SPU: Out of memory in CreateContext");
		return -1;
	}

	/* The first slot (slot 0) gets its own display list manager.
	 * All the other slots use the same display list manager. 
	 * This isn't great behavior, but it matches Chromium's default
	 * behaviors.
	 */
	if (!replicate_spu.displayListManager) {
		replicate_spu.displayListManager = crDLMNewDLM(0, NULL);
		if (!replicate_spu.displayListManager) {
			crWarning("replicatespu_CreateContext: could not initialize DLM");
		}
	}
	else {
		/* Let the DLM know that a second context is using the
		 * same display list manager, so it can manage when its
		 * resources are released.
		 */
		crDLMUseDLM(replicate_spu.displayListManager);
	}

	/* Fill in the new context info */
	context->State = crStateCreateContext(NULL, visual);
	context->serverCtx = serverCtx;
	context->rserverCtx[0] = serverCtx;
	context->visBits = visual;
	context->currentWindow = 0; /* not bound */
	context->dlmState
		= crDLMNewContext(replicate_spu.displayListManager);
	context->displayListMode = GL_FALSE; /* not compiling */
	context->displayListIdentifier = 0;

#if 0
	/* Set the Current pointers now.... */
	crStateSetCurrentPointers( context->State,
														 &(replicate_spu.thread[0].packer->current) );
#endif

	(void) done;
#if 0
	if (!done) {
		replicatespuStartVnc( dpyName );
		done = 1;
	} else
#endif
	{
		unsigned int i;

		for (i = 1; i < CR_MAX_REPLICANTS; i++) {
			int r_writeback = 1, rserverCtx = -1;

			if (replicate_spu.rserver[i].conn == NULL ||
					replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
				continue;

			if (replicate_spu.swap)
				crPackCreateContextSWAP( dpyName, visual, &rserverCtx, &r_writeback );
			else
				crPackCreateContext( dpyName, visual, &rserverCtx, &r_writeback );

			/* Flush buffer and get return value */
			replicatespuFlushOne( &(replicate_spu.thread[0]), i );

			while (r_writeback)
				crNetRecv();
			if (replicate_spu.swap)
				rserverCtx = (GLint) SWAP32(rserverCtx);

			if (rserverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE_notyet
				crUnlockMutex(&_ReplicateMutex);
#endif
				crError("Failure in replicatespu_CreateContext");
				return -1;  /* failed */
			}

			context->rserverCtx[i] = rserverCtx;
		}
	}


	if (!crStrcmp( headspuname, "nop" ))
		replicate_spu.NOP = 0;
	else
		replicate_spu.NOP = 1;

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif

	crHashtableAdd(replicate_spu.contextTable, freeCtxID, context);

	return freeCtxID++;
}


void REPLICATESPU_APIENTRY
replicatespu_DestroyContext( GLint ctx )
{
	unsigned int i;
	ContextInfo *context = (ContextInfo *) crHashtableSearch(replicate_spu.contextTable, ctx);
	GET_THREAD(thread);

	if (!context) {
		crWarning("Replicate SPU: DestroyContext, bad context %d", ctx);
		return;
	}
	CRASSERT(thread);

	replicatespuFlush( (void *)thread );

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {
		if (replicate_spu.rserver[i].conn == NULL ||
				replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
			continue;

		if (replicate_spu.swap)
			crPackDestroyContextSWAP( context->rserverCtx[i] );
		else
			crPackDestroyContext( context->rserverCtx[i] );

		replicatespuFlushOne(thread, i);
	}

	crStateDestroyContext( context->State );

	/* Although we only allocate a display list manager once,
	 * we free it every time; this is okay since the DLM itself
	 * will track its uses and will only release the resources
	 * when the last user has relinquished it.
	 */
	crDLMFreeDLM(replicate_spu.displayListManager);
	crDLMFreeContext(context->dlmState);

	if (thread->currentContext == context) {
		thread->currentContext = NULL;
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState(NULL);
	}

	/* zero, just to be safe */
	crMemZero(context, sizeof(ContextInfo));

	crHashtableDelete(replicate_spu.contextTable, ctx, crFree);
}


/**
 * Tell VNC server to begin monitoring the nativeWindow X window for
 * moves/resizes.
 * When the server-side VNC module notices such changes, it'll
 * send an rfbChromiumMoveResizeWindow message to the VNC viewer.
 */
void
replicatespuBeginMonitorWindow(WindowInfo *winInfo)
{
	int i;
	for (i = 1; i < CR_MAX_REPLICANTS; i++) {
		if (winInfo->id[i] >= 0) {
			XVncChromiumMonitor(replicate_spu.glx_display,
													winInfo->id[i], winInfo->nativeWindow);
		}
	}
}


/**
 * Tell VNC server to stop monitoring an X window.
 */
void
replicatespuEndMonitorWindow(WindowInfo *winInfo)
{
	const int crServerWindow = 0;
	/* only need to send one of these */
	XVncChromiumMonitor(replicate_spu.glx_display,
											crServerWindow, winInfo->nativeWindow);
}



/**
 * Callback called by crHashtableWalk() below.
 */
static void
destroyContextCallback(unsigned long key, void *data1, void *data2)
{
	if (key >= 1) {
		crDebug("Replicate SPU: %s %d\n", __func__, (int) key);
		replicatespu_DestroyContext((GLint) key);
	}
}

static void
destroyWindowCallback(unsigned long key, void *data1, void *data2)
{
	if (key >= 1) {
		crDebug("Replicate SPU: %s %d\n", __func__, (int) key);
		replicatespu_WindowDestroy((GLint) key);
	}
}


void
replicatespuDestroyAllWindowsAndContexts(void)
{
	crHashtableWalk(replicate_spu.contextTable, destroyContextCallback, NULL);
	crHashtableWalk(replicate_spu.windowTable, destroyWindowCallback, NULL);
}



void REPLICATESPU_APIENTRY
replicatespu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GLint *rserverCtx;
	unsigned int i;
	unsigned int show_window = 0;
	WindowInfo *winInfo = (WindowInfo *) crHashtableSearch( replicate_spu.windowTable, window );
	ContextInfo *newCtx = (ContextInfo *) crHashtableSearch( replicate_spu.contextTable, ctx );
	GET_THREAD(thread);

	if (!winInfo) {
		crWarning("Replicate SPU: Invalid window ID %d passed to MakeCurrent", window);
		return;
	}

	if (thread)
		replicatespuFlush( (void *)thread );

	if (!thread) {
		thread = replicatespuNewThread( crThreadID() );
	}
	CRASSERT(thread);
	CRASSERT(thread->packer);

	/* XXX shouldn't this just be done once? */
	replicatespuStartVnc( );

	if (newCtx) {

		newCtx->currentWindow = winInfo;

		/* XXX not sure this is needed anymore */
		if (replicate_spu.render_to_crut_window && !nativeWindow) {
			char response[8096];
			CRConnection *conn = crMothershipConnect();
			if (!conn) {
				crError("Couldn't connect to the mothership to get CRUT drawable-- "
								"I have no idea what to do!");
			}
			crMothershipGetParam( conn, "crut_drawable", response );
			nativeWindow = crStrToInt(response);
			crDebug("Replicate SPU: using CRUT drawable: 0x%x", nativeWindow);
			crMothershipDisconnect(conn);
		}

		if (replicate_spu.glx_display
				&& winInfo
				&& winInfo->nativeWindow != nativeWindow)
		{
			winInfo->nativeWindow = nativeWindow;
			replicatespuBeginMonitorWindow(winInfo);
			replicatespuRePositionWindow(winInfo);
			show_window = 1;
		}

		CRASSERT(newCtx->State);  /* verify valid */

		crPackSetContext( thread->packer );
		rserverCtx = newCtx->rserverCtx;
	}
	else {
		newCtx = NULL;
		rserverCtx = NULL;
	}

	/*
	 * Send the MakeCurrent to all crservers (vnc viewers)
	 */
	for (i = 0; i < CR_MAX_REPLICANTS; i++) {

		if (replicate_spu.rserver[i].conn == NULL ||
				replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
				continue;

		/* Note: app's native window ID not needed on server side; pass zero */
		if (replicate_spu.swap)
			crPackMakeCurrentSWAP( winInfo->id[i], 0, rserverCtx[i] );
		else
			crPackMakeCurrent( winInfo->id[i], 0, rserverCtx[i] );

		if (show_window) {
			/* We may find that the window was mapped before we
			 * called MakeCurrent, if that's the case then ensure
			 * the remote end gets the WindowShow event */
			if (winInfo->viewable) {
				if (replicate_spu.swap)
					crPackWindowShowSWAP( winInfo->id[i], GL_TRUE );
				else
					crPackWindowShow( winInfo->id[i], GL_TRUE );
			}
		}

		replicatespuFlushOne(thread, i);
	}

	if (newCtx) {
		crStateMakeCurrent( newCtx->State );
		crDLMSetCurrentState(newCtx->dlmState);
	}
	else {
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState(NULL);
	}

	thread->currentContext = newCtx;

	/* DEBUG */
	{
		GET_THREAD(t);
		(void) t;
		CRASSERT(t);
	}
}
