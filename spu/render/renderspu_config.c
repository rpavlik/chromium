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


static void __setDefaults( RenderSPU *render_spu )
{
	/* init in (nearly) the same order as declared in the struct */
	render_spu->window_title = crStrdup( "Chromium Render SPU" );
	render_spu->defaultX = render_spu->defaultY = 0;
	render_spu->defaultWidth = render_spu->defaultHeight = 256;
	render_spu->use_L2 = 0;
	render_spu->fullscreen = 0;
	render_spu->ontop = 0;

	render_spu->drawCursor = GL_FALSE;
	render_spu->cursorX = 0;
	render_spu->cursorY = 0;

#ifdef WINDOWS
	/*
	render_spu->depth_bits = 24;
	render_spu->stencil_bits = 0;
	render_spu->hWnd = 0;
	render_spu->hRC = 0;
	render_spu->device_context = 0;
	*/
#else
	/*
	render_spu->depth_bits = 8;
	render_spu->stencil_bits = 0;
	*/
	render_spu->display_string = NULL;
	render_spu->force_direct = 0;
	render_spu->try_direct = 1;
	render_spu->sync = 0;
#endif 
}

void renderspuGatherConfiguration( RenderSPU *render_spu )
{
	CRConnection *conn;
	char response[8096];

	__setDefaults( render_spu );

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* the defaults are (maybe) OK. */
		return;
	}

	crMothershipIdentifySPU( conn, render_spu->id );

	if (crMothershipGetSPUParam( conn, response, "window_geometry" ) )
	{
		float x, y, w, h;
		sscanf( response, "%f %f %f %f", &x, &y, &w, &h );
		if (w < 1 || h < 1) {
			crWarning("Render SPU window width and height must be at least one");
			if (w < 1)
				w = 1;
			if (h < 1)
				h = 1;
		}
		render_spu->defaultX = (int) x;
		render_spu->defaultY = (int) y;
		render_spu->defaultWidth = (int) w;
		render_spu->defaultHeight = (int) h;
	}

	if (crMothershipGetSPUParam( conn, response, "fullscreen" ) )
	{
		sscanf( response, "%d", &(render_spu->fullscreen) );
	}

	if (crMothershipGetSPUParam( conn, response, "ontop" ) )
	{
		sscanf( response, "%d", &(render_spu->ontop) );
	}

	if (crMothershipGetSPUParam( conn, response, "window_title" ) )
	{
		crFree( render_spu->window_title );
		render_spu->window_title = crStrdup( response );
	}

	if (crMothershipGetSPUParam( conn, response, "system_gl_path" ) )
	{
		crSetenv( "CR_SYSTEM_GL_PATH", response );
	}

#ifndef WINDOWS
	if (crMothershipGetSPUParam( conn, response, "try_direct" ) )
	{
		sscanf( response, "%d", &(render_spu->try_direct) );
	}

	if (crMothershipGetSPUParam( conn, response, "force_direct" ) )
	{
		sscanf( response, "%d", &(render_spu->force_direct) );
	}
#endif

#if 0
	if (crMothershipGetSPUParam( conn, response, "show_cursor" ) )
#else
	if (crMothershipGetParam( conn, "show_cursor", response ) )
#endif
	{
		int show_cursor;
		sscanf( response, "%d", &show_cursor );
		render_spu->drawCursor = show_cursor ? GL_TRUE : GL_FALSE;
		crDebug("show_cursor = %d", show_cursor);
	}
	else {
		crDebug("query show_cursor failed!\n");
	}

	crMothershipDisconnect( conn );
}
