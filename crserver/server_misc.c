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
