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

static void __setDefaults( void )
{
	comm_spu.i_am_the_server = 0;
}

void commspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		return;
	}
	crMothershipIdentifySPU( conn, comm_spu.id );

	/* CONFIGURATION STUFF HERE */

	if (crMothershipGetSPUParam( conn, response, "peer" ) )
	{
		comm_spu.peer_name = crStrdup( response );
	}
	else
	{
		crError( "No peer specified for the comm SPU?" );
	}

	if (crMothershipGetSPUParam( conn, response, "server" ) )
	{
		comm_spu.i_am_the_server = 1;
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%d", &(comm_spu.mtu) );

	crMothershipDisconnect( conn );
}
