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

	case GL_SERVER_VIEW_MATRIX_CR:
		/* Set this server's view matrix which will get premultiplied onto the
		 * modelview matrix.  For non-planar tilesort and stereo.
		 */
		CRASSERT(count == 17);
		CRASSERT(type == GL_FLOAT);
		/* values[0] is the server index. Ignored here but used in tilesort SPU */
		{
			const GLfloat *v = (const GLfloat *) values;
			cr_server.viewMatrix.m00 = v[1];
			cr_server.viewMatrix.m01 = v[2];
			cr_server.viewMatrix.m02 = v[3];
			cr_server.viewMatrix.m03 = v[4];
			cr_server.viewMatrix.m10 = v[5];
			cr_server.viewMatrix.m11 = v[6];
			cr_server.viewMatrix.m12 = v[7];
			cr_server.viewMatrix.m13 = v[8];
			cr_server.viewMatrix.m20 = v[9];
			cr_server.viewMatrix.m21 = v[10];
			cr_server.viewMatrix.m22 = v[11];
			cr_server.viewMatrix.m23 = v[12];
			cr_server.viewMatrix.m30 = v[13];
			cr_server.viewMatrix.m31 = v[14];
			cr_server.viewMatrix.m32 = v[15];
			cr_server.viewMatrix.m33 = v[16];
		}
		cr_server.viewOverride = GL_TRUE;
		break;

	case GL_SERVER_PROJECTION_MATRIX_CR:
		/* Set this server's projection matrix which will get replace the user's
		 * projection matrix.  For non-planar tilesort and stereo.
		 */
		CRASSERT(count == 17);
		CRASSERT(type == GL_FLOAT);
		/* values[0] is the server index. Ignored here but used in tilesort SPU */
		{
			const GLfloat *v = (const GLfloat *) values;
			cr_server.projectionMatrix.m00 = v[1];
			cr_server.projectionMatrix.m01 = v[2];
			cr_server.projectionMatrix.m02 = v[3];
			cr_server.projectionMatrix.m03 = v[4];
			cr_server.projectionMatrix.m10 = v[5];
			cr_server.projectionMatrix.m11 = v[6];
			cr_server.projectionMatrix.m12 = v[7];
			cr_server.projectionMatrix.m13 = v[8];
			cr_server.projectionMatrix.m20 = v[9];
			cr_server.projectionMatrix.m21 = v[10];
			cr_server.projectionMatrix.m22 = v[11];
			cr_server.projectionMatrix.m23 = v[12];
			cr_server.projectionMatrix.m30 = v[13];
			cr_server.projectionMatrix.m31 = v[14];
			cr_server.projectionMatrix.m32 = v[15];
			cr_server.projectionMatrix.m33 = v[16];
		}
		cr_server.projectionOverride = GL_TRUE;
		break;

	default:
		/* Pass the parameter info to the head SPU */
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
		/* Pass the parameter info to the head SPU */
		cr_server.head_spu->dispatch_table.ChromiumParameteriCR( target, value );
	}
}


void SERVER_DISPATCH_APIENTRY crServerDispatchChromiumParameterfCR(GLenum target, GLfloat value)
{
  switch (target) {
	case GL_SHARED_DISPLAY_LISTS_CR:
		cr_server.sharedDisplayLists = (int) value;
		break;
	case GL_SHARED_TEXTURE_OBJECTS_CR:
		cr_server.sharedTextureObjects = (int) value;
		break;
	case GL_SHARED_PROGRAMS_CR:
		cr_server.sharedPrograms = (int) value;
		break;
	default:
		/* Pass the parameter info to the head SPU */
		cr_server.head_spu->dispatch_table.ChromiumParameterfCR( target, value );
	}
}

