/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "dist_texturespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions dist_textureSPUOptions[] = {

   { NULL, BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};



void dist_texturespuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, dist_texture_spu.id );

	/* CONFIGURATION STUFF HERE */
	crSPUGetMothershipParams( conn, &dist_texture_spu, dist_textureSPUOptions );

	(void) response;

	crMothershipDisconnect( conn );
}
