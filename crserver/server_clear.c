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
	RunQueue *q = run_queue;
	if (cr_server.numExtents == 0)
	{
		cr_server.head_spu->dispatch_table.Clear( mask );
	}
	else
	{
		int scissor_on = 0;
		int i;

		if (q->client->ctx->viewport.scissorTest)
		{
			scissor_on = 1;
		}

		if (!scissor_on)
		{
			cr_server.head_spu->dispatch_table.Enable( GL_SCISSOR_TEST );
		}

		for ( i = 0; i < q->numExtents; i++ )
		{
			CRRunQueueExtent *extent = &q->extent[i];
			crServerSetOutputBounds( q->client->ctx, &extent->outputwindow,
					&q->imagespace, &extent->imagewindow );
			cr_server.head_spu->dispatch_table.Clear( mask );
		}
		if (!scissor_on)
		{
			cr_server.head_spu->dispatch_table.Disable( GL_SCISSOR_TEST );
		}
	}
}
