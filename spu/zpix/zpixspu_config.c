/* Copyright (c) 2003, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "zpixspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( ZpixSPU *zpix_spu )
{
       zpix_spu->verbose = 0;
       zpix_spu->debug   = 0;
       zpix_spu->no_diff = 0;         /* suppress  buffer differencing */
       zpix_spu->ztype   = ZLIB;
       zpix_spu->zparm    = -1;       /* zlib's default parm */
}

static void set_verbose( ZpixSPU *zpix_spu, const char *response )
{
       zpix_spu->verbose = crStrToInt( response );
       crDebug("Zpix SPU config: verbose = %d", zpix_spu->verbose);
}

static void set_debug( ZpixSPU *zpix_spu, const char *response )
{
       zpix_spu->debug = crStrToInt( response );
       crDebug("Zpix SPU config: debug = %d", zpix_spu->debug);
}

static void set_no_diff( ZpixSPU *zpix_spu, const char *response )
{
       zpix_spu->no_diff = crStrToInt( response );
       crDebug("Zpix SPU config: no_diff = %d", zpix_spu->no_diff);
}

static void set_ztype( ZpixSPU *zpix_spu, const char *response )
{
       zpix_spu->ztype = crStrToInt( response );
       crDebug("Zpix SPU config: ztype = %d", zpix_spu->ztype);
}

static void set_zparm( ZpixSPU *zpix_spu, const char *response )
{
       zpix_spu->zparm = crStrToInt( response );
       crDebug("Zpix SPU config: zparm = %d", zpix_spu->zparm);
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions zpixSPUOptions[] = {
   { "verbose", CR_BOOL, 1, "0", NULL, NULL,
      "Verbose messages true or false", (SPUOptionCB) set_verbose },
   { "ztype", CR_INT, 1, "1", NULL, NULL,
      "Compression type from ZTYPE enum", (SPUOptionCB) set_ztype },
   { "zparm", CR_INT, 1, "-1", NULL, NULL,
      "Compression parameter for ZTYPE", (SPUOptionCB) set_zparm },
   { "debug", CR_BOOL, 0, "0", NULL, NULL, 
      "Debug state true or false", (SPUOptionCB) set_debug },
   { "no_diff", CR_BOOL, 0, "0", NULL, NULL, 
      "Suppress buffer differencing", (SPUOptionCB) set_no_diff },
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void zpixspuGatherConfiguration( ZpixSPU *zpix_spu )
{
	CRConnection *conn;

	__setDefaults( zpix_spu );

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( zpix_spu, zpixSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, zpix_spu->id );

	crSPUGetMothershipParams( conn, (void *) zpix_spu, zpixSPUOptions );

	crMothershipDisconnect( conn );
}
