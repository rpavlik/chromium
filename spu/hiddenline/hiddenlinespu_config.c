/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"

#include "cr_mothership.h"
#include "cr_string.h"



static void set_buffer_size( void *spu, const char *response )
{
	(void) spu;
	sscanf( response, "%d", &(hiddenline_spu.buffer_size) );
}

static void set_fog_color( void *spu, const char *response )
{
	float r, g, b;
	(void) spu;
	CRASSERT(response[0] == '[');
	sscanf( response, "[ %f, %f, %f ]", &r, &g, &b );
	hiddenline_spu.fog_color[0] = r;
	hiddenline_spu.fog_color[1] = g;
	hiddenline_spu.fog_color[2] = b;
}

static void set_poly_color( void *spu, const char *response )
{
	float r, g, b;
	(void) spu;
	CRASSERT(response[0] == '[');
	sscanf( response, "[ %f, %f, %f ]", &r, &g, &b );
	hiddenline_spu.poly_r = r;
	hiddenline_spu.poly_g = g;
	hiddenline_spu.poly_b = b;
}

static void set_line_color( void *spu, const char *response )
{
	float r, g, b;
	(void) spu;
	CRASSERT(response[0] == '[');
	sscanf( response, "[ %f, %f, %f ]", &r, &g, &b );
	hiddenline_spu.line_r = r;
	hiddenline_spu.line_g = g;
	hiddenline_spu.line_b = b;
}

static void set_line_width( void *spu, const char *response )
{
	(void) spu;
	sscanf( response, "%f", &(hiddenline_spu.line_width) );
}

static void set_single_clear( void *spu, const char *response )
{
	(void) spu;
	sscanf( response, "%d", &(hiddenline_spu.single_clear) );
}

static void set_silhouette_mode( void *spu, const char *response )
{
	(void) spu;
	sscanf( response, "%d", &(hiddenline_spu.silhouette_mode) );
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions hiddenlineSPUOptions[] = {

	{ "buffer_size", CR_INT, 1, "32768", "128", "1048576", 
	  "Buffer Size (bytes)", (SPUOptionCB)set_buffer_size },

	{ "fog_color", CR_FLOAT, 3, "[0, 0, 0]", "[0, 0, 0]", "[1, 1, 1]",
	  "Fog Color (r, g, b)", (SPUOptionCB)set_fog_color },

	{ "poly_color", CR_FLOAT, 3, "[1, 1, 1]", "[0, 0, 0]", "[1, 1, 1]", 
	  "Polygon Color (r, g, b)", (SPUOptionCB)set_poly_color },

	{ "line_color", CR_FLOAT, 3, "[0, 0, 0]", "[0, 0, 0]", "[1, 1, 1]", 
	  "Line Color (r, g, b)", (SPUOptionCB)set_line_color },

	{ "line_width", CR_FLOAT, 1, "3", "0", "20",
	  "Line Width", (SPUOptionCB)set_line_width },

	{ "silhouette_mode", CR_BOOL, 1, "0", NULL, NULL,
	  "Silhouette mode", (SPUOptionCB)set_silhouette_mode },

	{ "single_clear", CR_BOOL, 1, "1", NULL, NULL,
	  "Single glClear per Frame", (SPUOptionCB)set_single_clear },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL }
};

void hiddenlinespuGatherConfiguration( SPU *child )
{
	CRConnection *conn;

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

	crMothershipDisconnect( conn );
}
