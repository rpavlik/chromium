/* Copyright (c) 2004, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "vncspu.h"

#include "cr_mothership.h"
#include "cr_string.h"


/**
 * Set default options for SPU
 */
static void setDefaults( void )
{
}


static void set_server_port(VncSPU *vnc_spu, const char *response)
{
	vnc_spu->server_port = crStrToInt(response);
}

static void set_screen_size( VncSPU *vnc_spu, const char *response )
{
	CRASSERT(response[0] == '[');
	sscanf(response, "[ %d, %d ]",
				 &vnc_spu->screen_width,	&vnc_spu->screen_height);
}

static void set_max_update_rate(VncSPU *vnc_spu, const char *response)
{
	vnc_spu->max_update_rate = crStrToInt(response);
}


/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
SPUOptions vncSPUOptions[] = {
	{ "server_port", CR_INT, 1, "-1", NULL, NULL,
		"VNC Server Port Number", (SPUOptionCB) set_server_port },
	{ "screen_size", CR_INT, 2, "[1024, 768]", "[1, 1]", NULL,
		"Screen Size", (SPUOptionCB) set_screen_size },
	{ "max_update_rate", CR_INT, 1, "10", "1", NULL,
		"Max client frame rate", (SPUOptionCB) set_max_update_rate },
	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


/**
 * Gather the config info for the SPU
 */
void vncspuGatherConfiguration( void )
{
	CRConnection *conn;

	setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( &vnc_spu, vncSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, vnc_spu.id );

	crSPUGetMothershipParams( conn, &vnc_spu, vncSPUOptions );

	crMothershipDisconnect( conn );
}
