/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchSelectBuffer( GLsizei size, GLuint *buffer )
{
	(void) size;
	(void) buffer;
	crError( "Unsupported network glSelectBuffer call." );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	GLubyte local_storage[4096];
	GLint bytes = 0;

	switch (type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		 bytes = count * sizeof(GLbyte);
		 break;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		 bytes = count * sizeof(GLshort);
		 break;
	case GL_INT:
	case GL_UNSIGNED_INT:
		 bytes = count * sizeof(GLint);
		 break;
	case GL_FLOAT:
		 bytes = count * sizeof(GLfloat);
		 break;
	case GL_DOUBLE:
		 bytes = count * sizeof(GLdouble);
		 break;
	default:
		 crError("Bad type in crServerDispatchGetChromiumParametervCR");
	}

	CRASSERT(bytes >= 0);
	CRASSERT(bytes < 4096);

	cr_server.head_spu->dispatch_table.GetChromiumParametervCR( target, index, type, count, local_storage );

	crServerReturnValue( local_storage, bytes );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	static unsigned int gather_connect_count = 0;

	switch (target) {
	case GL_SET_MAX_VIEWPORT_CR:
	    {
		GLint *maxDims = (GLint *)values;
		cr_server.limits.maxViewportDims[0] = maxDims[0];
		cr_server.limits.maxViewportDims[1] = maxDims[1];
	    }
	    break;

	case GL_TILE_INFO_CR:
		/* message from tilesort SPU to set new tile bounds */
		{
			int numTiles, muralWidth, muralHeight, server, tiles;
			int *tileBounds;
			CRASSERT(count >= 4);
			CRASSERT((count - 4) % 4 == 0); /* must be multiple of four */
			CRASSERT(type == GL_INT);
			numTiles = (count - 4) / 4;
			tileBounds = (GLint *) values;
			server = tileBounds[0];
			muralWidth = tileBounds[1];
			muralHeight = tileBounds[2];
			tiles = tileBounds[3];
			CRASSERT(tiles == numTiles);
			tileBounds += 4; /* skip over header values */
			crServerNewTiles(muralWidth, muralHeight, numTiles, tileBounds);
		}
		break;

	case GL_GATHER_DRAWPIXELS_CR:
		if ((cr_server.only_swap_once) && 
			(cr_server.curClient != cr_server.clients+cr_server.numClients-1))   
		{
			break;
		}
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( target, type, count, values );
		break;

	case GL_GATHER_CONNECT_CR:
		/* 
		 * We want the last connect to go through,
		 * otherwise we might deadlock in CheckWindowSize()
		 * in the readback spu
		 */
		gather_connect_count++;
		if ((cr_server.only_swap_once) && (gather_connect_count != cr_server.numClients)) 
		{
			break;
		}
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( target, type, count, values );
		gather_connect_count = 0;
		break;


	default:
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( target, type, count, values );
		break;
	}
}



/*
 * Replace the current tile list with a new one.
 * The boundaries are specified in mural space.
 * Input: muralWidth/Height - size of the overall mural
 *        numTiles - number of tiles
 * Input: tileBounds[0] = bounds[0].x
 *        tileBounds[1] = bounds[0].y
 *        tileBounds[2] = bounds[0].width
 *        tileBounds[3] = bounds[0].height
 *        tileBounds[4] = bounds[1].x
 *        ...
 */
void crServerNewTiles(int muralWidth, int muralHeight,
											int numTiles, const int *tileBounds)
{
	int i;

	crDebug("Reconfiguring tiles in crServerNewTiles:");
	crDebug("  New mural size: %d x %d", muralWidth, muralHeight);
	for (i = 0; i < numTiles; i++)
	{
		crDebug("  Tile %d: %d, %d  %d x %d", i,
						tileBounds[i*4], tileBounds[i*4+1],
						tileBounds[i*4+2], tileBounds[i*4+3]);
	}

	/*
	 * This section basically mimics what's done during crServerGetTileInfo()
	 */
	cr_server.muralWidth = muralWidth;
	cr_server.muralHeight = muralHeight;

	cr_server.numExtents = numTiles;
	CRASSERT(numTiles < CR_MAX_EXTENTS);  /* clamp instead? */
	cr_server.maxTileHeight = 0;
	for (i = 0; i < numTiles; i++)
	{
		const int x = tileBounds[i * 4 + 0];
		const int y = tileBounds[i * 4 + 1];
		const int w = tileBounds[i * 4 + 2];
		const int h = tileBounds[i * 4 + 3];
		cr_server.extents[i].x1 = x;
		cr_server.extents[i].y1 = y;
		cr_server.extents[i].x2 = x + w;
		cr_server.extents[i].y2 = y + h;
		if (h > cr_server.maxTileHeight)
			cr_server.maxTileHeight = h;
	}

	/* Check if we can use optimized bucketing */
	if (cr_server.optimizeBucket)
	{
		if (!crServerCheckTileLayout())
			cr_server.optimizeBucket = 0;
	}

	crServerInitializeTiling();

	/*
	 * This section mimics what's done during crServerAddToRunQueue()
	 */
	{
		/* find ourself in the run queue */
		RunQueue *q = run_queue;
		int found = 0;
		do
		{
			if (q->client == cr_server.curClient)
			{
				/* update our extent info */
				crServerInitializeQueueExtents(q);
				found = 1;
				break;
			}
			q = q->next;
		}
		while (q != run_queue);
		if (!found)
		{
			crError("Problem in crServerNewTiles: RunQueue entry for client %d not found!", cr_server.curClient->number);
		}
	}
}
