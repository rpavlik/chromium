/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "commspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"

#include <stdio.h>

static void set_peer( void *foo, const char *response )
{
   if (*response)
   {
      comm_spu.peer_name = crStrdup( response );
   }
   else
   {
      crError( "No peer specified for the comm SPU?" );
   }
}

static void set_server( void *foo, const char *response )
{
   comm_spu.i_am_the_server = crStrToInt( response );
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions commSPUOptions[] = {

   { "server", BOOL, 1, "0", NULL, NULL, 
     "Server Node", (SPUOptionCB)set_server },

   { "peer", STRING, 1, "", NULL, NULL, 
     "Peer Name", (SPUOptionCB)set_peer},

   { NULL, BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void commspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. 
		 */

		crError( "cannot connect to mothership" );
	}
	crMothershipIdentifySPU( conn, comm_spu.id );

	/* CONFIGURATION STUFF HERE */

	crSPUGetMothershipParams( conn, &comm_spu, commSPUOptions );

	crMothershipGetMTU( conn, response );
	sscanf( response, "%d", &(comm_spu.mtu) );

	crMothershipDisconnect( conn );
}
