/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_string.h"
#include "stdio.h"

#if defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(AIX) || defined(DARWIN) || defined(SunOS) || defined(OSF1)
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
#elif defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(AIX) || defined (DARWIN) || defined(SunOS) || defined(OSF1)
	dll->hinstLib = dlopen( dllname, RTLD_LAZY );
	dll_err = (char*) dlerror();
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
#elif defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(AIX) || defined(DARWIN) || defined(SunOS) || defined(OSF1)
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
		/* Are you sure there isn't some C++ mangling messing you up? */
		crWarning( "Couldn't get symbol \"%s\" in \"%s\"", symname, dll->name );
	}
	return data;
}

void crDLLClose( CRDLL *dll )
{
	int dll_err = 0;

	if (!dll) return;

#if defined(WINDOWS)
	FreeLibrary( dll->hinstLib );
#elif defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(AIX) || defined (DARWIN) || defined(SunOS) || defined(OSF1)
	dll_err = dlclose( dll->hinstLib );
#else
#error DSO
#endif

	if (dll_err)
		crWarning("Error closing DLL %s\n",dll->name);

	crFree( dll->name );
	crFree( dll );
}
