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
#include "cr_net.h"
#include "cr_environment.h"
#include "cr_process.h"
#include "cr_rand.h"
#include "stub.h"
#include <stdlib.h>
#include <signal.h>


/*
 * If you change this, see the comments in tilesortspu_context.c
 */
#define MAGIC_CONTEXT_BASE 500

#define CONFIG_LOOKUP_FILE ".crconfigs"

#ifdef WINDOWS
#define PYTHON_EXE "python.exe"
#else
#define PYTHON_EXE "python"
#endif


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
	int winX, winY;
	unsigned int winW, winH;
	WindowInfo *window;

	if (!stub.currentContext)
		return;

	window = stub.currentContext->currentDrawable;

	stubGetWindowGeometry( window, &winX, &winY, &winW, &winH );

	if (winW && winH) {
		if (stub.trackWindowSize) {
			if (winW != window->width || winH != window->height) {
				if (window->type == CHROMIUM)
					stub.spuDispatch.WindowSize( window->spuWindow, winW, winH );
				window->width = winW;
				window->height = winH;
			}
		}
		if (stub.trackWindowPos) {
			if (winX != window->x || winY != window->y) {
				if (window->type == CHROMIUM)
					stub.spuDispatch.WindowPosition( window->spuWindow, winX, winY );
				window->x = winX;
				window->y = winY;
			}
		}
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

	if (stub.trackWindowSize || stub.trackWindowPos) {
		/* patch-in special glClear/Viewport function to track window sizing */
		origClear = stub.spuDispatch.Clear;
		origViewport = stub.spuDispatch.Viewport;
		stub.spuDispatch.Clear = trapClear;
		stub.spuDispatch.Viewport = trapViewport;
	}

	crSPUCopyDispatchTable( &glim, &stub.spuDispatch );
}


/*
 * This is called when we exit.
 * We call the all the SPU's cleanup functions.
 */
static void stubSPUTearDown(void)
{
	SPU *the_spu = stub.spu;
	SPU *next_spu;

	while (1) {
		if (the_spu && the_spu->cleanup) {
			crWarning("Cleaning up SPU %s",the_spu->name);
			the_spu->cleanup();
		} else 
			break;
		the_spu = the_spu->superSPU;
		crDLLClose(the_spu->dll);
		crFree(the_spu);
		the_spu = next_spu;
	}
	stub.spu = NULL;

	crUnloadOpenGL();
}

static void stubExitHandler(void)
{
	/* kill the mothership we spawned earlier */
	if (stub.mothershipPID)
		crKill(stub.mothershipPID);

	stubSPUTearDown();
}

/*
 * Called when we receive a SIGTERM signal.
 */
static void stubSignalHandler(int signo)
{
	stubSPUTearDown();

	exit(0);  /* this causes stubExitHandler() to be called */
}


/*
 * Init variables in the stub structure, install signal handler.
 */
static void stubInitVars(void)
{
	WindowInfo *defaultWin;

#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&stub.mutex);
#endif

	/* At the very least we want CR_RGB_BIT. */
	stub.desiredVisual = CR_RGB_BIT;
	stub.haveNativeOpenGL = GL_FALSE;
	stub.spu = NULL;
	stub.appDrawCursor = 0;
	stub.minChromiumWindowWidth = 0;
	stub.minChromiumWindowHeight = 0;
	stub.maxChromiumWindowWidth = 0;
	stub.maxChromiumWindowHeight = 0;
	stub.matchWindowTitle = NULL;
	stub.threadSafe = GL_FALSE;
	stub.trackWindowSize = 0;
	stub.trackWindowPos = 0;
	stub.mothershipPID = 0;

	stub.freeContextNumber = MAGIC_CONTEXT_BASE;
	stub.contextTable = crAllocHashtable();
	stub.currentContext = NULL;

	stub.windowTable = crAllocHashtable();

	defaultWin = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	defaultWin->type = CHROMIUM;
	defaultWin->spuWindow = 0;  /* window 0 always exists */
	crHashtableAdd(stub.windowTable, 0, defaultWin);

#if 1
	atexit(stubExitHandler);
	signal(SIGTERM, stubSignalHandler);
	signal(SIGINT, stubSignalHandler);
#ifndef WINDOWS
	signal(SIGPIPE, SIG_IGN); /* the networking code should catch this */
#endif
#else
	(void) stubExitHandler;
	(void) stubSignalHandler;
#endif
}


