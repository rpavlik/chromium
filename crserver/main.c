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

CRServer cr_server;

void crServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
	(void) id;
}

int main( int argc, char *argv[] )
{
	int i;
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

	crNetInit(crServerRecv, crServerClose);
	crStateInit();
	crServerGatherConfiguration(mothership);
	for (i = 0 ; i < cr_server.numClients ; i++)
	{
		crServerAddToRunQueue( i );
	}
	if (cr_server.numExtents > 0)
	{
		for ( i = 0 ; i < cr_server.numClients ; i++)
		{
			crServerRecomputeBaseProjection( &(cr_server.clients[i].baseProjection) );
#if 00
	/* Moved into MakeCurrent() */
			cr_server.clients[i].ctx->viewport.outputDims.x1 = cr_server.underlyingDisplay[0];
			cr_server.clients[i].ctx->viewport.outputDims.x2 = cr_server.underlyingDisplay[2];
			cr_server.clients[i].ctx->viewport.outputDims.y1 = cr_server.underlyingDisplay[1];
			cr_server.clients[i].ctx->viewport.outputDims.y2 = cr_server.underlyingDisplay[3];
#endif
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
