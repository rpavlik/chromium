/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_applications.h"

RunQueue *run_queue = NULL;

void crServerAddToRunQueue( int index )
{
	CRClient *client = cr_server.clients + index;
	RunQueue *q = (RunQueue *) crAlloc( sizeof( *q ) );
	int i;
	int x, y, w, h, y_max;

	crDebug( "Adding to the run queue: %d", index );
	q->client = client;
	q->blocked = 0;

	x = cr_server.useL2 ? 2 : 0;
	y = 0;
	y_max = 0;

	q->imagespace.x1 = 0;
	q->imagespace.y1 = 0;
	q->imagespace.x2 = cr_server.muralWidth;
	q->imagespace.y2 = cr_server.muralHeight;

	q->numExtents = cr_server.numExtents;

	for ( i = 0; i < q->numExtents; i++ )
	{
		CRRunQueueExtent *extent = &q->extent[i];

		extent->imagewindow.x1 = cr_server.x1[i];
		extent->imagewindow.y1 = cr_server.y1[i];
		extent->imagewindow.x2 = cr_server.x2[i];
		extent->imagewindow.y2 = cr_server.y2[i];

		/* extent->display = find_output_display( extent->imagewindow ); */

		extent->bounds.x1 = ( (GLfloat) (2*extent->imagewindow.x1) /
				cr_server.muralWidth - 1.0f );
		extent->bounds.y1 = ( (GLfloat) (2*extent->imagewindow.y1) /
				cr_server.muralHeight - 1.0f );
		extent->bounds.x2 = ( (GLfloat) (2*extent->imagewindow.x2) /
				cr_server.muralWidth - 1.0f );
		extent->bounds.y2 = ( (GLfloat) (2*extent->imagewindow.y2) /
				cr_server.muralHeight - 1.0f );

		w = cr_server.x2[i] - cr_server.x1[i];
		h = cr_server.y2[i] - cr_server.y1[i];

		if ( x + w > (int) cr_server.underlyingDisplay[2] )
		{
			y += y_max;
			x = ((cr_server.useL2) ? 2 : 0);
			y_max = 0;
		}

		extent->outputwindow.x1 = x;
		extent->outputwindow.y1 = ( cr_server.underlyingDisplay[3] - cr_server.maxTileHeight - y );
		extent->outputwindow.x2 = x + w;
		extent->outputwindow.y2 = ( cr_server.underlyingDisplay[3] - cr_server.maxTileHeight - y + h );

		if ( y_max < h )
		{
			y_max = h;
		}

		x += w + ((cr_server.useL2) ? 2 : 0);
	}

	if (!run_queue)
	{
		run_queue = q;
		q->next = q;
		q->prev = q;
	}
	else
	{
		q->next = run_queue->next;
		run_queue->next->prev = q;

		q->prev = run_queue;
		run_queue->next = q;
	}
}

static RunQueue *__getNextClient(void)
{
	for (;;)
	{
		if (run_queue) 
		{
			int all_blocked = 1;
			int done_something = 0;
			RunQueue *start = run_queue;
			while (!done_something || run_queue != start)
			{
				done_something = 1;
				if (!run_queue->blocked)
				{
					all_blocked = 0;
				}
				if (!run_queue->blocked && run_queue->client->conn->messageList)
				{
					return run_queue;
				}
				run_queue = run_queue->next;
			}

			if (all_blocked)
			{
				crError( "DEADLOCK!" );
			}
		}
		/* no one had any work, get some! */
		crNetRecv();
	}
	/* UNREACHED */
	/* return NULL; */
}

void crServerSerializeRemoteStreams(void)
{
	for (;;)
	{
		RunQueue *q = __getNextClient();
		CRClient *client = q->client;
		CRMessage *msg;
		/*char debug_buf[8096]; */

		cr_server.curClient = client;
		/*sprintf( debug_buf, "     ---- Switching contexts to connection 0x%p ----", client->conn ); 
		 *cr_server.dispatch.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf ); */
#if 00
		crStateMakeCurrent( client->ctx );
#else
		if (client->currentCtx)
			crStateMakeCurrent( client->currentCtx );
#endif
		for( ;; )
		{
			CRMessageOpcodes *msg_opcodes;
			char *data_ptr;

			(void) crNetGetMessage( cr_server.curClient->conn, &msg );
			if (msg->header.type != CR_MESSAGE_OPCODES)
			{
				crError( "SPU %d sent me CRAP (type=0x%x)", client->spu_id, msg->header.type );
			}

			msg_opcodes = (CRMessageOpcodes *) msg;
			/*if (!q->blocked) 
			 *{ 
			 *sprintf( debug_buf, "     ---- Packet! (0x%p) ! ----", msg_opcodes ); 
			 *cr_server.dispatch.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf ); 
			 *} */
			data_ptr = (char *) msg_opcodes + sizeof( *msg_opcodes) + ((msg_opcodes->numOpcodes + 3 ) & ~0x03);
			crUnpack( data_ptr, data_ptr-1, msg_opcodes->numOpcodes, &(cr_server.dispatch) );
			crNetFree( cr_server.curClient->conn, msg );
			if (q->blocked)
			{
				break;
			}
		}
		run_queue = run_queue->next;
	}
}

int crServerRecv( CRConnection *conn, void *buf, unsigned int len )
{
	(void) conn;
	(void) buf;
	(void) len;
	return 0; /* Never handle anything -- let packets queue up per-client instead. */
}
