/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_url.h"
#include "cr_mem.h"

void tilesortspuReadPixels( CRMessageReadPixels *rp, unsigned int len )
{
	int payload_len = len - sizeof( *rp );
	char *dest_ptr;
	char *src_ptr = (char*)rp + sizeof(*rp);

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

	--tilesort_spu.ReadPixels;
}

int tilesortspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;
	
	switch( msg->header.type )
	{
		case CR_MESSAGE_READ_PIXELS:
			tilesortspuReadPixels( &(msg->readPixels), len );
			break;
		default:
			/*crWarning( "Why is the tilesort SPU getting a message of type 0x%x?", msg->type ); */
			return 0; /* NOT HANDLED */
	}
	(void) len;	
	return 1; /* HANDLED */
}

void tilesortspuConnectToServers( void )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	int i;
	char hostname[4096], protocol[4096];
	unsigned short port;

	int some_net_traffic = 0;

	CRASSERT(thread0->net);
	CRASSERT(thread0->pack);

	crNetInit( tilesortspuReceiveData, NULL );

	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		CRNetServer *net = &(thread0->net[i]);
		crNetServerConnect( net );
		
		/* Tear the URL apart into relevant portions. 
		 * 
		 * just trying to get at the protocol so we can figure out if 
		 * we should override the user's choices for synconswap etc. */

		if ( !crParseURL( net->name, protocol, hostname, &port, 0 ) )
		{
			crError( "Malformed URL: \"%s\"", net->name );
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
