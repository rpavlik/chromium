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
#include "replicatespu.h"
#include "replicatespu_proto.h"

#include <X11/Xmd.h>
#include <X11/extensions/vnc.h>

extern int NOP;

static void replicatespuWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	crMemcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	*writeback = 0;
}

static void replicatespuReadback( CRMessageReadback *rb, unsigned int len )
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

static void replicatespuReadPixels( CRMessageReadPixels *rp, unsigned int len )
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

	--replicate_spu.ReadPixels;
}

static int replicatespuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->header.type )
	{
		case CR_MESSAGE_READ_PIXELS:
			replicatespuReadPixels( &(msg->readPixels), len );
			break;
		case CR_MESSAGE_WRITEBACK:
			replicatespuWriteback( &(msg->writeback) );
			break;
		case CR_MESSAGE_READBACK:
			replicatespuReadback( &(msg->readback), len );
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

	if (replicate_spu.swap)
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

static void replicatespuCheckVncEvents()
{
	if (replicate_spu.glx_display) {
		while (XPending(replicate_spu.glx_display)) {
#if 0
			int VncConn = replicate_spu.VncEventsBase + XVncConnected;
			int VncDisconn = replicate_spu.VncEventsBase + XVncDisconnected;
#endif
			int VncChromiumConn = replicate_spu.VncEventsBase + XVncChromiumConnected;
			XEvent event;
		
			XNextEvent (replicate_spu.glx_display, &event);

#if 0
			if (event.type == VncConn) {
				crWarning("GOT CONNECTION!!!!\n");
			} 
			else
#endif
			if (event.type == VncChromiumConn) {
				XVncConnectedEvent *e = (XVncConnectedEvent*) &event;
				struct in_addr addr;

				addr.s_addr = e->ipaddress;
				crWarning("ReplicateSPU - someone just connected!\n");
				if (e->ipaddress) {
					replicatespuReplicateCreateContext(e->ipaddress);
				} else {
					crWarning("Someone connected, but with no ipaddress ???????????\n");
				}
			} 
#if 0
			else
			if (event.type == VncDisconn) {
				crWarning("GOT DISCONNECTION!!!!\n");
			}
#endif
		}
	}
}

/*
 * This is called from either the Pack SPU and the packer library whenever
 * we need to send a data buffer to the server.
 */
void replicatespuFlush(void *arg )
{
	ThreadInfo *thread = (ThreadInfo *) arg;
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf;
	unsigned int i;
	unsigned int fired = 0;

	/* we should _always_ pass a valid <arg> value */
	CRASSERT(thread);
	buf = &(thread->buffer);
	CRASSERT(buf);

	crPackGetBuffer( thread->packer, buf );

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

	/* Send pack buffer to the primary connection, but if it's the nopspu
	 * we can drop it on the floor, but not if we've turned off broadcast */
	if (NOP || !thread->broadcast) {
		if ( buf->holds_BeginEnd )
			crNetBarf( thread->server.conn, &(buf->pack), hdr, len );
		else
			crNetSend( thread->server.conn, &(buf->pack), hdr, len );
		fired = 1;
	}

	/* Now send it to all our replicants */
	for (i = 1; i < CR_MAX_REPLICANTS; i++) 
	{
		if (thread->broadcast && (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION))
		{
			if ( buf->holds_BeginEnd )
				crNetBarf( replicate_spu.rserver[i].conn, &(buf->pack), hdr, len );
			else
				crNetSend( replicate_spu.rserver[i].conn, &(buf->pack), hdr, len );
			fired = 1;
		}
	}

	if (fired)
		buf->pack = crNetAlloc( thread->server.conn );

	if ( buf->mtu > thread->server.conn->mtu )
		buf->mtu = thread->server.conn->mtu;

	crPackSetBuffer( thread->packer, buf );

	crPackResetPointers(thread->packer);

	/* 
	 * As the *Flush() routine is fairly consistently called, we
	 * can use this as our event loop too. Check for vnc events
	 * during flush. 
	 */
	if (replicate_spu.vncAvailable)
		replicatespuCheckVncEvents();

	(void) arg;
}

void replicatespuHuge( CROpcode opcode, void *buf )
{
	GET_THREAD(thread);
	unsigned int          len;
	unsigned char        *src;
	CRMessageOpcodes *msg;
	int i;

	CRASSERT(thread);

	/* packet length is indicated by the variable length field, and
	   includes an additional word for the opcode (with alignment) and
	   a header */
	len = ((unsigned int *) buf)[-1];
	if (replicate_spu.swap)
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

	if (replicate_spu.swap)
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

	/* Send pack buffer to the primary connection, but if it's the nopspu
	 * we can drop it on the floor, but not if we've turned off broadcast */
	if (NOP || !thread->broadcast) {
		crNetSend( thread->server.conn, NULL, src, len );
	}

	/* Now send it to all our replicants */
	for (i = 1; i < CR_MAX_REPLICANTS; i++) 
	{
		if (thread->broadcast && (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION))
		{
			crNetSend( replicate_spu.rserver[i].conn, NULL, src, len );
		}
	}

	crPackFree( buf );
}

void replicatespuConnectToServer( CRNetServer *server )
{
	crNetInit( replicatespuReceiveData, NULL );
	crNetServerConnect( server );
}
