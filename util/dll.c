#include "cr_mem.h"
#include "cr_error.h"
#include "cr_dll.h"

#if defined(IRIX) || defined(IRIX64) || defined(Linux)
#include <dlfcn.h>
#endif

CRDLL *crDLLOpen( const char *dllname )
{
	CRDLL *dll;
	
	dll = (CRDLL *) crAlloc( sizeof( CRDLL ) );
	dll->name = (char *) crAlloc( strlen(dllname) + 1 );
	strcpy( dll->name, dllname );

#if defined(WINDOWS)
	dll->hinstLib = LoadLibrary( dllname );
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
	dll->hinstLib = dlopen( dllname, RTLD_NOW /* RTLD_LAZY */ );
#else
#error DSO
#endif

	if (!dll->hinstLib)
	{
		crError( "DLL Loader couldn't find %s", dllname );
	}
	return dll;
}

CRDLLFunc crDLLGetNoError( CRDLL *dll, const char *symname )
{
#if defined(WINDOWS)
	return (CRDLLFunc) GetProcAddress( dll->hinstLib, symname );
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
	return (CRDLLFunc) dlsym( dll->hinstLib, symname );
#else
#error CR DLL ARCHITETECTURE
#endif
}

CRDLLFunc crDLLGet( CRDLL *dll, const char *symname )
{
	CRDLLFunc data = crDLLGetNoError( dll, symname );
	if (!data)
	{
		crError( "Couldn't get symbol \"%s\" in \"%s\".  Are you "
						   "sure there isn't some C++ mangling messing you "
						   "up?", symname, dll->name );
	}
	return data;
}

void crDLLClose( CRDLL *dll )
{
#if defined(WINDOWS)
	FreeLibrary( dll->hinstLib );
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
	dlclose( dll->hinstLib );
#else
#error DSO
#endif
	crFree( dll->name );
	crFree( dll );
}
