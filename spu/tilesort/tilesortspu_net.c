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

#include <memory.h>

int tilesortspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	return 0; /* NOT HANDLED */
}

void tilesortspuConnectToServers( void )
{
	int i;
	char hostname[4096], protocol[4096];
	unsigned short port;

	int some_net_traffic = 0;

	crNetInit( tilesortspuReceiveData, NULL );

	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		crNetServerConnect( &(server->net) );
		
		/* Tear the URL apart into relevant portions. 
		 * 
		 * just trying to get at the protocol so we can figure out if 
		 * we should override the user's choices for synconswap etc. */

		if ( !crParseURL( server->net.name, protocol, hostname, &port, 0 ) )
		{
			crError( "Malformed URL: \"%s\"", server->net.name );
		}

		if (!crStrcmp( protocol, "tcpip" ) ||
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
