/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "cr_string.h"
#include "cr_mem.h"
#include <signal.h>
#include <stdlib.h>

CRServer cr_server;

int CRServerMain( int argc, char *argv[] );

static void crServerTearDown( void )
{
	SPU *the_spu = cr_server.head_spu;
	CRBarrier *barrier;
	CRSemaphore *sema;
	unsigned int i, num_elements;

	/* Free all context info */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (cr_server.context[i] != NULL) {
			crStateDestroyContext( cr_server.context[i] );
		}
	}

	crFree( cr_server.clients );
	crFree( cr_server.overlap_intens );

	num_elements = crHashtableNumElements( cr_server.semaphores );
	for (i = 0; i < num_elements; i++) {
		sema = (CRSemaphore *) crHashtableSearch( cr_server.semaphores, i);
		crFree( sema );
		crHashtableDelete(cr_server.semaphores, i, GL_TRUE);
	}

	num_elements = crHashtableNumElements( cr_server.barriers );
	for (i = 0; i < num_elements; i++) {
		barrier = (CRBarrier *) crHashtableSearch( cr_server.barriers, i);
		crFree( barrier );
		crHashtableDelete(cr_server.barriers, i, GL_TRUE);
	}

	crFreeHashtable(cr_server.semaphores);
	crFreeHashtable(cr_server.barriers);

	while (1) {
		if (the_spu && the_spu->cleanup) {
			crWarning("Cleaning up SPU %s",the_spu->name);
			the_spu->cleanup();
		} else 
			break;
		the_spu = the_spu->superSPU;
	}
}

static void crServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
	(void) id;
}

static void crServerCleanup( int sigio )
{
	crServerTearDown();

	exit(0);
}

int CRServerMain( int argc, char *argv[] )
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

	signal( SIGTERM, crServerCleanup );
	signal( SIGINT, crServerCleanup );
#ifndef WINDOWS
	signal( SIGPIPE, SIG_IGN );
#endif

	cr_server.firstCallCreateContext = GL_TRUE;
	cr_server.firstCallMakeCurrent = GL_TRUE;

	crNetInit(crServerRecv, crServerClose);
	crStateInit();

	crServerGatherConfiguration(mothership);

	crStateLimitsInit( &(cr_server.limits) );

	for (j = 0 ; j < cr_server.numClients ; j++)
	{
		crServerAddToRunQueue( &cr_server.clients[j] );
	}

	crServerInitializeTiling();
	crServerInitDispatch();
	crStateDiffAPI( &(cr_server.head_spu->dispatch_table) );

	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );

	cr_server.barriers = crAllocHashtable();
	cr_server.semaphores = crAllocHashtable();

	crServerSerializeRemoteStreams();

	crServerTearDown();

	return 0;
}


/*
 * After we've received the tile parameters from the mothership
 * we do all the initialization to perform tile sorting.
 */
void crServerInitializeTiling(void)
{

	if (cr_server.numExtents > 0)
	{
		unsigned int j;
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
}

#if 0
int main( int argc, char *argv[] )
{
	return CRServerMain( argc, argv );
}
#endif
