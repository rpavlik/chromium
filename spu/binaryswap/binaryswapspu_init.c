/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_url.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "binaryswapspu.h"
#include <stdio.h>
#include <math.h>

extern SPUNamedFunctionTable _cr_binaryswap_table[];

SPUFunctions binaryswap_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_binaryswap_table /* THE ACTUAL FUNCTIONS */
};

Binaryswapspu binaryswap_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _BinaryswapTSD;
#endif

static int
binaryswapspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
	(void) conn;
	(void) buf;
	(void) len;
	return 0; /* NOT HANDLED */
}


/**
 *******************************************************************
 *
 * Set up connections for OOB
 *
 *******************************************************************/
static void
binaryswapspuConnectToPeer( void )
{
	char hostname[4096], protocol[4096];
	unsigned short *ports;
	int i;
	
	/* initialize recv function */
	crNetInit( binaryswapspuReceiveData, NULL );
	CRASSERT(binaryswap_spu.stages > 0);
	
	/* set up arrary for ports */
	ports = crAlloc(binaryswap_spu.stages*sizeof(unsigned short));
	
	/* Loop through and check hostnames and such */
	for(i=0; i<binaryswap_spu.stages; i++){
		if (!crParseURL( binaryswap_spu.swap_partners[i], protocol, hostname,
				 &ports[i], (unsigned short)(BINARYSWAP_SPU_PORT+(i*2)))){
			crFree(ports);
			crError( "Malformed URL: \"%s\"", binaryswap_spu.swap_partners[i] );
		}
	}
	
	/* allocate send/recv arrays for OOB communication */
	binaryswap_spu.peer_recv = crAlloc(binaryswap_spu.stages*
					   sizeof(CRConnection*));
	binaryswap_spu.peer_send = crAlloc(binaryswap_spu.stages*
					   sizeof(CRConnection*));
	
	/* tracks network connection order */
	binaryswap_spu.highlow = crAlloc(binaryswap_spu.stages*sizeof(int));
	
	/* set up accept connect paths for OOB */
	for(i=0; i<binaryswap_spu.stages; i++){
		/* lower of pair => accept,connect */
		binaryswap_spu.highlow[i] = (binaryswap_spu.node_num%((int)pow(2, i+1)))
			/((int)pow(2, i));
		if(binaryswap_spu.highlow[i]){
			binaryswap_spu.peer_recv[i] = crNetAcceptClient( protocol, NULL,
									 (short) (ports[i]+1), 
									 binaryswap_spu.mtu, 1 );
			binaryswap_spu.peer_send[i] = crNetConnectToServer( binaryswap_spu.swap_partners[i], 
									    ports[i], 
									    binaryswap_spu.mtu, 
									    1 );
		}
		/* higher of pair => connect,accept */
		else{
			binaryswap_spu.peer_send[i] = crNetConnectToServer( binaryswap_spu.swap_partners[i], 
									    (short) (ports[i]+1),
									    binaryswap_spu.mtu, 
									    1 );
			binaryswap_spu.peer_recv[i] = crNetAcceptClient( protocol, NULL,ports[i], 
									 binaryswap_spu.mtu, 1 );
		}
	}
	/* cleanup */
	crFree(ports);
}

static SPUFunctions *
binaryswapspuInit( int id, SPU *child, SPU *self,
		   unsigned int context_id,
		   unsigned int num_contexts )
{
	WindowInfo *window;
	(void) context_id;
	(void) num_contexts;
	
#ifdef CHROMIUM_THREADSAFE
	crDebug("Binaryswap SPU: thread-safe");
#endif
	
	crMemZero(&binaryswap_spu, sizeof(binaryswap_spu));
	binaryswap_spu.id = id;
	binaryswap_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(binaryswap_spu.child) );
		crSPUCopyDispatchTable( &(binaryswap_spu.child), &(child->dispatch_table) );
		binaryswap_spu.has_child = 1;
	}
	else
	{
		/* don't override any API functions - use the Render SPU functions */
		static SPUNamedFunctionTable empty_table[] = {
			{ NULL, NULL }
		};
		binaryswap_functions.table = empty_table;
	}
	crSPUInitDispatchTable( &(binaryswap_spu.super) );
	crSPUCopyDispatchTable( &(binaryswap_spu.super), &(self->superSPU->dispatch_table) );
	binaryswapspuGatherConfiguration( &binaryswap_spu );
	
	binaryswap_spu.contextTable = crAllocHashtable();
	binaryswap_spu.windowTable = crAllocHashtable();
	
	/* create my default window (window number 0) */
	window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	CRASSERT(window);
	window->index = 0;
	window->renderWindow = 0; /* default render SPU window */
	window->childWindow = 0;  /* default child SPU window */
	crHashtableAdd(binaryswap_spu.windowTable, 0, window);
	
	binaryswapspuConnectToPeer();

	return &binaryswap_functions;
}

static void
binaryswapspuSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(binaryswap_spu.self) );
	crSPUCopyDispatchTable( &(binaryswap_spu.self), self );
	
	binaryswap_spu.server = (CRServer *)(self->server);
}

static int
binaryswapspuCleanup(void)
{
	return 1;
}

extern SPUOptions binaryswapspuOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "binaryswap";
	*super = "render";
	*init = binaryswapspuInit;
	*self = binaryswapspuSelfDispatch;
	*cleanup = binaryswapspuCleanup;
	*options = binaryswapspuOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
