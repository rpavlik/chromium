/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
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
	thread->broadcast = 1;

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

static void replicatespuStartVnc( void )
{
	VncConnectionList *vnclist;
	int num_conn;
	int i;
	struct in_addr addr;
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
	if (!XVncQueryExtension(replicate_spu.glx_display, &maj, &min)) {
		crWarning("Replicate SPU: VNC extension is not available.");
		replicate_spu.vncAvailable = GL_FALSE;
	} else {
		crWarning("Replicate SPU: VNC extension available.");
		replicate_spu.vncAvailable = GL_TRUE;
	}

	if (replicate_spu.vncAvailable) {

		replicate_spu.VncEventsBase = XVncGetEventBase(replicate_spu.glx_display);
		XVncSelectNotify(replicate_spu.glx_display, 1);

		vnclist = XVncListConnections(replicate_spu.glx_display, &num_conn);
		for (i = 0; i < num_conn; i++) {

			addr.s_addr = vnclist->ipaddress;

			if (vnclist->ipaddress) {
				replicatespuReplicateCreateContext(vnclist->ipaddress);
			} else {
				crWarning("Replicate SPU: vnclist with no ipaddress ???????????.");
			}

			vnclist++;
		}
	}
}


GLint REPLICATESPU_APIENTRY replicatespu_CreateContext( const char *dpyName, GLint visual )
{
	static int done = 0;
	int writeback = 1;
	GLint serverCtx = (GLint) -1;
	int slot;
	char headspuname[10];

	replicatespuFlush( &(replicate_spu.thread[0]) );

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#endif
	if (!replicate_spu.glx_display)
		replicate_spu.glx_display = XOpenDisplay(dpyName);

	replicate_spu.thread[0].broadcast = 0;

	crPackSetContext( replicate_spu.thread[0].packer );

	if (replicate_spu.swap)
	{
		crPackGetChromiumParametervCRSWAP( GL_HEAD_SPU_NAME_CR, 0, GL_BYTE, 6, headspuname, &writeback );
	}
	else
	{
		crPackGetChromiumParametervCR( GL_HEAD_SPU_NAME_CR, 0, GL_BYTE, 6, headspuname, &writeback );
	}
	replicatespuFlush( &(replicate_spu.thread[0]) );
	while (writeback)
		crNetRecv();

	writeback = 1;

	/* Pack the command */
	if (replicate_spu.swap)
		crPackCreateContextSWAP( dpyName, visual, &serverCtx, &writeback );
	else
		crPackCreateContext( dpyName, visual, &serverCtx, &writeback );

	/* Flush buffer and get return value */
	replicatespuFlush( &(replicate_spu.thread[0]) );

	while (writeback)
		crNetRecv();

	if (replicate_spu.swap) {
		serverCtx = (GLint) SWAP32(serverCtx);
	}
	if (serverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE_notyet
		crUnlockMutex(&_ReplicateMutex);
#endif
		crWarning("Replicate SPU: Failure in replicatespu_CreateContext");
		return -1;  /* failed */
	}

	/* find an empty context slot */
	for (slot = 0; slot < replicate_spu.numContexts; slot++) {
		if (!replicate_spu.context[slot].State) {
			/* found empty slot */
			break;
		}
	}
	if (slot == replicate_spu.numContexts) {
		replicate_spu.numContexts++;
	}

	/* The first slot (slot 0) gets its own display list manager.
	 * All the other slots use the same display list manager. 
	 * This isn't great behavior, but it matches Chromium's default
	 * behaviors.
	 */
	if (slot == 0) {
		replicate_spu.context[slot].displayListManager = 
			crDLMNewDLM(0, NULL);
		if (!replicate_spu.context[slot].displayListManager) {
			crWarning("replicatespu_CreateContext: could not initialize DLM");
		}
	}
	else {
		replicate_spu.context[slot].displayListManager = 
			replicate_spu.context[0].displayListManager;
		/* Let the DLM know that a second context is using the
		 * same display list manager, so it can manage when its
		 * resources are released.
		 */
		crDLMUseDLM(replicate_spu.context[slot].displayListManager);
	}

	/* Fill in the new context info */
	replicate_spu.context[slot].State = crStateCreateContext(NULL, visual);
	replicate_spu.context[slot].serverCtx = serverCtx;
	replicate_spu.context[slot].rserverCtx[0] = serverCtx;
	replicate_spu.context[slot].visBits = visual;
	replicate_spu.context[slot].currentWindow = 0; /* not bound */
	replicate_spu.context[slot].dlmState = crDLMNewContext(
			replicate_spu.context[slot].displayListManager);
	replicate_spu.context[slot].displayListMode = GL_FALSE; /* not compiling */
	replicate_spu.context[slot].displayListIdentifier = 0;

#if 0
	/* Set the Current pointers now.... */
	crStateSetCurrentPointers( replicate_spu.context[slot].State,
						 &(replicate_spu.thread[0].packer->current) );
#endif

	replicate_spu.thread[0].broadcast = 1;

	(void) done;
#if 0
	if (!done) {
		replicatespuStartVnc( dpyName );
		done = 1;
	} else
#endif
	{
		unsigned int i;
		int r_writeback = 1;
		GLint rserverCtx = (GLint) -1;

		replicate_spu.thread[0].broadcast = 0;

		for (i = 1; i < CR_MAX_REPLICANTS; i++) {

			if (replicate_spu.rserver[i].conn == NULL || replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
				continue;

			r_writeback = 1;
			rserverCtx = (GLint) -1;

			replicate_spu.thread[0].server.conn = replicate_spu.rserver[i].conn;

			if (replicate_spu.swap)
				crPackCreateContextSWAP( dpyName, visual, &rserverCtx, &r_writeback );
			else
				crPackCreateContext( dpyName, visual, &rserverCtx, &r_writeback );

			/* Flush buffer and get return value */
			replicatespuFlush( &(replicate_spu.thread[0]) );

			while (r_writeback)
				crNetRecv();

			if (replicate_spu.swap) {
				rserverCtx = (GLint) SWAP32(rserverCtx);
			}

			if (rserverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE_notyet
				crUnlockMutex(&_ReplicateMutex);
#endif
				crError("Failure in replicatespu_CreateContext");
				return -1;  /* failed */
			}

			replicate_spu.context[slot].rserverCtx[i] = rserverCtx;
			
			/* restore it */
			replicate_spu.thread[0].server.conn = replicate_spu.rserver[0].conn;
		}
	}

	replicate_spu.thread[0].broadcast = 1;

	if (!crStrcmp( headspuname, "nop" ))
		replicate_spu.NOP = 0;
	else
		replicate_spu.NOP = 1;

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif

	return MAGIC_OFFSET + slot;
}


void REPLICATESPU_APIENTRY replicatespu_DestroyContext( GLint ctx )
{
	const int slot = ctx - MAGIC_OFFSET;
	unsigned int i;
	ContextInfo *context;
	GET_THREAD(thread);

	CRASSERT(slot >= 0);
	CRASSERT(slot < replicate_spu.numContexts);
	CRASSERT(thread);

	context = &(replicate_spu.context[slot]);

	replicatespuFlush( (void *)thread );

	thread->broadcast = 0;

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {

		if (replicate_spu.rserver[i].conn == NULL || replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
				continue;
		thread->server.conn = replicate_spu.rserver[i].conn;


		if (replicate_spu.swap)
			crPackDestroyContextSWAP( context->rserverCtx[i] );
		else
			crPackDestroyContext( context->rserverCtx[i] );

		replicatespuFlush( (void *)thread );
	}

	thread->server.conn = replicate_spu.rserver[0].conn;
	thread->broadcast = 1;

	crStateDestroyContext( context->State );

	context->State = NULL;
	context->serverCtx = 0;
	/* Although we only allocate a display list manager once,
	 * we free it every time; this is okay since the DLM itself
	 * will track its uses and will only release the resources
	 * when the last user has relinquished it.
	 */
	crDLMFreeDLM(context->displayListManager);
	crDLMFreeContext(context->dlmState);
	context->displayListManager = NULL;
	context->dlmState = NULL;

	if (thread->currentContext == context) {
		thread->currentContext = NULL;
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState(NULL);
	}
}


void REPLICATESPU_APIENTRY replicatespu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GLint *rserverCtx;
	ContextInfo *newCtx;
	unsigned int i;
	unsigned int show_window = 0;
	WindowInfo *winInfo = (WindowInfo *) crHashtableSearch( replicate_spu.windowTable, window );
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

	replicatespuStartVnc( );

	if (ctx) {
		const int slot = ctx - MAGIC_OFFSET;

		CRASSERT(slot >= 0);
		CRASSERT(slot < replicate_spu.numContexts);

		newCtx = &replicate_spu.context[slot];

		newCtx->currentWindow = winInfo->id;

		if (replicate_spu.glx_display && winInfo && winInfo->nativeWindow != nativeWindow) {
			/* tell VNC to monitor this window id, for window moves */
			XVncChromiumMonitor( replicate_spu.glx_display, winInfo->id, nativeWindow );
			winInfo->nativeWindow = nativeWindow;

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

	thread->broadcast = 0;

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {

		if (replicate_spu.rserver[i].conn == NULL || replicate_spu.rserver[i].conn->type == CR_NO_CONNECTION)
				continue;
		thread->server.conn = replicate_spu.rserver[i].conn;


		if (replicate_spu.swap)
			crPackMakeCurrentSWAP( winInfo->id, nativeWindow, rserverCtx[i] );
		else
			crPackMakeCurrent( winInfo->id, nativeWindow, rserverCtx[i] );

		replicatespuFlush( (void *)thread );
	}

	thread->server.conn = replicate_spu.rserver[0].conn;
	thread->broadcast = 1;

	if (ctx) {
		crStateMakeCurrent( newCtx->State );
		crDLMSetCurrentState(newCtx->dlmState);
	}
	else {
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState(NULL);
	}

	thread->currentContext = newCtx;

	if (show_window) {
		/* We may find that the window was mapped before we
		 * called MakeCurrent, if that's the case then ensure
		 * the remote end gets the WindowShow event */
		if (winInfo->viewable) 
		{
			if (replicate_spu.swap)
				crPackWindowShowSWAP( winInfo->id, GL_TRUE );
			else
				crPackWindowShow( winInfo->id, GL_TRUE );
		}

		replicatespuFlush( (void *)thread );
	}

	{
		GET_THREAD(t);
		(void) t;
		CRASSERT(t);
	}
}
