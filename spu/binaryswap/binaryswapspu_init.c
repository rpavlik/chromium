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
#include "cr_string.h"
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
 * Increment the 'port' part of the given URL by the given amount.
 * Return new URL string.
 */
static char *
incr_url_port(char *url, int portIncr)
{
	char protocol[1000], hostname[8000];
	unsigned short port, default_port = 0;
	char *newURL;

	crParseURL(url, protocol, hostname, &port, default_port);
	CRASSERT(port);
	port += portIncr;

	newURL = crAlloc(crStrlen(url) + 10); /* 10 extra is plenty */
	sprintf(newURL, "%s://%s:%d", protocol, hostname, port);

	return newURL;
}


/**
 *******************************************************************
 *
 * Set up connections for OOB
 *
 *******************************************************************/
static void
binaryswapspuConnectToPeers( void )
{
	char hostname[4096], protocol[4096];
	int i, stage, numNodes;

	crDebug("Setting up peer connections:");

	/*
	 * Number of BinarySwap SPUs we have
	 */
	numNodes = 1 << binaryswap_spu.stages;

	/*
	 * Give the peers port numbers if they don't already have them and
	 * replace 'localhost' with our real hostname if necessary.
	 */
	for (i = 0; i < numNodes; i++) {
		unsigned short port, defaultPort;
		int len;
		char *newName;

		defaultPort = BINARYSWAP_SPU_PORT + i * binaryswap_spu.stages;

		if (!crParseURL(binaryswap_spu.peer_names[i], protocol, hostname,
										&port, defaultPort)) {
			crError("Binaryswap SPU: Invalid URL: %s", binaryswap_spu.peer_names[i]);
		}

		if (crStrcmp(hostname, "localhost") == 0) {
			crGetHostname(hostname, sizeof(hostname));
		}

		/* allocate/create new string */
		len = crStrlen(protocol) + crStrlen(hostname) + 20;
		newName = crAlloc(len);
		sprintf(newName, "%s://%s:%d", protocol, hostname, port);
		crFree(binaryswap_spu.peer_names[i]);
		binaryswap_spu.peer_names[i] = newName;
	}

	/*
	 * Print list of all peers
	 */
	for (i = 0; i < numNodes; i++) {
		crDebug("Peer Node %d: %s %s", i, binaryswap_spu.peer_names[i],
						i == binaryswap_spu.node_num ? "(me)" : "");
	}
	crDebug("Swapping Stages: %d", binaryswap_spu.stages);

	/*
	 * Build list of swap partners.  Note that we have to increment the
	 * partner's "base" port number by the compositing stage.  That'll result
	 * in the port number we'll connect to via crNetConnectToServer().
	 */
	binaryswap_spu.swap_partners = crAlloc(binaryswap_spu.stages*sizeof(char*));
	for (stage = 0; stage < binaryswap_spu.stages; stage++) {

		/* are we the high in the pair? */
		if ((binaryswap_spu.node_num % ((int)pow(2, stage+1)))/((int)pow(2, stage)))
		{
			int k = binaryswap_spu.node_num - (int)pow(2, stage);
			binaryswap_spu.swap_partners[stage]
				= incr_url_port(binaryswap_spu.peer_names[k], stage);
		}
		/* or the low? */
		else {
			int k = binaryswap_spu.node_num + (int)pow(2, stage);
			binaryswap_spu.swap_partners[stage]
				= incr_url_port(binaryswap_spu.peer_names[k], stage);
		}
		crDebug("Partner node for stage %d: %s",
						stage, binaryswap_spu.swap_partners[stage]);
	}	
	
	/* initialize recv function */
	crNetInit( binaryswapspuReceiveData, NULL );
	CRASSERT(binaryswap_spu.stages > 0);
	
	/* allocate send/recv arrays for OOB communication */
	binaryswap_spu.peer_recv = crAlloc(binaryswap_spu.stages *
																		 sizeof(CRConnection*));
	binaryswap_spu.peer_send = crAlloc(binaryswap_spu.stages *
																		 sizeof(CRConnection*));
	
	/* tracks network connection order */
	binaryswap_spu.highlow = crAlloc(binaryswap_spu.stages * sizeof(int));
	
	/* set up accept connect paths for OOB */
	for (stage = 0; stage < binaryswap_spu.stages; stage++) {
		const char *myURL = binaryswap_spu.peer_names[binaryswap_spu.node_num];
		unsigned short myPort;

		crParseURL( myURL, protocol, hostname, &myPort, 0 );
		myPort += stage;

		binaryswap_spu.highlow[stage] = (binaryswap_spu.node_num % ((int)pow(2, stage+1)))
			/ ((int)pow(2, stage));

		/* do accept/connect in the order depending on highlow[] */
		if (binaryswap_spu.highlow[stage]){
			/* lower of pair => accept then connect */
			/*
			crDebug("High Stage %d: Accepting connection on port %d", stage, myPort);
			*/
			binaryswap_spu.peer_recv[stage]
				= crNetAcceptClient(protocol, hostname, myPort, binaryswap_spu.mtu, 1);

			/*
			crDebug("High Stage %d: Connecting to server at %s", stage,
							binaryswap_spu.swap_partners[stage]);
			*/
			binaryswap_spu.peer_send[stage]
				= crNetConnectToServer(binaryswap_spu.swap_partners[stage],
															 0, binaryswap_spu.mtu, 1);
		}
		else {
			/* higher of pair => connect then accept */
			/*
			crDebug("Low Stage %d: Connecting to server at %s", stage,
							binaryswap_spu.swap_partners[stage]);
			*/
			binaryswap_spu.peer_send[stage]
				= crNetConnectToServer(binaryswap_spu.swap_partners[stage],
															 0, binaryswap_spu.mtu, 1);

			/*
			crDebug("Low Stage %d: Accepting connection on port %d", stage, myPort);
			*/
			binaryswap_spu.peer_recv[stage]
				= crNetAcceptClient(protocol, hostname, myPort, binaryswap_spu.mtu, 1);
		}
	}
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
	
	binaryswapspuConnectToPeers();

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
