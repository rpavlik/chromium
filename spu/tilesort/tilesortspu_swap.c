/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_SwapBuffers(void)
{
	int writeback = tilesort_spu.num_servers;
	tilesortspuFlush( tilesort_spu.ctx );
	crPackSwapBuffers( );
	if (tilesort_spu.syncOnSwap)
	{
		crPackWriteback( &writeback );
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
	if (tilesort_spu.syncOnSwap)
	{
		while(writeback)
		{
			crNetRecv();
		}
	}
}
