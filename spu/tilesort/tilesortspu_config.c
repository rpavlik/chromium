#include "tilesortspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"

#include <stdio.h>

static void __setDefaults( void )
{
	tilesort_spu.num_servers = 0;
	tilesort_spu.servers = NULL;
}

void tilesortspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	char **serverchain, **serverlist;
	int num_servers;

	// Connect to the mothership and identify ourselves.
	
	conn = crMothershipConnect( );
	crMothershipIdentifySPU( conn, tilesort_spu.id );

	// The response to this tells us how many servers, what the
	// SPU id of their first SPU is, and where they are.
	//
	// For example:  2 1 tcpip://foo 2 tcpip://bar
	//
	// We need the SPU ID since the tiling information is actually
	// associated with the render SPU that must be at the head of the
	// server chain for tile/sorting to work properly.

	crMothershipGetServers( conn, response );

	serverchain = crStrSplitn( response, " ", 1 );
	num_servers = crStrToInt( serverchain[0] );
	serverlist = crStrSplit( serverchain[1], "," );

	tilesort_spu.num_servers = num_servers;
	tilesort_spu.servers = (TileSortSPUServer *) crAlloc( num_servers * sizeof( *(tilesort_spu.servers) ) );

	crWarning( "Got %d servers!", num_servers );

	__setDefaults();

	crMothershipDisconnect( conn );
}
