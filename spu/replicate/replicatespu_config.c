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

static void
setDefaults( void )
{
	crMemZero(replicate_spu.context, CR_MAX_CONTEXTS * sizeof(ContextInfo));
	replicate_spu.numContexts = 0;

	crMemZero(replicate_spu.thread, MAX_THREADS * sizeof(ThreadInfo));
	replicate_spu.numThreads = 0;
}


static void
render_to_crut_window( ReplicateSPU *replicate_spu, const char *response )
{
	sscanf( response, "%d", &(replicate_spu->render_to_crut_window) );
}


SPUOptions replicateSPUOptions[] = {
	{ "render_to_crut_window", CR_BOOL, 1, "0", NULL, NULL,
		"Render to CRUT window", (SPUOptionCB) render_to_crut_window },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void
replicatespuGatherConfiguration( const SPU *child_spu )
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

	setDefaults();

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

	crMothershipDisconnect( conn );
}
