/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_dll.h"
#include "cr_error.h"
#include "cr_spu.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef WINDOWS
#define DLL_SUFFIX ".dll"
#define DLL_PREFIX ""
#else
#ifdef AIX
#define DLL_SUFFIX ".o"
#define DLL_PREFIX "lib"
#else
#define DLL_SUFFIX ".so"
#define DLL_PREFIX "lib"
#endif
#endif


static char *__findDLL( char *name, char *dir )
{
	static char path[8092];
	
	if (!dir)
	{
		sprintf ( path, "%s%sspu%s", DLL_PREFIX, name, DLL_SUFFIX );
	}
	else
	{
		sprintf ( path, "%s/%s%sspu%s", dir, DLL_PREFIX, name, DLL_SUFFIX );
	}
	return path;
}


/* This is a cut-down version of crSPULoad() from spuload.c.  It just
 * does enough to retreive the options list from the spu, but doesn't
 * call any initialization functions from which the spu might try to
 * connect to the mothership, etc.  
 */
SPU * SPULoadLite( SPU *child, int id, char *name, char *dir, void *server )
{
	SPU *the_spu;
	char *path;

	CRASSERT( name != NULL );

	the_spu = (SPU*)crAlloc( sizeof( *the_spu ) );
	the_spu->id = id;
	path = __findDLL( name, dir );
	the_spu->dll = crDLLOpen( path );
	the_spu->entry_point = 
		(SPULoadFunction) crDLLGetNoError( the_spu->dll, SPU_ENTRY_POINT_NAME );
	if (!the_spu->entry_point)
	{
		crError( "Couldn't load the SPU entry point \"%s\" from SPU \"%s\"!", 
				SPU_ENTRY_POINT_NAME, name );
	}

	/* This basicall calls the SPU's SPULoad() function */
	if (!the_spu->entry_point( &(the_spu->name), &(the_spu->super_name), 
				   &(the_spu->init), &(the_spu->self), 
				   &(the_spu->cleanup),
				   &(the_spu->options),
				   &(the_spu->spu_flags)) )
	{
		crError( "I found the SPU \"%s\", but loading it failed!", name );
	}
	
	/* ... and that's all
	 */
	return the_spu;
}



static void print_spu_header( SPU *spu )
{
   printf("SPU %s: (packer %s, terminal %s, max-servers %s)\n", 
	  spu->name,
	  (spu->spu_flags & SPU_HAS_PACKER) ? "y" : "n",
	  (spu->spu_flags & SPU_IS_TERMINAL) ? "y" : "n",
	  ((spu->spu_flags & SPU_MAX_SERVERS_ONE) ? "one" : 
	   (spu->spu_flags & SPU_MAX_SERVERS_UNLIMITED) ? "unlimited" : 
	   "zero"));
}

static const char *type_string[] = {
   "BOOL", 
   "INT",
   "FLOAT",
   "STRING"
};

static void print_option( SPUOptions *opt )
{
   printf("\t\"%s\", %s, \"%s\", %d, \"%s\", ", 
	  opt->option, 
	  type_string[(int)opt->type],
	  opt->description,
	  opt->numValues,
	  opt->deflt);

   if (opt->min)
      printf("\"%s\", ", opt->min);
   else
      printf("NULL, ");

   if (opt->max)
      printf("\"%s\"", opt->min);
   else
      printf("NULL");
   
   printf("\n");
}

static void print_spu_footer( SPU *spu )
{
   printf("\n\n");
}



int main(int argc, char *argv[])
{
	SPU *spu;
	SPUOptions *opt;

	while (--argc) {
	   spu = SPULoadLite( NULL, 1, *++argv, NULL, NULL );

	   print_spu_header( spu );

	   for ( opt = &spu->options[0] ; opt->option ; opt++ ) 
	      print_option( opt );

	   print_spu_footer( spu );

	   /* How to unload the spu???
	    */
	}

	return 0;
}
