/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include "cr_mothership.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_warp.h"
#include "cr_hull.h"


/**
 * Examine the server tile boundaries to compute the overall max
 * viewport dims.  Then send those dims to the servers.
 *
 * XXX \todo This isn't used!?!
 */
void
tilesortspuComputeMaxViewport(WindowInfo *winInfo)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLint totalDims[2];
	int i;


	/* release geometry buffer, if it's bound */
	crPackReleaseBuffer( thread0->packer );

	/*
	 * It's hard to say what the max viewport size should be.
	 * We've changed this computation a few times now.
	 * For now, we set it to twice the mural size, or at least 4K.
	 * One problem is that the mural size can change dynamically...
	 */
	totalDims[0] = 2 * winInfo->muralWidth;
	totalDims[1] = 2 * winInfo->muralHeight;
	if (totalDims[0] < 4096)
		totalDims[0] = 4096;
	if (totalDims[1] < 4096)
		totalDims[1] = 4096;

	tilesort_spu.limits.maxViewportDims[0] = totalDims[0];
	tilesort_spu.limits.maxViewportDims[1] = totalDims[1];

	/* 
	 * Once we've computed the maximum viewport size, we send
	 * a message to each server with its new viewport parameters.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		crPackSetBuffer( thread0->packer, &(thread0->buffer[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);
		else
			crPackChromiumParametervCR(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);

		/* release server buffer */
		crPackReleaseBuffer( thread0->packer );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );
	}
}


/**
 * Send the current tilesort tile info to all the servers.
 */
void
tilesortspuSendTileInfoToServers( WindowInfo *winInfo )
{
	GET_THREAD(thread);
	int i;

	/* release geometry buffer */
	crPackReleaseBuffer( thread->packer );

	/* loop over servers */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		ServerWindowInfo *servWinInfo = winInfo->server + i;
		int tileInfo[4 + 4 * CR_MAX_EXTENTS], arraySize;
		int j;

		/* build tileInfo array */
		tileInfo[0] = i;
		tileInfo[1] = winInfo->muralWidth;
		tileInfo[2] = winInfo->muralHeight;
		tileInfo[3] = servWinInfo->num_extents;
		for (j = 0; j < servWinInfo->num_extents; j++)
		{
			int w = servWinInfo->extents[j].x2 - servWinInfo->extents[j].x1;
			int h = servWinInfo->extents[j].y2 - servWinInfo->extents[j].y1;
			tileInfo[4 + j * 4 + 0] = servWinInfo->extents[j].x1;
			tileInfo[4 + j * 4 + 1] = servWinInfo->extents[j].y1;
			tileInfo[4 + j * 4 + 2] = w;
			tileInfo[4 + j * 4 + 3] = h;
		}
		arraySize = 4 + 4 * servWinInfo->num_extents;

		/* pack/send to server[i] */
		crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_TILE_INFO_CR, GL_INT,
										   arraySize, tileInfo);
		else
			crPackChromiumParametervCR(GL_TILE_INFO_CR, GL_INT,
									   arraySize, tileInfo);

		/* release server buffer */
		crPackReleaseBuffer( thread->packer );
	}

	/* Restore default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
}


/**
 * Get the tile information (count, extents, etc) from the servers
 * and build our corresponding data structures.
 * Input: conn - mothership connection
 */
