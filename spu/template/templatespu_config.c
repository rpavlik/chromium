/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "templatespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

void templatespuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, template_spu.id );

	/* CONFIGURATION STUFF HERE */

	(void) response;

	crMothershipDisconnect( conn );
}
