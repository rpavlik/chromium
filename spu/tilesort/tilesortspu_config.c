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
#include "cr_warp.h"
#include "cr_hull.h"

static void
__setDefaults(void)
{
	tilesort_spu.numThreads = 1;

	tilesort_spu.num_servers = 0;
	tilesort_spu.servers = NULL;
	tilesort_spu.muralWidth = 0;
	tilesort_spu.muralHeight = 0;
	tilesort_spu.muralColumns = 0;
	tilesort_spu.muralRows = 0;

	tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
	tilesort_spu.inDrawPixels = 0;

/*  	tilesort_spu.splitBeginEnd = 1; */
/*  	tilesort_spu.broadcast = 0; */
/*  	tilesort_spu.drawBBOX = 0; */
/*  	tilesort_spu.bboxLineWidth = 5; */
/*  	tilesort_spu.syncOnFinish = 1; */
/*  	tilesort_spu.syncOnSwap = 1; */
/*  	tilesort_spu.fakeWindowWidth = 0; */
/*  	tilesort_spu.fakeWindowHeight = 0; */
/*  	tilesort_spu.scaleToMuralSize = GL_TRUE; */

#ifdef WINDOWS
	tilesort_spu.client_hdc = NULL;
	tilesort_spu.client_hwnd = NULL;
#else
	tilesort_spu.glx_display = NULL;
	tilesort_spu.glx_drawable = 0;
#endif
}


static void
set_split_begin_end(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.splitBeginEnd));
}

static void
set_broadcast(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.broadcast));
}

static void
set_optimize_bucket(void *foo, const char *response)
{
	sscanf(response, "%d", &tilesort_spu.optimizeBucketing);
	/* 0 = no optimization */
	/* 1 = all tiles same size, use hashing */
	/* 2 = non-uniform grid, semi-optimized bucketing */
	/* 3 = non-rectangular grid, corners given by world_extents[][], should be uber slow */
}

static void
set_sync_on_swap(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.syncOnSwap));
}

static void
set_sync_on_finish(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.syncOnFinish));
}

static void
set_draw_bbox(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.drawBBOX));
}

static void
set_bbox_line_width(void *foo, const char *response)
{
	sscanf(response, "%f", &(tilesort_spu.bboxLineWidth));
}

static void
set_fake_window_dims(void *foo, const char *response)
{
	float w, h;
	if (response[0] == '[')
		sscanf(response, "[ %f, %f ]", &w, &h);
	else
		sscanf(response, "%f %f", &w, &h);
	tilesort_spu.fakeWindowWidth = (unsigned int) w;
	tilesort_spu.fakeWindowHeight = (unsigned int) h;
}

static void
set_scale_to_mural_size(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.scaleToMuralSize));
}

static void
set_emit(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.emit_GATHER_POST_SWAPBUFFERS));
}

static void
set_local_tile_spec(void *foo, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu.localTileSpec));
}

