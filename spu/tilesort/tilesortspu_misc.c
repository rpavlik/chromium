/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"

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
		tilePos[0] = (GLint) (pos[0] * tilesort_spu.widthScale) - server->x1[0];
		tilePos[1] = (GLint) (pos[1] * tilesort_spu.heightScale) - server->y1[0];

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
			tileInfo[4 + j * 4 + 0] = server->x1[j];
			tileInfo[4 + j * 4 + 1] = server->y1[j];
			tileInfo[4 + j * 4 + 2] = server->x2[j];
			tileInfo[4 + j * 4 + 3] = server->y2[j];
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


#if 0
	/* This is a temporary hack! */
static void recomputeTiling(int width, int height)
{
	CRASSERT(tilesort_spu.num_servers == 2);

	tilesort_spu.muralWidth = width;
	tilesort_spu.muralHeight = height;

	tilesort_spu.servers[0].num_extents = 1;
	tilesort_spu.servers[0].x1[0] = 0;
	tilesort_spu.servers[0].y1[0] = 0;
	tilesort_spu.servers[0].x2[0] = width / 2;
	tilesort_spu.servers[0].y2[0] = height;

	tilesort_spu.servers[1].num_extents = 1;
	tilesort_spu.servers[1].x1[0] = width / 2;
	tilesort_spu.servers[1].y1[0] = 0;
	tilesort_spu.servers[1].x2[0] = width;
	tilesort_spu.servers[1].y2[0] = height;

	tilesortspuBucketingInit();
#if 0
	/* this leads to crashes - investigate */
	tilesortspuComputeMaxViewport();
#endif
	sendTileInfoToServers();
}
#endif


/*
 * This is just one possible algorithm for dynamic reconfiguration...
 */
static void recomputeTiling(int muralWidth, int muralHeight)
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
	for (i = 0; i < tileRows; i++) {
		int height, y;
		if (i == tileRows - 1)
			height = muralHeight - tileHeight * (tileRows - 1);
		else
			height = tileHeight;
		y = i * tileHeight;
		CRASSERT(height > 0);

		for (j = 0; j < tileCols; j++) {
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
			tilesort_spu.servers[server].x1[t] = x;
			tilesort_spu.servers[server].y1[t] = y;
			tilesort_spu.servers[server].x2[t] = x + width;
			tilesort_spu.servers[server].y2[t] = y + height;
			tilesort_spu.servers[server].num_extents = t + 1;

			tile++;
		}
	}

	crDebug("tilesort SPU: Reconfigured tiling:");
	crDebug("  Mural size: %d x %d", muralWidth, muralHeight);
	for (server = 0; server < numServers; server++)
		crDebug("  Server %d: %d tiles",
						server, tilesort_spu.servers[server].num_extents);

	tilesortspuBucketingInit();
#if 0
	/* this leads to crashes - investigate */
	tilesortspuComputeMaxViewport();
#endif
	sendTileInfoToServers();
}


void TILESORTSPU_APIENTRY tilesortspu_WindowSize(GLint window, GLint w, GLint h)
{
	/*
	 * If we're driving a large mural, we probably don't want to
	 * resize the mural in response to the app window size changing
	 * (except maybe to preserve aspect ratio?).
	 *
	 * But if we're using the tilesort SPU for tile-reassembly (ala
	 * Lightning-2) then we'll want to resize all the tiles so that
	 * the set of tiles matches the app window size.
	 */
	recomputeTiling(w, h);
}


void TILESORTSPU_APIENTRY tilesortspu_WindowPosition(GLint window, GLint x, GLint y)
{
	/* do nothing */
}


GLint TILESORTSPU_APIENTRY tilesortspu_crCreateWindow(void *dpy, GLint visBits)
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
		if (values) {
			GLfloat *bbox = (GLfloat *)values;
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
			return;
		}
		/* fallthrough */
	case GL_DEFAULT_BBOX_CR:  /* GL_CR_bounding_box */
		thread->packer->bounds_min = maxVector;
		thread->packer->bounds_max = minVector;
		/*crWarning( "I should really switch to the bbox API now, but API switching doesn't work" ); */
		thread->packer->updateBBOX = 1;
		tilesort_spu.providedBBOX = GL_DEFAULT_BBOX_CR;
		break;

	case GL_TILE_INFO_CR:  /* GL_CR_tile_info */
		/* save info and forward to the servers */

		/* XXX to do */

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
			int server = index >> 16;
			int tile = index & 0xffff;
			if (server < tilesort_spu.num_servers &&
					tile < tilesort_spu.servers[server].num_extents) {
				((GLint *) values)[0] = (GLint) tilesort_spu.servers[server].x1[tile];
				((GLint *) values)[1] = (GLint) tilesort_spu.servers[server].y1[tile];
				((GLint *) values)[2] = (GLint) tilesort_spu.servers[server].x2[tile];
				((GLint *) values)[3] = (GLint) tilesort_spu.servers[server].y2[tile];
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
