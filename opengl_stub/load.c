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
#include "stub.h"
#include <stdlib.h>
#include <signal.h>


SPUDispatchTable glim;
Stub stub;


static void stubInitNativeDispatch( void )
{
#define MAX_FUNCS 1000
	SPUNamedFunctionTable gl_funcs[MAX_FUNCS];
	int numFuncs;

	numFuncs = crLoadOpenGL( &stub.wsInterface, gl_funcs );

	stub.haveNativeOpenGL = (numFuncs > 0);

	/* XXX call this after context binding */
	numFuncs += crLoadOpenGLExtensions( &stub.wsInterface, gl_funcs + numFuncs );

	CRASSERT(numFuncs < MAX_FUNCS);

	crSPUInitDispatchTable( &stub.nativeDispatch );
	crSPUInitDispatch( &stub.nativeDispatch, gl_funcs );
	crSPUInitDispatchNops( &stub.nativeDispatch );
#undef MAX_FUNCS
}


/* Pointer to the SPU's real glClear and glViewport functions */
static ClearFunc_t origClear;
static ViewportFunc_t origViewport;


static void stubCheckWindowSize(void)
{
	unsigned int winW, winH;
	GLint c = stub.currentContext;
	if (c < 0)
		return;
	stubGetWindowSize( &(stub.Context[c]), &winW, &winH );
	if (winW != stub.spuWindowWidth || winH != stub.spuWindowHeight) {
		stub.spuDispatch.WindowSize( stub.spuWindow, winW, winH );
		stub.spuWindowWidth = winW;
		stub.spuWindowHeight = winH;
	}
}


/* Override the head SPU's glClear function.
 * We're basically trapping this function so that we can poll the
 * application window size at a regular interval.
 */
static void SPU_APIENTRY trapClear(GLbitfield mask)
{
	stubCheckWindowSize();
	/* call the original SPU glClear function */
	origClear(mask);
}

/*
 * As above, but for glViewport.  Most apps call glViewport before
 * glClear when a window is resized.
 */
static void SPU_APIENTRY trapViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
	stubCheckWindowSize();
	/* call the original SPU glViewport function */
	origViewport(x, y, w, h);
}


/*
 * Use the GL function pointers in <spu> to initialize the static glim
 * dispatch table.
 */
static void stubInitSPUDispatch(SPU *spu)
{
	crSPUInitDispatchTable( &stub.spuDispatch );
	crSPUCopyDispatchTable( &stub.spuDispatch, &(spu->dispatch_table) );

	if (stub.trackWindowSize) {
		/* patch-in special glClear/Viewport function to track window sizing */
		origClear = stub.spuDispatch.Clear;
		origViewport = stub.spuDispatch.Viewport;
		stub.spuDispatch.Clear = trapClear;
		stub.spuDispatch.Viewport = trapViewport;
	}

	crSPUCopyDispatchTable( &glim, &stub.spuDispatch );
}


/*
 * This is called by the SIGTERM signal handler when we exit.
 * We call the all the SPU's cleanup functions.
 */
static void stubCleanup(int signo)
{
	SPU *the_spu = stub.spu;

	while (1) {
		if (the_spu && the_spu->cleanup) {
			printf("Cleaning up SPU %s\n",the_spu->name);
			the_spu->cleanup();
		} else 
			break;
		the_spu = the_spu->superSPU;
	}

	exit(0);
}


/*
 * Init variables in the stub structure, install signal handler.
 */
static void stubInitVars(void)
{
#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&stub.mutex);
#endif

	/* At the very least we want CR_RGB_BIT. */
	stub.desiredVisual = CR_INVALID_VISUAL_BIT;
	stub.haveNativeOpenGL = GL_FALSE;
	stub.spu = NULL;
	stub.appDrawCursor = 0;
	stub.minChromiumWindowWidth = 0;
	stub.minChromiumWindowHeight = 0;
	stub.matchWindowTitle = NULL;
	stub.threadSafe = GL_FALSE;
	stub.spuWindowWidth = 0;
	stub.spuWindowHeight = 0;
	stub.trackWindowSize = 0;

	signal(SIGTERM, stubCleanup);
}


void StubInit(void)
{
	/* Here is where we contact the mothership to find out what we're supposed
	 * to  be doing.  Networking code in a DLL initializer.  I sure hope this 
	 * works :) 
	 * 
	 * HOW can I pass the mothership address to this if I already know it?
	 */
	
	CRConnection *conn = NULL;
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
	
	stubInitVars();

	/* this is set by the app_faker! */
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
		crWarning( "Couldn't connect to the mothership -- I have no idea what to do!" ); 
		crWarning( "For the purposes of this demonstration, I'm loading the RENDER SPU!" );
		crStrcpy( response, "1 0 render" );
	}
	else
	{
		crMothershipIdentifyOpenGL( conn, response, app_id );
	}
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

	if (conn && crMothershipGetParam( conn, "show_cursor", response ) ) {
		sscanf( response, "%d", &stub.appDrawCursor );
	}

	if (conn && crMothershipGetParam( conn, "minimum_window_size", response )
		&& response[0]) {
		int w, h;
		sscanf( response, "[%d, %d]", &w, &h );
		crDebug( "minimum_window_size: %d x %d", w, h );
		stub.minChromiumWindowWidth = w;
		stub.minChromiumWindowHeight = h;
	}

	if (conn && crMothershipGetParam( conn, "match_window_title", response )
		&& response[0]) {
		crDebug("match_window_title: %s\n", response );
		stub.matchWindowTitle = crStrdup( response );
	}

	if (conn && crMothershipGetParam( conn, "track_window_size", response ) ) {
		sscanf( response, "%d", &stub.trackWindowSize );
	}

	if (conn && crMothershipGetSPUDir( conn, response ))
	{
		spu_dir = response;
	}
	else
	{
		spu_dir = NULL;
	}

	if (conn)
	{
		crMothershipDisconnect( conn );
	}

	stub.spu = crSPULoadChain( num_spus, spu_ids, spu_names, spu_dir, NULL );

	crFree( spuchain );
	crFree( spu_ids );
	crFree( spu_names );

	crSPUInitDispatchTable( &glim );

	/* This is unlikely to change -- We still want to initialize our dispatch 
	 * table with the functions of the first SPU in the chain. */
	stubInitSPUDispatch( stub.spu );

	/* Load pointers to native OpenGL functions into stub.nativeDispatch */
	stubInitNativeDispatch();
}



/* Sigh -- we can't do initialization at load time, since Windows forbids 
 * the loading of other libraries from DLLMain. */

#ifdef LINUX
/* GCC crap 
 *void (*stub_init_ptr)(void) __attribute__((section(".ctors"))) = __stubInit; */
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Windows crap */
BOOL WINAPI DllMain( HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved )
{
	(void) lpvReserved;
	(void) instance;
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		/*__stubInit(); 
		 *DebugBreak(); 
		 *printf ("!!!!!!!!!!!!!!!!!Process attach!\n"); */
	}
	if (fdwReason == DLL_THREAD_ATTACH)
	{
		/*__stubInit(); 
		 *DebugBreak(); 
		 *printf ("!!!!!!!!!!!!!!!!!Thread attach!\n"); */
	}
	return TRUE;
}
#endif
