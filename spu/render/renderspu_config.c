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


static void set_window_geometry( RenderSPU *render_spu, const char *response )
{
   float x, y, w, h;
   sscanf( response, "%f %f %f %f", &x, &y, &w, &h );
   render_spu->defaultX = (int) x;
   render_spu->defaultY = (int) y;
   render_spu->defaultWidth = (int) w;
   render_spu->defaultHeight = (int) h;
}

static void set_fullscreen( RenderSPU *render_spu, const char *response )
{
   sscanf( response, "%d", &(render_spu->fullscreen) );
}

static void set_on_top( RenderSPU *render_spu, const char *response )
{
   sscanf( response, "%d", &(render_spu->ontop) );
}

static void set_system_gl_path( RenderSPU *render_spu, const char *response )
{
   crSetenv( "CR_SYSTEM_GL_PATH", response );
}

static void set_title( RenderSPU *render_spu, const char *response )
{
   crFree( render_spu->window_title );
   render_spu->window_title = crStrdup( response );
}

#ifndef WINDOWS
static void set_try_direct( RenderSPU *render_spu, const char *response )
{
   sscanf( response, "%d", &(render_spu->try_direct) );
}

static void set_force_direct( RenderSPU *render_spu, const char *response )
{
   sscanf( response, "%d", &(render_spu->force_direct) );
}
#endif


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions renderSPUOptions[] = {
#ifndef WINDOWS
   { "try_direct", CR_BOOL, 1, "1", NULL, NULL, 
     "Try Direct Rendering", (SPUOptionCB)set_try_direct  },

   { "force_direct", CR_BOOL, 1, "0", NULL, NULL, 
     "Force Direct Rendering", (SPUOptionCB)set_force_direct },
#endif

   { "fullscreen", CR_BOOL, 1, "0", NULL, NULL, 
     "Full-screen Window", (SPUOptionCB)set_fullscreen },

   { "on_top", CR_BOOL, 1, "0", NULL, NULL, 
     "Display on top", (SPUOptionCB)set_on_top },

   { "title", CR_STRING, 1, "Chromium Render SPU", NULL, NULL, 
     "Window Title", (SPUOptionCB)set_title },

   { "window_geometry", CR_INT, 4, "0 0 256 256", "0 0 1 1", NULL, 
     "Window Geometry (x,y,w,h)", (SPUOptionCB)set_window_geometry },

   { "system_gl_path", CR_STRING, 1, "/usr/lib/", NULL, NULL, 
     "System GL Path", (SPUOptionCB)set_system_gl_path},

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void renderspuGatherConfiguration( RenderSPU *render_spu )
{
	CRConnection *conn;
	char response[8096];

	conn = crMothershipConnect( );
	
	if (conn) {
		crMothershipIdentifySPU( conn, render_spu->id );
		
		crSPUGetMothershipParams( conn, 
					  (void *)render_spu,
					  renderSPUOptions );
		
		/* Additional configuration not expressed in the 
		 * SPUOptions array:
		 */
		if (crMothershipGetParam( conn, "show_cursor", response ) ) {
			int show_cursor;
			sscanf( response, "%d", &show_cursor );
			render_spu->drawCursor = show_cursor 
				? GL_TRUE : GL_FALSE;
			crDebug("show_cursor = %d", show_cursor);
		}
		else {
		}

		crMothershipDisconnect( conn );
	}
	else {
		crSPUSetDefaultParams( (void *)render_spu, renderSPUOptions );
	}

	/* Some initialization that doesn't really have anything to do
	 * with configuration but which was done here before:
	 */
	render_spu->use_L2 = 0;
	render_spu->cursorX = 0;
	render_spu->cursorY = 0;
#ifndef WINDOWS	
	render_spu->sync = 0;
	render_spu->display_string = NULL;
#endif
}


