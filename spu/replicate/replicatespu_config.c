/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_spu.h"
#include "cr_mem.h"

#include <stdio.h>

static void __setDefaults( void )
{
	crMemZero(replicate_spu.context, CR_MAX_CONTEXTS * sizeof(ContextInfo));
	replicate_spu.numContexts = 0;

	crMemZero(replicate_spu.thread, MAX_THREADS * sizeof(ThreadInfo));
	replicate_spu.numThreads = 0;
}

/* No SPU options yet. Well.. not really.. 
 */
SPUOptions replicateSPUOptions[] = {
	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void replicatespuGatherConfiguration( const SPU *child_spu )
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
	crMothershipIdentifySPU( conn, replicate_spu.id );

	__setDefaults();

	crSPUGetMothershipParams( conn, &replicate_spu, replicateSPUOptions );

	crMothershipGetServers( conn, response );

	sscanf( response, "%d %s", &num_servers, servername );

	if (num_servers == 1)
	{
		replicate_spu.name = crStrdup( servername );
	}
	else
	{
		crError( "Bad server specification for Pack SPU %d", replicate_spu.id );
	}

	replicate_spu.buffer_size = crMothershipGetMTU( conn );

	/* get a buffer which can hold one big big opcode (counter computing
	 * against packer/pack_buffer.c) */
	replicate_spu.buffer_size = ((((replicate_spu.buffer_size - sizeof(CRMessageOpcodes)) * 5 + 3) / 4 + 0x03) & ~0x03) + sizeof(CRMessageOpcodes);

	crMothershipDisconnect( conn );
}
