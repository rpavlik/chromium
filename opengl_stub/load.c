/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_mothership.h"
#include "cr_environment.h"

SPU *stub_spu = NULL;
extern void FakerInit( SPU *fns );

void StubInit(void)
{
	// Here is where we contact the mothership to find out what we're supposed to
	// be doing.  Networking code in a DLL initializer.  I sure hope this
	// works :)
	//
	// HOW can I pass the mothership address to this if I already know it?
	
	CRConnection *conn;
	char response[1024];
	char **spuchain;
	int num_spus;
	int *spu_ids;
	char **spu_names;
	char *spu_dir;
	char * app_id;
	int i;

	static int stub_initialized = 0;
	if (stub_initialized)
		return;
	stub_initialized = 1;
	
	// this is set by the app_faker!
	app_id = crGetenv( "CR_APPLICATION_ID_NUMBER" );

	if (!app_id)
	{
		crWarning( "the OpenGL faker was loaded without crappfaker!\n"
							 "Defaulting to an application id of -1!\n"
							 "This won't work if you're debugging a parallel application!\n"
							 "In this case, set the CR_APPLICATION_ID_NUMBER environment\n"
							 "variable to the right thing (see opengl_stub/load.c)" );
		app_id = "-1";
	}
	conn = crMothershipConnect( );
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" ); 
	}
	crMothershipIdentifyOpenGL( conn, response, app_id );
	crDebug( "response = \"%s\"", response );
	spuchain = crStrSplit( response, " " );
	num_spus = crStrToInt( spuchain[0] );
	spu_ids = (int *) crAlloc( num_spus * sizeof( *spu_ids ) );
	spu_names = (char **) crAlloc( num_spus * sizeof( *spu_names ) );
	for (i = 0 ; i < num_spus ; i++)
	{
		spu_ids[i] = crStrToInt( spuchain[2*i+1] );
		spu_names[i] = crStrdup( spuchain[2*i+2] );
		crDebug( "SPU %d/%d: (%d) \"%s\"", i+1, num_spus, spu_ids[i], spu_names[i] );
	}

	if (crMothershipGetSPUDir( conn, response ))
	{
		spu_dir = response;
	}
	else
	{
		spu_dir = NULL;
	}

	crMothershipDisconnect( conn );

	stub_spu = crSPULoadChain( num_spus, spu_ids, spu_names, spu_dir );

	crFree( spuchain );
	crFree( spu_ids );
	crFree( spu_names );


	// This is unlikely to change -- We still want to initialize our dispatch
	// table with the functions of the first SPU in the chain.

	FakerInit( stub_spu );
}

// Sigh -- we can't do initialization at load time, since Windows forbids
// the loading of other libraries from DLLMain.

#ifdef LINUX
// GCC crap
//void (*stub_init_ptr)(void) __attribute__((section(".ctors"))) = __stubInit;
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Windows crap
BOOL WINAPI DllMain( HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved )
{
	(void) lpvReserved;
	(void) instance;
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		//__stubInit();
		//DebugBreak();
		//printf ("!!!!!!!!!!!!!!!!!Process attach!\n");
	}
	if (fdwReason == DLL_THREAD_ATTACH)
	{
		//__stubInit();
		//DebugBreak();
		//printf ("!!!!!!!!!!!!!!!!!Thread attach!\n");
	}
	return TRUE;
}
#endif
