/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "chromium.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchGenTextures( GLsizei n, GLuint *textures )
{
	GLuint *local_textures = (GLuint *) crAlloc( n*sizeof( *local_textures) );
	(void) textures;
	cr_server.head_spu->dispatch_table.GenTextures( n, local_textures );
	crServerReturnValue( local_textures, n*sizeof( *local_textures ) );
	crFree( local_textures );
}
