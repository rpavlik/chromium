#include "packspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"

void packspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	(void) conn;	
	(void) buf;	
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

void packspuFlush( void )
{
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf = &(pack_spu.buffer);
	crPackGetBuffer( buf );

	if ( buf->opcode_current == buf->opcode_start )
		return;

	hdr = __prependHeader( buf, &len );
	hdr->senderId = 0;

	crNetSend( pack_spu.conn, &(buf->pack), hdr, len );
	buf->pack = crNetAlloc( pack_spu.conn );
	crPackSetBuffer( buf );
	crPackResetPointers();
}

void packspuConnectToServer( void )
{
	crNetInit( packspuReceiveData, NULL );

	pack_spu.conn = crNetConnectToServer( pack_spu.server_name, 7000, pack_spu.buffer_size );

	pack_spu.buffer.pack = crNetAlloc( pack_spu.conn );
	pack_spu.buffer.size = pack_spu.buffer_size;
	crPackSetBuffer( &pack_spu.buffer );
	crPackResetPointers();
	crPackFlushFunc( packspuFlush );
}
