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

	__setDefaults();

	// Connect to the mothership and identify ourselves.
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		// defaults are fine.
		return;
	}
	crMothershipIdentifySPU( conn, print_spu.id );

	if (crMothershipSPUParam( conn, response, "log_file") )
	{
		print_spu.fp = fopen( response, "w" );
		if (print_spu.fp == NULL)
		{
			crError( "Couldn't open print SPU log file %s", response );
		}
	}

	crMothershipDisconnect( conn );
}
