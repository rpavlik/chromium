/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "fpsspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void report_frames(FpsSPU *spu, const char *response )
{
	sscanf( response, "%d", &(spu->report_frames) );
}

static void report_seconds(FpsSPU *spu, const char *response )
{
	sscanf( response, "%f", &(spu->report_seconds) );
}

static void report_at_end(FpsSPU *spu, const char *response )
{
	sscanf( response, "%d", &(spu->report_at_end) );
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions fpsSPUOptions[] = {
   { "report_frames", CR_INT, 1, "100", "0", NULL, "How many frames between reports", (SPUOptionCB)report_frames},
   { "report_seconds", CR_FLOAT, 1, "10.0", "0", NULL, "How many seconds between reports", (SPUOptionCB)report_seconds},
   { "report_at_end", CR_BOOL, 1, "0", NULL, NULL, "Provide a summary report at the end", (SPUOptionCB)report_at_end},
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};

void fpsspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	crSPUSetDefaultParams( (void *)&fps_spu, fpsSPUOptions );
	fps_spu.total_frames = 0;

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( ); if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		return;
	}
	crMothershipIdentifySPU( conn, fps_spu.id );

	/* CONFIGURATION STUFF HERE */
	crSPUGetMothershipParams( conn, &fps_spu, fpsSPUOptions);

	(void) response;

	crMothershipDisconnect( conn );
}
