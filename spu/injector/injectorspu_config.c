/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "injectorspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

static void set_oob_url( InjectorSPU* spu, const char* response )
{
	spu->oob_url = crStrdup( response ) ;
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions injectorSPUOptions[] = {
   { "oob_url", CR_STRING, 1, "tcpip://127.0.0.1:8194", NULL, NULL,
     "URL of stream to inject", (SPUOptionCB) set_oob_url },
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void injectorspuGatherConfiguration( InjectorSPU* spu )
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( spu, injectorSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, spu->id );

	crSPUGetMothershipParams( conn, spu, injectorSPUOptions );

	crMothershipDisconnect( conn );

	spu->oob_conn = NULL ;
}
