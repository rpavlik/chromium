#include "cr_mem.h"
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_string.h"

#if defined(IRIX) || defined(IRIX64) || defined(Linux)
#include <dlfcn.h>
#endif

CRDLL *crDLLOpen( const char *dllname )
{
	CRDLL *dll;
	char *dll_err;
	
	dll = (CRDLL *) crAlloc( sizeof( CRDLL ) );
	dll->name = crStrdup( dllname );

#if defined(WINDOWS)
	dll->hinstLib = LoadLibrary( dllname );
	dll_err = NULL;
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
	dll->hinstLib = dlopen( dllname, RTLD_NOW /* RTLD_LAZY */ );
	dll_err = dlerror();
#else
#error DSO
#endif

	if (!dll->hinstLib)
	{
		if (dll_err)
		{
			crDebug( "DLL_ERROR: %s", dll_err );
		}
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
