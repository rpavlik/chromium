/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_unpack.h"
#include "cr_error.h"
#include "cr_mem.h"

#if 0
static int QueueSize( void )
{
		RunQueue *q = cr_server.run_queue;
		RunQueue *qStart = cr_server.run_queue;
		int count = 0;

		do {
				count++;
				q = q->next;
		} while (q != qStart);
		return count;
}
#endif

/*
 * Add another client to the server's list.
 * XXX remove similar, redundant code in server_config.c
 */
static void crServerAddNewClient( void )
{
	CRClient *newClient;
	int n;

	/* Save current client number */
	n = cr_server.curClient->number;

	/* This messes up all our pointers..... So we need to fix them up.. */
	crRealloc((void **)&cr_server.clients, sizeof(*cr_server.clients) * (cr_server.numClients + 1));

	/* Fix up our current client with the realloc'ed data */
	cr_server.curClient = &cr_server.clients[n];

	/* now we can allocate our new client data */
	newClient = &(cr_server.clients[cr_server.numClients]);
	crMemZero(newClient, sizeof(CRClient));

	/* 
	 * Because we've Realloced the client data above, the run_queue list
 	 * is now void of all pointer information. We need to re-init
	 * the run_queue's client pointers here.
	 */
	{
		RunQueue *q = cr_server.run_queue;
		RunQueue *qStart = cr_server.run_queue;
		do {
			q->client = &cr_server.clients[q->number];
			q = q->next;
		} while (q != qStart);
	}

	newClient->number = cr_server.numClients;
	newClient->spu_id = cr_server.clients[0].spu_id;
	newClient->conn = crNetAcceptClient( cr_server.protocol, NULL, cr_server.tcpip_port, cr_server.mtu, 1 );

	crServerAddToRunQueue( newClient );
	if (cr_server.numExtents > 0)
		 crServerRecomputeBaseProjection( &(newClient->baseProjection), 0, 0, cr_server.muralWidth, cr_server.muralHeight );

	cr_server.numClients++;
}

/*
 * Each client has an entry in the run queue (a RunQueue object).
 * This function initialized the per-client tiling information
 * for the client's queue entry.
 * We basically loop over the RunQueue's extent[] array (which gives
 * the bounds of each mural tile) and carve out a space for it in the
 * 'underlying' window.  We carve/allocate in simple top-to-bottom
 * raster order.
 */
void crServerInitializeQueueExtents(RunQueue *q)
{
	int i;
	int x, y, w, h, y_max;

	x = cr_server.useL2 ? 2 : 0;
	y = 0;
	y_max = 0;

	/* the queue's image space is the whole mural space */
	q->imagespace.x1 = 0;
	q->imagespace.y1 = 0;
	q->imagespace.x2 = cr_server.muralWidth;
	q->imagespace.y2 = cr_server.muralHeight;

	q->numExtents = cr_server.numExtents;

	/* Basically just copy the server's list of tiles to the RunQueue
	 * and compute some derived tile information.
	 */
	for ( i = 0; i < q->numExtents; i++ )
	{
		CRRunQueueExtent *extent = &q->extent[i];

		extent->imagewindow = cr_server.extents[i]; /* x1,y1,x2,y2 */

		/* extent->display = find_output_display( extent->imagewindow ); */

		/* Compute normalized tile bounds.
		 * That is, x1, y1, x2, y2 will be in the range [-1, 1] where
		 * x1=-1, y1=-1, x2=1, y2=1 corresponds to the whole mural.
		 */
		extent->bounds.x1 = ( (GLfloat) (2*extent->imagewindow.x1) /
				cr_server.muralWidth - 1.0f );
		extent->bounds.y1 = ( (GLfloat) (2*extent->imagewindow.y1) /
				cr_server.muralHeight - 1.0f );
		extent->bounds.x2 = ( (GLfloat) (2*extent->imagewindow.x2) /
				cr_server.muralWidth - 1.0f );
		extent->bounds.y2 = ( (GLfloat) (2*extent->imagewindow.y2) /
				cr_server.muralHeight - 1.0f );

		/* Width and height of tile, in mural pixels */
		w = cr_server.extents[i].x2 - cr_server.extents[i].x1;
		h = cr_server.extents[i].y2 - cr_server.extents[i].y1;

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

		if (extent->outputwindow.y1 < 0)
			crWarning("Ran out of room for tiles in this server's window!!!");

		cr_server.outputwindow[i] = extent->outputwindow; /* x1,y1,x2,y2 */

		if ( y_max < h )
		{
			y_max = h;
		}

		x += w + ((cr_server.useL2) ? 2 : 0);
	}
}


