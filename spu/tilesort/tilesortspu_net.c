#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"

#include <memory.h>

void tilesortspuWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	memcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	(*writeback)--;
}

void tilesortspuReadback( CRMessageReadback *rb, unsigned int len )
{
	// minus the header, the destination pointer,
	// *and* the implicit writeback pointer at the head.

	int payload_len = len - sizeof( *rb );
	int *writeback;
	void *dest_ptr; 
	memcpy( &writeback, &(rb->writeback_ptr), sizeof( writeback ) );
	memcpy( &dest_ptr, &(rb->readback_ptr), sizeof( dest_ptr ) );

	(*writeback)--;
	memcpy( dest_ptr, ((char *)rb) + sizeof(*rb), payload_len );
}

int tilesortspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->type )
	{
		case CR_MESSAGE_WRITEBACK:
			tilesortspuWriteback( &(msg->writeback) );
			break;
		case CR_MESSAGE_READBACK:
			tilesortspuReadback( &(msg->readback), len );
			break;
		default:
			crWarning( "Why is the tilesort SPU getting a message of type %d?", msg->type );
			return 0; // NOT HANDLED
	}
	crNetFree( conn, buf );
	(void) len;	
	return 1;  // HANDLED
}

void tilesortspuConnectToServers( void )
{
	int i;

	crNetInit( tilesortspuReceiveData, NULL );

	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		crNetServerConnect( &(server->net) );
	}
}
