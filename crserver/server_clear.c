/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


void SERVER_DISPATCH_APIENTRY crServerDispatchClear( GLenum mask )
{
	const RunQueue *q = run_queue;

	/* If the GL_SINGLE_CLIENT_BIT_CR bit is present we'll only execute
	 * this glClear() for the 0th client.
	 */
	if ((mask & GL_SINGLE_CLIENT_BIT_CR) && q->client->number != 0)
		return;

	/* We don't really have to strip off this bit here since the render
	 * SPU will do it.  But play it safe.
	 */
	mask &= ~GL_SINGLE_CLIENT_BIT_CR;

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


void SERVER_DISPATCH_APIENTRY crServerDispatchSwapBuffers( void )
{
	const RunQueue *q = run_queue;

	/* We only do SwapBuffers for the 0th client */
	if (q->client->number != 0)
		return;

	cr_server.head_spu->dispatch_table.SwapBuffers();
}
