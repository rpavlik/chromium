#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"

#include "server.h"

static void __setDefaults( void )
{
	cr_server.tcpip_port = 7000;
}

void crServerGatherConfiguration(void)
{
	CRConnection *conn;
	char response[8096];

	char **spuchain;
	int num_spus;
	int *spu_ids;
	char **spu_names;
	unsigned int mtu;
	int i;

	char **clientchain, **clientlist;
	int num_clients;

	__setDefaults();
	
	conn = crMothershipConnect( );

	// The response will tell which SPUs to load
	crMothershipIdentifyServer( conn, response );

	spuchain = crStrSplit( response, " " );
	num_spus = crStrToInt( spuchain[0] );
	spu_ids = (int *) crAlloc( num_spus * sizeof( *spu_ids ) );
	spu_names = (char **) crAlloc( num_spus * sizeof( *spu_names ) );
	for (i = 0 ; i < num_spus ; i++)
	{
		spu_ids[i] = crStrToInt( spuchain[2*i+1] );
		spu_names[i] = crStrdup( spuchain[2*i+2] );
		crDebug( "SPU %d/%d: (%d) \"%s\"", i+1, num_spus, spu_ids[i], spu_names[i] );
	}

	if (crMothershipGetSPUDir( conn, response ))
	{
		crSetenv( "SPU_DIR", response );
	}

	cr_server.head_spu = crSPULoadChain( num_spus, spu_ids, spu_names );

	crFree( spu_ids );
	crFree( spu_names );
	crFree( spuchain );

	if (crMothershipServerParam( conn, response, "port" ))
	{
		cr_server.tcpip_port = crStrToInt( response );
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%ud", &mtu );
	crNetSetMTU( mtu );

	// The response will tell us what protocols we need to serve
	// example: "3 tcpip 1,gm 2,via 10"

	crMothershipGetClients( conn, response );
	
	clientchain = crStrSplitn( response, " ", 1 );
	num_clients = crStrToInt( clientchain[0] );
	clientlist = crStrSplit( clientchain[1], "," );

	cr_server.num_clients = num_clients;
	cr_server.clients = (CRClient *) crAlloc( num_clients * sizeof( *(cr_server.clients) ) );

	for (i = 0 ; i < num_clients ; i++)
	{
		char protocol[1024];

		sscanf( clientlist[i], "%s %d", protocol, &(cr_server.clients[i].spu_id) );
		cr_server.clients[i].conn = crNetAcceptClient( protocol, cr_server.tcpip_port );
	}

	crMothershipDisconnect( conn );
}
