/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_pack.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "packspu.h"
#include "packspu_proto.h"

static void packspuWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	crMemcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	*writeback = 0;
}

static void packspuReadback( CRMessageReadback *rb, unsigned int len )
{
	/* minus the header, the destination pointer, 
	 * *and* the implicit writeback pointer at the head. */

	int payload_len = len - sizeof( *rb );
	int *writeback;
	void *dest_ptr; 
	crMemcpy( &writeback, &(rb->writeback_ptr), sizeof( writeback ) );
	crMemcpy( &dest_ptr, &(rb->readback_ptr), sizeof( dest_ptr ) );

	*writeback = 0;
	crMemcpy( dest_ptr, ((char *)rb) + sizeof(*rb), payload_len );
}

static void packspuReadPixels( CRMessageReadPixels *rp, unsigned int len )
{
	int payload_len = len - sizeof( *rp );
	char *dest_ptr;
	const char *src_ptr = (char *) rp + sizeof(*rp);

	crMemcpy ( &(dest_ptr), &(rp->pixels), sizeof(dest_ptr));

	if (rp->alignment == 1 &&
		rp->skipRows == 0 &&
		rp->skipPixels == 0 &&
		rp->stride == rp->bytes_per_row) {
		/* no special packing is needed */
		crMemcpy ( dest_ptr, ((char *)rp) + sizeof(*rp), payload_len );
	}
	else {
		/* need special packing */
#if 0
		CRPixelPackState packing;
		packing.rowLength = 0;
		packing.skipRows = rp->skipRows;
		packing.skipPixels = rp->skipPixels;
		packing.alignment = rp->alignment;
		packing.imageHeight = 0;
		packing.skipImages = 0;
		packing.swapBytes = GL_FALSE;
		packing.psLSBFirst = GL_FALSE;
		crPixelCopy2D( rp->bytes_per_row, rp->rows,
				dest_ptr, rp->format, rp->type, &packing,
				src_ptr, rp->format, rp->type, NULL);
#else
		GLuint row;
		for (row = 0; row < rp->rows; row++) {
		   crMemcpy( dest_ptr, src_ptr, rp->bytes_per_row );
		   src_ptr += rp->bytes_per_row;
		   dest_ptr += rp->stride;
		}
#endif
	}

	--pack_spu.ReadPixels;
}

static int packspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->header.type )
	{
		case CR_MESSAGE_READ_PIXELS:
			packspuReadPixels( &(msg->readPixels), len );
			break;
		case CR_MESSAGE_WRITEBACK:
			packspuWriteback( &(msg->writeback) );
			break;
		case CR_MESSAGE_READBACK:
			packspuReadback( &(msg->readback), len );
			break;
		default:
			/*crWarning( "Why is the pack SPU getting a message of type 0x%x?", msg->type ); */
			return 0; /* NOT HANDLED */
	}
	(void) len;	
	return 1; /* HANDLED */
}

static CRMessageOpcodes *
__prependHeader( CRPackBuffer *buf, unsigned int *len, unsigned int senderID )
{
	int num_opcodes;
	CRMessageOpcodes *hdr;

	CRASSERT( buf );
	CRASSERT( buf->opcode_current < buf->opcode_start );
	CRASSERT( buf->opcode_current >= buf->opcode_end );
	CRASSERT( buf->data_current > buf->data_start );
	CRASSERT( buf->data_current <= buf->data_end );

	num_opcodes = buf->opcode_start - buf->opcode_current;
	hdr = (CRMessageOpcodes *) 
		( buf->data_start - ( ( num_opcodes + 3 ) & ~0x3 ) - sizeof(*hdr) );

	CRASSERT( (void *) hdr >= buf->pack );

	if (pack_spu.swap)
	{
		hdr->header.type = (CRMessageType) SWAP32(CR_MESSAGE_OPCODES);
		hdr->numOpcodes  = SWAP32(num_opcodes);
	}
	else
	{
		hdr->header.type = CR_MESSAGE_OPCODES;
		hdr->numOpcodes  = num_opcodes;
	}

	*len = buf->data_current - (unsigned char *) hdr;

	return hdr;
}


/*
 * This is called from either the Pack SPU and the packer library whenever
 * we need to send a data buffer to the server.
 */
void packspuFlush(void *arg )
{
	ThreadInfo *thread = (ThreadInfo *) arg;
	ContextInfo *ctx;
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf;

	/* we should _always_ pass a valid <arg> value */
	CRASSERT(thread);
	ctx = thread->currentContext;
	buf = &(thread->buffer);
	CRASSERT(buf);

	/* We're done packing into the current buffer, unbind it */
	crPackReleaseBuffer( thread->packer );

	/*
	printf("%s thread=%p thread->id = %d thread->pc=%p t2->id=%d t2->pc=%p packbuf=%p packbuf=%p\n",
		   __FUNCTION__, (void*) thread, (int) thread->id, thread->packer,
		   (int) t2->id, t2->packer,
		   buf->pack, thread->packer->buffer.pack);
	*/

	if ( buf->opcode_current == buf->opcode_start ) {
           /*
           printf("%s early return\n", __FUNCTION__);
           */
           /* XXX these calls seem to help, but might be appropriate */
           crPackSetBuffer( thread->packer, buf );
           crPackResetPointers(thread->packer);
           return;
	}

	hdr = __prependHeader( buf, &len, 0 );

	CRASSERT( thread->server.conn );

	if ( buf->holds_BeginEnd )
		crNetBarf( thread->server.conn, &(buf->pack), hdr, len );
	else
		crNetSend( thread->server.conn, &(buf->pack), hdr, len );

	buf->pack = crNetAlloc( thread->server.conn );

	if ( buf->mtu > thread->server.conn->mtu )
		buf->mtu = thread->server.conn->mtu;

	crPackSetBuffer( thread->packer, buf );

	crPackResetPointers(thread->packer);
	(void) arg;
}

void packspuHuge( CROpcode opcode, void *buf )
{
	GET_THREAD(thread);
	unsigned int          len;
	unsigned char        *src;
	CRMessageOpcodes *msg;

	CRASSERT(thread);

	/* packet length is indicated by the variable length field, and
	   includes an additional word for the opcode (with alignment) and
	   a header */
	len = ((unsigned int *) buf)[-1];
	if (pack_spu.swap)
	{
		/* It's already been swapped, swap it back. */
		len = SWAP32(len);
	}
	len += 4 + sizeof(CRMessageOpcodes);

	/* write the opcode in just before the length */
	((unsigned char *) buf)[-5] = (unsigned char) opcode;

	/* fix up the pointer to the packet to include the length & opcode
       & header */
	src = (unsigned char *) buf - 8 - sizeof(CRMessageOpcodes);

	msg = (CRMessageOpcodes *) src;

	if (pack_spu.swap)
	{
		msg->header.type = (CRMessageType) SWAP32(CR_MESSAGE_OPCODES);
		msg->numOpcodes  = SWAP32(1);
	}
	else
	{
		msg->header.type = CR_MESSAGE_OPCODES;
		msg->numOpcodes  = 1;
	}

	CRASSERT( thread->server.conn );
	crNetSend( thread->server.conn, NULL, src, len );
	crPackFree( buf );
}

void packspuConnectToServer( CRNetServer *server )
{
	crNetInit( packspuReceiveData, NULL );
	crNetServerConnect( server );
}
