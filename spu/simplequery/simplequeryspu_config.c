/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "simplequeryspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

SPUOptions simplequerySPUOptions[] = {
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};

void simplequeryspuGatherConfiguration( void )
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( &simplequery_spu, simplequerySPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, simplequery_spu.id );

	crSPUGetMothershipParams( conn, &simplequery_spu, simplequerySPUOptions );

	crMothershipDisconnect( conn );
}
