#include "packspu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mothership.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

void packspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];
	char servername[8096];
	int num_servers;

	conn = crMothershipConnect();
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}
	crMothershipIdentifySPU( conn, pack_spu.id );

	__setDefaults();

	crMothershipGetServers( conn, response );

	sscanf( response, "%d %s", &num_servers, servername );

	if (num_servers == 1)
	{
		pack_spu.server.name = crStrdup( servername );
	}
	else
	{
		crError( "Bad server specification for Pack SPU %d", pack_spu.id );
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%d", &(pack_spu.server.buffer_size) );

	crMothershipDisconnect( conn );
}
