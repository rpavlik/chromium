#include "packspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"

#include <memory.h>

void packspuWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	memcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	*writeback = 0;
}

void packspuReadback( CRMessageReadback *rb, unsigned int len )
{
	// minus the header, the destination pointer,
	// *and* the implicit writeback pointer at the head.

	int payload_len = len - sizeof( *rb );
	int *writeback;
	void *dest_ptr; 
	memcpy( &writeback, &(rb->writeback_ptr), sizeof( writeback ) );
	memcpy( &dest_ptr, &(rb->readback_ptr), sizeof( dest_ptr ) );

	*writeback = 0;
	memcpy( dest_ptr, ((char *)rb) + sizeof(*rb), payload_len );
}

void packspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->type )
	{
		case CR_MESSAGE_WRITEBACK:
			packspuWriteback( &(msg->writeback) );
			break;
		case CR_MESSAGE_READBACK:
			packspuReadback( &(msg->readback), len );
			break;
		default:
			crError( "Why is the pack SPU getting a message of type %d?", msg->type );
			break;
	}
	crNetFree( conn, buf );
	(void) conn;	
	(void) len;	
}

static CRMessageOpcodes *
__prependHeader( CRPackBuffer *buf, unsigned int *len )
{
	int num_opcodes;
	CRMessageOpcodes *hdr;

	CRASSERT( buf->opcode_current < buf->opcode_start );
	CRASSERT( buf->opcode_current >= buf->opcode_end );
	CRASSERT( buf->data_current > buf->data_start );
	CRASSERT( buf->data_current <= buf->data_end );

	num_opcodes = buf->opcode_start - buf->opcode_current;
	hdr = (CRMessageOpcodes *) 
		( buf->data_start - ( ( num_opcodes + 3 ) & ~0x3 ) - sizeof(*hdr) );

	CRASSERT( (void *) hdr >= buf->pack );

	hdr->type       = CR_MESSAGE_OPCODES;
	hdr->senderId   = (unsigned int) ~0;  /* to be initialized by caller */
	hdr->numOpcodes = num_opcodes;

	*len = buf->data_current - (unsigned char *) hdr;

	return hdr;
}

void packspuFlush( void *arg )
{
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf = &(pack_spu.buffer);
	crPackGetBuffer( buf );

	if ( buf->opcode_current == buf->opcode_start )
		return;

	hdr = __prependHeader( buf, &len );
	hdr->senderId = 0;

	crNetSend( pack_spu.server.conn, &(buf->pack), hdr, len );
	buf->pack = crNetAlloc( pack_spu.server.conn );
	crPackSetBuffer( buf );
	crPackResetPointers(0); // don't need extra room like tilesort
	(void) arg;
}

void packspuHuge( CROpcode opcode, void *buf )
{
	unsigned int          len;
	unsigned char        *src;
	CRMessageOpcodes *msg;

	/* packet length is indicated by the variable length field, and
	   includes an additional word for the opcode (with alignment) and
	   a header */
	len = ((unsigned int *) buf)[-1] + 4 + sizeof(CRMessageOpcodes);

	/* write the opcode in just before the length */
	((unsigned char *) buf)[-5] = (unsigned char) opcode;

	/* fix up the pointer to the packet to include the length & opcode
       & header */
	src = (unsigned char *) buf - 8 - sizeof(CRMessageOpcodes);

	msg = (CRMessageOpcodes *) src;

	msg->type       = CR_MESSAGE_OPCODES;
	msg->senderId   = pack_spu.server.conn->sender_id;
	msg->numOpcodes = 1;

	crNetSend( pack_spu.server.conn, NULL, src, len );
}

void packspuConnectToServer( void )
{
	crNetInit( packspuReceiveData, NULL );

	crNetServerConnect( &(pack_spu.server) );

	crPackInitBuffer( &(pack_spu.buffer), crNetAlloc( pack_spu.server.conn ), pack_spu.server.buffer_size, 0 );
	crPackSetBuffer( &pack_spu.buffer );
	crPackFlushFunc( packspuFlush );
	crPackSendHugeFunc( packspuHuge );
}