void
tilesortspuGetTilingFromServers(CRConnection *conn, WindowInfo *winInfo)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	char response[8096];
	char **serverchain, **serverlist;
	int num_servers;
	int i;

	crDebug("Getting tile information from servers.");
	/* The response to this tells us how many servers and where they are 
	 *
	 * For example:  2 tcpip://foo tcpip://bar 
	 */
	crMothershipGetServers(conn, response);

	serverchain = crStrSplitn(response, " ", 1);
	num_servers = crStrToInt(serverchain[0]);
	CRASSERT(num_servers == tilesort_spu.num_servers);
	serverlist = crStrSplit(serverchain[1], ",");

	tilesort_spu.thread[0].netServer =
		(CRNetServer *) crCalloc(num_servers * sizeof(CRNetServer));
	tilesort_spu.thread[0].buffer =
		(CRPackBuffer *) crCalloc(num_servers * sizeof(CRPackBuffer));

	/** XXX: \todo there is lots of overlap between these cases. merge! */
	if (tilesort_spu.localTileSpec)
	{
		char **displaylist, **displaychain;
		double *corners;
		int num_displays, idx;
		double world_bbox[4];

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
			(WarpDisplayInfo *) crAlloc(num_displays * sizeof(WarpDisplayInfo));

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

		crHullInteriorBox(corners, idx / 2, world_bbox);
		crFree(corners);

		/* dummy values in case i forget to override them later */
		winInfo->muralWidth = tilesort_spu.displays[0].width;
		winInfo->muralHeight = tilesort_spu.displays[0].height;

		winInfo->passiveStereo = GL_FALSE;  /* may be set true below */

		for (i = 0; i < num_servers; i++)
		{
			char server_url[1024];
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			char **tilechain, **tilelist;
			int tile;
			double cent[2], Sx, Sy;

			sscanf(serverlist[i], "%s", server_url);

			crDebug("Server %d: %s", i + 1, server_url);
			thread0->netServer[i].name = crStrdup(server_url);
			thread0->netServer[i].buffer_size = tilesort_spu.buffer_size;

			/* response is just like regular GetTiles, but each tile
			 * is preceeded by a display id */
			if (!crMothershipGetDisplayTiles(conn, response, i))
			{
				crError("No tile information for server %d!  I can't continue!", i);
			}

			tilechain = crStrSplitn(response, " ", 1);
			servWinInfo->num_extents = crStrToInt(tilechain[0]);
			tilelist = crStrSplit(tilechain[1], ",");

			servWinInfo->eyeFlags = EYE_LEFT | EYE_RIGHT;
			if (crMothershipGetServerParamFromSPU( conn, i,
																						 "stereo_view", response)) {
				if (crStrcmp(response, "left") == 0) {
					servWinInfo->eyeFlags = EYE_LEFT;
					winInfo->passiveStereo = GL_TRUE;
				}
				else if (crStrcmp(response, "right") == 0) {
					servWinInfo->eyeFlags = EYE_RIGHT;
					winInfo->passiveStereo = GL_TRUE;
				}
			}

			Sx = 2.0 / (world_bbox[2] - world_bbox[0]);
			Sy = 2.0 / (world_bbox[3] - world_bbox[1]);

			cent[0] = (world_bbox[0] + world_bbox[2]) / (2.);
			cent[1] = (world_bbox[1] + world_bbox[3]) / (2.);

			for (tile = 0; tile < servWinInfo->num_extents; tile++)
			{
				int id, x, y, w, h, a;
				float disp_w, disp_h, warped[2], *align;

				sscanf(tilelist[tile], "%d %d %d %d %d", &id, &x, &y, &w, &h);

				/* this is the local coordinate tiling */
				servWinInfo->display_ndx[tile] = -1;
				for (a = 0; a < num_displays; a++)
				{
					if (tilesort_spu.displays[a].id == id)
						servWinInfo->display_ndx[tile] = a;
				}
				if (servWinInfo->display_ndx[tile] == -1)
					crError("Invalid Display ID %d", id);
				servWinInfo->extents[tile].x1 = x;
				servWinInfo->extents[tile].y1 = y;
				servWinInfo->extents[tile].x2 = x + w;
				servWinInfo->extents[tile].y2 = y + h;

				/* warp the tile to mural space */
				disp_w = (float) tilesort_spu.displays[servWinInfo->display_ndx[tile]].width;
				disp_h = (float) tilesort_spu.displays[servWinInfo->display_ndx[tile]].height;

				servWinInfo->world_extents[tile][0] = (float) x / disp_w;
				servWinInfo->world_extents[tile][1] = (float) y / disp_h;

				servWinInfo->world_extents[tile][2] = (float) x / disp_w;
				servWinInfo->world_extents[tile][3] = (float) (y + h) / disp_h;

				servWinInfo->world_extents[tile][4] = (float) (x + w) / disp_w;
				servWinInfo->world_extents[tile][5] = (float) (y + h) / disp_h;

				servWinInfo->world_extents[tile][6] = (float) (x + w) / disp_w;
				servWinInfo->world_extents[tile][7] = (float) y / disp_h;

				for (a = 0; a < 8; a++)
					servWinInfo->world_extents[tile][a] =
						(float)2.0 * servWinInfo->world_extents[tile][a] - (float)1.0;

				align = tilesort_spu.displays[servWinInfo->display_ndx[tile]].correct_inv;
				for (a = 0; a < 4; a++)
				{
					crWarpPoint(align, servWinInfo->world_extents[tile] + 2 * a, warped);

					warped[0] = (warped[0] - (float)cent[0]) * (float) Sx;
					warped[1] = (warped[1] - (float)cent[1]) * (float) Sy;

					crDebug("%f %f warps to %f %f", servWinInfo->world_extents[tile][2 * a],
									servWinInfo->world_extents[tile][2 * a + 1], warped[0],
									warped[1]);

					servWinInfo->world_extents[tile][2 * a] = warped[0];
					servWinInfo->world_extents[tile][2 * a + 1] = warped[1];
				}


				/** XXX: \todo check that the tile fits w/i the display it is on */
			}
			crFreeStrings(tilechain);
			crFreeStrings(tilelist);
		}

		crFreeStrings(displaychain);
		crFreeStrings(displaylist);

		/* can't do any fancy bucketing with this */
		tilesort_spu.defaultBucketMode = WARPED_GRID;
	}
	else
	{
		/*
		 * Get the list of tiles from all servers.
		 * And compute mural size.
		 */
		winInfo->muralWidth = 0;
		winInfo->muralHeight = 0;
		for (i = 0; i < num_servers; i++)
		{
			char server_url[1024];
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			char **tilechain, **tilelist;
			int tile;

			sscanf(serverlist[i], "%s", server_url);

			crDebug("Server %d: %s", i + 1, server_url);
			thread0->netServer[i].name = crStrdup(server_url);
			thread0->netServer[i].buffer_size = tilesort_spu.buffer_size;

			if (!crMothershipGetTiles(conn, response, i))
			{
				crError("No tile information for server %d!  I can't continue!", i);
			}

			tilechain = crStrSplitn(response, " ", 1);
			servWinInfo->num_extents = crStrToInt(tilechain[0]);
			tilelist = crStrSplit(tilechain[1], ",");

			for (tile = 0; tile < servWinInfo->num_extents; tile++)
			{
				int x, y, w, h;

				sscanf(tilelist[tile], "%d %d %d %d", &x, &y, &w, &h);
				servWinInfo->extents[tile].x1 = x;
				servWinInfo->extents[tile].y1 = y;
				servWinInfo->extents[tile].x2 = x + w;
				servWinInfo->extents[tile].y2 = y + h;

				/* update mural size */
				if (servWinInfo->extents[tile].x2 > (int) winInfo->muralWidth)
				{
					winInfo->muralWidth = servWinInfo->extents[tile].x2;
				}
				if (servWinInfo->extents[tile].y2 > (int) winInfo->muralHeight)
				{
					winInfo->muralHeight = servWinInfo->extents[tile].y2;
				}
			}

			crFreeStrings(tilechain);
			crFreeStrings(tilelist);

			/* Determine if the server should receieve left, right or both eye
			 * views when running in passive stereo mode.
			 */
			servWinInfo->eyeFlags = EYE_LEFT | EYE_RIGHT;
			if (crMothershipGetServerParamFromSPU( conn, i,
																						 "stereo_view", response)) {
				if (crStrcmp(response, "left") == 0) {
					servWinInfo->eyeFlags = EYE_LEFT;
					winInfo->passiveStereo = GL_TRUE;
				}
				else if (crStrcmp(response, "right") == 0) {
					servWinInfo->eyeFlags = EYE_RIGHT;
					winInfo->passiveStereo = GL_TRUE;
				}
			}

			/* get view matrices from the server */
			if (crMothershipGetServerParamFromSPU( conn, i,
																						 "view_matrix", response)) {
				crMatrixInitFromString(&servWinInfo->viewMatrix[0], response);
				if (!crMatrixIsIdentity(&servWinInfo->viewMatrix[0]))
					winInfo->matrixSource = MATRIX_SOURCE_SERVERS;
			}
			if (crMothershipGetServerParamFromSPU( conn, i,
																						 "right_view_matrix", response)) {
				crMatrixInitFromString(&servWinInfo->viewMatrix[1], response);
				if (!crMatrixIsIdentity(&servWinInfo->viewMatrix[1]))
					winInfo->matrixSource = MATRIX_SOURCE_SERVERS;
			}
			/*
			crMatrixPrint("Left view", &servWinInfo->viewMatrix[0]);
			crMatrixPrint("Right view", &servWinInfo->viewMatrix[1]);
			*/

			/* Also get overriding projection matrix.
			 * Note that this matrix is only relevant to FRUSTUM bucketing.
			 */
			if (crMothershipGetServerParamFromSPU( conn, i,
																			 "projection_matrix", response)) {
				/** XXX \todo projection_matrix is obsolete, keep for a while though */
				crMatrixInitFromString(&servWinInfo->projectionMatrix[0], response);
				if (!crMatrixIsIdentity(&servWinInfo->projectionMatrix[0]))
					winInfo->matrixSource = MATRIX_SOURCE_SERVERS;
			}
			if (crMothershipGetServerParamFromSPU( conn, i,
																			 "right_projection_matrix", response)) {
				crMatrixInitFromString(&servWinInfo->projectionMatrix[1], response);
				if (!crMatrixIsIdentity(&servWinInfo->projectionMatrix[1]))
					winInfo->matrixSource = MATRIX_SOURCE_SERVERS;
			}
			/*
			crMatrixPrint("Left proj", &servWinInfo->projectionMatrix[0]);
			crMatrixPrint("Right proj", &servWinInfo->projectionMatrix[1]);
			*/
		}
	}

	crFreeStrings(serverchain);
	crFreeStrings(serverlist);

	tilesortspuBucketingInit( winInfo );
}



