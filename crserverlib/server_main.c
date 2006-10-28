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
#include "cr_hash.h"
#include <signal.h>
#include <stdlib.h>
#define DEBUG_FP_EXCEPTIONS 0
#if DEBUG_FP_EXCEPTIONS
#include <fpu_control.h>
#include <math.h>
#endif

/**
 * \mainpage CrServerLib 
 *
 * \section CrServerLibIntroduction Introduction
 *
 * Chromium consists of all the top-level files in the cr
 * directory.  The core module basically takes care of API dispatch,
 * and OpenGL state management.
 */


/**
 * CRServer global data
 */
CRServer cr_server;

int tearingdown = 0; /* can't be static */


/**
 * Return pointer to server's first SPU.
 */
SPU*
crServerHeadSPU(void)
{
	 return cr_server.head_spu;
}



static void DeleteBarrierCallback( void *data )
{
	CRServerBarrier *barrier = (CRServerBarrier *) data;
	crFree(barrier->waiting);
	crFree(barrier);
}


static void deleteContextCallback( void *data )
{
	CRContext *c = (CRContext *) data;
	crStateDestroyContext(c);
}


static void crServerTearDown( void )
{
	GLint i;

	/* avoid a race condition */
	if (tearingdown)
		return;

	tearingdown = 1;

	crStateSetCurrent( NULL );

	cr_server.curClient = NULL;
	cr_server.run_queue = NULL;

	crFree( cr_server.overlap_intens );
	cr_server.overlap_intens = NULL;

	/* Deallocate all semaphores */
	crFreeHashtable(cr_server.semaphores, crFree);
	cr_server.semaphores = NULL;
 
	/* Deallocate all barriers */
	crFreeHashtable(cr_server.barriers, DeleteBarrierCallback);
	cr_server.barriers = NULL;

	/* Free all context info */
	crFreeHashtable(cr_server.contextTable, deleteContextCallback);

	/* Free vertex programs */
	crFreeHashtable(cr_server.programTable, crFree);

	for (i = 0; i < cr_server.numClients; i++) {
		if (cr_server.clients[i]) {
			CRConnection *conn = cr_server.clients[i]->conn;
			crNetFreeConnection(conn);
			crFree(cr_server.clients[i]);
		}
	}
	cr_server.numClients = 0;

#if 1
	/* disable these two lines if trying to get stack traces with valgrind */
	crSPUUnloadChain(cr_server.head_spu);
	cr_server.head_spu = NULL;
#endif

	crUnloadOpenGL();
}


static void
crServerConnCloseCallback( CRConnection *conn )
{
	int i, closingClient = -1, connCount = 0;
	crDebug("CRServer: client connection closed");
	/* count number of connections remaining */
	for (i = 0; i < cr_server.numClients; i++) {
		if (cr_server.clients[i] && cr_server.clients[i]->conn) {
			connCount++;
			if (conn == cr_server.clients[i]->conn) {
				closingClient = i;
			}
		}
	}
	if (closingClient >= 0 && connCount == 1 && cr_server.exitIfNoClients) {
		crWarning("CRServer: Last client disconnected - exiting.");
		exit(0);
	}
}


static void
crServerCleanup( int sigio )
{
	crServerTearDown();

	tearingdown = 0;

	exit(0);
}


void
crServerSetPort(int port)
{
	cr_server.tcpip_port = port;
}



static void
crPrintHelp(void)
{
	printf("Usage: crserver [OPTIONS]\n");
	printf("Options:\n");
	printf("  -mothership URL  Specifies URL for contacting the mothership.\n");
	printf("                   URL is of the form [protocol://]hostname[:port]\n");
	printf("  -port N          Specifies the port number this server will listen to.\n");
	printf("  -help            Prints this information.\n");
}


/**
 * Do CRServer initializations.  After this, we can begin servicing clients.
 */
void
crServerInit(int argc, char *argv[])
{
	int i;
	char *mothership = NULL;
	CRMuralInfo *defaultMural;

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
		else if (!crStrcmp( argv[i], "-port" ))
		{
			/* This is the port on which we'll accept client connections */
			if (i == argc - 1)
			{
				crError( "-port requires an argument" );
			}
			cr_server.tcpip_port = crStrToInt(argv[i+1]);
			i++;
		}
		else if (!crStrcmp( argv[i], "-vncmode" ))
		{
			cr_server.vncMode = 1;
		}
		else if (!crStrcmp( argv[i], "-help" ))
		{
			crPrintHelp();
			exit(0);
		}
	}

	signal( SIGTERM, crServerCleanup );
	signal( SIGINT, crServerCleanup );
#ifndef WINDOWS
	signal( SIGPIPE, SIG_IGN );
#endif

#if DEBUG_FP_EXCEPTIONS
	{
		fpu_control_t mask;
		_FPU_GETCW(mask);
		mask &= ~(_FPU_MASK_IM | _FPU_MASK_DM | _FPU_MASK_ZM
							| _FPU_MASK_OM | _FPU_MASK_UM);
		_FPU_SETCW(mask);
	}
#endif

	cr_server.firstCallCreateContext = GL_TRUE;
	cr_server.firstCallMakeCurrent = GL_TRUE;

	/*
	 * Create default mural info and hash table.
	 */
	cr_server.muralTable = crAllocHashtable();
	defaultMural = (CRMuralInfo *) crCalloc(sizeof(CRMuralInfo));
	crHashtableAdd(cr_server.muralTable, 0, defaultMural);

	cr_server.programTable = crAllocHashtable();

	crNetInit(crServerRecv, crServerConnCloseCallback);
	crStateInit();

	crServerGatherConfiguration(mothership);

	crStateLimitsInit( &(cr_server.limits) );

	/*
	 * Default context
	 */
	cr_server.contextTable = crAllocHashtable();
	cr_server.DummyContext = crStateCreateContext( &cr_server.limits,
																								 CR_RGB_BIT | CR_DEPTH_BIT, NULL );
	cr_server.curClient->currentCtx = cr_server.DummyContext;

	crServerInitDispatch();
	crStateDiffAPI( &(cr_server.head_spu->dispatch_table) );

	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );

	cr_server.barriers = crAllocHashtable();
	cr_server.semaphores = crAllocHashtable();
}



int
CRServerMain(int argc, char *argv[])
{
	crServerInit(argc, argv);

	crServerSerializeRemoteStreams();

	crServerTearDown();

	tearingdown = 0;

	return 0;
}
