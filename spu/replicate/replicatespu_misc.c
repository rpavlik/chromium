/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_mem.h"
#include "replicatespu.h"
#include "replicatespu_proto.h"

void REPLICATESPU_APIENTRY replicatespu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{

	CRMessage msg;
	int len;
	
	GET_THREAD(thread);

	
	switch(target)
	{
		case GL_GATHER_PACK_CR:
			/* flush the current pack buffer */
			replicatespuFlush( (void *) thread );

			/* the connection is thread->server.conn */
			msg.header.type = CR_MESSAGE_GATHER;
			msg.gather.offset = 69;
			len = sizeof(CRMessageGather);
			crNetSend(thread->server.conn, NULL, &msg, len);
			break;
			
		default:
			if (replicate_spu.swap)
				crPackChromiumParametervCRSWAP(target, type, count, values);
			else
				crPackChromiumParametervCR(target, type, count, values);
	}
}

GLint REPLICATESPU_APIENTRY replicatespu_RenderMode( GLenum mode )
{
	/* ignore this, use the feedbackSPU if needed */
	return 0;
}

void REPLICATESPU_APIENTRY replicatespu_Finish( void )
{
	int writeback;
	unsigned int i;
	GET_THREAD(thread);

	replicatespuFlush( (void *) thread );

	thread->broadcast = 0;

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {
		writeback = 0;
		if (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION) {
			thread->server.conn = replicate_spu.rserver[i].conn;
			writeback = 1;

			if (replicate_spu.swap)
			{
				crPackFinishSWAP(  );
				crPackWritebackSWAP( &writeback );
			}
			else
			{
				crPackFinish(  );
				crPackWriteback( &writeback );
			}
			replicatespuFlush( (void *) thread );
			while (writeback)
				crNetRecv();
		}
	}

	thread->server.conn = replicate_spu.rserver[0].conn;

	thread->broadcast = 1;
}


GLint REPLICATESPU_APIENTRY replicatespu_WindowCreate( const char *dpyName, GLint visBits )
{
	unsigned int i;
	int writeback;
	GLint return_val;
	static GLint freeWinID = 400;
	WindowInfo *winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	GET_THREAD(thread);
#if 0
	ThreadInfo *thread = &(replicate_spu.thread[0]);
#endif

	if (thread)
		replicatespuFlush( (void *) thread );

	if (!thread)
		thread = replicatespuNewThread( crThreadID() );

	if (!winInfo)
		return -1;

	replicatespuFlush( (void *) thread );
	crPackSetContext( thread->packer );

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_ReplicateMutex);
#endif

	thread->broadcast = 0;

	for (i = 0; i < CR_MAX_REPLICANTS; i++) {
		writeback = 0;
		return_val = -1;

		if (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION) {

			thread->server.conn = replicate_spu.rserver[i].conn;
			writeback = 1;

			if (replicate_spu.swap)
			{
				crPackWindowCreateSWAP( dpyName, visBits, &return_val, &writeback );
			}
			else
			{
				crPackWindowCreate( dpyName, visBits, &return_val, &writeback );
			}
			replicatespuFlush( (void *) thread );

			while (writeback)
				crNetRecv();

			if (replicate_spu.swap)
			{
				return_val = (GLint) SWAP32(return_val);
			}

			if (i == 0) {
			winInfo->id = return_val;
			winInfo->visBits = visBits;
			winInfo->width = 0;
			winInfo->height = 0;
			winInfo->nativeWindow = 0;

			crHashtableAdd(replicate_spu.windowTable, freeWinID, winInfo);
			}
		}
	}

	thread->broadcast = 1;
	thread->server.conn = replicate_spu.rserver[0].conn;

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_ReplicateMutex);
#endif

	if (winInfo->id != -1)
		return freeWinID++;
	else
		return -1;
}

void REPLICATESPU_APIENTRY replicatespu_WindowSize( GLint win, GLint w, GLint h )
{
	WindowInfo *winInfo = (WindowInfo *) crHashtableSearch( replicate_spu.windowTable, win );
	GET_THREAD(thread);

	winInfo->width = w;
	winInfo->height = h;

	if (replicate_spu.swap)
	{
		crPackWindowSizeSWAP( win, w, h );
	}
	else
	{
		crPackWindowSize( win, w, h );
	}
	replicatespuFlush( (void *) thread );
}