static void
set_bucket_mode(void *foo, const char *response)
{
	if (crStrcmp(response, "Broadcast") == 0)
		tilesort_spu.bucketMode = BROADCAST;
	else if (crStrcmp(response, "Test All Tiles") == 0)
		tilesort_spu.bucketMode = TEST_ALL_TILES;
	else if (crStrcmp(response, "Uniform Grid") == 0)
		tilesort_spu.bucketMode = UNIFORM_GRID;
	else if (crStrcmp(response, "Non-Uniform Grid") == 0)
		tilesort_spu.bucketMode = NON_UNIFORM_GRID;
	else if (crStrcmp(response, "Random") == 0)
		tilesort_spu.bucketMode = RANDOM;
	else if (crStrcmp(response, "Warped Grid") == 0)
		tilesort_spu.bucketMode = WARPED_GRID;
	else
	{
		crWarning("Bad value (%s) for tilesort bucket_mode", response);
		tilesort_spu.bucketMode = BROADCAST;
	}
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions tilesortSPUOptions[] = {
	{"split_begin_end", CR_BOOL, 1, "1", NULL, NULL,
	 "Split glBegin/glEnd", (SPUOptionCB) set_split_begin_end},

	{"sync_on_swap", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on SwapBuffers", (SPUOptionCB) set_sync_on_swap},

	{"sync_on_finish", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on glFinish", (SPUOptionCB) set_sync_on_finish},

	{"draw_bbox", CR_BOOL, 1, "0", NULL, NULL,
	 "Draw Bounding Boxes", (SPUOptionCB) set_draw_bbox},

	{"bbox_line_width", CR_INT, 1, "5", "0", "10",
	 "Bounding Box Line Width", (SPUOptionCB) set_bbox_line_width},

	{"fake_window_dims", CR_INT, 2, "0, 0", "0, 0", NULL,
	 "Fake Window Dimensions (w, h)", (SPUOptionCB) set_fake_window_dims},

	{"scale_to_mural_size", CR_BOOL, 1, "1", NULL, NULL,
	 "Scale to Mural Size", (SPUOptionCB) set_scale_to_mural_size},

	{"emit_GATHER_POST_SWAPBUFFERS", CR_BOOL, 1, "0", NULL, NULL,
	 "Emit a glParameteri After SwapBuffers", (SPUOptionCB) set_emit},

	{"local_tile_spec", CR_BOOL, 1, "0", NULL, NULL,
	 "Specify Tiles Relative to Displays", (SPUOptionCB) set_local_tile_spec},

	{"bucket_mode", CR_ENUM, 1, "Test All Tiles",
	 "'Broadcast', 'Test All Tiles', 'Uniform Grid', 'Non-Uniform Grid', 'Random', 'Warped Grid'", NULL,
	 "Geometry Bucketing Method", (SPUOptionCB) set_bucket_mode},

        /* OBSOLETE option! */
	{"broadcast", CR_BOOL, 1, "0", NULL, NULL,
	 "Broadcast (obsolete)", (SPUOptionCB) set_broadcast},

        /* OBSOLETE option! */
	{"optimize_bucket", CR_BOOL, 1, "1", NULL, NULL,
	 "Optimize Bucket (obsolete)", (SPUOptionCB) set_optimize_bucket},

	{NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL},
};


void
tilesortspuGatherConfiguration(const SPU * child_spu)
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */

	conn = crMothershipConnect();
	if (!conn)
	{
		crError
			("Couldn't connect to the mothership -- I have no idea what to do!");
	}
	crMothershipIdentifySPU(conn, tilesort_spu.id);

	crSPUGetMothershipParams(conn, (void *) &tilesort_spu, tilesortSPUOptions);

        /* If tilesort_spu.optimizeBucketing is not 1 (the default) use
         * its value to override bucketMode.
         * Similarly, if tilesort_spu.broadcast is not 0, use its value
         * to override bucketMode.
         * XXX remove this code when we remove the 'optimize_bucket' and
         * 'broadcast' config options.
         */
        if (tilesort_spu.optimizeBucketing != 1)
          tilesort_spu.bucketMode = TEST_ALL_TILES;
        if (tilesort_spu.broadcast)
          tilesort_spu.bucketMode = BROADCAST;

	tilesort_spu.MTU = crMothershipGetMTU(conn);
	crDebug("Got the MTU as %d", tilesort_spu.MTU);

	/* get a buffer which can hold one big big opcode (counter computing
	 * against packer/pack_buffer.c)
	 */
	tilesort_spu.buffer_size =
		((((tilesort_spu.MTU - sizeof(CRMessageOpcodes)) * 5 + 3) / 4 +
			0x3) & ~0x3) + sizeof(CRMessageOpcodes);

	tilesortspuGetTileInformation(conn);
	tilesortspuComputeRowsColumns();

	crWarning("Total output dimensions = (%d, %d)", tilesort_spu.muralWidth,
						tilesort_spu.muralHeight);

	crSPUPropogateGLLimits(conn, tilesort_spu.id, child_spu,
												 &tilesort_spu.limits);

	crMothershipDisconnect(conn);
}



/*
 * Get the tile information (count, extents, etc) from the servers
 * and build our corresponding data structures.
 * Input: conn - mothership connection
 */
void
tilesortspuGetTileInformation(CRConnection * conn)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	char response[8096];
	char **serverchain, **serverlist;
	int optTileWidth = 0, optTileHeight = 0;
	int num_servers;
	int i;

	/* The response to this tells us how many servers and where they are 
	 *
	 * For example:  2 tcpip://foo tcpip://bar 
	 */
	crMothershipGetServers(conn, response);

	serverchain = crStrSplitn(response, " ", 1);
	num_servers = crStrToInt(serverchain[0]);

	if (num_servers == 0)
	{
		crError("No servers specified for a tile/sort SPU?!");
	}
	serverlist = crStrSplit(serverchain[1], ",");

	tilesort_spu.num_servers = num_servers;
	tilesort_spu.servers =
		(TileSortSPUServer *) crCalloc(num_servers *
																	 sizeof(*(tilesort_spu.servers)));

	tilesort_spu.thread[0].net =
		(CRNetServer *) crCalloc(num_servers * sizeof(CRNetServer));
	tilesort_spu.thread[0].pack =
		(CRPackBuffer *) crCalloc(num_servers * sizeof(CRPackBuffer));

	crDebug("Got %d servers!", num_servers);

	/* XXX: there is lots of overlap between these cases. merge! */
	if (tilesort_spu.localTileSpec)
	{
		char **displaylist, **displaychain;
		double *corners;
		int num_displays, idx;

		/* 
		 * Here we has tiles specified relative to the origin of 
		 * whichever display they happen to land in. First, gather
		 * the list of displays, then get the tiles that belong on it 
		 */
		crMothershipGetDisplays(conn, response);
		displaychain = crStrSplitn(response, " ", 1);
		displaylist = crStrSplit(displaychain[1], ",");

		num_displays = crStrToInt(displaychain[0]);

		corners = (double *) crAlloc(num_displays * 8 * sizeof(double));

		tilesort_spu.displays =
			(display_t *) crAlloc(num_displays * sizeof(display_t));

		idx = 0;

		for (i = 0; i < num_displays; i++)
		{
			float pnt[2], warped[2];

			sscanf(displaylist[i],
						 "%d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						 &tilesort_spu.displays[i].id, &tilesort_spu.displays[i].width,
						 &tilesort_spu.displays[i].height,
						 &tilesort_spu.displays[i].correct[0],
						 &tilesort_spu.displays[i].correct[1],
						 &tilesort_spu.displays[i].correct[2],
						 &tilesort_spu.displays[i].correct[3],
						 &tilesort_spu.displays[i].correct[4],
						 &tilesort_spu.displays[i].correct[5],
						 &tilesort_spu.displays[i].correct[6],
						 &tilesort_spu.displays[i].correct[7],
						 &tilesort_spu.displays[i].correct[8],
						 &tilesort_spu.displays[i].correct_inv[0],
						 &tilesort_spu.displays[i].correct_inv[1],
						 &tilesort_spu.displays[i].correct_inv[2],
						 &tilesort_spu.displays[i].correct_inv[3],
						 &tilesort_spu.displays[i].correct_inv[4],
						 &tilesort_spu.displays[i].correct_inv[5],
						 &tilesort_spu.displays[i].correct_inv[6],
						 &tilesort_spu.displays[i].correct_inv[7],
						 &tilesort_spu.displays[i].correct_inv[8]);

			pnt[0] = -1;
			pnt[1] = -1;
			crWarpPoint(tilesort_spu.displays[i].correct_inv, pnt, warped);
			corners[idx] = warped[0];
			corners[idx + 1] = warped[1];
			idx += 2;
			pnt[0] = -1;
			pnt[1] = 1;
			crWarpPoint(tilesort_spu.displays[i].correct_inv, pnt, warped);
			corners[idx] = warped[0];
			corners[idx + 1] = warped[1];
			idx += 2;
			pnt[0] = 1;
			pnt[1] = 1;
			crWarpPoint(tilesort_spu.displays[i].correct_inv, pnt, warped);
			corners[idx] = warped[0];
			corners[idx + 1] = warped[1];
			idx += 2;
			pnt[0] = 1;
			pnt[1] = -1;
			crWarpPoint(tilesort_spu.displays[i].correct_inv, pnt, warped);
			corners[idx] = warped[0];
			corners[idx + 1] = warped[1];
			idx += 2;
		}

		crHullInteriorBox(corners, idx / 2, tilesort_spu.world_bbox);
		crFree(corners);

		/* dummy values in case i forget to override them later */
		tilesort_spu.muralWidth = tilesort_spu.displays[0].width;
		tilesort_spu.muralHeight = tilesort_spu.displays[0].height;

		for (i = 0; i < num_servers; i++)
		{
			char server_url[1024];
			TileSortSPUServer *server = tilesort_spu.servers + i;
			char **tilechain, **tilelist;
			int tile;
			double cent[2], Sx, Sy;

			sscanf(serverlist[i], "%s", server_url);

			crDebug("Server %d: %s", i + 1, server_url);
			thread0->net[i].name = crStrdup(server_url);
			thread0->net[i].buffer_size = tilesort_spu.MTU;

			/* response is just like regular GetTiles, but each tile
			 * is preceeded by a display id */
			if (!crMothershipGetDisplayTiles(conn, response, i))
			{
				crError("No tile information for server %d!  I can't continue!", i);
			}

			tilechain = crStrSplitn(response, " ", 1);
			server->num_extents = crStrToInt(tilechain[0]);
			tilelist = crStrSplit(tilechain[1], ",");

			Sx = 2.0 / (tilesort_spu.world_bbox[2] - tilesort_spu.world_bbox[0]);
			Sy = 2.0 / (tilesort_spu.world_bbox[3] - tilesort_spu.world_bbox[1]);

			cent[0] =
				(tilesort_spu.world_bbox[0] + tilesort_spu.world_bbox[2]) / (2.);
			cent[1] =
				(tilesort_spu.world_bbox[1] + tilesort_spu.world_bbox[3]) / (2.);

			for (tile = 0; tile < server->num_extents; tile++)
			{
				int id, x, y, w, h, a;
				float disp_w, disp_h, warped[2], *align;

				sscanf(tilelist[tile], "%d %d %d %d %d", &id, &x, &y, &w, &h);

				/* this is the local coordinate tiling */
				server->display_ndx[tile] = -1;
				for (a = 0; a < num_displays; a++)
				{
					if (tilesort_spu.displays[a].id == id)
						server->display_ndx[tile] = a;
				}
				if (server->display_ndx[tile] == -1)
					crError("Invalid Display ID %d", id);
				server->extents[tile].x1 = x;
				server->extents[tile].y1 = y;
				server->extents[tile].x2 = x + w;
				server->extents[tile].y2 = y + h;

				/* warp the tile to mural space */
				disp_w = (float) tilesort_spu.displays[server->display_ndx[tile]].width;
				disp_h = (float) tilesort_spu.displays[server->display_ndx[tile]].height;

				server->world_extents[tile][0] = (float) x / disp_w;
				server->world_extents[tile][1] = (float) y / disp_h;

				server->world_extents[tile][2] = (float) x / disp_w;
				server->world_extents[tile][3] = (float) (y + h) / disp_h;

				server->world_extents[tile][4] = (float) (x + w) / disp_w;
				server->world_extents[tile][5] = (float) (y + h) / disp_h;

				server->world_extents[tile][6] = (float) (x + w) / disp_w;
				server->world_extents[tile][7] = (float) y / disp_h;

				for (a = 0; a < 8; a++)
					server->world_extents[tile][a] =
						(float)2.0 * server->world_extents[tile][a] - (float)1.0;

				align = tilesort_spu.displays[server->display_ndx[tile]].correct_inv;
				for (a = 0; a < 4; a++)
				{
					crWarpPoint(align, server->world_extents[tile] + 2 * a, warped);

					warped[0] = (warped[0] - (float)cent[0]) * (float) Sx;
					warped[1] = (warped[1] - (float)cent[1]) * (float) Sy;

					crDebug("%f %f warps to %f %f", server->world_extents[tile][2 * a],
									server->world_extents[tile][2 * a + 1], warped[0],
									warped[1]);

					server->world_extents[tile][2 * a] = warped[0];
					server->world_extents[tile][2 * a + 1] = warped[1];
				}


				/* XXX: check that the tile fits w/i the display it is on */

			}
			crFreeStrings(tilechain);
			crFreeStrings(tilelist);
		}

		crFreeStrings(displaychain);
		crFreeStrings(displaylist);

		/* can't do any fancy bucketing with this */
		tilesort_spu.bucketMode = WARPED_GRID;
	}
	else
	{
		/*
		 * Get the list of tiles from all servers.
		 * Make sure they're all the same size if bucketMode is UNIFORM_GRID.
		 */
		for (i = 0; i < num_servers; i++)
		{
			char server_url[1024];
			TileSortSPUServer *server = tilesort_spu.servers + i;
			char **tilechain, **tilelist;
			int tile;

			sscanf(serverlist[i], "%s", server_url);

			crDebug("Server %d: %s", i + 1, server_url);
			thread0->net[i].name = crStrdup(server_url);
			thread0->net[i].buffer_size = tilesort_spu.buffer_size;

			if (!crMothershipGetTiles(conn, response, i))
			{
				crError("No tile information for server %d!  I can't continue!", i);
			}

			tilechain = crStrSplitn(response, " ", 1);
			server->num_extents = crStrToInt(tilechain[0]);

			tilelist = crStrSplit(tilechain[1], ",");

			for (tile = 0; tile < server->num_extents; tile++)
			{
				int x, y, w, h;

				sscanf(tilelist[tile], "%d %d %d %d", &x, &y, &w, &h);
				server->extents[tile].x1 = x;
				server->extents[tile].y1 = y;
				server->extents[tile].x2 = x + w;
				server->extents[tile].y2 = y + h;

				/* update mural size */
				if (server->extents[tile].x2 > (int) tilesort_spu.muralWidth)
				{
					tilesort_spu.muralWidth = server->extents[tile].x2;
				}
				if (server->extents[tile].y2 > (int) tilesort_spu.muralHeight)
				{
					tilesort_spu.muralHeight = server->extents[tile].y2;
				}

				/* make sure tile is right size for UNIFORM_GRID */
				if (tilesort_spu.bucketMode == UNIFORM_GRID)
				{
					if (optTileWidth == 0 && optTileHeight == 0)
					{
						optTileWidth = w;
						optTileHeight = h;
					}
					else if (w != optTileWidth || h != optTileHeight)
					{
						crWarning("Tile %d on server %d is not the right size!", i, tile);
						crWarning("All tiles must be same size with optimize_bucket.");
						crWarning("Turning off tilesort SPU's optimize_bucket.");
						tilesort_spu.bucketMode = TEST_ALL_TILES;
					}
				}
			}

			crFreeStrings(tilechain);
			crFreeStrings(tilelist);
		}
	}

	crFreeStrings(serverchain);
	crFreeStrings(serverlist);
}


/*
 * Examine the tilesort extents to try to compute the number of
 * tile rows and columns.
 */
void
tilesortspuComputeRowsColumns(void)
{
	int i;

	tilesort_spu.muralColumns = 0;
	tilesort_spu.muralRows = 0;

	/*
	 * This is a bit simple-minded, but usually works.
	 * We look for the number of tiles with x1=0, that's the number of rows.
	 * Then look for number of tiles with y1==0, that's the number of columns.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int j;

		for (j = 0; j < server->num_extents; j++)
		{
			if (server->extents[j].x1 == 0)
				tilesort_spu.muralRows++;
			if (server->extents[j].y1 == 0)
				tilesort_spu.muralColumns++;
		}
	}

	/* Set rows/columns to zero if there's any doubt. */
	if (tilesort_spu.muralColumns == 0 || tilesort_spu.muralRows == 0)
	{
		tilesort_spu.muralColumns = 0;
		tilesort_spu.muralRows = 0;
	}

	crDebug("Tilesort size: %d columns x %d rows\n",
					tilesort_spu.muralColumns, tilesort_spu.muralRows);
}
