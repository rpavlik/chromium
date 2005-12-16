/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_net.h"
#include "cr_error.h"
#include "replicatespu.h"


static void
replicatespuWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	crMemcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	*writeback = 0;
}


static void
replicatespuReadPixels( CRMessageReadPixels *rp, unsigned int len )
{
	crNetRecvReadPixels(rp, len);
	--replicate_spu.ReadPixels;
}


static int
replicatespuReceiveData( CRConnection *conn, CRMessage *msg, unsigned int len )
{
	(void) conn;

	switch (msg->header.type) {
		case CR_MESSAGE_READ_PIXELS:
			replicatespuReadPixels( &(msg->readPixels), len );
			return 1; /* handled */
		case CR_MESSAGE_WRITEBACK:
			replicatespuWriteback( &(msg->writeback) );
			return 1; /* handled */
		default:
			;
	}
	return 0; /* not handled */
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

	CRASSERT(buf->pack);
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


/**
 * Flush buffered commands, sending them to just one server
 */
void
replicatespuFlushOne(ThreadInfo *thread, int server)
{
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf;
	CRConnection *conn;

	CRASSERT(server >= 0);
	CRASSERT(server < CR_MAX_REPLICANTS);
	CRASSERT(thread);
	buf = &(thread->buffer);
	CRASSERT(buf);
	CRASSERT(buf->pack);

	crPackReleaseBuffer( thread->packer );

	if ( buf->opcode_current == buf->opcode_start ) {
		/* XXX these calls seem to help, but might be appropriate */
		crPackSetBuffer( thread->packer, buf );
		crPackResetPointers(thread->packer);
		return;
	}

	hdr = __prependHeader( buf, &len, 0 );

	conn = replicate_spu.rserver[server].conn;
	CRASSERT(conn);
	CRASSERT(conn->type != CR_NO_CONNECTION);

	crNetSend( conn, NULL, hdr, len );

	/* The network may have found a new mtu */
	buf->mtu = thread->server.conn->mtu;

	crPackSetBuffer( thread->packer, buf );

	crPackResetPointers(thread->packer);
}


/**
 * Flush/send pending commands to all servers.
 */
void
replicatespuFlushAll(ThreadInfo *thread)
{
	replicatespuFlush((void *) thread);

	/* 
	 * As the *Flush() routine is fairly consistently called, we
	 * can use this as our event loop too. Check for vnc events
	 * during flush. 
	 */
	if (replicate_spu.vncAvailable)
		replicatespuCheckVncEvents();
}


/*
 * This is called from either replicatespuFlushAll or the packer library
 * whenever we need to send a data buffer to the server.
 */
void replicatespuFlush(void *arg )
{
	ThreadInfo *thread = (ThreadInfo *) arg;
	unsigned int len;
	CRMessageOpcodes *hdr;
	CRPackBuffer *buf;
	unsigned int i;

	/* we should _always_ pass a valid <arg> value */
	CRASSERT(thread);
	buf = &(thread->buffer);
	CRASSERT(buf);
	CRASSERT(buf->pack);

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

	/* Send pack buffer to the primary connection, but if it's the nopspu
	 * we can drop it on the floor, but not if we've turned off broadcast */
	if (1/*replicate_spu.NOP*/) {
		if ( buf->holds_BeginEnd )
			crNetBarf( thread->server.conn, NULL, hdr, len );
		else
			crNetSend( thread->server.conn, NULL, hdr, len );
	}

	/* Now send it to all our replicants */
	for (i = 1; i < CR_MAX_REPLICANTS; i++) 
	{
		if (replicate_spu.rserver[i].conn &&
				replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION)
		{
			if ( buf->holds_BeginEnd )
				crNetBarf( replicate_spu.rserver[i].conn, NULL, hdr, len );
			else
				crNetSend( replicate_spu.rserver[i].conn, NULL, hdr, len );
		}
	}

	/* The network may have found a new mtu */
	buf->mtu = thread->server.conn->mtu;

	crPackSetBuffer( thread->packer, buf );

	crPackResetPointers(thread->packer);
}


/**
 * XXX NOTE: there's a lot of duplicate code here common to the
 * pack, tilesort and replicate SPUs.  Try to simplify someday!
 */
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
	if (1/*replicate_spu.NOP*/) {
		crNetSend( thread->server.conn, NULL, src, len );
	}

	/* Now send it to all our replicants */
	for (i = 1; i < CR_MAX_REPLICANTS; i++) 
	{
		if (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION)
		{
			crNetSend( replicate_spu.rserver[i].conn, NULL, src, len );
		}
	}
}


void replicatespuConnectToServer( CRNetServer *server )
{
	crNetInit( replicatespuReceiveData, NULL );
	crNetServerConnect( server );
}
