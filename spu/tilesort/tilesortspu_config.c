/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "tilesortspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_applications.h"
#include "cr_spu.h"


static void __setDefaults( void )
{
	tilesort_spu.numThreads = 1;

	tilesort_spu.num_servers = 0;
	tilesort_spu.servers = NULL;
	tilesort_spu.splitBeginEnd = 1;
	tilesort_spu.broadcast = 0;
	tilesort_spu.optimizeBucketing = 1;
	tilesort_spu.muralWidth = 0;
	tilesort_spu.muralHeight = 0;
	tilesort_spu.drawBBOX = 0;
	tilesort_spu.bboxLineWidth = 5;

	tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
	tilesort_spu.inDrawPixels = 0;

	tilesort_spu.syncOnFinish = 1;
	tilesort_spu.syncOnSwap = 1;

	tilesort_spu.fakeWindowWidth = 0;
	tilesort_spu.fakeWindowHeight = 0;

	tilesort_spu.scaleToMuralSize = GL_TRUE;

#ifdef WINDOWS
	tilesort_spu.client_hdc = NULL;
	tilesort_spu.client_hwnd = NULL;
#else
	tilesort_spu.glx_display = NULL;
	tilesort_spu.glx_drawable = 0;
#endif
}


void tilesortspuGatherConfiguration( const SPU *child_spu )
{
	CRConnection *conn;
	char response[8096];
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);

	char **serverchain, **serverlist;
	int num_servers;
	int i;

	int optTileWidth = 0, optTileHeight = 0;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}
	crMothershipIdentifySPU( conn, tilesort_spu.id );

	crMothershipGetMTU( conn, response );
	sscanf( response, "%d", &tilesort_spu.MTU );

	crDebug( "Got the MTU as %d", tilesort_spu.MTU );

	if (crMothershipGetSPUParam( conn, response, "split_begin_end") )
	{
		sscanf( response, "%d", &(tilesort_spu.splitBeginEnd) );
	}

	if (crMothershipGetSPUParam( conn, response, "broadcast") )
	{
		sscanf( response, "%d", &(tilesort_spu.broadcast) );
	}

	if (crMothershipGetSPUParam( conn, response, "optimize_bucket") )
	{
		sscanf( response, "%d", &(tilesort_spu.optimizeBucketing) );
	}

	if (crMothershipGetSPUParam( conn, response, "sync_on_swap") )
	{
		sscanf( response, "%d", &(tilesort_spu.syncOnSwap) );
	}

	if (crMothershipGetSPUParam( conn, response, "sync_on_finish") )
	{
		sscanf( response, "%d", &(tilesort_spu.syncOnFinish) );
	}

	if (crMothershipGetSPUParam( conn, response, "draw_bbox") )
	{
		sscanf( response, "%d", &(tilesort_spu.drawBBOX) );
	}

	if (crMothershipGetSPUParam( conn, response, "bbox_line_width") )
	{
		sscanf( response, "%f", &(tilesort_spu.bboxLineWidth) );
	}

	if (crMothershipGetSPUParam( conn, response, "fake_window_dims") )
	{
		float w,h;
		sscanf( response, "%f %f", &w, &h );
		tilesort_spu.fakeWindowWidth = (unsigned int) w;
		tilesort_spu.fakeWindowHeight = (unsigned int) h;
	}

	if (crMothershipGetSPUParam( conn, response, "scale_to_mural_size") )
	{
		sscanf( response, "%d", &(tilesort_spu.scaleToMuralSize) );
	}

	/* The response to this tells us how many servers and where they are 
	 *
	 * For example:  2 tcpip://foo tcpip://bar 
     */
	crMothershipGetServers( conn, response );

	serverchain = crStrSplitn( response, " ", 1 );
	num_servers = crStrToInt( serverchain[0] );

	if (num_servers == 0)
	{
		crError( "No servers specified for a tile/sort SPU?!" );
	}
	serverlist = crStrSplit( serverchain[1], "," );

	tilesort_spu.num_servers = num_servers;
	tilesort_spu.servers = (TileSortSPUServer *) crCalloc( num_servers * sizeof( *(tilesort_spu.servers) ) );

	tilesort_spu.thread[0].net = (CRNetServer *) crCalloc( num_servers * sizeof(CRNetServer) );
	tilesort_spu.thread[0].pack = (CRPackBuffer *) crCalloc( num_servers * sizeof(CRPackBuffer) );

	crDebug( "Got %d servers!", num_servers );

	/*
	 * Get the list of tiles from all servers.
	 * Make sure they're all the same size if optimizeBucketing is true.
	 */
	for (i = 0 ; i < num_servers ; i++)
	{
		char server_url[1024];
		TileSortSPUServer *server = tilesort_spu.servers + i;
		char **tilechain, **tilelist;
		int tile;

		sscanf( serverlist[i], "%s", server_url );

		crDebug( "Server %d: %s", i+1, server_url );
		thread0->net[i].name = crStrdup( server_url );
		thread0->net[i].buffer_size = tilesort_spu.MTU;

		if (!crMothershipGetTiles( conn, response, i ))
		{
			crError( "No tile information for server %d!  I can't continue!", i );
		}

		tilechain = crStrSplitn( response, " ", 1 );
		server->num_extents = crStrToInt( tilechain[0] );

		tilelist = crStrSplit( tilechain[1], "," );

		for (tile = 0; tile < server->num_extents ; tile++)
		{
			int w,h;
			sscanf( tilelist[tile], "%d %d %d %d", &(server->x1[tile]), 
					&(server->y1[tile]), &w, &h );

			server->x2[tile] = server->x1[tile] + w;
			server->y2[tile] = server->y1[tile] + h;

			/* update mural size */
			if (server->x2[tile] > (int) tilesort_spu.muralWidth )
			{
				tilesort_spu.muralWidth = server->x2[tile];
			}
			if (server->y2[tile] > (int) tilesort_spu.muralHeight )
			{
				tilesort_spu.muralHeight = server->y2[tile];
			}

			/* make sure tile is right size for optimizeBucket */
			if (tilesort_spu.optimizeBucketing) {
				if (optTileWidth == 0 && optTileHeight == 0) {
					optTileWidth = w;
					optTileHeight = h;
				}
				else if (w != optTileWidth || h != optTileHeight) {
					crWarning("Tile %d on server %d is not the right size!",
										i, tile);
					crWarning("All tiles must be same size with optimize_bucket.");
					crWarning("Turning off tilesort SPU's optimize_bucket.");
					tilesort_spu.optimizeBucketing = 0;
				}
			}

		}

		crFreeStrings( tilechain );
		crFreeStrings( tilelist );
	}
	crWarning( "Total output dimensions = (%d, %d)", tilesort_spu.muralWidth, tilesort_spu.muralHeight );

	crSPUPropogateGLLimits( conn, tilesort_spu.id, child_spu, &tilesort_spu.limits );

	/* XXX there may be a better way of doing this */
	tilesort_spu.limits.maxViewportDims[0] = MAX_MURAL_WIDTH;
	tilesort_spu.limits.maxViewportDims[1] = MAX_MURAL_HEIGHT;

	crFreeStrings( serverchain );
	crFreeStrings( serverlist );

	crMothershipDisconnect( conn );
}
