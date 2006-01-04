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
	crMemZero(replicate_spu.thread, MAX_THREADS * sizeof(ThreadInfo));
	replicate_spu.numThreads = 0;
}


static void
render_to_crut_window( ReplicateSPU *replicate_spu, const char *response )
{
	sscanf( response, "%d", &(replicate_spu->render_to_crut_window) );
}


static void
set_sync_on_swap(ReplicateSPU *replicate_spu, const char *response)
{
	sscanf(response, "%d", &(replicate_spu->sync_on_swap));
}

static void
set_chromium_start_port(ReplicateSPU *replicate_spu, const char *response)

{
	sscanf(response, "%d", &(replicate_spu->chromium_start_port));
}


SPUOptions replicateSPUOptions[] = {
	{ "render_to_crut_window", CR_BOOL, 1, "0", NULL, NULL,
		"Render to CRUT window", (SPUOptionCB) render_to_crut_window },
	{ "sync_on_swap", CR_BOOL, 1, "1", NULL, NULL,
		"Sync on SwapBuffers", (SPUOptionCB) set_sync_on_swap },
	{ "chromium_start_port", CR_INT, 1, "7000", "1024", "65000",
	 	"Chromium Start Port", (SPUOptionCB) set_chromium_start_port },

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
		crError("Replicate SPU: Couldn't connect to the mothership - fatal error.");
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
		crError("Replicate SPU: Expected one initial server.  "
						"Check your mothership configuration script for problems.");
	}

	replicate_spu.buffer_size = crMothershipGetMTU( conn );

	crMothershipDisconnect( conn );
}
