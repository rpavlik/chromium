/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_url.h"
#include "cr_error.h"
#include "binaryswapspu.h"

#include <stdio.h>
#include <math.h>

extern SPUNamedFunctionTable binaryswap_table[];

BinaryswapSPU binaryswap_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _BinaryswapTSD;
#endif

SPUFunctions binaryswap_functions = {
  NULL, /* CHILD COPY */
  NULL, /* DATA */
  binaryswap_table /* THE ACTUAL FUNCTIONS */
};

int binaryswapspuReceiveData( CRConnection *conn, void *buf, unsigned int len )
{
  (void) conn;
  (void) buf;
  (void) len;
  
  return 0; /* Just let the work pile up for later viewing */
}

/*******************************************************************
 *
 * Set up connections for OOB
 *
 *******************************************************************/
void binaryswapspuConnectToPeer( void )
{
  char hostname[4096], protocol[4096];
  unsigned short *ports;
  int i;
  
  /* initialize recv function */
  crNetInit( binaryswapspuReceiveData, NULL );
  
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
  binaryswap_spu.peer_recv = crAlloc(binaryswap_spu.numnodes*
				     sizeof(CRConnection*));
  binaryswap_spu.peer_send = crAlloc(binaryswap_spu.numnodes*
				     sizeof(CRConnection*));
  
  /* tracks network connection order */
  binaryswap_spu.highlow = crAlloc(binaryswap_spu.numnodes);
  
  /* set up accept connect paths for OOB */
  for(i=0; i<binaryswap_spu.stages; i++){
    /* lower of pair => accept,connect */
    binaryswap_spu.highlow[i] = (binaryswap_spu.node_num%((int)pow(2, i+1)))
      /((int)pow(2, i));
    if(binaryswap_spu.highlow[i]){
      binaryswap_spu.peer_recv[i] = crNetAcceptClient( protocol, 
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
      binaryswap_spu.peer_recv[i] = crNetAcceptClient( protocol, ports[i], 
						       binaryswap_spu.mtu, 1 );
    }
  }
  /* cleanup */
  crFree(ports);
}



SPUFunctions *binaryswapSPUInit( int id, SPU *child, SPU *self,
				 unsigned int context_id,
				 unsigned int num_contexts )
{
  (void) context_id;
  (void) num_contexts;
  
#ifdef CHROMIUM_THREADSAFE
  crDebug("Binaryswap SPU: thread-safe");
#endif
  
  crMemZero(&binaryswap_spu, sizeof(binaryswap_spu));
  binaryswap_spu.id = id;
  binaryswap_spu.has_child = 0;
  if (child){
    crSPUInitDispatchTable( &(binaryswap_spu.child) );
    crSPUCopyDispatchTable( &(binaryswap_spu.child), &(child->dispatch_table) );
    binaryswap_spu.has_child = 1;
  }
  crSPUInitDispatchTable( &(binaryswap_spu.super) );
  crSPUCopyDispatchTable( &(binaryswap_spu.super), &(self->superSPU->dispatch_table) );
  binaryswapspuGatherConfiguration( &binaryswap_spu );
   
  binaryswapspuConnectToPeer(); 
  crStateInit();
  
  return &binaryswap_functions;
}

void binaryswapSPUSelfDispatch(SPUDispatchTable *self)
{
  crSPUInitDispatchTable( &(binaryswap_spu.self) );
  crSPUCopyDispatchTable( &(binaryswap_spu.self), self );
  
  binaryswap_spu.server = (CRServer *)(self->server);
}

int binaryswapSPUCleanup(void)
{ 
  if(binaryswap_spu.outgoing_msg != NULL){
    crFree(binaryswap_spu.outgoing_msg);
  }
  return 1;
}

extern SPUOptions binaryswapSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
  *name = "binaryswap";
  *super = "render";
  *init = binaryswapSPUInit;
  *self = binaryswapSPUSelfDispatch;
  *cleanup = binaryswapSPUCleanup;
  *options = binaryswapSPUOptions;
  *flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
  
  return 1;
}
