/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "chromium.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


void SERVER_DISPATCH_APIENTRY crServerDispatchClear( GLenum mask )
{
	const RunQueue *q = run_queue;

	if (cr_server.only_swap_once)
	{
		/* NOTE: we only do the clear for the _last_ client in the list.
		 * This is because in multi-threaded apps the zeroeth client may
		 * be idle and never call glClear at all.  See threadtest.c
		 * It's pretty likely that the last client will be active.
		 */
		if ((mask & GL_COLOR_BUFFER_BIT) &&
			(cr_server.curClient != &(cr_server.clients[cr_server.numClients - 1])))
		   return;
	}

	if (cr_server.numExtents == 0)
	{
		cr_server.head_spu->dispatch_table.Clear( mask );
	}
	else
	{
		int scissor_on, i;

		scissor_on = q->client->currentCtx->viewport.scissorTest;

		if (!scissor_on)
		{
			cr_server.head_spu->dispatch_table.Enable( GL_SCISSOR_TEST );
		}

		for ( i = 0; i < q->numExtents; i++ )
		{
			const CRRunQueueExtent *extent = &q->extent[i];
			crServerSetOutputBounds( q->client->currentCtx, &extent->outputwindow,
					&q->imagespace, &extent->imagewindow );
			cr_server.head_spu->dispatch_table.Clear( mask );
		}
		if (!scissor_on)
		{
			cr_server.head_spu->dispatch_table.Disable( GL_SCISSOR_TEST );
		}
	}
}


void SERVER_DISPATCH_APIENTRY crServerDispatchSwapBuffers( GLint window, GLint flags )
{
	if (cr_server.only_swap_once)
	{
		/* NOTE: we only do the clear for the _last_ client in the list.
		 * This is because in multi-threaded apps the zeroeth client may
		 * be idle and never call glClear at all.  See threadtest.c
		 * It's pretty likely that the last client will be active.
		 */
		if (cr_server.curClient != &(cr_server.clients[cr_server.numClients - 1]))
		{
			return;
		}
	}
	cr_server.head_spu->dispatch_table.SwapBuffers( window, flags );
}
