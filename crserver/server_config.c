/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"

#include "server.h"

static void __setDefaults( void )
{
	cr_server.tcpip_port = 7000;
	cr_server.numExtents = 0;
	cr_server.curExtent = 0;
	cr_server.muralWidth = 0;
	cr_server.muralHeight = 0;
	cr_server.optimizeBucket = 1;
	cr_server.useL2 = 0;
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
	int numClients;
	char **tilechain, **tilelist;
	int num_servers;
	int a_non_file_client;

	char **serverchain;


#if 0
	CRLimitsState limits;
#endif

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

	/* The response will tell which SPUs to load */
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

	cr_server.head_spu = crSPULoadChain( num_spus, spu_ids, spu_names, spu_dir, &cr_server );

	/* Need to do this as early as possible */
	cr_server.head_spu->dispatch_table.GetIntegerv( GL_VIEWPORT,
		(GLint*)cr_server.underlyingDisplay );

	/* Get OpenGL limits from first SPU */
	crSPUQueryGLLimits( conn, spu_ids[0], &cr_server.limits);

	/* XXX revisit viewport clamping someday */
	cr_server.limits.maxViewportDims[0] = MAX_MURAL_WIDTH;
	cr_server.limits.maxViewportDims[1] = MAX_MURAL_HEIGHT;

	crFree( spu_ids );
	crFree( spu_names );
	crFree( spuchain );


	if (crMothershipGetServerParam( conn, response, "port" ))
	{
		cr_server.tcpip_port = crStrToInt( response );
	}
	if (crMothershipGetServerParam( conn, response, "optimize_bucket" ))
	{
		cr_server.optimizeBucket = crStrToInt( response );
	}
	if (crMothershipGetServerParam( conn, response, "lightning2" ))
	{
		cr_server.useL2 = crStrToInt( response );
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%ud", &mtu );

	/* The response will tell us what protocols we need to serve 
	 * example: "3 tcpip 1,gm 2,via 10" */

	crMothershipGetClients( conn, response );
	
	clientchain = crStrSplitn( response, " ", 1 );
	numClients = crStrToInt( clientchain[0] );
	if (numClients == 0)
	{
		crError( "I have no clients!  What's a poor server to do?" );
	}
	clientlist = crStrSplit( clientchain[1], "," );

	cr_server.numClients = numClients;
	cr_server.clients = (CRClient *) crAlloc( numClients * sizeof( *(cr_server.clients) ) );

	if (!crMothershipGetServerTiles( conn, response ))
	{
		crDebug( "No tiling information for server!" );
	}
	else
	{
		/* response is of the form: "N x y w h, x y w h, ..."
		 * where N is the number of tiles and each set of 'x y w h'
		 * values describes the tile location and size.
		 */
		tilechain = crStrSplitn( response, " ", 1 );
		cr_server.numExtents = crStrToInt( tilechain[0] );
		cr_server.maxTileHeight = 0;
		tilelist = crStrSplit( tilechain[1], "," );
		for (i = 0 ; i < cr_server.numExtents; i++)
		{
			float x, y, w, h;
			sscanf( tilelist[i], "%f %f %f %f", &x, &y, &w, &h );
			cr_server.x1[i] = (int) x;
			cr_server.y1[i] = (int) y;
			cr_server.x2[i] = cr_server.x1[i] + (int) w;
			cr_server.y2[i] = cr_server.y1[i] + (int) h;
			if (h > cr_server.maxTileHeight)
			{
				cr_server.maxTileHeight = (int) h;
			}
			crDebug( "Added tile: %d %d %d %d", cr_server.x1[i], cr_server.y1[i], cr_server.x2[i], cr_server.y2[i] );
		}
	}

	a_non_file_client = -1;
	for (i = 0 ; i < numClients ; i++)
	{
		char protocol[1024];
		int j;

		sscanf( clientlist[i], "%s %d", protocol, &(cr_server.clients[i].spu_id) );
		cr_server.clients[i].conn = crNetAcceptClient( protocol, cr_server.tcpip_port, mtu, 1 );

		/* Init all context-related variables */
		for (j = 0; j < CR_MAX_CONTEXTS; j++)
			cr_server.clients[i].context[j] = NULL;
		cr_server.clients[i].currentCtx = NULL;

		cr_server.clients[i].number = i;

		if (crStrncmp(protocol,"file",crStrlen("file")))
		{
			a_non_file_client = i;
		}
	}
	cr_server.SpuContext = 0;

	if (a_non_file_client != -1)
	{
	/* Sigh -- the servers need to know how big the whole mural is if we're 
	 * doing tiling, so they can compute their base projection.  For now, 
	 * just have them pretend to be one of their client SPU's, and redo 
	 * the configuration step of the tilesort SPU.  Basically this is a dirty 
	 * way to figure out who the other servers are.  It *might* matter 
	 * which SPU we pick for certain graph configurations, but we'll cross 
	 * that bridge later.
	 *
	 * As long as we're at it, we're going to verify that all the tile
	 * sizes are uniform when optimizeBucket is true.
	 */

		int optTileWidth = 0, optTileHeight = 0;

		crMothershipIdentifySPU( conn, cr_server.clients[0].spu_id );
		crMothershipGetServers( conn, response );

	/* crMothershipGetServers() response is of the form
	 * "N protocol://ip:port protocol://ipnumber:port ..."
	 * For example: "2 tcpip://10.0.0.1:7000 tcpip://10.0.0.2:7000"
	 */
		serverchain = crStrSplitn( response, " ", 1 );
		num_servers = crStrToInt( serverchain[0] );

		if (num_servers == 0)
		{
			crError( "No servers specified for SPU %d?!", cr_server.clients[0].spu_id );
		}

		num_servers = num_servers;

		for (i = 0 ; i < num_servers ; i++)
		{
			int numExtents;
			int tile;
			if (!crMothershipGetTiles( conn, response, i ))
			{
				break;
			}

			tilechain = crStrSplitn( response, " ", 1 );
			numExtents = crStrToInt( tilechain[0] );

			tilelist = crStrSplit( tilechain[1], "," );

			for (tile = 0; tile < numExtents ; tile++)
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

				if (cr_server.optimizeBucket) {
					if (optTileWidth == 0 && optTileHeight == 0) {
						/* first tile */
						optTileWidth = w;
						optTileHeight = h;
					}
					else {
						/* subsequent tile - make sure it's the same size as first */
						if (w != optTileWidth || h != optTileHeight) {
							crWarning("Tile %d on server %d is not the right size!",
												i, tile);
							crWarning("All tiles must be same size with optimize_bucket.");
							crWarning("Turning off server's optimize_bucket.");
							cr_server.optimizeBucket = 0;
						}
					}
				}
			}
		}
		crWarning( "Total output dimensions = (%d, %d)", cr_server.muralWidth, cr_server.muralHeight );
	}
	else
	{
		crWarning( "It looks like there are nothing but file clients.  That suits me just fine." );
	}


	crMothershipDisconnect( conn );
}
