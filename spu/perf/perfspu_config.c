/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "perfspu.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

static void set_perf_log_file( perfSPU *perf_spu, const char *response )
{
   crSetenv( "CR_PERF_LOG_FILE", response );
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions perfSPUOptions[] = {
   { "perf_log_file", CR_STRING, 1, "", NULL, NULL, 
     "Performance Tests Log File", (SPUOptionCB)set_perf_log_file},

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};

void perfspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		return;
	}
	crMothershipIdentifySPU( conn, perf_spu.id );

	crSPUGetMothershipParams( conn, &perf_spu, perfSPUOptions );

	perf_spu.log_file = 0;

	(void) response;

	crMothershipDisconnect( conn );
}
