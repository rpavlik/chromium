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
	hiddenline_spu.poly_r = .75;
	hiddenline_spu.poly_g = .75;
	hiddenline_spu.poly_b = 0;
	hiddenline_spu.line_r = 0;
	hiddenline_spu.line_g = 0;
	hiddenline_spu.line_b = 0;
	hiddenline_spu.line_width = 1;
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

	if (crMothershipGetSPUParam( conn, response, "poly_color" ))
	{
		sscanf( response, "%f %f %f", &(hiddenline_spu.poly_r), &(hiddenline_spu.poly_g), &(hiddenline_spu.poly_b) );
	}

	if (crMothershipGetSPUParam( conn, response, "line_color" ))
	{
		sscanf( response, "%f %f %f", &(hiddenline_spu.line_r), &(hiddenline_spu.line_g), &(hiddenline_spu.line_b) );
	}

	if (crMothershipGetSPUParam( conn, response, "line_width" ))
	{
		sscanf( response, "%f", &(hiddenline_spu.line_width) );
	}

	crSPUPropogateGLLimits( conn, hiddenline_spu.id, child, &(hiddenline_spu.limits) );

	crMothershipDisconnect( conn );
}
