/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "cr_string.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"

#include <float.h>

static const GLvectorf maxVector = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
static const GLvectorf minVector = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

static void PropogateCursorPosition(const GLint pos[2])
{
	GET_THREAD(thread);
	int i;

	/* The default buffer */
	crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		GLint tilePos[2];

		/*
		printf("%s() num_extents = %d\n", __FUNCTION__, server->num_extents);
		printf("fakeWinWidth/Height = %d, %d\n",
			   tilesort_spu.fakeWindowWidth, tilesort_spu.fakeWindowHeight);
		printf("muralWidth/Height = %d, %d\n",
			   (int) tilesort_spu.muralWidth, (int) tilesort_spu.muralHeight);
		printf("Scale %g, %g\n", tilesort_spu.widthScale, tilesort_spu.heightScale);
		*/

		/* transform the client window cursor position to the tile position */
		tilePos[0] = (GLint) (pos[0] * tilesort_spu.widthScale) - server->extents[0].x1;
		tilePos[1] = (GLint) (pos[1] * tilesort_spu.heightScale) - server->extents[0].y1;

		crPackSetBuffer( thread->packer, &(thread->pack[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_CURSOR_POSITION_CR, GL_INT, 2, tilePos);
		else
			crPackChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, tilePos);

		crPackGetBuffer( thread->packer, &(thread->pack[i]) );
	}

	/* The default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
}


static void resetVertexCounters(void)
{
	/*GET_THREAD(thread);*/
	int i;
	for (i = 0; i < tilesort_spu.num_servers; i++)
		tilesort_spu.servers[i].vertexCount = 0;
}


/*
 * Send the current tilesort tile info to all the servers.
 */