/**
 * Default algorithm for tile layout.  We use this when we need to get a
 * new tile layout and no other method is available (mothership query,
 * DMX query, etc).
 */
static void
defaultNewTiling( WindowInfo *winInfo )
{
	int tileRows, tileCols, tileWidth, tileHeight, server;
	int i, j, tile, t;

	crDebug("Computing default tiling for %d x %d mural.",
					winInfo->muralWidth, winInfo->muralHeight);

	/* Pick a reasonable number of tile rows and columns for the
	 * number of servers we have.
	 * Remember, this is basically a last resort for tile layout.
	 */
	switch (tilesort_spu.num_servers) {
	case 0:
		crError("Zero servers in defaultNewTiling!\n");
		return;
	case 1:
		tileCols = 1;
		tileRows = 1;
		break;
	case 2:
		tileCols = 2;
		tileRows = 1;
		break;
	case 4:
		tileCols = 2;
		tileRows = 2;
		break;
	case 6:
		tileCols = 3;
		tileRows = 2;
		break;
	case 8:
		tileCols = 4;
		tileRows = 2;
		break;
	case 9:
		tileCols = 3;
		tileRows = 3;
		break;
	case 16:
		tileCols = 4;
		tileRows = 4;
		break;
	default:
		tileCols = tilesort_spu.num_servers;
		tileRows = 1;
		break;
	}

	crDebug("Default Tile Layout: rows: %d  columns:%d\n", tileRows, tileCols);

	/* compute approx tile size */
	tileWidth = winInfo->muralWidth / tileCols;
	tileHeight = winInfo->muralHeight / tileRows;

	/* reset per-server tile count */
	for (server = 0; server < tilesort_spu.num_servers; server++)
		winInfo->server[server].num_extents = 0;

	/* use raster-order layout */
	tile = 0;
	for (i = tileRows - 1; i >= 0; i--)
	{
		int height, y;
		if (i == tileRows - 1)
			height = winInfo->lastHeight - tileHeight * (tileRows - 1);
		else
			height = tileHeight;
		y = i * tileHeight;
		CRASSERT(height > 0);

		for (j = 0; j < tileCols; j++)
		{
			int width, x;
			if (j == tileCols - 1)
				width = winInfo->muralWidth - tileWidth * (tileCols - 1);
			else
				width = tileWidth;
			x = j * tileWidth;
			CRASSERT(width > 0);

			/* save this tile's info */
			server = tile % tilesort_spu.num_servers;

			t = winInfo->server[server].num_extents;
			winInfo->server[server].extents[t].x1 = x;
			winInfo->server[server].extents[t].y1 = y;
			winInfo->server[server].extents[t].x2 = x + width;
			winInfo->server[server].extents[t].y2 = y + height;
			winInfo->server[server].num_extents = t + 1;

			tile++;
		}
	}
}


