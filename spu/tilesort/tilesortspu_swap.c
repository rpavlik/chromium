/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"

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

#if 0
	/* debug code */
	{
		int i;
		for (i = 1; i < tilesort_spu.num_servers; i++) {
			 if (winInfo->server[i].window != winInfo->server[0].window) {
					crDebug("Different window IDs: %d != %d",
									winInfo->server[i].window, winInfo->server[0].window);
			 }
		}
	}
#endif

	if (tilesort_spu.swap)
	{
		crPackSwapBuffersSWAP( serverWindow, flags );
	}
	else
	{
		crPackSwapBuffers( serverWindow, flags );
	}
	if (tilesort_spu.syncOnSwap)
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
	if (tilesort_spu.syncOnSwap)
	{
		while(writeback)
		{
			crNetRecv();
		}

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
