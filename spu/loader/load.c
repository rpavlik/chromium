#include "cr_mem.h"
#include "cr_string.h"
#include "cr_dll.h"
#include "cr_error.h"
#include "cr_spu.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef WINDOWS
#define DLL_EXTENSION ".dll"
#else
#define DLL_EXTENSION ".so"
#endif

extern void __buildDispatch( SPU *spu );

static char *__findDLL( char *name )
{
	static char path[8092];
	
	char *dir = getenv( "SPU_DIR" );
	if (!dir)
		dir = ".";
	sprintf ( path, "%s/%s%s", dir, name, DLL_EXTENSION );
	return path;
}

SPU *LoadSPU( char *name )
{
	SPU *the_spu;
	char *path;

	CRASSERT( name != NULL );

	the_spu = CRAlloc( sizeof( *the_spu ) );
	path = __findDLL( name );
	the_spu->dll = CRDLLOpen( path );
	the_spu->entry_point = 
		(SPULoadFunction) CRDLLGetNoError( the_spu->dll, SPU_ENTRY_POINT_NAME );
	if (!the_spu->entry_point)
	{
		CRError( "Couldn't load the SPU entry point \"%s\" from SPU \"%s\"!", 
				SPU_ENTRY_POINT_NAME, name );
	}

	if (!the_spu->entry_point( &(the_spu->name), &(the_spu->super_name), 
				&(the_spu->init), &(the_spu->parent), 
				&(the_spu->cleanup), &(the_spu->nargs), 
				&(the_spu->args) ) )
	{
		CRError( "I found the SPU \"%s\", but loading it failed!", name );
	}
	if (CRStrcmp(the_spu->name,"error"))
	{
		if (the_spu->super_name == NULL)
		{
			the_spu->super_name = "errorspu";
		}
		the_spu->superSPU = LoadSPU( the_spu->super_name );
	}
	else
	{
		the_spu->superSPU = NULL;
	}
	the_spu->function_table = the_spu->init( NULL, 0, 0, 1, 0, NULL, NULL );
	__buildDispatch( the_spu );

	return the_spu;
}