/**
 * Ask the mothership for a new tile layout.
 * Return: GL_TRUE if successfull, GL_FALSE otherwise.
 */
static GLboolean
getTilingFromMothership( WindowInfo *winInfo )
{
	char response[8000];
	char **n_tiles;
	char **tiles;
	int numTiles, i;
	int maxX, maxY;

	CRConnection *conn = crMothershipConnect();
	CRASSERT(conn);

	crMothershipIdentifySPU(conn, tilesort_spu.id);
	crMothershipRequestTileLayout(conn, response, winInfo->muralWidth, winInfo->muralHeight);
	crMothershipDisconnect(conn);

	crDebug("Getting tile information from mothership.");

	n_tiles = crStrSplitn(response, " ", 1);
	numTiles = crStrToInt( n_tiles[0] );
	if (numTiles <= 0)
		return GL_FALSE;  /* failure */

	/* remove old tile list */
	for (i = 0; i < tilesort_spu.num_servers; i++)
		winInfo->server[i].num_extents = 0;

	/* parse new tile string */
	maxX = maxY = 0;
	CRASSERT(n_tiles[1]);
	tiles = crStrSplit(n_tiles[1], ",");
	for (i = 0; i < numTiles; i++)
	{
		int server, x, y, w, h, t;
		sscanf(tiles[i], "%d %d %d %d %d", &server, &x, &y, &w, &h);
		/*crDebug("Tile on %d: %d %d %d %d\n", server, x1, y1, x2, y2);*/
		t = winInfo->server[server].num_extents;
		winInfo->server[server].extents[t].x1 = x;
		winInfo->server[server].extents[t].y1 = y;
		winInfo->server[server].extents[t].x2 = x + w;
		winInfo->server[server].extents[t].y2 = y + h;
		winInfo->server[server].num_extents = t + 1;
		/* update maxX, maxY */
		if (x + w > maxX)
			maxX = x + w;
		if (y + h > maxY)
			maxY = y + h;
	}

	return GL_TRUE;
}


