/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchBoundsInfo( GLrecti *bounds, GLbyte *payload, GLint len, GLint num_opcodes )
{
	crWarning( "Ignoring BoundsInfo call!" );
	(void) bounds;
	(void) payload;
	(void) len;
	(void) num_opcodes;
}
