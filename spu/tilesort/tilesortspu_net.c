/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_pixeldata.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_url.h"
#include "cr_mem.h"

static void
tilesortspuReceiveReadPixels( const CRMessageReadPixels *rp, unsigned int len )
{
	GET_THREAD(thread);

	/* we better be expecting a ReadPixels result! */
	CRASSERT(thread->currentContext->readPixelsCount > 0);

	crNetRecvReadPixels(rp, len);

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
			tilesortspuReceiveReadPixels( &(msg->readPixels), len );
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


static void
tilesortspuCloseCallback(CRConnection *conn)
{
	int i;
	GET_THREAD(thread);
	for (i = 0; i < tilesort_spu.num_servers; i++) {
		if (thread->netServer[i].conn == conn) {
				crDebug("Tilesort SPU: server socket closed - exiting.");
			exit(0);
		}
	}
}


void
tilesortspuConnectToServers( void )
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	int i;
	char hostname[4096], protocol[4096];
	unsigned short port;

	int some_net_traffic = 0;

	CRASSERT(thread0->netServer);
	CRASSERT(thread0->buffer);

	crNetInit( tilesortspuReceiveData, tilesortspuCloseCallback );

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
