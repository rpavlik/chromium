/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "binaryswapspu.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_mem.h"
#include <string.h>
#include <math.h>

static void set_peers( void *foo, const char *response )
{
	char *nodes;
	char *token;
	int i;
	
	/* figure out peer information! */
	/* grab network configuration */
	/* get count of things first so we can allocate space */
	int count = 0;
	nodes = crStrdup(response);
	if((token = strtok(nodes, ",\n\t\0 ")) != NULL){
		count = 1;
		while((token = strtok(NULL, ",\n\t\0 ")) != NULL){
			count++;
		}
	}
	else{
		crError("Bad mojo: I can't figure out how many peers you have!");
	}
	crFree(nodes);
	
	/* actually store the network peer list */
	nodes = crStrdup(response);
	binaryswap_spu.peer_names = crAlloc(count*sizeof(char*));
	if((token = strtok(nodes, ",\n\t ")) != NULL){
		i = 0;
		binaryswap_spu.peer_names[i++] = crStrdup(token);
		while((token = strtok(NULL, ",\n\t ")) != NULL){
			binaryswap_spu.peer_names[i++] = crStrdup(token);
		}
	}
	crFree(nodes);
	
	/* figure out how many stages we have */
	binaryswap_spu.stages = (int)(log(count)/log(2)+0.1);
	crDebug("Swapping Stages: %d", binaryswap_spu.stages);
}


static void set_node( void *spu, const char *response )
{
	(void) spu;
	if (*response) {
		binaryswap_spu.node_num = crStrToInt( response );
	}
	else {
		crError( "No node number specified for the binaryswap SPU?" );
	}
}

static void set_swapmtu( void *spu, const char *response )
{
	(void) spu;
	if (*response) {
		binaryswap_spu.mtu = crStrToInt( response );
	}
	else { 
		binaryswap_spu.mtu = -1;
		crWarning("No swap MTU specified, using default from mothership" );
	}
}

static void set_type( void *spu, const char *response )
{
	(void) spu;
	if(crStrcmp( response, "alpha") == 0){
		binaryswap_spu.alpha_composite = 1;
		binaryswap_spu.depth_composite = 0;
	}
	else if(crStrcmp( response, "depth") == 0){
		binaryswap_spu.depth_composite = 1;
		binaryswap_spu.alpha_composite = 0;
	}
	else{
		crError( "Bad composite type specified for the binaryswap SPU?" );
	}
}

static void set_local_visualization( Binaryswapspu *binaryswap_spu, const char *response )
{
	binaryswap_spu->local_visualization = crStrToInt( response );
}

static void __setDefaults( Binaryswapspu *binaryswap_spu )
{
	/* misc */
	binaryswap_spu->barrierSize = 0;
	binaryswap_spu->bbox = NULL;
	
	/* config options */
	binaryswap_spu->resizable = 0;
	binaryswap_spu->local_visualization = 0;
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions binaryswapspuOptions[] = {
	
	{ "peers", CR_STRING, 1, "", NULL, NULL, 
	  "Peers", (SPUOptionCB)set_peers},
	
	{ "node_number", CR_STRING, 1, "", NULL, NULL, 
	  "Node Number", (SPUOptionCB)set_node},
	
	{ "local_visualization", CR_BOOL, 1, "1", NULL, NULL,
	  "Local Visualization", (SPUOptionCB)set_local_visualization },
	
	{ "type", CR_ENUM, 1, "depth", "'depth', 'alpha'", NULL, 
	  "Composite type",  (SPUOptionCB)set_type},
	
	{ "swapmtu", CR_INT, 1, "-1", "1024", NULL, 
	  "MTU size for swapping", (SPUOptionCB)set_swapmtu},
	
	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void binaryswapspuGatherConfiguration( Binaryswapspu *binaryswap_spu )
{
	CRConnection *conn;
	int i;
	
	__setDefaults( binaryswap_spu );
	
	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
	        crSPUSetDefaultParams( binaryswap_spu, binaryswapspuOptions );
		return;
	}
	crMothershipIdentifySPU( conn, binaryswap_spu->id );
	
	crSPUGetMothershipParams( conn, (void *)binaryswap_spu, binaryswapspuOptions );
	
	
	/* Get the render SPU's resizable setting */
	{
		char response[1000];
		if (crMothershipGetSPUParam( conn, response, "resizable" )) {
			int resizable = 0;
			sscanf(response, "%d", &resizable);
			binaryswap_spu->resizable = resizable;
		}
	}
	/* build list of swap partners */
	binaryswap_spu->swap_partners = crAlloc(binaryswap_spu->stages*sizeof(char*));
	for( i=0; i<binaryswap_spu->stages; i++ ){
		/* are we the high in the pair? */
		if((binaryswap_spu->node_num%((int)pow(2, i+1)))/((int)pow(2, i)))
		{
			binaryswap_spu->swap_partners[i] = crStrdup(binaryswap_spu->peer_names[binaryswap_spu->node_num - (int)pow(2, i)]);
		}
		/* or the low? */
		else{
			binaryswap_spu->swap_partners[i] = crStrdup(binaryswap_spu->peer_names[binaryswap_spu->node_num + (int)pow(2, i)]);
		}
	}	
	
	if(binaryswap_spu->mtu == -1)
	{
		binaryswap_spu->mtu = crMothershipGetMTU( conn );
	}
	
	crMothershipDisconnect( conn );
}
