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

	cr_server.numClients++;
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
					cr_server.curClient = NULL;
					crWarning("No RUN_QUEUE!! (quitting) \n");
				} 
				else
				{
					if (cr_server.run_queue == q)
						cr_server.run_queue = q->next;
					if (cr_server.curClient == q->client)
						cr_server.curClient = NULL;
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
		CRMessage *msg;

		/* no more clients, quit */
		if (!q)
			return;

		cr_server.curClient = q->client;

		for( ;; )
		{
			CRMessageOpcodes *msg_opcodes;
			char *data_ptr;
			unsigned int len;

			/* Don't use GetMessage, because it pulls stuff off
			 * the network too quickly */
			len = crNetPeekMessage( cr_server.curClient->conn, &msg );
			if (len == 0) {
#if 1
				/* new code path */
				if (cr_server.curClient->currentCtx &&
						(cr_server.curClient->currentCtx->lists.currentIndex != 0 ||
						 cr_server.curClient->currentCtx->current.inBeginEnd)) {
					/* We're between glNewList/EndList or glBegin/End.  We can't
					 * context switch because that'll screw things up.
					 */
					/*
					printf("currentIndex=%d inBeginEnd=%d\n",
								 cr_server.curClient->currentCtx->lists.currentIndex,
								 cr_server.curClient->currentCtx->current.inBeginEnd);
					*/
					CRASSERT(!q->blocked);
					crNetRecv();
					continue;
				}
				else
				{
					/* get next client */
					break;
				}
#else
				/* old code path */
				break;
#endif
			}
			else {
				/*
				printf("got %d bytes\n", len);
				*/
			}

			CRASSERT(len > 0);

			if (msg->header.type != CR_MESSAGE_OPCODES)
			{
				crError( "SPU %d sent me CRAP (type=0x%x)",
								 cr_server.curClient->spu_id, msg->header.type );
			}


			/* Do the context switch here.  No sense in switching before we
			 * really have any work to process.  This is a no-op if we're
			 * not really switching contexts.
			 */
			crStateMakeCurrent( cr_server.curClient->currentCtx );

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
