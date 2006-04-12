/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "state/cr_stateerror.h"

/**
 * Implementation of glAccum for tilesorter
 * \param op
 * \param value
 */
void TILESORTSPU_APIENTRY tilesortspu_Accum( GLenum op, GLfloat value )
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE) {
		if (tilesort_spu.lazySendDLists)
			crDLMCompileAccum(op, value);
		else if (tilesort_spu.swap)
			crPackAccumSWAP( op, value );
		else
			crPackAccum( op, value );
		return;
	}

	tilesortspuFlush( thread );

	if (tilesort_spu.swap)
		crPackAccumSWAP( op, value );
	else
		crPackAccum( op, value );

	tilesortspuBroadcastGeom(GL_TRUE);
}

/**
 * Implementation of glClean for tilesorter
 * \param mask
 */
void TILESORTSPU_APIENTRY tilesortspu_Clear( GLbitfield mask )
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	WindowInfo *winInfo = thread->currentContext->currentWindow;

	/* This is a good place to check for new tiling geometry when
	 * the DMX window is changed.
	 */
#ifdef USE_DMX
	if (tilesort_spu.trackWindowPosition && !dlMode) {
		if (winInfo->isDMXWindow && winInfo->xwin) {
			if (tilesortspuUpdateWindowInfo(winInfo)) {
				tilesortspuGetNewTiling(winInfo);
			}
		}
	}

	if (winInfo->newBackendWindows) {
		tilesortspuGetNewTiling(winInfo);
	}
#endif

	if (dlMode != GL_FALSE) {
		/* just creating and/or compiling display lists */
		if (tilesort_spu.lazySendDLists)
			crDLMCompileClear(mask);
		else if (tilesort_spu.swap)
			crPackClearSWAP(mask);
		else
			crPackClear(mask);
		return;
	}

	if (winInfo->passiveStereo) {
		/* only send Clear to left/right servers */
		int i;

		tilesortspuFlush( thread );

		crPackReleaseBuffer( thread->packer );

		/* Send glClear command to those servers designated as left/right
		 * which match the current glDrawBuffer setting (stereo).
		 */
		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			const ServerWindowInfo *servWinInfo = winInfo->server + i;

			if (servWinInfo->eyeFlags & thread->currentContext->stereoDestFlags) {
				crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

				if (tilesort_spu.swap)
					crPackClearSWAP(mask);
				else
					crPackClear(mask);

				crPackReleaseBuffer( thread->packer );

				tilesortspuSendServerBuffer( i );
			}
		}

		/* Restore the default pack buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
	}
	else {
		/* not doing stereo, truly broadcast glClear */
		tilesortspuFlush( thread );
		if (tilesort_spu.swap)
			crPackClearSWAP( mask );
		else
			crPackClear( mask );
		tilesortspuBroadcastGeom(GL_TRUE);
	}
}


/**
 * Implementation of glFinish for tilesorter
 */
void TILESORTSPU_APIENTRY tilesortspu_Finish(void)
{
	GET_THREAD(thread);
	GLint writeback = tilesort_spu.num_servers;
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackFinishSWAP( );
	}
	else
	{
		crPackFinish( );
	}
	if (tilesort_spu.syncOnFinish)
	{
		if (tilesort_spu.swap)
		{
			crPackWritebackSWAP( &writeback );
		}
		else
		{
			crPackWriteback( &writeback );
		}
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
	if (tilesort_spu.syncOnFinish)
	{
		while(writeback)
		{
			crNetRecv();
		}
	}
}

/**
 * Implementation of glFlush for tilesorter
 */
