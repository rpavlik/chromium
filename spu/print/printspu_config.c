#include "printspu.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"

#include <stdio.h>

static void __setDefaults( void )
{
	print_spu.fp = stderr;
}

void printspuGatherConfiguration( void )
{
	CRConnection *conn;
	char response[8096];
	char log_path[8096];

	__setDefaults();

	// Connect to the mothership and identify ourselves.
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		// defaults are fine.
		return;
	}
	crMothershipIdentifySPU( conn, print_spu.id );

	if (!crMothershipSPUParam( conn, log_path, "log_path") )
	{
		strcpy( log_path, "." );
	}

	if (crMothershipSPUParam( conn, response, "log_file") )
	{
		sprintf( log_path, "%s/%s", log_path, response );
		print_spu.fp = fopen( log_path, "w" );
		if (print_spu.fp == NULL)
		{
			crError( "Couldn't open print SPU log file %s", log_path );
		}
	}

	crMothershipDisconnect( conn );
}
