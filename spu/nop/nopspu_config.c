/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "nopspu.h"

#include <stdio.h>
#include <string.h>
#ifndef WINDOWS
#include <unistd.h>
#endif

static void __setDefaults( void )
{
	nop_spu.return_ids = 1;
}

static void set_return_ids( void *foo, const char *response )
{
   sscanf( response, "%d", &(nop_spu.return_ids) );
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions nopSPUOptions[] = {

   { "return_ids", CR_BOOL, 1, "1", "0", "1",
     "Generate sequential window id's on creation", (SPUOptionCB)set_return_ids },

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },

};


void nopspuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, nop_spu.id );

	crSPUGetMothershipParams( conn, &nop_spu, nopSPUOptions );

	(void) response;

	crMothershipDisconnect( conn );
}