static void sendTileInfoToServers(void)
{
	GET_THREAD(thread);
	int i;

	/* Save default buffer */
	crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

	/* loop over servers */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		int tileInfo[4 + 4 * CR_MAX_EXTENTS], arraySize;
		int j;

		/* build tileInfo array */
		tileInfo[0] = i;
		tileInfo[1] = tilesort_spu.muralWidth;
		tileInfo[2] = tilesort_spu.muralHeight;
		tileInfo[3] = server->num_extents;
		for (j = 0; j < server->num_extents; j++)
		{
			int w = server->extents[j].x2 - server->extents[j].x1;
			int h = server->extents[j].y2 - server->extents[j].y1;
			tileInfo[4 + j * 4 + 0] = server->extents[j].x1;
			tileInfo[4 + j * 4 + 1] = server->extents[j].y1;
			tileInfo[4 + j * 4 + 2] = w;
			tileInfo[4 + j * 4 + 3] = h;
		}
		arraySize = 4 + 4 * server->num_extents;

		/* pack/send to server[i] */
		crPackSetBuffer( thread->packer, &(thread->pack[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_TILE_INFO_CR, GL_INT,
										   arraySize, tileInfo);
		else
			crPackChromiumParametervCR(GL_TILE_INFO_CR, GL_INT,
									   arraySize, tileInfo);

		crPackGetBuffer( thread->packer, &(thread->pack[i]) );
	}

	/* Restore default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
}


/*
 * Default algorithm for tile layout.  We use this when the
 * user hasn't set a TileLayoutFunction in the config script.
 */
static void defaultNewTiling(int muralWidth, int muralHeight)
{
	int tileCols, tileRows, tileWidth, tileHeight, numServers, server;
	int i, j, tile, t;

	if (tilesort_spu.muralColumns == 0 || tilesort_spu.muralRows == 0)
	{
		crDebug("Can't auto-recompute tiling (unknown rows/columns)");
		return;
	}

	tilesort_spu.muralWidth = muralWidth;
	tilesort_spu.muralHeight = muralHeight;

	numServers = tilesort_spu.num_servers;

	/* compute approx tile size */
	tileCols = tilesort_spu.muralColumns;
	tileRows = tilesort_spu.muralRows;
	tileWidth = muralWidth / tileCols;
	tileHeight = muralHeight / tileRows;

	/* reset per-server tile count */
	for (server = 0; server < numServers; server++)
		tilesort_spu.servers[server].num_extents = 0;

	/* use raster-order layout */
	tile = 0;
	for (i = tileRows - 1; i >= 0; i--)
	{
		int height, y;
		if (i == tileRows - 1)
			height = muralHeight - tileHeight * (tileRows - 1);
		else
			height = tileHeight;
		y = i * tileHeight;
		CRASSERT(height > 0);

		for (j = 0; j < tileCols; j++)
		{
			int width, x;
			if (j == tileCols - 1)
				width = muralWidth - tileWidth * (tileCols - 1);
			else
				width = tileWidth;
			x = j * tileWidth;
			CRASSERT(width > 0);

			/* save this tile's info */
			server = tile % numServers;

			t = tilesort_spu.servers[server].num_extents;
			tilesort_spu.servers[server].extents[t].x1 = x;
			tilesort_spu.servers[server].extents[t].y1 = y;
			tilesort_spu.servers[server].extents[t].x2 = x + width;
			tilesort_spu.servers[server].extents[t].y2 = y + height;
			tilesort_spu.servers[server].num_extents = t + 1;

			tile++;
		}
	}

	tilesortspuBucketingInit();
#if 0
	/* this leads to crashes - investigate */
	tilesortspuComputeMaxViewport();
#endif
	sendTileInfoToServers();
}


/*
 * Ask the mothership for the new tile layout.
 */
static GLboolean getNewTiling(int muralWidth, int muralHeight)
{
	char response[8000];
	char **n_tiles;
	char **tiles;
	int numTiles, i;
	int maxX, maxY;

	CRConnection *conn = crMothershipConnect();
	CRASSERT(conn);
	crMothershipIdentifySPU(conn, tilesort_spu.id);
	crMothershipRequestTileLayout(conn, response, muralWidth, muralHeight);
	crMothershipDisconnect(conn);

	n_tiles = crStrSplitn(response, " ", 1);
	numTiles = crStrToInt( n_tiles[0] );
	if (numTiles <= 0)
		return GL_FALSE;  /* failure */

	/* remove old tile list */
	for (i = 0; i < tilesort_spu.num_servers; i++)
		tilesort_spu.servers[i].num_extents = 0;

	/* parse new tile string */
	maxX = maxY = 0;
	CRASSERT(n_tiles[1]);
	tiles = crStrSplit(n_tiles[1], ",");
	for (i = 0; i < numTiles; i++)
	{
		int server, x, y, w, h, t;
		sscanf(tiles[i], "%d %d %d %d %d", &server, &x, &y, &w, &h);
		/*crDebug("Tile on %d: %d %d %d %d\n", server, x1, y1, x2, y2);*/
		t = tilesort_spu.servers[server].num_extents;
		tilesort_spu.servers[server].extents[t].x1 = x;
		tilesort_spu.servers[server].extents[t].y1 = y;
		tilesort_spu.servers[server].extents[t].x2 = x + w;
		tilesort_spu.servers[server].extents[t].y2 = y + h;
		tilesort_spu.servers[server].num_extents = t + 1;
		/* update maxX, maxY */
		if (x + w > maxX)
			maxX = x + w;
		if (y + h > maxY)
			maxY = y + h;
	}

	/* new mural size */
#if 0
	/* old code - remove someday */
	tilesort_spu.muralWidth = muralWidth;
	tilesort_spu.muralHeight = muralHeight;
#else
	/* Set mural size to max of X and Y tile edges.  What this implies is
	 * that if the tile size is fixed, but the rows and columns is variable,
	 * the mural size will be a multiple of the tile size.
	 */

	tilesort_spu.muralWidth = maxX;
	tilesort_spu.muralHeight = maxY;
#endif

#if 0
	/* this leads to crashes - investigate */
	tilesortspuComputeMaxViewport();
#endif
	sendTileInfoToServers();
	return GL_TRUE;
}


void TILESORTSPU_APIENTRY tilesortspu_WindowSize(GLint window, GLint w, GLint h)
{
	int server, i;

	/*
	 * If we're driving a large mural, we probably don't want to
	 * resize the mural in response to the app window size changing
	 * (except maybe to preserve aspect ratio?).
	 *
	 * But if we're using the tilesort SPU for tile-reassembly (ala
	 * Lightning-2) then we'll want to resize all the tiles so that
	 * the set of tiles matches the app window size.
	 */
	if (tilesort_spu.bucketMode == UNIFORM_GRID)
	{
		crDebug("Tilesort SPU asked to resize window, "
						"but bucket_mode is Uniform Grid "
						"(or optimize_bucketing is enabled).  No resize.");
		return;
	}
	else if (tilesort_spu.bucketMode == TEST_ALL_TILES ||
					 tilesort_spu.bucketMode == NON_UNIFORM_GRID)
	{
		GLboolean goodGrid;

		if (!getNewTiling(w, h))
			defaultNewTiling(w, h);

		/* check if we can use non-uniform grid bucketing */
		goodGrid = tilesortspuInitGridBucketing();
		if (goodGrid)
			tilesort_spu.bucketMode = NON_UNIFORM_GRID;
		else
			tilesort_spu.bucketMode = TEST_ALL_TILES;
	}
	else
	{
		CRASSERT(tilesort_spu.bucketMode == BROADCAST ||
						 tilesort_spu.bucketMode == WARPED_GRID ||
						 tilesort_spu.bucketMode == RANDOM);
		/*
		 * XXX not sure what to do about RANDOM or WARPED_GRID option here.
		 */
		
		/*
		 * For WARPED_GRID and local_tile_spec, the mural size should the size of 
		 * the display. 
		 *
		 * If we proceed with defaultNewTiling(), the mural size ends up as the 
		 * faked window size.
		 */ 
		if (tilesort_spu.bucketMode != WARPED_GRID)
		{
			if (!getNewTiling(w, h))
			{
				defaultNewTiling(w, h);
			}
		}
	}

	/* print new tile information */
	crDebug("tilesort SPU: Reconfigured tiling:");
	crDebug("  Mural size: %d x %d",
					tilesort_spu.muralWidth, tilesort_spu.muralHeight);
	for (server = 0; server < tilesort_spu.num_servers; server++)
	{
		crDebug("  Server %d: %d tiles",
						server, tilesort_spu.servers[server].num_extents);
		for (i = 0; i < tilesort_spu.servers[server].num_extents; i++)
		{
			crDebug("    Tile %d: %d, %d .. %d, %d", i,
							tilesort_spu.servers[server].extents[i].x1,
							tilesort_spu.servers[server].extents[i].y1,
							tilesort_spu.servers[server].extents[i].x2,
							tilesort_spu.servers[server].extents[i].y2);
		}
	}
}


void TILESORTSPU_APIENTRY tilesortspu_WindowPosition(GLint window, GLint x, GLint y)
{
	/* do nothing */
}


GLint TILESORTSPU_APIENTRY tilesortspu_WindowCreate( const char *dpy, GLint visBits)
{
	int i;

	/* find an empty slot in windows[] array */
	for (i = 0; i < MAX_WINDOWS; i++) {
		if (tilesort_spu.windows_inuse[i] == 0)
			break;
	}
	if (i == MAX_WINDOWS)
		return -1;

	tilesort_spu.windows_inuse[i] = 1; /* used */

	return i;
}

void TILESORTSPU_APIENTRY tilesortspu_ChromiumParameteriCR(GLenum target, GLint value)
{
	GET_THREAD(thread);
	int i;

	(void) value;

	switch (target) {
	case GL_RESET_VERTEX_COUNTERS_CR:  /* GL_CR_tilesort_info */
		resetVertexCounters();
		break;
	default:
		/* The default buffer */
		crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->pack[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParameteriCRSWAP( target, value );
			else
				crPackChromiumParameteriCR( target, value );

			crPackGetBuffer( thread->packer, &(thread->pack[i]) );
		}

		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_ChromiumParameterfCR(GLenum target, GLfloat value)
{
	GET_THREAD(thread);
	int i;

	(void) value;

	switch (target) {
	case GL_RESET_VERTEX_COUNTERS_CR:  /* GL_CR_tilesort_info */
		resetVertexCounters();
		break;
	default:
		/* The default buffer */
		crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->pack[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParameterfCRSWAP( target, value );
			else
				crPackChromiumParameterfCR( target, value );

			crPackGetBuffer( thread->packer, &(thread->pack[i]) );
		}

		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	GET_THREAD(thread);
	int i;

	switch (target) {
	case GL_CURSOR_POSITION_CR:  /* GL_CR_cursor_position */
		PropogateCursorPosition((GLint *) values);
		break;
	case GL_SCREEN_BBOX_CR:  /* GL_CR_bounding_box */
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 8);
		if (values) {
			const GLfloat *bbox = (const GLfloat *) values;
			thread->packer->bounds_min.x = bbox[0];
			thread->packer->bounds_min.y = bbox[1];
			thread->packer->bounds_min.z = bbox[2];
			thread->packer->bounds_min.w = bbox[3];
			thread->packer->bounds_max.x = bbox[4];
			thread->packer->bounds_max.y = bbox[5];
			thread->packer->bounds_max.z = bbox[6];
			thread->packer->bounds_max.w = bbox[7];
			/*crWarning( "I should really switch to the non-bbox API now, but API switching doesn't work" ); */
			thread->packer->updateBBOX = 0;
			tilesort_spu.providedBBOX = target;
		}
		else {
			/* disable bbox */
			thread->packer->bounds_min = maxVector;
			thread->packer->bounds_max = minVector;
			thread->packer->updateBBOX = 1;
			tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
		}
		break;
	case GL_OBJECT_BBOX_CR:  /* GL_CR_bounding_box */
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 6);
		if (values) {
			const GLfloat *bbox = (const GLfloat *) values;
			thread->packer->bounds_min.x = bbox[0];
			thread->packer->bounds_min.y = bbox[1];
			thread->packer->bounds_min.z = bbox[2];
			thread->packer->bounds_min.w = 1.0;
			thread->packer->bounds_max.x = bbox[3];
			thread->packer->bounds_max.y = bbox[4];
			thread->packer->bounds_max.z = bbox[5];
			thread->packer->bounds_max.w = 1.0;
			/*crWarning( "I should really switch to the non-bbox API now, but API switching doesn't work" ); */
			thread->packer->updateBBOX = 0;
			tilesort_spu.providedBBOX = target;
		}
		else {
			/* disable bbox */
			thread->packer->bounds_min = maxVector;
			thread->packer->bounds_max = minVector;
			thread->packer->updateBBOX = 1;
			tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
		}
		break;
	case GL_DEFAULT_BBOX_CR:  /* GL_CR_bounding_box */
		CRASSERT(count == 0);
		thread->packer->bounds_min = maxVector;
		thread->packer->bounds_max = minVector;
		/*crWarning( "I should really switch to the bbox API now, but API switching doesn't work" ); */
		thread->packer->updateBBOX = 1;
		tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
		break;

	case GL_TILE_INFO_CR:  /* GL_CR_tile_info */
		{
			const int *ivalues = (const int *) values;
			const int server = ivalues[0];
			const int muralWidth = ivalues[1];
			const int muralHeight = ivalues[2];
			const int numTiles = ivalues[3];
			int i;
			tilesort_spu.muralWidth = muralWidth;
			tilesort_spu.muralHeight = muralHeight;
			tilesort_spu.servers[server].num_extents = numTiles;
			for (i = 0; i < numTiles; i++)
			{
				tilesort_spu.servers[server].extents[i].x1 = ivalues[4 + i * 4 + 0];
				tilesort_spu.servers[server].extents[i].y1 = ivalues[4 + i * 4 + 1];
				tilesort_spu.servers[server].extents[i].x2 = ivalues[4 + i * 4 + 2];
				tilesort_spu.servers[server].extents[i].y2 = ivalues[4 + i * 4 + 3];
			}

			tilesortspuBucketingInit();
#if 0
			/* this leads to crashes - investigate */
			tilesortspuComputeMaxViewport();
#endif
			sendTileInfoToServers();
		}
		break;

	default:
		/* Propogate the data to the servers */

		/* Save default buffer */
		crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->pack[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParametervCRSWAP( target, type, count, values );
			else
				crPackChromiumParametervCR( target, type, count, values );

			crPackGetBuffer( thread->packer, &(thread->pack[i]) );
		}

		/* Restore default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_GetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	GET_THREAD(thread);
	int writeback = tilesort_spu.num_servers ? 1 : 0;
	int i;

	switch (target) {
	case GL_MURAL_SIZE_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count == 2)
		{
			((GLint *) values)[0] = tilesort_spu.muralWidth;
			((GLint *) values)[1] = tilesort_spu.muralHeight;
		}
		break;
	case GL_NUM_SERVERS_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count == 1)
			*((GLint *) values) = tilesort_spu.num_servers;
		else
			*((GLint *) values) = 0;
		break;
  case GL_NUM_TILES_CR:
		if (type == GL_INT && count == 1 && index < (GLuint) tilesort_spu.num_servers)
			*((GLint *) values) = tilesort_spu.servers[index].num_extents;
		break;
	case GL_TILE_BOUNDS_CR:
		if (type == GL_INT && count == 4)
		{
			GLint *ivalues = (GLint *) values;
			int server = index >> 16;
			int tile = index & 0xffff;
			if (server < tilesort_spu.num_servers &&
					tile < tilesort_spu.servers[server].num_extents) {
				ivalues[0] = (GLint) tilesort_spu.servers[server].extents[tile].x1;
				ivalues[1] = (GLint) tilesort_spu.servers[server].extents[tile].y1;
				ivalues[2] = (GLint) tilesort_spu.servers[server].extents[tile].x2;
				ivalues[3] = (GLint) tilesort_spu.servers[server].extents[tile].y2;
			}
		}
		break;
	case GL_VERTEX_COUNTS_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count >= 1)
		{
			int n = tilesort_spu.num_servers;
			if (count < n)
				n = count;
			for (i = 0; i < n; i++)
			{
				TileSortSPUServer *server = tilesort_spu.servers + i;
				((GLint *) values)[i] = server->vertexCount;
			}
		}
		break;
	default:
		/* The default buffer */
		crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->pack[i]) );

			if (tilesort_spu.swap)
				crPackGetChromiumParametervCRSWAP( target, index, type, count, values, &writeback );
			else
				crPackGetChromiumParametervCR( target, index, type, count, values, &writeback );

			crPackGetBuffer( thread->packer, &(thread->pack[i]) );
			
			tilesortspuFlush( (void *) thread );

			while (writeback)
				crNetRecv();
		}
		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
		break;
	}
}
