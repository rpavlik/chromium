/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"


void * TILESORTSPU_APIENTRY
tilesortspu_MapBufferARB( GLenum target, GLenum access )
{
	return crStateMapBufferARB( target, access );
}


GLboolean TILESORTSPU_APIENTRY
tilesortspu_UnmapBufferARB( GLenum target )
{
	return crStateUnmapBufferARB( target );
}


void TILESORTSPU_APIENTRY
tilesortspu_BufferDataARB( GLenum target, GLsizeiptrARB size,
													 const GLvoid * data, GLenum usage )
{
	crStateBufferDataARB( target, size, data, usage );
}


void TILESORTSPU_APIENTRY
tilesortspu_BufferSubDataARB( GLenum target, GLintptrARB offset,
															GLsizeiptrARB size, const GLvoid * data )
{
	crStateBufferSubDataARB( target, offset, size, data );
}

void TILESORTSPU_APIENTRY
tilesortspu_GetBufferSubDataARB( GLenum target, GLintptrARB offset,
																 GLsizeiptrARB size, GLvoid * data )
{
	crStateGetBufferSubDataARB( target, offset, size, data );
}

