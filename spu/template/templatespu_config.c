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

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions templatespuSPUOptions[] = {
   { NULL, BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void templatespuGatherConfiguration( void )
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( &template_spu, templatespuSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, template_spu.id );

	crSPUGetMothershipParams( conn, &template_spu, templatespuSPUOptions );

	crMothershipDisconnect( conn );
}
