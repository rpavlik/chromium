/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "templatespu.h"

#include "cr_mothership.h"
#include "cr_string.h"


/**
 * Set default options for SPU
 */
static void setDefaults( void )
{
}
/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
SPUOptions templateSPUOptions[] = {
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


/**
 * Gather the config info for the SPU
 */
void templatespuGatherConfiguration( void )
{
	CRConnection *conn;

	setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( &template_spu, templateSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, template_spu.id );

	crSPUGetMothershipParams( conn, &template_spu, templateSPUOptions );

	crMothershipDisconnect( conn );
}
