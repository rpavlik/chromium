#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"

#include "server.h"

static void __setDefaults( void )
{
	cr_server.tcpip_port = 7000;
	cr_server.num_extents = 0;
	cr_server.current_extent = 0;
	cr_server.muralWidth = 0;
	cr_server.muralHeight = 0;
}

void crServerGatherConfiguration(char *mothership)
{
	CRConnection *conn;
	char response[8096];

	char **spuchain;
	int num_spus;
	int *spu_ids;
	char **spu_names;
	char *spu_dir;
	unsigned int mtu;
	int i;

	char **clientchain, **clientlist;
	int num_clients;
	char **tilechain, **tilelist;
	int num_servers;

	char **serverchain, **serverlist;

	__setDefaults();

	if (mothership)
	{
		crSetenv( "CRMOTHERSHIP", mothership);
	}
	
	conn = crMothershipConnect( );

	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}

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
		spu_dir = response;
	}
	else
	{
		spu_dir = NULL;
	}

	cr_server.head_spu = crSPULoadChain( num_spus, spu_ids, spu_names, spu_dir );

	crFree( spu_ids );
	crFree( spu_names );
	crFree( spuchain );

	if (crMothershipServerParam( conn, response, "port" ))
	{
		cr_server.tcpip_port = crStrToInt( response );
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%ud", &mtu );

	// The response will tell us what protocols we need to serve
	// example: "3 tcpip 1,gm 2,via 10"

	crMothershipGetClients( conn, response );
	
	clientchain = crStrSplitn( response, " ", 1 );
	num_clients = crStrToInt( clientchain[0] );
	clientlist = crStrSplit( clientchain[1], "," );

	cr_server.num_clients = num_clients;
	cr_server.clients = (CRClient *) crAlloc( num_clients * sizeof( *(cr_server.clients) ) );

	if (!crMothershipGetServerTiles( conn, response ))
	{
		crDebug( "No tiling information for server!" );
	}
	else
	{
		tilechain = crStrSplitn( response, " ", 1 );
		cr_server.num_extents = crStrToInt( tilechain[0] );
		tilelist = crStrSplit( tilechain[1], "," );
		for (i = 0 ; i < cr_server.num_extents; i++)
		{
			float x, y, w, h;
			sscanf( tilelist[i], "%f %f %f %f", &x, &y, &w, &h );
			cr_server.x1[i] = (int) x;
			cr_server.y1[i] = (int) y;
			cr_server.x2[i] = cr_server.x1[i] + (int) w;
			cr_server.y2[i] = cr_server.y1[i] + (int) h;
			crDebug( "Added tile: %d %d %d %d", cr_server.x1[i], cr_server.y1[i], cr_server.x2[i], cr_server.y2[i] );
		}
	}

	for (i = 0 ; i < num_clients ; i++)
	{
		char protocol[1024];

		sscanf( clientlist[i], "%s %d", protocol, &(cr_server.clients[i].spu_id) );
		cr_server.clients[i].conn = crNetAcceptClient( protocol, cr_server.tcpip_port, mtu );
		cr_server.clients[i].ctx = crStateCreateContext();
		crStateSetCurrentPointers( cr_server.clients[i].ctx, &(cr_server.current) );
		crServerAddToRunQueue( i );
	}

	// Sigh -- the servers need to know how big the whole mural is if we're
	// doing tiling, so they can compute their base projection.  For now,
	// just have them pretend to be one of their client SPU's, and redo
	// the configuration step of the tilesort SPU.  Basically this is a dirty
	// way to figure out who the other servers are.  It *might* matter
	// which SPU we pick for certain graph configurations, but we'll cross
	// that bridge later.

	crMothershipIdentifySPU( conn, cr_server.clients[0].spu_id );
	crMothershipGetServers( conn, response );

	serverchain = crStrSplitn( response, " ", 1 );
	num_servers = crStrToInt( serverchain[0] );

	if (num_servers == 0)
	{
		crError( "No servers specified for SPU %d?!", cr_server.clients[0].spu_id );
	}
	serverlist = crStrSplit( serverchain[1], "," );

	num_servers = num_servers;

	for (i = 0 ; i < num_servers ; i++)
	{
		int num_extents;
		int tile;
		if (!crMothershipGetTiles( conn, response, i ))
		{
			break;
		}

		tilechain = crStrSplitn( response, " ", 1 );
		num_extents = crStrToInt( tilechain[0] );

		tilelist = crStrSplit( tilechain[1], "," );

		for (tile = 0; tile < num_extents ; tile++)
		{
			int w,h;
			int x1, y1;
			int x2, y2;
			sscanf( tilelist[tile], "%d %d %d %d", &x1, &y1, &w, &h );

			x2 = x1 + w;
			y2 = y1 + h;

			if (x2 > (int) cr_server.muralWidth )
			{
				cr_server.muralWidth = x2;
			}
			if (y2 > (int) cr_server.muralHeight )
			{
				cr_server.muralHeight = y2;
			}
		}
	}
	crWarning( "Total output dimensions = (%d, %d)", cr_server.muralWidth, cr_server.muralHeight );

	crMothershipDisconnect( conn );
}
