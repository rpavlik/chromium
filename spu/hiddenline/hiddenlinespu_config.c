/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>


void set_buffer_size( void *foo, const char *response )
{
	sscanf( response, "%d", &(hiddenline_spu.buffer_size) );
}

void set_poly_color( void *foo, const char *response )
{
	float r, g, b;
	if( response[0] == '[' )
		sscanf( response, "[ %f, %f, %f ]", &r, &g, &b );
	else if( crStrchr( response, ',' ))
		sscanf( response, "%f, %f, %f", &r, &g, &b );
	else
		sscanf( response, "%f %f %f", &r, &g, &b );
	hiddenline_spu.poly_r = r;
	hiddenline_spu.poly_g = g;
	hiddenline_spu.poly_b = b;
}

void set_line_color( void *foo, const char *response )
{
	float r, g, b;
	if( response[0] == '[' )
		sscanf( response, "[ %f, %f, %f ]", &r, &g, &b );
	else if( crStrchr( response, ',' ))
		sscanf( response, "%f, %f, %f", &r, &g, &b );
	else
		sscanf( response, "%f %f %f", &r, &g, &b );
	hiddenline_spu.line_r = r;
	hiddenline_spu.line_g = g;
	hiddenline_spu.line_b = b;
}

void set_line_width( void *foo, const char *response )
{
	sscanf( response, "%f", &(hiddenline_spu.line_width) );
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions hiddenlineSPUOptions[] = {

	{ "buffer_size", CR_INT, 1, "32768", "128", "1048576", 
	  "Buffer Size (bytes)", (SPUOptionCB)set_buffer_size },

	{ "poly_color", CR_FLOAT, 3, ".75, .75, 0", "0, 0, 0", "1, 1, 1", 
	  "Polygon Color (r, g, b)", (SPUOptionCB)set_poly_color },

	{ "line_color", CR_FLOAT, 3, "0, 0, 0", "0, 0, 0", "1, 1, 1", 
	  "Line Color (r, g, b)", (SPUOptionCB)set_line_color },

	{ "line_width", CR_FLOAT, 1, "1", "0", "20", 
	  "Line Width", (SPUOptionCB)set_line_width },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL }
};

void hiddenlinespuGatherConfiguration( SPU *child )
{
	CRConnection *conn;

	hiddenline_spu.clear_r = 
		hiddenline_spu.clear_g = 
		hiddenline_spu.clear_b = 0.0f;

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( &hiddenline_spu, hiddenlineSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, hiddenline_spu.id );

	crSPUGetMothershipParams( conn, &hiddenline_spu, hiddenlineSPUOptions );

	crSPUPropogateGLLimits( conn, hiddenline_spu.id, child, &(hiddenline_spu.limits) );

	crMothershipDisconnect( conn );
}
