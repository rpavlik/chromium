/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "wetspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
	wet_spu.density = 5;
	wet_spu.raininess = 10;
	wet_spu.mesh_dice = 100;
	wet_spu.time_scale = .1f;
	wet_spu.ripple_freq = 2;
	wet_spu.ripple_scale = 1;
	wet_spu.ior = 1.2f;
}

void wetspuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, wet_spu.id );

	if (crMothershipGetSPUParam( conn, response, "density") )
	{
		sscanf( response, "%d", &(wet_spu.density) );
	}
	if (crMothershipGetSPUParam( conn, response, "raininess") )
	{
		sscanf( response, "%d", &(wet_spu.raininess) );
	}
	if (crMothershipGetSPUParam( conn, response, "mesh_dice") )
	{
		sscanf( response, "%d", &(wet_spu.mesh_dice) );
	}
	if (crMothershipGetSPUParam( conn, response, "ripple_freq") )
	{
		sscanf( response, "%f", &(wet_spu.ripple_freq) );
	}
	if (crMothershipGetSPUParam( conn, response, "ripple_scale") )
	{
		sscanf( response, "%f", &(wet_spu.ripple_scale) );
	}
	if (crMothershipGetSPUParam( conn, response, "time_scale") )
	{
		sscanf( response, "%f", &(wet_spu.time_scale) );
	}
	if (crMothershipGetSPUParam( conn, response, "ior") )
	{
		sscanf( response, "%f", &(wet_spu.ior) );
	}

	crMothershipDisconnect( conn );
}
