/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "renderspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_environment.h"

#include <stdio.h>


static void __setDefaults( void )
{
	render_spu.window_x = render_spu.window_y = 0;
	render_spu.window_width = render_spu.window_height = 256;
#ifndef WINDOWS
	render_spu.depth_bits = 8;
	render_spu.stencil_bits = 0;
	render_spu.force_direct = 0;
	render_spu.try_direct = 1;
#else
	render_spu.depth_bits = 24;
	render_spu.stencil_bits = 0;
#endif 
	render_spu.fullscreen = 0;
	render_spu.ontop = 0;
	render_spu.use_L2 = 0;
	render_spu.window_title = crStrdup( "Chromium Render SPU" );
}

void renderspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* the defaults are (maybe) OK. */
		return;
	}

	crMothershipIdentifySPU( conn, render_spu.id );

	if (crMothershipGetSPUParam( conn, response, "window_geometry" ) )
	{
		float x,y,w,h;
		sscanf( response, "%f %f %f %f", &x, &y, &w, &h );
		render_spu.window_x = (int)x;
		render_spu.window_y = (int)y;
		render_spu.window_width = (int)w;
		render_spu.window_height = (int)h;
	}

	if (crMothershipGetSPUParam( conn, response, "fullscreen" ) )
	{
		sscanf( response, "%d", &(render_spu.fullscreen) );
	}

	if (crMothershipGetSPUParam( conn, response, "ontop" ) )
	{
		sscanf( response, "%d", &(render_spu.ontop) );
	}

	if (crMothershipGetSPUParam( conn, response, "stencil_bits" ) )
	{
		sscanf( response, "%d", &(render_spu.stencil_bits) );
	}

	if (crMothershipGetSPUParam( conn, response, "window_title" ) )
	{
		crFree( render_spu.window_title );
		render_spu.window_title = crStrdup( response );
	}

	if (crMothershipGetSPUParam( conn, response, "system_gl_path" ) )
	{
		crSetenv( "CR_SYSTEM_GL_PATH", response );
	}

#ifndef WINDOWS
	if (crMothershipGetSPUParam( conn, response, "try_direct" ) )
	{
		sscanf( response, "%d", &(render_spu.try_direct) );
	}

	if (crMothershipGetSPUParam( conn, response, "force_direct" ) )
	{
		sscanf( response, "%d", &(render_spu.force_direct) );
	}
#endif

	crMothershipDisconnect( conn );
}
