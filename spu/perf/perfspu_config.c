/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "perfspu.h"

#include <stdio.h>
#include <string.h>
#ifndef WINDOWS
#include <unistd.h>
#endif

static void __setDefaults( void )
{
	perf_spu.log_file = stderr;
	perf_spu.frame_counter = 0;
	perf_spu.timer_event = 0.0f;
	perf_spu.old_draw_pixels = 0;
	perf_spu.old_read_pixels = 0;
	perf_spu.draw_pixels = 0;
	perf_spu.read_pixels = 0;
	crMemset(&perf_spu.framestats, 0, sizeof(PerfPrim));
	crMemset(&perf_spu.old_framestats, 0, sizeof(PerfPrim));
	crMemset(&perf_spu.timerstats, 0, sizeof(PerfPrim));
	crMemset(&perf_spu.old_timerstats, 0, sizeof(PerfPrim));
}

void set_log_file( void *foo, const char *response )
{
   char filename[8096];
   char ffilename[8096];
   char hostname[100];
   char *tmp;

   strncpy( filename, response, strlen( response ));

   /* break up the string with our specialized tokens.
    *
    * %H - hostname
    * .....
    * .... more to come.....
    */
   while ( 1 ) {

	tmp = strpbrk(filename, "%");
	if (!tmp) break;

	switch (tmp[1]) {

		case 'H':
			crGetHostname(hostname, sizeof(hostname));
			strncpy(tmp, "%s", 2);
			sprintf(ffilename, filename, hostname);
			break;

		default:
			crError("Unknown perf SPU tag\n");
			break;
	
	}
   	strcpy(filename, ffilename);
   }

   if (crStrcmp( ffilename, "stderr" ) == 0) {
      perf_spu.log_file = stderr;
   } 
   else if (crStrcmp( ffilename, "stdout" ) == 0) {
      perf_spu.log_file = stdout;
   }
   else if (ffilename) {
      perf_spu.log_file = fopen( ffilename, "w" );
      if (perf_spu.log_file == NULL) {
	 crError( "Couldn't open perf SPU log file %s", ffilename );
      }
   }
   else
      perf_spu.log_file = stderr;
}

void set_token( void *foo, const char *response )
{
   strncpy(perf_spu.token, response, strlen(response));
}

void set_swapdumpcount( void *foo, const char *response )
{
   sscanf( response, "%d", &(perf_spu.swapdumpcount) );
}

void set_separator( void *foo, const char *response )
{
   strncpy(perf_spu.separator, response, strlen(response));
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions perfSPUOptions[] = {

   { "perf_log_file", CR_STRING, 1, "stderr", NULL, NULL, 
     "Performance SPU Log file name (or stdout,stderr)", (SPUOptionCB)set_log_file },

   { "perf_set_token", CR_STRING, 1, "", NULL, NULL, 
     "Performance SPU filter token", (SPUOptionCB)set_token },

   { "perf_set_swapdumpcount", CR_INT, 1, "0", "0", "99999",
     "Performance SPU Dump Statistics on SwapBuffers Count", (SPUOptionCB)set_swapdumpcount },

   { "perf_set_log_separator", CR_STRING, 1, "\t", NULL, NULL, 
     "Performance SPU Log File separator", (SPUOptionCB)set_separator },

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },

};


void perfspuGatherConfiguration( void )
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
	crMothershipIdentifySPU( conn, perf_spu.id );

	crSPUGetMothershipParams( conn, &perf_spu, perfSPUOptions );

	(void) response;

	crMothershipDisconnect( conn );
}