void crServerAddToRunQueue( CRClient *client )
{
	RunQueue *q = (RunQueue *) crAlloc( sizeof( *q ) );
	static int added = 0;

	added++;
	crDebug( "Adding to the run queue: client=%p number=%d count=%d",
					 (void *)client, client->number, added );

	q->number = client->number;
	q->client = client;
	q->blocked = 0;

	crServerInitializeQueueExtents(q);

	if (!cr_server.run_queue)
	{
		cr_server.run_queue = q;
		q->next = q;
		q->prev = q;
	}
	else
	{
		q->next = cr_server.run_queue->next;
		cr_server.run_queue->next->prev = q;

		q->prev = cr_server.run_queue;
		cr_server.run_queue->next = q;
	}
}

static void crServerDeleteFromRunQueue( CRClient *client )
{
	if (cr_server.run_queue)
	{
		RunQueue *q = cr_server.run_queue;
		RunQueue *qStart = cr_server.run_queue; 
		do {
			if (q->client == client)
			{
				crWarning( "Deleting from the run queue: client=%p number=%d",
								 client, client->number );
				if ((q->next == q->prev) && (q->next == q) && (cr_server.run_queue == q))
				{
					/* Only one client */
					crFree(q);
					cr_server.run_queue = NULL;
					crWarning("No RUN_QUEUE!! (quitting) \n");
				} 
				else
				{
					if (cr_server.run_queue == q)
						cr_server.run_queue = q->next;
					q->prev->next = q->next;
					q->next->prev = q->prev;
					crFree(q);
				}
				cr_server.numClients--;
				return;
			}
			q = q->next;
		} while (q != qStart);
	}
}


static RunQueue *__getNextClient(void)
{
	for (;;)
	{
		if (cr_server.run_queue) 
		{
			int all_blocked = 1;
			int done_something = 0;
			RunQueue *start = cr_server.run_queue;

 			if (!cr_server.run_queue->client->conn || cr_server.run_queue->client->conn->type == CR_NO_CONNECTION) {
 				crServerDeleteFromRunQueue( cr_server.run_queue->client );
				start = cr_server.run_queue;
			}
 
 			if (cr_server.run_queue == NULL)
 				return NULL;
 
			while (!done_something || cr_server.run_queue != start)
			{
				done_something = 1;
				if (!cr_server.run_queue->blocked)
				{
					all_blocked = 0;
				}
				if (!cr_server.run_queue->blocked && cr_server.run_queue->client->conn && cr_server.run_queue->client->conn->messageList)
				{
					return cr_server.run_queue;
				}
				cr_server.run_queue = cr_server.run_queue->next;
			}

			if (all_blocked)
			{
				crError( "DEADLOCK! (numClients=%d)\n", cr_server.numClients );
				if (cr_server.numClients < cr_server.maxBarrierCount) {
					crError("Waiting for more clients!!!\n");
					while (cr_server.numClients < cr_server.maxBarrierCount) {
						crNetRecv();
					}
				}
				/*
				crError( "DEADLOCK! (numClients=%d qSize=%d)", cr_server.numClients, QueueSize() );
				*/
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
		CRClient *client;
		CRMessage *msg;

		/* no more clients, quit */
		if (!q)
			return;

		client = q->client;

		cr_server.curClient = client;

		crStateMakeCurrent( client->currentCtx );

		for( ;; )
		{
			CRMessageOpcodes *msg_opcodes;
			char *data_ptr;
			unsigned int len;

			/* Don't use GetMessage, because it pulls stuff off
			 * the network too quickly */
			len = crNetPeekMessage( cr_server.curClient->conn, &msg );
			if (len == 0)
				break;

			if (msg->header.type != CR_MESSAGE_OPCODES)
			{
				crError( "SPU %d sent me CRAP (type=0x%x)", client->spu_id, msg->header.type );
			}

			msg_opcodes = (CRMessageOpcodes *) msg;
			data_ptr = (char *) msg_opcodes + 
				sizeof( *msg_opcodes) + 
				((msg_opcodes->numOpcodes + 3 ) & ~0x03);
			crUnpack( data_ptr, 
					data_ptr-1, 
					msg_opcodes->numOpcodes, 
					&(cr_server.dispatch) );
			crNetFree( cr_server.curClient->conn, msg );
			if (q->blocked)
			{
				break;
			}
		}
		cr_server.run_queue = cr_server.run_queue->next;
	}
}

int crServerRecv( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->header.type )
	{
		/* Called when using multiple threads */
		case CR_MESSAGE_NEWCLIENT:
			crServerAddNewClient();
			break;
		default:
			/*crWarning( "Why is the crserver getting a message of type 0x%x?", msg->header.type ); */
			return 0; /* NOT HANDLED */
	}
	(void) len;	
	return 1; /* HANDLED */
}
