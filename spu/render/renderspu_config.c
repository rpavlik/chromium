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
#include "cr_url.h"

#include <stdio.h>


static void set_window_geometry( RenderSPU *render_spu, const char *response )
{
	float x, y, w, h;
	if (response[0] == '[')
		sscanf( response, "[ %f, %f, %f, %f ]", &x, &y, &w, &h );
	else if (crStrchr(response, ','))
		sscanf( response, "%f, %f, %f, %f", &x, &y, &w, &h );
	else
		sscanf( response, "%f %f %f %f", &x, &y, &w, &h );
	render_spu->defaultX = (int) x;
	render_spu->defaultY = (int) y;
	render_spu->defaultWidth = (int) w;
	render_spu->defaultHeight = (int) h;
}

static void set_default_visual( RenderSPU *render_spu, const char *response )
{
	if (crStrlen(response) > 0) {
		if (crStrstr(response, "rgb"))
				render_spu->default_visual |= CR_RGB_BIT;
		if (crStrstr(response, "alpha"))
				render_spu->default_visual |= CR_ALPHA_BIT;
		if (crStrstr(response, "z") || crStrstr(response, "depth"))
				render_spu->default_visual |= CR_DEPTH_BIT;
		if (crStrstr(response, "stencil"))
				render_spu->default_visual |= CR_STENCIL_BIT;
		if (crStrstr(response, "accum"))
				render_spu->default_visual |= CR_ACCUM_BIT;
		if (crStrstr(response, "stereo"))
				render_spu->default_visual |= CR_STEREO_BIT;
		if (crStrstr(response, "multisample"))
				render_spu->default_visual |= CR_MULTISAMPLE_BIT;
		if (crStrstr(response, "double"))
				render_spu->default_visual |= CR_DOUBLE_BIT;
	}
}

static void set_display_string( RenderSPU *render_spu, const char *response )
{
   	crStrncpy(render_spu->display_string, response, sizeof(render_spu->display_string));
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
	if (crStrlen(response) > 0)
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

static void render_to_app_window( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->render_to_app_window) );
}

static void render_to_crut_window( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->render_to_crut_window) );
}

static void resizable( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->resizable) );
}

static void set_borderless( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->borderless) );
}

static void set_cursor( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->drawCursor) );
}

static void gather_url( RenderSPU *render_spu, const char *response )
{
	char protocol[4096], hostname[4096];
	unsigned short port;
	
	if (!crParseURL(response, protocol, hostname, &port, 0))
	{
		crError( "Malformed URL: \"%s\"", response );
	}

	render_spu->gather_port = port;
}

static void gather_userbuf( RenderSPU *render_spu, const char *response )
{
	sscanf( response, "%d", &(render_spu->gather_userbuf_size) );
}

static void set_lut8( RenderSPU *render_spu, const char *response )
{
	int a;	
	char **lut;
	
	if (!response[0]) return;

	lut = crStrSplit(response, ",");
	if (!lut) return;

	for (a=0; a<256; a++)
	{
		render_spu->lut8[0][a]	= crStrToInt(lut[a]);
		render_spu->lut8[1][a]	= crStrToInt(lut[256+a]);
		render_spu->lut8[2][a]	= crStrToInt(lut[512+a]);
	}

	crFreeStrings(lut);

	render_spu->use_lut8 = 1;
}

static void set_master_url ( RenderSPU *render_spu, char *response )
{
	if (response[0])
		render_spu->swap_master_url = crStrdup( response );
	else
		render_spu->swap_master_url = NULL;
}

static void set_is_master ( RenderSPU *render_spu, char *response )
{
	render_spu->is_swap_master = crStrToInt( response );
}

static void set_num_clients ( RenderSPU *render_spu, char *response )
{
	render_spu->num_swap_clients = crStrToInt( response );
}


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
     "Display on Top", (SPUOptionCB)set_on_top },

   { "render_to_app_window", CR_BOOL, 1, "0", NULL, NULL,
     "Render to Application window", (SPUOptionCB)render_to_app_window },

   { "render_to_crut_window", CR_BOOL, 1, "0", NULL, NULL,
     "Render to CRUT window", (SPUOptionCB)render_to_crut_window },

   { "resizable", CR_BOOL, 1, "0", NULL, NULL,
     "Resizable Window", (SPUOptionCB)resizable },

   { "title", CR_STRING, 1, "Chromium Render SPU", NULL, NULL, 
     "Window Title", (SPUOptionCB)set_title },

   { "window_geometry", CR_INT, 4, "0, 0, 256, 256", "0, 0, 1, 1", NULL, 
     "Default Window Geometry (x,y,w,h)", (SPUOptionCB)set_window_geometry },

   { "default_visual", CR_STRING, 1, "rgb", NULL, NULL,
     "Default GL Visual", (SPUOptionCB) set_default_visual },

   { "borderless", CR_BOOL, 1, "0", NULL, NULL,
     "Borderless Window", (SPUOptionCB) set_borderless },

   { "show_cursor", CR_BOOL, 1, "0", NULL, NULL,
     "Show Software Cursor", (SPUOptionCB) set_cursor },

   { "system_gl_path", CR_STRING, 1, "", NULL, NULL, 
     "System GL Path", (SPUOptionCB)set_system_gl_path },

   { "display_string", CR_STRING, 1, "", NULL, NULL, 
     "X Display String", (SPUOptionCB)set_display_string },

   { "gather_url", CR_STRING, 1, "", NULL, NULL,
     "Gatherer URL", (SPUOptionCB)gather_url},

   { "gather_userbuf_size", CR_INT, 1, "0", NULL, NULL,
     "Size of Buffer to Allocate for Gathering",
     (SPUOptionCB)gather_userbuf},

   { "lut8", CR_STRING, 1, "", NULL, NULL,
     "8 bit RGB LUT", (SPUOptionCB)set_lut8},

   { "swap_master_url", CR_STRING, 1, "", NULL, NULL,
     "The URL to the master swapper", (SPUOptionCB)set_master_url },

   { "is_swap_master", CR_BOOL, 1, "0", NULL, NULL,
     "Is this the swap master", (SPUOptionCB)set_is_master },

   { "num_swap_clients", CR_INT, 1, "1", NULL, NULL,
     "How many swaps to wait on", (SPUOptionCB)set_num_clients },

     
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void renderspuGatherConfiguration( RenderSPU *render_spu )
{
	CRConnection *conn;
	int a;

	for (a=0; a<256; a++)
	{
		render_spu->lut8[0][a] = 
		render_spu->lut8[1][a] = 
		render_spu->lut8[2][a] = a;
	}
	render_spu->use_lut8 = 0;

	conn = crMothershipConnect( );
	if (conn) {
		crMothershipIdentifySPU( conn, render_spu->id );
		crSPUGetMothershipParams( conn, 
					  (void *)render_spu,
					  renderSPUOptions );
		
		render_spu->swap_mtu = crMothershipGetMTU( conn );
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
#endif
}


