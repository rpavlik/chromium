/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "cr_string.h"
#include <signal.h>
#include <stdlib.h>

CRServer cr_server;

void crServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
	(void) id;
}

void ServerCleanup( int sigio )
{
	SPU *the_spu = cr_server.head_spu;

	while (1) {
		if (the_spu && the_spu->cleanup) {
			printf("Cleaning up SPU %s\n",the_spu->name);
			the_spu->cleanup();
		} else 
			break;
		the_spu = the_spu->superSPU;
	}

	exit(0);
}

int main( int argc, char *argv[] )
{
	int i;
	unsigned int j;
	char *mothership = NULL;
	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrcmp( argv[i], "-mothership" ))
		{
			if (i == argc - 1)
			{
				crError( "-mothership requires an argument" );
			}
			mothership = argv[i+1];
			i++;
		}
	}

	signal( SIGTERM, ServerCleanup );
#ifndef WINDOWS
	signal( SIGPIPE, ServerCleanup );
#endif
	crNetInit(crServerRecv, crServerClose);
	crStateInit();
	crServerGatherConfiguration(mothership);
	for (j = 0 ; j < cr_server.numClients ; j++)
	{
		crServerAddToRunQueue( &cr_server.clients[j] );
	}
	if (cr_server.numExtents > 0)
	{
		for ( j = 0 ; j < cr_server.numClients ; j++)
		{
			crServerRecomputeBaseProjection( &(cr_server.clients[j].baseProjection), 0, 0, cr_server.muralWidth, cr_server.muralHeight );
		}
		cr_server.head_spu->dispatch_table.MatrixMode( GL_PROJECTION );
		cr_server.head_spu->dispatch_table.LoadMatrixf( (GLfloat *) &(cr_server.clients[0].baseProjection) );
		if (cr_server.optimizeBucket)
		{
			crServerFillBucketingHash();
		}
	}
	crServerInitDispatch();
	crStateDiffAPI( &(cr_server.head_spu->dispatch_table) );

	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );

	cr_barriers = crAllocHashtable();
	cr_semaphores = crAllocHashtable();
	crServerSerializeRemoteStreams();
	return 0;
}
