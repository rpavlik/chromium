#include "server.h"
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

	crDebug( "Adding to the run queue: %d", index );
	q->client = client;
	q->blocked = 0;
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
		// no one had any work, get some!
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
		int len;
		char debug_buf[8096];

		cr_server.current_client = client;
		sprintf( debug_buf, "     ---- Switching contexts to connection 0x%p ----", client->conn );
		cr_server.dispatch.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf );
		crStateMakeCurrent( client->ctx );
		for( ;; )
		{
			CRMessageOpcodes *msg_opcodes;
			char *data_ptr;

			len = crNetGetMessage( cr_server.current_client->conn, &msg );
			if (msg->type != CR_MESSAGE_OPCODES)
			{
				crError( "SPU %d sent me CRAP (type=%d)", client->spu_id, msg->type );
			}

			msg_opcodes = (CRMessageOpcodes *) msg;
			if (!q->blocked)
			{
				sprintf( debug_buf, "     ---- Packet! (0x%p) ! ----", msg_opcodes );
				cr_server.dispatch.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf );
			}
			data_ptr = (char *) msg_opcodes + sizeof( *msg_opcodes) + ((msg_opcodes->numOpcodes + 3 ) & ~0x03);
			crUnpack( data_ptr, data_ptr-1, msg_opcodes->numOpcodes, &(cr_server.dispatch) );
			crNetFree( cr_server.current_client->conn, msg );
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
	return 0; // Never handle anything -- let packets queue up per-client instead.
}
