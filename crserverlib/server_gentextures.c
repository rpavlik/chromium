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


void SERVER_DISPATCH_APIENTRY crServerDispatchGenProgramsNV( GLsizei n, GLuint * ids )
{
	GLuint *local_progs = (GLuint *) crAlloc( n*sizeof( *local_progs) );
	(void) ids;
	cr_server.head_spu->dispatch_table.GenProgramsNV( n, local_progs );
	crServerReturnValue( local_progs, n*sizeof( *local_progs ) );
	crFree( local_progs );
}


void SERVER_DISPATCH_APIENTRY crServerDispatchGenFencesNV( GLsizei n, GLuint * ids )
{
	GLuint *local_fences = (GLuint *) crAlloc( n*sizeof( *local_fences) );
	(void) ids;
	cr_server.head_spu->dispatch_table.GenFencesNV( n, local_fences );
	crServerReturnValue( local_fences, n*sizeof( *local_fences ) );
	crFree( local_fences );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGenProgramsARB( GLsizei n, GLuint * ids )
{
	GLuint *local_progs = (GLuint *) crAlloc( n*sizeof( *local_progs) );
	(void) ids;
	cr_server.head_spu->dispatch_table.GenProgramsARB( n, local_progs );
	crServerReturnValue( local_progs, n*sizeof( *local_progs ) );
	crFree( local_progs );
}
