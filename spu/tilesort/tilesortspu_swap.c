/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"


/**
 * Implementation of SwapBuffers for tilesorter
 *
 * \param window
 * \param flags
 */
void TILESORTSPU_APIENTRY tilesortspu_SwapBuffers( GLint window, GLint flags )
{
	GET_THREAD(thread);
	int writeback = tilesort_spu.num_servers;
	WindowInfo *winInfo;
	int serverWindow;

	tilesortspuFlush( thread );

	winInfo = tilesortspuGetWindowInfo(window, 0);
	CRASSERT(winInfo);
	/* NOTE: winInfo->server[n].window should be the same for all n! */
	serverWindow = winInfo->server[0].window;

	/* Here's where we force quad-buffered stereo rendering.
	 * We use an X trick to make the app draw each frame twice.
	 */
	if (winInfo->forceQuadBuffering) {
		if (winInfo->parity == 0) {
			/* we're swapping after drawing an even-numbered frame.
			 * Even frames are for left eye, odd frames are for right eye.
			 * Now, switch draw buffer to the right.
			 */
			/*
			crDebug("Swap: done with left.  Send expose to 0x%x",
							(int) winInfo->xwin);
			*/
			tilesortspu_DrawBuffer(GL_BACK_RIGHT);
			/*
			 * Trigger drawing the right view by sending an expose event to
			 * the app to trick it into redrawing.
			 */
#ifdef WINDOWS
			/** XXX \todo is there a Window equivalent here??? */
#elif defined(DARWIN)
			/** XXX \todo is there a Darwin equivalent here??? */
#else
			CRASSERT(winInfo->dpy);
			CRASSERT(winInfo->xwin);
			XClearArea( winInfo->dpy, winInfo->xwin, 0, 0,
				    winInfo->lastWidth, winInfo->lastHeight,
				    True );
			XSync(winInfo->dpy, 0);
#endif
		}
		else
		{
			/*
			crDebug("Swap: done with right, now left");
			*/
			/* We're swapping after drawing an odd frame.
			 * Now, switch draw buffer back to the left.
			 */
			tilesortspu_DrawBuffer(GL_BACK_LEFT);
		}
		/* flip parity bit for next frame */
		winInfo->parity = !winInfo->parity;

		/* only swap prior to even (left) frames */
		if (winInfo->parity)
			return;
	}

	if (tilesort_spu.swap)
		crPackSwapBuffersSWAP( serverWindow, flags );
	else
		crPackSwapBuffers( serverWindow, flags );

	if (tilesort_spu.syncOnSwap)
	{
		if (tilesort_spu.swap)
			crPackWritebackSWAP( &writeback );
		else
			crPackWriteback( &writeback );
	}

	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();

	if (tilesort_spu.syncOnSwap)
	{
		while(writeback)
			crNetRecv();
	}

	/* want to emit a parameteri here */
	if (tilesort_spu.emit_GATHER_POST_SWAPBUFFERS)
	{
		if (tilesort_spu.swap)
			crPackChromiumParameteriCRSWAP(GL_GATHER_POST_SWAPBUFFERS_CR, serverWindow);
		else
			crPackChromiumParameteriCR(GL_GATHER_POST_SWAPBUFFERS_CR, serverWindow);
	}
}