/**
 * Get new tiling for DMX.
 */
#ifdef USE_DMX
static GLboolean
getTilingFromDMX( WindowInfo *winInfo )
{
	GLint i;

	crDebug("Getting tile information from DMX.");

	tilesortspuUpdateWindowInfo(winInfo);

	if (!winInfo->xwin)
		 return GL_FALSE;

	for (i = 0; i < tilesort_spu.num_servers; i++) {
		const BackendWindowInfo *backend = winInfo->backendWindows + i;
		/* set tile pos/size in mural coords (front-end window coords) */
		if (backend->visrect.x1 < backend->visrect.x2 &&
				backend->visrect.y1 < backend->visrect.y2) {
			winInfo->server[i].num_extents = 1;
		}
		else {
			winInfo->server[i].num_extents = 0;
		}

		/*
		 * Need to invert Y axis at this point.  The backend window info is
		 * in X window coords (y=0=top).  The tilesort extents are in OpenGL
		 * coords (y=0=bottom).
		 */
		winInfo->server[i].extents[0].x1 = backend->visrect.x1;
		winInfo->server[i].extents[0].y1 = winInfo->muralHeight - backend->visrect.y2;
		winInfo->server[i].extents[0].x2 = backend->visrect.x2;
		winInfo->server[i].extents[0].y2 = winInfo->muralHeight - backend->visrect.y1;
	}

	return GL_TRUE;
}
#endif

