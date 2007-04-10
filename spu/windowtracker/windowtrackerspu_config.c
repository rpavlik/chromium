/* Copyright (c) 2007, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "windowtrackerspu.h"
#include "cr_mothership.h"
#include "cr_string.h"


/**
 * Set default options for SPU
 */
static void setDefaults( void )
{
	windowtracker_spu.display = NULL;
	windowtracker_spu.window_title = NULL;
	windowtracker_spu.dpy = NULL;
	windowtracker_spu.win = 0;
}


static void
set_display(WindowtrackerSPU *spu, const char *response)
{
	spu->display = crStrdup(response);
}

static void
set_window_title(WindowtrackerSPU *spu, const char *response)
{
	spu->window_title = crStrdup(response);
}


/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
SPUOptions windowtrackerSPUOptions[] = {
	{ "display", CR_STRING, 1, NULL, NULL, NULL,
		"Name of display where window is to be found",
		(SPUOptionCB) set_display
	},
	{ "window_title", CR_STRING, 1, NULL, NULL, NULL,
		"Name (*pattern*) of window to track size/position of",
		(SPUOptionCB) set_window_title
	},
	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


/**
 * Gather the config info for the SPU
 */
void windowtrackerspuGatherConfiguration( void )
{
	CRConnection *conn;

	setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( &windowtracker_spu, windowtrackerSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, windowtracker_spu.id );

	crSPUGetMothershipParams( conn, &windowtracker_spu, windowtrackerSPUOptions );

	crMothershipDisconnect( conn );
}