void TILESORTSPU_APIENTRY tilesortspu_Flush(void)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackFlushSWAP();
	}
	else
	{
		crPackFlush();
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

/**
 * Create a barrier with the given name and count.  The count indicates
 * how many clients must meet at the barrier before it is released.
 * If count is zero, the crserver will automatically set count to the number
 * of clients currently connected to the server.
 */
void TILESORTSPU_APIENTRY tilesortspu_BarrierCreateCR(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_BarrierCreateCR called during DL compilation");
	    return;
	}

	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierCreateCRSWAP(name,count);
	}
	else
	{
		crPackBarrierCreateCR(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

/**
 * Destroys the named barrier.
 * \param name barrier name
 */
void TILESORTSPU_APIENTRY tilesortspu_BarrierDestroyCR(GLuint name)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_BarrierDestroyCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierDestroyCRSWAP(name);
	}
	else
	{
		crPackBarrierDestroyCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

/**
 * Causes the caller to wait on the barrier until all peers meet the
 * barrier.
 * \param name  the semaphore ID
 */
void TILESORTSPU_APIENTRY tilesortspu_BarrierExecCR(GLuint name)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_BarrierExecCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierExecCRSWAP(name);
	}
	else
	{
		crPackBarrierExecCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

/**
 * Creates a semaphore with the given name and value.  Value indicates
 * the initial semaphore value.
 * \param name  the semaphore ID
 * \param count  the initial semaphore count
 */
void TILESORTSPU_APIENTRY tilesortspu_SemaphoreCreateCR(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_SemaphoreCreateCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreCreateCRSWAP(name,count);
	}
	else
	{
		crPackSemaphoreCreateCR(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

/**
 * Destroys the named semaphore.
 * \param name
 */
void TILESORTSPU_APIENTRY tilesortspu_SemaphoreDestroyCR(GLuint name)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_SemaphoreDestroyCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreDestroyCRSWAP(name);
	}
	else
	{
		crPackSemaphoreDestroyCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}


/**
 * Decrements the named semaphore.  This is the <em>wait</em> operation.
 * \param name
 */
void TILESORTSPU_APIENTRY tilesortspu_SemaphorePCR(GLuint name)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_SemaphorePCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphorePCRSWAP(name);
	}
	else
	{
		crPackSemaphorePCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}


/**
 * Increments the named semaphore.  This is the <em>signal</em> operation.
 * \param name
 */
void TILESORTSPU_APIENTRY tilesortspu_SemaphoreVCR(GLuint name)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	/* check if we're compiling a display list, if so, error! */
	if (dlMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
		"tilesortspu_SemaphoreVCR called during DL compilation");
	    return;
	}
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreVCRSWAP(name);
	}
	else
	{
		crPackSemaphoreVCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}




/**
 * Need to delete local textures and textures on servers.
 * XXX \todo this function should probably go elsewhere.
 * \param n
 * \param textures
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteTextures(GLsizei n, const GLuint *textures)
{
	GET_THREAD(thread);
	tilesortspuFlush(thread);
	crStateDeleteTextures(n, textures);
	if (tilesort_spu.swap)
		crPackDeleteTexturesSWAP(n, textures);
	else
		crPackDeleteTextures(n, textures);
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}


/**
 * Need to delete local programs and programs on servers.
 * XXX \todo this function should probably go elsewhere.
 * \param n
 * \param programs
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteProgramsARB(GLsizei n, const GLuint *programs)
{
	GET_THREAD(thread);
	tilesortspuFlush(thread);
	crStateDeleteProgramsARB(n, programs);
	if (tilesort_spu.swap)
		crPackDeleteProgramsARBSWAP(n, programs);
	else
		crPackDeleteProgramsARB(n, programs);
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}


/**
 * Need to delete local buffers and buffers on servers.
 * XXX \todo this function should probably go elsewhere.
 * \param n
 * \param buffers
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
	GET_THREAD(thread);
	tilesortspuFlush(thread);
	crStateDeleteBuffersARB(n, buffers);
	if (tilesort_spu.swap)
		crPackDeleteBuffersARBSWAP(n, buffers);
	else
		crPackDeleteBuffersARB(n, buffers);
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}
