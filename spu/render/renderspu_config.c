#include "renderspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"

#include <stdio.h>


static void __setDefaults( void )
{
	render_spu.window_x = render_spu.window_y = 0;
	render_spu.window_width = render_spu.window_height = 256;
#ifndef WINDOWS
	render_spu.depth_bits = 8;
	render_spu.stencil_bits = 0;
#else
	render_spu.depth_bits = 32;
	render_spu.stencil_bits = 0;
#endif 
	render_spu.fullscreen = 0;
	render_spu.use_L2 = 0;
}

void renderspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	// Connect to the mothership and identify ourselves.
	
	conn = crMothershipConnect( );
	crMothershipIdentifySPU( conn, render_spu.id );

	__setDefaults();

	if (crMothershipSPUParam( conn, response, "window_geometry" ) )
	{
		sscanf( response, "%d %d %d %d", &(render_spu.window_x), 
				&(render_spu.window_y), &(render_spu.window_width),
				&(render_spu.window_height) );
	}
}
