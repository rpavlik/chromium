#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"

#include <memory.h>

void SERVER_DISPATCH_APIENTRY crServerDispatchWriteback( int *writeback )
{
	(void) writeback;
	crServerWriteback( );
}

void crServerWriteback(void)
{
	CRMessageWriteback *wb = (CRMessageWriteback *) crAlloc( sizeof( *wb ) );
	wb->type = CR_MESSAGE_WRITEBACK;
	memcpy( &(wb->writeback_ptr), &(cr_server.writeback_ptr), sizeof( wb->writeback_ptr ) );
	crNetSend( cr_server.clients[cr_server.cur_client].conn, NULL, wb, sizeof( *wb ) );
	crFree( wb );
}
