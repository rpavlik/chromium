/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packspu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_spu.h"
#include "cr_mem.h"

#include <stdio.h>

static void __setDefaults( void )
{
	crMemZero(pack_spu.context, CR_MAX_CONTEXTS * sizeof(ContextInfo));
	pack_spu.numContexts = 0;

	crMemZero(pack_spu.thread, MAX_THREADS * sizeof(ThreadInfo));
	pack_spu.numThreads = 0;
}


void set_emit( void *foo, const char *response )
{
	sscanf( response, "%d", &(pack_spu.emit_GATHER_POST_SWAPBUFFERS) );
}

/* No SPU options yet. Well.. not really.. 
 */
SPUOptions packSPUOptions[] = {
	{ "emit_GATHER_POST_SWAPBUFFERS", CR_BOOL, 1, "0", NULL, NULL, 
	  "Emit a parameteri after SwapBuffers", (SPUOptionCB)set_emit },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void packspuGatherConfiguration( const SPU *child_spu )
{
	CRConnection *conn;
	char response[8096];
	char servername[8096];
	int num_servers;

	conn = crMothershipConnect();
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}
	crMothershipIdentifySPU( conn, pack_spu.id );

	__setDefaults();

	crSPUGetMothershipParams( conn, &pack_spu, packSPUOptions );

	crMothershipGetServers( conn, response );

	sscanf( response, "%d %s", &num_servers, servername );

	if (num_servers == 1)
	{
		pack_spu.name = crStrdup( servername );
	}
	else
	{
		crError( "Bad server specification for Pack SPU %d", pack_spu.id );
	}

	pack_spu.buffer_size = crMothershipGetMTU( conn );

	/* get a buffer which can hold one big big opcode (counter computing
	 * against packer/pack_buffer.c) */
	pack_spu.buffer_size = ((((pack_spu.buffer_size - sizeof(CRMessageOpcodes)) * 5 + 3) / 4 + 0x03) & ~0x03) + sizeof(CRMessageOpcodes);

	crSPUPropogateGLLimits( conn, pack_spu.id, child_spu, &pack_spu.limits );

	crMothershipDisconnect( conn );
}
