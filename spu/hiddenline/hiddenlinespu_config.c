/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
	hiddenline_spu.buffer_size = 32*1024;
	hiddenline_spu.clear_r = 
		hiddenline_spu.clear_g = 
		hiddenline_spu.clear_b = 0.0f;
}

void hiddenlinespuGatherConfiguration( SPU *child )
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
	crMothershipIdentifySPU( conn, hiddenline_spu.id );

	if (crMothershipGetSPUParam( conn, response, "buffer_size" ))
	{
		sscanf( response, "%d", &(hiddenline_spu.buffer_size) );
	}

	crSPUPropogateGLLimits( conn, hiddenline_spu.id, child, &(hiddenline_spu.limits) );

	crMothershipDisconnect( conn );
}
