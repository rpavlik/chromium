/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "cr_error.h" 
#include "server_dispatch.h"
#include "server.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels )
{
	crError( "glGetTexImage isn't *ever* allowed to be on the wire!" );
	(void) target;
	(void) level;
	(void) format;
	(void) type;
	(void) pixels;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchGetCompressedTexImageARB( GLenum target, GLint level, GLvoid *img )
{
	crError( "glGetCompressedTexImageARB isn't *ever* allowed to be on the wire!" );
	(void) target;
	(void) level;
	(void) img;
}

