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
