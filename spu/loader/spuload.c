#include "cr_mem.h"
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
#define DLL_SUFFIX ".so"
#define DLL_PREFIX "lib"
#endif

extern void __buildDispatch( SPU *spu );

static char *__findDLL( char *name )
{
	static char path[8092];
	
	char *dir = getenv( "SPU_DIR" );
	if (!dir)
		dir = ".";
	sprintf ( path, "%s/%s%s%s", dir, DLL_PREFIX, name, DLL_SUFFIX );
	return path;
}

// Load a single SPU from disk and initialize it.  Is there any reason
// to export this from the SPU loader library?

SPU * crSPULoad( SPU *child, int id, char *name )
{
	SPU *the_spu;
	char *path;

	CRASSERT( name != NULL );

	the_spu = crAlloc( sizeof( *the_spu ) );
	path = __findDLL( name );
	the_spu->dll = crDLLOpen( path );
	the_spu->entry_point = 
		(SPULoadFunction) crDLLGetNoError( the_spu->dll, SPU_ENTRY_POINT_NAME );
	if (!the_spu->entry_point)
	{
		crError( "Couldn't load the SPU entry point \"%s\" from SPU \"%s\"!", 
				SPU_ENTRY_POINT_NAME, name );
	}

	if (!the_spu->entry_point( &(the_spu->name), &(the_spu->super_name), 
				&(the_spu->init), &(the_spu->self), 
				&(the_spu->cleanup) ) )
	{
		crError( "I found the SPU \"%s\", but loading it failed!", name );
	}
	if (crStrcmp(the_spu->name,"error"))
	{
		if (the_spu->super_name == NULL)
		{
			the_spu->super_name = "errorspu";
		}
		the_spu->superSPU = crSPULoad( child, id, the_spu->super_name );
	}
	else
	{
		the_spu->superSPU = NULL;
	}
	the_spu->function_table = the_spu->init( id, child, the_spu->superSPU, 0, 1 );
	__buildDispatch( the_spu );
	the_spu->self( &(the_spu->dispatch_table) );

	return the_spu;
}

// Load the entire chain of SPUs and initialize all of them.
// This function returns the first one in the chain

SPU * crSPULoadChain( int count, int *ids, char **names )
{
	int i;
	SPU *spu = NULL;
	CRASSERT( count > 0 );

	for (i = count-1 ; i >= 0 ; i--)
	{
		int spu_id = ids[i];
		char *spu_name = names[i];
		
		// This call passes the previous version of spu, which is the SPU's
		// "child" in this chain.

		spu = crSPULoad( spu, spu_id, spu_name );
	}
	return spu;
}
