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
	tilesort_spu.num_servers = 0;
	tilesort_spu.servers = NULL;
	tilesort_spu.splitBeginEnd = 1;
	tilesort_spu.broadcast = 0;
	tilesort_spu.optimizeBucketing = 1;
	tilesort_spu.muralWidth = 0;
	tilesort_spu.muralHeight = 0;
	tilesort_spu.drawBBOX = 0;
	tilesort_spu.bboxLineWidth = 5;

	tilesort_spu.providedBBOX = CR_DEFAULT_BBOX_HINT;
	tilesort_spu.inDrawPixels = 0;

	tilesort_spu.syncOnFinish = 1;
	tilesort_spu.syncOnSwap = 1;

	tilesort_spu.fakeWindowWidth = 0;
	tilesort_spu.fakeWindowHeight = 0;
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

	char **serverchain, **serverlist;
	int num_servers;
	int i;

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
	tilesort_spu.servers = (TileSortSPUServer *) crAlloc( num_servers * sizeof( *(tilesort_spu.servers) ) );

	crDebug( "Got %d servers!", num_servers );

	for (i = 0 ; i < num_servers ; i++)
	{
		char server_url[1024];
		TileSortSPUServer *server = tilesort_spu.servers + i;
		char **tilechain, **tilelist;
		int tile;

		sscanf( serverlist[i], "%s", server_url );

		crDebug( "Server %d: %s", i+1, server_url );
		server->net.name = crStrdup( server_url );
		server->net.buffer_size = tilesort_spu.MTU;

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

			if (server->x2[tile] > (int) tilesort_spu.muralWidth )
			{
				tilesort_spu.muralWidth = server->x2[tile];
			}
			if (server->y2[tile] > (int) tilesort_spu.muralHeight )
			{
				tilesort_spu.muralHeight = server->y2[tile];
			}
		}

                crFreeStrings( tilechain );
                crFreeStrings( tilelist );
	}
	crWarning( "Total output dimensions = (%d, %d)", tilesort_spu.muralWidth, tilesort_spu.muralHeight );

        crSPUPropogateGLLimits( conn, tilesort_spu.id, child_spu, &tilesort_spu.limits );

        crFreeStrings( serverchain );
        crFreeStrings( serverlist );

	crMothershipDisconnect( conn );
}
