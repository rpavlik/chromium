/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "readbackspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
	readback_spu.extract_depth = 0;
	readback_spu.local_visualization = 1;
}

void readbackspuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, readback_spu.id );

	if (crMothershipGetSPUParam( conn, response, "extract_depth" ))
	{
		readback_spu.extract_depth = crStrToInt( response );
	}
	if (crMothershipGetSPUParam( conn, response, "local_visualization" ))
	{
		readback_spu.local_visualization = crStrToInt( response );
	}

	crMothershipDisconnect( conn );
}
