/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "mattespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

static void set_matte_region( MatteSPU *spu, const char *response )
{
    int x, y, width, height;
    CRASSERT(response[0] == '[');
    if (sscanf( response, "[ %d, %d, %d, %d ]", &x, &y, &width, &height) == 4) {
	spu->matteRegion.x = x;
	spu->matteRegion.y = y;
	spu->matteRegion.width = width;
	spu->matteRegion.height = height;
    }
    else {
	crDebug("matte SPU: illegal region %s ignored.", response);
    }
}

static void set_matte_color( MatteSPU *spu, const char *response )
{
    float r, g, b, a;
    CRASSERT(response[0] == '[');
    if (sscanf( response, "[ %f, %f, %f, %f ]", &r, &g, &b, &a ) == 4) {
	spu->matteColor.red = r;
	spu->matteColor.green = g;
	spu->matteColor.blue = b;
	spu->matteColor.alpha = a;
    }
    else {
	crDebug("matte SPU: illegal matte color %s ignored.", response);
    }
}

static void set_use_matte_color(MatteSPU *spu, const char *response)
{
    spu->useMatteColor = crStrToInt(response);
}

static void set_every_clear(MatteSPU *spu, const char *response)
{
    spu->matteEveryClear = crStrToInt(response);
}

/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
SPUOptions matteSPUOptions[] = {
    { "matte_region", CR_INT, 4, "[0, 0, 1024, 1024]", NULL, NULL, 
	"Matte Region (x, y, width, height)", (SPUOptionCB)set_matte_region },

    { "use_matte_color", CR_BOOL, 1, "1", NULL, NULL,
	"Whether to use the Matte Color", (SPUOptionCB)set_use_matte_color },

    { "matte_color", CR_FLOAT, 4, "[0, 0, 0, 0]", NULL, NULL, 
	"Matte Color (r,g,b,a)", (SPUOptionCB)set_matte_color },

    { "every_clear", CR_BOOL, 1, "0", NULL, NULL,
	"Whether to redraw the matte at every clear", (SPUOptionCB) set_every_clear},

    { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


/**
 * Gather the config info for the SPU
 */
void mattespuGatherConfiguration( void )
{
	CRConnection *conn;

	/* Connect to the mothership and identify ourselves. */
	conn = crMothershipConnect( );
	if (conn) {
	    crMothershipIdentifySPU( conn, matte_spu.id );
	    crSPUGetMothershipParams( conn, &matte_spu, matteSPUOptions );
	    crMothershipDisconnect( conn );
	}
	else {
	    crSPUSetDefaultParams( &matte_spu, matteSPUOptions );
	}
}
