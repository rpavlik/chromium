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
void
crServerAddNewClient(void)
{
	CRClient *newClient;
	int n = -1;

	/* Save current client number */
	if (cr_server.curClient)
		n = cr_server.curClient->number;

	/* Reallocate cr_server.clients[] array.
	* This messes up all our pointers..... So we need to fix them up..
	*/
	crRealloc((void **)&cr_server.clients, sizeof(*cr_server.clients) * (cr_server.numClients + 1));

	/* Fix up our current client with the realloc'ed data */
	if (n == -1)
		cr_server.curClient = &cr_server.clients[0]; /* the new client */
	else
		cr_server.curClient = &cr_server.clients[n];

	/* now we can allocate our new client data */
	newClient = &(cr_server.clients[cr_server.numClients]);
	crMemZero(newClient, sizeof(CRClient));

	/* 
	 * Because we've Realloced the client data above, the run_queue's pointers
	 * are no longer valid.  Re-init the run_queue's client pointers here.
	 */
	if (cr_server.run_queue) {
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
	newClient->currentCtx = cr_server.DummyContext;

	crServerAddToRunQueue( newClient );

	cr_server.numClients++;
}


void crServerAddToRunQueue( CRClient *client )
{
	RunQueue *q = (RunQueue *) crAlloc( sizeof( *q ) );
	static int totalAdded = 0;

	totalAdded++;
	crDebug( "Adding to the run queue: client=%p number=%d count=%d",
					 (void *)client, client->number, totalAdded );

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
				crDebug("Deleting client %d from the run queue.", client->number);
				/* this test seems a bit excessive */
				if ((q->next == q->prev) && (q->next == q) && (cr_server.run_queue == q))
				{
					/* We're removing/deleting the only client */
					CRASSERT(cr_server.numClients == 1);
					crFree(q);
					cr_server.run_queue = NULL;
					cr_server.curClient = NULL;
					/* XXX we're not really exiting??? */
					crDebug("Empty run queue!");
					/* XXX add a config option to specify whether the crserver
					 * should exit when there's no more clients.
					 */
				} 
				else
				{
					/* remove from doubly linked list and free the node */
					if (cr_server.curClient == q->client)
						cr_server.curClient = NULL;
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


/**
 * Find the next client in the run queue that's not blocked and has a
 * waiting message.
 * Check if all clients are blocked (on barriers, semaphores), if so we've
 * deadlocked!
 * If no clients have a waiting message, call crNetRecv to get something
 * if 'block' is true, else return NULL if 'block' if false.
 */
static RunQueue *
getNextClient(GLboolean block)
{
	while (1)
	{
		if (cr_server.run_queue) 
		{
			GLboolean all_blocked = GL_TRUE;
			GLboolean done_something = GL_FALSE;
			RunQueue *start = cr_server.run_queue;

 			if (!cr_server.run_queue->client->conn
					|| cr_server.run_queue->client->conn->type == CR_NO_CONNECTION) {
 				crServerDeleteFromRunQueue( cr_server.run_queue->client );
				start = cr_server.run_queue;
			}
 
 			if (cr_server.run_queue == NULL) {
				/* empty queue */
 				return NULL;
			}

			while (!done_something || cr_server.run_queue != start)
			{
				done_something = GL_TRUE;
				if (!cr_server.run_queue->blocked)
				{
					all_blocked = GL_FALSE;
				}
				if (!cr_server.run_queue->blocked
						&& cr_server.run_queue->client->conn
						&& cr_server.run_queue->client->conn->messageList.head)
				{
					/* OK, this client isn't blocked and has a queued message */
					return cr_server.run_queue;
				}
				cr_server.run_queue = cr_server.run_queue->next;
			}

			if (all_blocked)
			{
				 /* XXX crError is fatal?  Should this be an info/warning msg? */
				crError( "crserver: DEADLOCK! (numClients=%d, all blocked)",
								 cr_server.numClients );
				if (cr_server.numClients < cr_server.maxBarrierCount) {
					crError("Waiting for more clients!!!");
					while (cr_server.numClients < cr_server.maxBarrierCount) {
						crNetRecv();
					}
				}
			}
		}

		if (!block)
			 return NULL;

		/* no one had any work, get some! */
		crNetRecv();

	} /* while */

	/* UNREACHED */
	/* return NULL; */
}


/**
 * This function takes the given message (which should be a buffer of
 * rendering commands) and executes it.
 */
static void
crServerDispatchMessage(CRMessage *msg)
{
	 const CRMessageOpcodes *msg_opcodes;
	 int opcodeBytes;
	 const char *data_ptr;

	 CRASSERT(msg->header.type == CR_MESSAGE_OPCODES);

	 msg_opcodes = (const CRMessageOpcodes *) msg;
	 opcodeBytes = (msg_opcodes->numOpcodes + 3) & ~0x03;

	 data_ptr = (const char *) msg_opcodes + sizeof(CRMessageOpcodes) + opcodeBytes;
	 crUnpack(data_ptr,                 /* first command's operands */
						data_ptr - 1,             /* first command's opcode */
						msg_opcodes->numOpcodes,  /* how many opcodes */
						&(cr_server.dispatch));  /* the CR dispatch table */
}



typedef enum
{
	CLIENT_GONE = 1, /* the client has disconnected */
  CLIENT_NEXT = 2, /* we can advance to next client */
  CLIENT_MORE = 3  /* we need to keep servicing current client */
} ClientStatus;


static ClientStatus
crServerServiceClient(const RunQueue *qEntry)
{
	CRMessage *msg;

	/* set current client pointer */
	cr_server.curClient = qEntry->client;

	/* service current client as long as we can */
	while (1) {
		unsigned int len;

		/* Check if the current client connection has gone away */
		if (!cr_server.curClient->conn
				|| cr_server.curClient->conn->type == CR_NO_CONNECTION) {
			crServerDeleteFromRunQueue( cr_server.curClient );
			return CLIENT_GONE;
		}

		/* Don't use GetMessage, because it pulls stuff off
		 * the network too quickly */
		len = crNetPeekMessage( cr_server.curClient->conn, &msg );
		if (len == 0) {
			/* No message ready at this time.
			 * See if we can advance to the next client, or if we're in the
			 * middle of something and need to stick with this client.
			 */
			if (cr_server.curClient->currentCtx &&
					(cr_server.curClient->currentCtx->lists.currentIndex != 0 ||
					 cr_server.curClient->currentCtx->current.inBeginEnd ||
					 cr_server.curClient->currentCtx->occlusion.currentQueryObject))
			{
				/* We're between glNewList/EndList or glBegin/End or inside a
				 * glBeginQuery/EndQuery sequence.
				 * We can't context switch because that'll screw things up.
				 */
				CRASSERT(!qEntry->blocked);
				crNetRecv();
				continue; /* return CLIENT_MORE; */
			}
			else {
				/* get next client */
				return CLIENT_NEXT;
			}
		}
		else {
			/* Got a message of length 'len' bytes */
			/*crDebug("got %d bytes", len);*/
		}

		CRASSERT(len > 0);
		if (msg->header.type != CR_MESSAGE_OPCODES) {
			crError( "SPU %d sent me CRAP (type=0x%x)",
							 cr_server.curClient->spu_id, msg->header.type );
		}

		/* Do the context switch here.  No sense in switching before we
		 * really have any work to process.  This is a no-op if we're
		 * not really switching contexts.
		 *
		 * XXX This isn't entirely sound.  The crStateMakeCurrent() call
		 * will compute the state difference and dispatch it using
		 * the head SPU's dispatch table.
		 *
		 * This is a problem if this is the first buffer coming in,
		 * and the head SPU hasn't had a chance to do a MakeCurrent()
		 * yet (likely because the MakeCurrent() command is in the
		 * buffer itself).
		 *
		 * At best, in this case, the functions are no-ops, and
		 * are essentially ignored by the SPU.  In the typical
		 * case, things aren't too bad; if the SPU just calls
		 * crState*() functions to update local state, everything
		 * will work just fine.
		 *
		 * In the worst (but unusual) case where a nontrivial
		 * SPU is at the head of a crserver's SPU chain (say,
		 * in a multiple-tiered "tilesort" arrangement, as
		 * seen in the "multitilesort.conf" configuration), the
		 * SPU may rely on state set during the MakeCurrent() that
		 * may not be present yet, because no MakeCurrent() has
		 * yet been dispatched.
		 *
		 * This headache will have to be revisited in the future;
		 * for now, SPUs that could head a crserver's SPU chain
		 * will have to detect the case that their functions are
		 * being called outside of a MakeCurrent(), and will have
		 * to handle the situation gracefully.  (This is currently
		 * the case with the "tilesort" SPU.)
		 */
		crStateMakeCurrent( cr_server.curClient->currentCtx );

		/* Force scissor, viewport and projection matrix update in
		 * crServerSetOutputBounds().
		 */
		cr_server.currentSerialNo = 0;

		/* Commands get dispatched here */
		crServerDispatchMessage(msg);

		crNetFree( cr_server.curClient->conn, msg );

		if (qEntry->blocked) {
			/* Note/assert: we should not be inside a glBegin/End or glNewList/
			 * glEndList pair at this time!
			 */
			return CLIENT_NEXT;
		}

	}
}



/**
 * Check if any of the clients need servicing.
 * If so, service one client and return.
 * Else, just return.
 */
void
crServerServiceClients(void)
{
	RunQueue *q;

	crNetRecv();  /* does not block */

	q = getNextClient(GL_FALSE); /* don't block */
	if (q) {
		ClientStatus stat = crServerServiceClient(q);
		if (stat == CLIENT_NEXT && cr_server.run_queue->next) {
			/* advance to next client */
			cr_server.run_queue = cr_server.run_queue->next;
		}
	}
}




/**
 * Main crserver loop.  Service connections from all connected clients.
 */
void
crServerSerializeRemoteStreams(void)
{
	while (1)
	{
#if 1 /** THIS CODE BLOCK SHOULD GO AWAY SOMEDAY **/
		ClientStatus stat;
		RunQueue *q = getNextClient(GL_TRUE); /* block */

		/* no more clients, quit */
		if (!q)
			return;

		stat = crServerServiceClient(q);

		/* check if all clients have gone away */
		if (!cr_server.run_queue)
			 return;

		/* advance to next client */
		cr_server.run_queue = cr_server.run_queue->next;
#else
		crServerServiceClients();
#endif
	}
}


/**
 * This will be called by the network layer when it's received a new message.
 */
int crServerRecv( CRConnection *conn, CRMessage *msg, unsigned int len )
{
	(void) len;

	switch( msg->header.type )
	{
		/* Called when using multiple threads */
		case CR_MESSAGE_NEWCLIENT:
			crServerAddNewClient();
			return 1;
		default:
			/*crWarning( "Why is the crserver getting a message of type 0x%x?",
				msg->header.type ); */
			;
	}
	return 0; /* not handled */
}
