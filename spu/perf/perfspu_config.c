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
	perf_spu.log_filename = NULL;
	perf_spu.frame_counter = 0;
	perf_spu.clear_counter = 0;
	perf_spu.total_frames = 0;
	perf_spu.mothership_log = 0;
	perf_spu.timer_event = 0.0f;
	crMemset(&perf_spu.framestats, 0, sizeof(PerfData));
	crMemset(&perf_spu.old_framestats, 0, sizeof(PerfData));
	crMemset(&perf_spu.timerstats, 0, sizeof(PerfData));
	crMemset(&perf_spu.old_timerstats, 0, sizeof(PerfData));
}

void set_log_file( void *foo, const char *response )
{
   char filename[8096];
   char ffilename[8096];
   char hostname[100];
   char *tmp;

   if (crStrcmp( response, "stderr" ) == 0) {
      perf_spu.log_file = stderr;
   } 
   else if (crStrcmp( response, "stdout" ) == 0) {
      perf_spu.log_file = stdout;
   }
   else {
      strncpy( filename, response, strlen( filename ));

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

      perf_spu.log_filename = (char *) crAlloc( crStrlen(ffilename) + 1 );
      strcpy(perf_spu.log_filename, ffilename);
   }
}

void set_token( void *foo, const char *response )
{
   strncpy(perf_spu.token, response, strlen(perf_spu.token));
}

void set_dump_on_swap_count( void *foo, const char *response )
{
   sscanf( response, "%d", &(perf_spu.dump_on_swap_count) );
}

void set_separator( void *foo, const char *response )
{
   strncpy(perf_spu.separator, response, strlen(perf_spu.separator));
}

void set_mothership_log( void *foo, const char *response )
{
   sscanf( response, "%d", &(perf_spu.mothership_log) );
}

void set_dump_on_flush( void *foo, const char *response )
{
   sscanf( response, "%d", &(perf_spu.dump_on_flush) );
}

void set_dump_on_finish( void *foo, const char *response )
{
   sscanf( response, "%d", &(perf_spu.dump_on_finish) );
}

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions perfSPUOptions[] = {

   { "perf_log_file", CR_STRING, 1, "stderr", NULL, NULL, 
     "Log file name (or stdout,stderr)", (SPUOptionCB)set_log_file },

   { "perf_set_token", CR_STRING, 1, "\t", NULL, NULL, 
     "Filter token", (SPUOptionCB)set_token },

   { "perf_set_log_separator", CR_STRING, 1, "\t", NULL, NULL, 
     "Log file separator", (SPUOptionCB)set_separator },

   { "perf_set_dump_on_swap_count", CR_INT, 1, "0", "0", "99999",
     "Dump statistics on SwapBuffers count", (SPUOptionCB)set_dump_on_swap_count },

   { "perf_set_mothership_log", CR_BOOL, 1, "0", "0", "1",
     "Log to Mothership", (SPUOptionCB)set_mothership_log },

   { "perf_set_dump_on_flush", CR_BOOL, 1, "0", "0", "1",
     "Dump statistics on glFlush", (SPUOptionCB)set_dump_on_flush },

   { "perf_set_dump_on_finish", CR_BOOL, 1, "0", "0", "1",
     "Dump statistics on glFinish", (SPUOptionCB)set_dump_on_finish },

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