/**
 * When the window is resized or moved and we need to get a new tile
 * layout, we call this function.  It'll either get the new tile layout
 * from the mothership, from DMX or from the default layout function.
 */
void
tilesortspuGetNewTiling(WindowInfo *winInfo)
{
#if !( defined(WINDOWS) || defined(DARWIN) )
	GET_THREAD(thread);
#endif

	switch (winInfo->bucketMode) {
		case UNIFORM_GRID:
		case NON_UNIFORM_GRID:
		case TEST_ALL_TILES:
		case BROADCAST:
		case RANDOM:
#ifdef USE_DMX
			if (tilesort_spu.useDMX) {
				if (!getTilingFromDMX(winInfo)) {
					defaultNewTiling(winInfo);
				}
			} else
#endif
			{
				if (!getTilingFromMothership(winInfo)) {
					defaultNewTiling(winInfo);
				}
			}

			tilesortspuBucketingInit(winInfo);
			tilesortspuSendTileInfoToServers(winInfo);
			break;
		case WARPED_GRID:
			/*
			 * For WARPED_GRID and local_tile_spec, the mural size should the
			 * size of the display. 
			 *
			 * If we proceed with defaultNewTiling(), the mural size ends up as
			 * the faked window size.  WRONG!
			 */ 
			crDebug("Current bucket_mode is Warped Grid.  Don't know how to "
							"setup a new tile layout for that mode!!!");
			break;
		default:
			crError("Invalid bucketMode in tilesortspu_WindowSize()");
			return;
	}

	/* This forces the GL context to update it's raster origin */
	winInfo->validRasterOrigin = GL_FALSE;

#if !( defined(WINDOWS) || defined(DARWIN) )
	/*
	 * If we've redone the tiling and now we've got new back-end DMX windows
	 * we have to do a MakeCurrent to bind the back-end OpenGL renderers to
	 * the new windows (because the user might not call glXMakeCurrent).
	 */
	if (tilesort_spu.useDMX &&
	    thread->currentContext &&
	    thread->currentContext->currentWindow &&
	    thread->currentContext->currentWindow->newBackendWindows) {
		tilesortspu_MakeCurrent(thread->currentContext->currentWindow->id,
					(GLint) thread->currentContext->currentWindow->xwin,
					thread->currentContext->id);
		thread->currentContext->currentWindow->newBackendWindows = GL_FALSE;
	}
#endif

	/* print new tile information */
	{
		int server, i;

		crDebug("tilesort SPU: Reconfigured tiling:");
		crDebug("  Mural size: %d x %d", winInfo->muralWidth, winInfo->muralHeight);
		for (server = 0; server < tilesort_spu.num_servers; server++)
		{
			crDebug("  Server %d: %d tiles",
				server, winInfo->server[server].num_extents);
			for (i = 0; i < winInfo->server[server].num_extents; i++)
			{
				crDebug("    Tile %d: %d, %d .. %d, %d", i,
					winInfo->server[server].extents[i].x1,
					winInfo->server[server].extents[i].y1,
					winInfo->server[server].extents[i].x2,
					winInfo->server[server].extents[i].y2);
			}
		}
	}
}
