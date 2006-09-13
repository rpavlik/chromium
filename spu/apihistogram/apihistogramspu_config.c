/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "apihistogramspu.h"

#include "cr_mothership.h"
#include "cr_string.h"


/**
 * Set default options for SPU
 */
static void setDefaults( void )
{
	apihistogram_spu.fp = stderr;
}



static void set_log_file( void *foo, const char *response )
{
	if (crStrcmp( response, "stderr" ) == 0)
	{
		apihistogram_spu.fp = stderr;
	} 
	else if (crStrcmp( response, "stdout" ) == 0)
	{
		apihistogram_spu.fp = stdout;
	}
	else if (*response)
	{
		apihistogram_spu.fp = fopen( response, "w" );
		if (apihistogram_spu.fp == NULL)
		{
			crError( "API Histogram SPU: Couldn't open log file %s", response );
		}
	}
	else
	{
		apihistogram_spu.fp = stderr;
	}
}


/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
SPUOptions apihistogramSPUOptions[] = {
	{ "log_file", CR_STRING, 1, "stderr", NULL, NULL, 
	  "Log file name (or stdout, stderr)", (SPUOptionCB)set_log_file },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


/**
 * Gather the config info for the SPU
 */
void apihistogramspuGatherConfiguration( void )
{
	CRConnection *conn;

	setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( &apihistogram_spu, apihistogramSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, apihistogram_spu.id );

	crSPUGetMothershipParams( conn, &apihistogram_spu, apihistogramSPUOptions );

	crMothershipDisconnect( conn );
}
