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
	CRMuralInfo *mural = cr_server.curClient->currentMural;
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
			int numTiles, muralWidth, muralheight, server, tiles;
			int *tileBounds;
			CRASSERT(count >= 4);
			CRASSERT((count - 4) % 4 == 0); /* must be multiple of four */
			CRASSERT(type == GL_INT);
			numTiles = (count - 4) / 4;
			tileBounds = (GLint *) values;
			server = tileBounds[0];
			muralWidth = tileBounds[1];
			muralheight = tileBounds[2];
			tiles = tileBounds[3];
			CRASSERT(tiles == numTiles);
			tileBounds += 4; /* skip over header values */
			crServerNewMuralTiling(mural, muralWidth, muralheight, numTiles, tileBounds);
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


void SERVER_DISPATCH_APIENTRY crServerDispatchChromiumParameteriCR(GLenum target, GLint value)
{
  switch (target) {
	case GL_SHARED_DISPLAY_LISTS_CR:
		cr_server.sharedDisplayLists = value;
		break;
	case GL_SHARED_TEXTURE_OBJECTS_CR:
		cr_server.sharedTextureObjects = value;
		break;
	case GL_SHARED_PROGRAMS_CR:
		cr_server.sharedPrograms = value;
		break;
	default:
		cr_server.head_spu->dispatch_table.ChromiumParameteriCR( target, value );
	}
}

