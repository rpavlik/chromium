/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "printspu.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_spu.h"

static void __setDefaults( void )
{
	print_spu.fp = stderr;
}

void printspuGatherConfiguration( const SPU *child_spu )
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

	if (crMothershipGetSPUParam( conn, response, "log_file") )
	{
		print_spu.fp = fopen( response, "w" );
		if (print_spu.fp == NULL)
		{
			crError( "Couldn't open print SPU log file %s", response );
		}
	}

        crSPUPropogateGLLimits( conn, print_spu.id, child_spu, NULL );

	crMothershipDisconnect( conn );
}
