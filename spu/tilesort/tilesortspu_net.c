/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_url.h"
#include "cr_mem.h"

static void
tilesortspuReadPixels( const CRMessageReadPixels *rp, unsigned int len )
{
	GET_THREAD(thread);
	int payload_len = len - sizeof( *rp );
	char *dest_ptr;
	const char *src_ptr = (const char *) rp + sizeof(*rp);

	/* we better be expecting a ReadPixels result! */
	CRASSERT(thread->currentContext->readPixelsCount > 0);

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

	thread->currentContext->readPixelsCount--;
	CRASSERT(thread->currentContext->readPixelsCount >= 0);
}

/**
 * This is a callback function that's called from the crNet receiver
 * code in util/net.c.  We're interested in handling CR_MESSAGE_READ_PIXELS
 * messages (i.e. the return of pixel data) only.
 */
static int
tilesortspuReceiveData( CRConnection *conn, CRMessage *msg, unsigned int len )
{
	switch( msg->header.type )
	{
		case CR_MESSAGE_READ_PIXELS:
			tilesortspuReadPixels( &(msg->readPixels), len );
			return 1; /* HANDLED */
		default:
			/*
			crWarning("Why is the tilesort SPU getting a message of type 0x%x?",
								msg->type);
			*/
			break;
	}
	return 0; /* NOT HANDLED */
}

void tilesortspuConnectToServers( void )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	int i;
	char hostname[4096], protocol[4096];
	unsigned short port;

	int some_net_traffic = 0;

	CRASSERT(thread0->netServer);
	CRASSERT(thread0->buffer);

	crNetInit( tilesortspuReceiveData, NULL );

	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		CRNetServer *netServer = &(thread0->netServer[i]);

		crNetServerConnect( netServer );
		if (!netServer->conn) {
			crError("Tilesort SPU: Unable to connect to server %d!", i);
		}

		/* the connection may have already detected a smaller MTU */
		if (tilesort_spu.MTU > netServer->conn->mtu)
			tilesort_spu.MTU = netServer->conn->mtu;

		/* Tear the URL apart into relevant portions. 
		 * 
		 * just trying to get at the protocol so we can figure out if 
		 * we should override the user's choices for synconswap etc.
		 */
		if ( !crParseURL( netServer->name, protocol, hostname, &port, 0 ) )
		{
			crError( "Malformed URL: \"%s\"", netServer->name );
		}

		if (!crStrcmp( protocol, "tcpip" ) ||
		    !crStrcmp( protocol, "udptcpip" ) ||
		    !crStrcmp( protocol, "gm" ) ) 
		{
			some_net_traffic = 1;
		}
	}
	if (!some_net_traffic)
	{
		tilesort_spu.syncOnFinish = 0;
		tilesort_spu.syncOnSwap = 0;
	}
}
