/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <assert.h>
#include "packspu.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "cr_string.h"

#define MAGIC_OFFSET 3000


/*
 * Allocate a new ThreadInfo structure, setup a connection to the
 * server, allocate/init a packer context, bind this ThreadInfo to
 * the calling thread with crSetTSD().
 * We'll always call this function at least once even if we're not
 * using threads.
 */
ThreadInfo *packspuNewThread( unsigned long id )
{
	ThreadInfo *thread;

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_PackMutex);
#else
	CRASSERT(pack_spu.numThreads == 0);
#endif

	CRASSERT(pack_spu.numThreads < MAX_THREADS);
	thread = &(pack_spu.thread[pack_spu.numThreads]);

	thread->id = id;
	thread->currentContext = NULL;

	/* connect to the server */
	thread->server.name = crStrdup( pack_spu.name );
	thread->server.buffer_size = pack_spu.buffer_size;
	if (pack_spu.numThreads == 0) {
		packspuConnectToServer( &(thread->server) );
		CRASSERT(thread->server.conn);
		pack_spu.swap = thread->server.conn->swap;
	}
	else {
		/* a new pthread */
		crNetNewClient( pack_spu.thread[0].server.conn, &(thread->server));
		CRASSERT(thread->server.conn);
	}

	/* packer setup */
	CRASSERT(thread->packer == NULL);
	thread->packer = crPackNewContext( pack_spu.swap );
	crPackInitBuffer( &(thread->buffer), crNetAlloc(thread->server.conn),
					  thread->server.buffer_size, 0 );
	crPackSetBuffer( thread->packer, &thread->buffer );
	crPackFlushFunc( thread->packer, packspuFlush );
	crPackFlushArg( thread->packer, (void *) thread );
	crPackSendHugeFunc( thread->packer, packspuHuge );
	crPackSetContext( thread->packer );

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&_PackTSD, thread);
#endif

	pack_spu.numThreads++;

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_PackMutex);
#endif
	return thread;
}


GLint PACKSPU_APIENTRY packspu_CreateContext( const char *dpyName, GLint visual )
{
	int writeback = pack_spu.thread[0].server.conn->type == CR_DROP_PACKETS ? 0 : 1;
	GLint serverCtx = (GLint) -1;
	int slot;
#if 0
	/* I don't know what to do with this - Brian */

	GLint return_val = (GLint) 5000; /* HUMUNGOUS HACK TO MATCH SERVER NUMBERING */

	/* The hack exists solely to make file networking work for now.  This is
	 * totally gross, but since the server expects the numbers to start from 5000,
	 * we need to write them out this way.  This would be marginally less gross
	 * if the numbers (500 and 5000) were maybe some sort of #define'd constants
	 * somewhere so the client and the server could be aware of how each other
	 * were numbering things in cases like file networking where they actually
	 * care. 
	 *
	 * 	-Humper 
	 *
	 * 	*/
#endif

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_PackMutex);
#endif

	/* Pack the command */
	if (pack_spu.swap)
		crPackCreateContextSWAP( dpyName, visual, &serverCtx, &writeback );
	else
		crPackCreateContext( dpyName, visual, &serverCtx, &writeback );

	/* Flush buffer and get return value */
	packspuFlush( &(pack_spu.thread[0]) );
	if (pack_spu.thread[0].server.conn->type != CR_FILE)
	{
		while (writeback)
			crNetRecv();
	}

	if (pack_spu.swap) {
		serverCtx = (GLint) SWAP32(serverCtx);
	}
	if (serverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&_PackMutex);
#endif
		return -1;  /* failed */
	}

	/* find an empty context slot */
	for (slot = 0; slot < pack_spu.numContexts; slot++) {
		if (!pack_spu.context[slot].clientState) {
			/* found empty slot */
			break;
		}
	}
	if (slot == pack_spu.numContexts) {
		pack_spu.numContexts++;
	}

	/* Fill in the new context info */
	pack_spu.context[slot].clientState = crStateCreateContext(&pack_spu.limits);
	pack_spu.context[slot].serverCtx = serverCtx;

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_PackMutex);
#endif

	return MAGIC_OFFSET + slot;
}


void PACKSPU_APIENTRY packspu_DestroyContext( GLint ctx )
{
	const int slot = ctx - MAGIC_OFFSET;
	ContextInfo *context;
	GET_THREAD(thread);

	CRASSERT(slot >= 0);
	CRASSERT(slot < pack_spu.numContexts);
	CRASSERT(thread);

	context = &(pack_spu.context[slot]);

	if (pack_spu.swap)
		crPackDestroyContextSWAP( context->serverCtx );
	else
		crPackDestroyContext( context->serverCtx );

	/* Deallocate this context slot */
	crFree(context->clientState);
	context->clientState = NULL;
	context->serverCtx = 0;

	if (thread->currentContext == context) {
		thread->currentContext = NULL;
		crPackSetContext( NULL );
		crStateMakeCurrent( NULL );
	}
}


void PACKSPU_APIENTRY packspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GET_THREAD(thread);
	GLint serverCtx;
	ContextInfo *newCtx;

	if (!thread) {
		thread = packspuNewThread( crThreadID() );
	}
	CRASSERT(thread);
	CRASSERT(thread->packer);

	if (ctx) {
		const int slot = ctx - MAGIC_OFFSET;

		CRASSERT(slot >= 0);
		CRASSERT(slot < pack_spu.numContexts);

		newCtx = &pack_spu.context[slot];
		CRASSERT(newCtx->clientState);  /* verify valid */

		thread->currentContext = newCtx;

		crPackSetContext( thread->packer );
		crStateMakeCurrent( newCtx->clientState );
		serverCtx = pack_spu.context[slot].serverCtx;
	}
	else {
		thread->currentContext = NULL;
		crPackSetContext( NULL );
		crStateMakeCurrent( NULL );
		newCtx = NULL;
		serverCtx = 0;
	}

	if (pack_spu.swap)
		crPackMakeCurrentSWAP( window, nativeWindow, serverCtx );
	else
		crPackMakeCurrent( window, nativeWindow, serverCtx );

	{
		GET_THREAD(t);
		CRASSERT(t);
	}
}


extern void PACKSPU_APIENTRY packspu_GetIntegerv( GLenum pname, GLint *params );

void PACKSPU_APIENTRY packspu_Finish( void )
{
#if 0
	GET_THREAD(thread);
	if (pack_spu.swap)
	{
		crPackFinishSWAP(  );
	}
	else
	{
		crPackFinish(  );
	}
	packspuFlush( pack_spu, (void *) thread );
#else
	/* Finish should not return until the pipeline has been flushed and
	 * rendering is completed.  Accomplish this with a round-trip command
	 * such as glGetIntegerv.
	 */
	GLint k;
	packspu_GetIntegerv(GL_BLEND, &k);
#endif
}
