/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_url.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "commspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable comm_table[];

SPUFunctions comm_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	comm_table /* THE ACTUAL FUNCTIONS */
};

int commspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	(void) conn;
	(void) buf;
	(void) len;

	return 0; /* Just let the work pile up for later viewing */
}

void commspuConnectToPeer( void )
{
	char hostname[4096], protocol[4096];
	unsigned short port;

	crNetInit( commspuReceiveData, NULL );

	if (!crParseURL( comm_spu.peer_name, protocol, hostname, &port, COMM_SPU_PORT ) )
	{
		crError( "Malformed URL: \"%s\"", comm_spu.peer_name );
	}
	if (comm_spu.i_am_the_server)
	{
		comm_spu.peer_recv = crNetAcceptClient( protocol, NULL, (short) (port+1), comm_spu.mtu, 1 );
		comm_spu.peer_send = crNetConnectToServer( comm_spu.peer_name, port, comm_spu.mtu, 1 );
	}
	else
	{
		comm_spu.peer_send = crNetConnectToServer( comm_spu.peer_name, (short) (port+1), comm_spu.mtu, 1 );
		comm_spu.peer_recv = crNetAcceptClient( protocol, NULL, port, comm_spu.mtu, 1 );
	}
}

SPUFunctions *commSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

	comm_spu.id = id;
	comm_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(comm_spu.child) );
		crSPUCopyDispatchTable( &(comm_spu.child), &(child->dispatch_table) );
		comm_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(comm_spu.super) );
	crSPUCopyDispatchTable( &(comm_spu.super), &(self->superSPU->dispatch_table) );
	commspuGatherConfiguration();

	commspuConnectToPeer();
	comm_spu.num_bytes = sizeof( *(comm_spu.msg) );
	/* Uncomment this to test fragmented messages */
	/* comm_spu.num_bytes = sizeof( *(comm_spu.msg) ) + 3*comm_spu.peer_send->mtu; */
	comm_spu.msg = crAlloc( comm_spu.num_bytes );

	return &comm_functions;
}

void commSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(comm_spu.self) );
	crSPUCopyDispatchTable( &(comm_spu.self), self );
}

int commSPUCleanup(void)
{
	return 1;
}

extern SPUOptions commSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "comm";
	*super = "passthrough";
	*init = commSPUInit;
	*self = commSPUSelfDispatch;
	*cleanup = commSPUCleanup;
	*options = commSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