static char **LookupMothershipConfig(const char *procName)
{
	const int procNameLen = crStrlen(procName);
	FILE *f;
	char *home, configPath[1000];

	/* first, check if the CR_CONFIG env var is set */
	{
		const char *conf = crGetenv("CR_CONFIG");
		if (conf)
			return crStrSplit(conf, " ");
	}

	/* second, look up config name from config file */
	home = crGetenv("HOME");
	if (home)
		sprintf(configPath, "%s/%s", home, CONFIG_LOOKUP_FILE);
	else
		crStrcpy(configPath, CONFIG_LOOKUP_FILE); /* from current dir */

	f = fopen(configPath, "r");
	if (!f) {
		return NULL;
	}

	while (!feof(f)) {
		char line[1000];
		char **args;
		fgets(line, 999, f);
		line[crStrlen(line) - 1] = 0; /* remove trailing newline */
		if (crStrncmp(line, procName, procNameLen) == 0 &&
			(line[procNameLen] == ' ' || line[procNameLen] == '\t')) 
		{
			crWarning("Using Chromium configuration for %s from %s",
								procName, configPath);
			args = crStrSplit(line + procNameLen + 1, " ");
			return args;
		}
	}
	fclose(f);
	return NULL;
}


void stubInit(void)
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
	/* Quadrics defaults */
	int my_rank = 0;
	int low_context  = CR_QUADRICS_DEFAULT_LOW_CONTEXT;
	int high_context = CR_QUADRICS_DEFAULT_HIGH_CONTEXT;
	char *low_node  = "none";
	char *high_node = "none";
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

	/* contact mothership and get my spu chain */
	conn = crMothershipConnect( );
	if (conn)
	{
		/* response will be the client's SPU chain */
		crMothershipIdentifyOpenGL( conn, response, app_id );
	}
	else
	{
		/* No mothership running.  Try spawning it ourselves with a config
		 * file that we'll get from an env var or the ~/.crconfig file.
		 */
		char procName[1000], currentDir[1000], **args;

		crGetProcName(procName, 999);
		crGetCurrentDir(currentDir, 999);

		args = LookupMothershipConfig(procName);
		if (!args)
		{
			/* try default */
			args = LookupMothershipConfig("*");
		}

		if (args) {
			/* Build the argument vector and try spawning the mothership! */
			int mothershipPort = 10000;
			char *argv[1000];

			argv[0] = PYTHON_EXE;
			for (i = 0; args[i]; i++)
			{
				if (crStrcmp(args[i], "%p") == 0)
					argv[1 + i] = procName;
				else if (crStrcmp(args[i], "%d") == 0)
					argv[1 + i] = currentDir;
				else if (crStrcmp(args[i], "%m") == 0) {
					/* generate random port for mothership */
					char portString[10];
					crRandAutoSeed();
					mothershipPort = crRandInt(10001, 10100);
					sprintf(portString, "%d", mothershipPort);
					argv[1 + i] = portString;
				}
				else
					argv[1 + i] = args[i];
			}
			i++;
			argv[i++] = NULL;

			if (mothershipPort != 10000) {
				char localHost[1000], mothershipStr[1010];
				crGetHostname(localHost, 1000);
				sprintf(mothershipStr, "%s:%d", localHost, mothershipPort);
				crSetenv("CRMOTHERSHIP", mothershipStr);
			}

			crDebug("Spawning mothership with argv:");
			for (i = 0; argv[i]; i++) {
					crDebug("argv[%d] = '%s'", i, argv[i]);
			}

			stub.mothershipPID = crSpawn(PYTHON_EXE, (const char **) argv );
			crSleep(1);

			crFreeStrings(args);
			conn = crMothershipConnect( );
			if (conn)
			{
				crMothershipIdentifyOpenGL( conn, response, app_id );
			}
		}
	}

	if (!conn)
	{
		crWarning( "Couldn't connect to the mothership -- I have no idea what to do!(1)" ); 
		crWarning( "For the purposes of this demonstration, I'm loading the RENDER SPU!" );
		crStrcpy( response, "1 0 render" );
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

	if (conn && crMothershipGetFakerParam( conn, response, "show_cursor" ) ) {
		sscanf( response, "%d", &stub.appDrawCursor );
	}

	if (conn && crMothershipGetFakerParam( conn, response, "minimum_window_size" )
		&& response[0]) {
		int w, h;
		sscanf( response, "[%d, %d]", &w, &h );
		crDebug( "minimum_window_size: %d x %d", w, h );
		stub.minChromiumWindowWidth = w;
		stub.minChromiumWindowHeight = h;
	}

	if (conn && crMothershipGetFakerParam( conn, response, "maximum_window_size" )
		&& response[0]) {
		int w, h;
		sscanf( response, "[%d, %d]", &w, &h );
		crDebug( "maximum_window_size: %d x %d", w, h );
		stub.maxChromiumWindowWidth = w;
		stub.maxChromiumWindowHeight = h;
	}

	if (conn && crMothershipGetFakerParam( conn, response, "match_window_title" )
		&& response[0]) {
		crDebug("match_window_title: %s\n", response );
		stub.matchWindowTitle = crStrdup( response );
	}

	if (conn && crMothershipGetFakerParam( conn, response, "track_window_size" ) ) {
		sscanf( response, "%d", &stub.trackWindowSize );
	}

	if (conn && crMothershipGetFakerParam( conn, response, "track_window_position" ) ) {
		sscanf( response, "%d", &stub.trackWindowPos );
	}

	if (conn && crMothershipGetFakerParam( conn, response, "match_window_count" )
		&& response[0]) {
		int c;
		sscanf( response, "%d", &c );
		crDebug( "match_window_count: %d", c);
		stub.matchChromiumWindowCount = c;
	}

#if 0
	if (conn && crMothershipGetSPUDir( conn, response ))
#else
	if (conn && crMothershipGetFakerParam( conn, response, "spu_dir" ) && crStrlen(response) > 0)
#endif
	{
		spu_dir = crStrdup(response);
	}
	else
	{
		spu_dir = NULL;
	}

	if (conn && crMothershipGetRank( conn, response ))
	{
		my_rank = crStrToInt( response );
	}
	crNetSetRank( my_rank );

	if (conn && crMothershipGetParam( conn, "low_context", response ))
	{
		low_context = crStrToInt( response );
	}

	if (conn && crMothershipGetParam( conn, "high_context", response ))
	{
		high_context = crStrToInt( response );
	}
	crNetSetContextRange( low_context, high_context );

	if (conn && crMothershipGetParam( conn, "low_node", response ))
	{
		low_node = crStrdup( response );
	}

	if (conn && crMothershipGetParam( conn, "high_node", response ))
	{
		high_node = crStrdup( response );
	}
	crNetSetNodeRange( low_node, high_node );

	if (conn)
	{
		crMothershipDisconnect( conn );
	}

	/*
	 * NOTICE:
	 * if you add new app node config options, please add them to the
	 * configuration options list in mothership/tools/crtypes.py
	 */

	stub.spu = crSPULoadChain( num_spus, spu_ids, spu_names, spu_dir, NULL );

	crFree( spuchain );
	crFree( spu_ids );
	for (i = 0; i < num_spus; ++i)
		crFree(spu_names[i]);
	crFree(spu_dir);
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
