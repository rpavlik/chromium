/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_mothership.h"
#include "cr_url.h"
#include "renderspu.h"
#include <stdio.h>

static SPUNamedFunctionTable _cr_render_table[1000];

SPUFunctions render_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_render_table /* THE ACTUAL FUNCTIONS */
};

RenderSPU render_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _RenderTSD;
#endif

static void swapsyncConnect(void)
{
	char hostname[4096], protocol[4096];
	unsigned short port;

	crNetInit(NULL, NULL);

	if (!crParseURL( render_spu.swap_master_url, protocol, hostname, 
					&port, 9876))
		crError( "Bad URL: %s", render_spu.swap_master_url );

	if (render_spu.is_swap_master)
	{
		int a;

		render_spu.swap_conns = (CRConnection **)crAlloc(
						render_spu.num_swap_clients*sizeof(CRConnection *));
		for (a=0; a<render_spu.num_swap_clients; a++)
		{
			render_spu.swap_conns[a] = crNetAcceptClient( protocol, hostname, port,
														render_spu.swap_mtu, 1);
		}
	}
	else
	{
		render_spu.swap_conns = (CRConnection **)crAlloc(sizeof(CRConnection *));

		render_spu.swap_conns[0] = crNetConnectToServer(render_spu.swap_master_url,
									port, render_spu.swap_mtu, 1);
		if (!render_spu.swap_conns[0])
			crError("Failed connection");
	}
}


static SPUFunctions *
renderSPUInit( int id, SPU *child, SPU *self,
               unsigned int context_id, unsigned int num_contexts )
{
	int numFuncs, numSpecial;
	GLint defaultWin, defaultCtx;
	/* Don't ask for ALPHA, if we don't have it, we fail immediately */
	GLuint visualBits = CR_RGB_BIT;
	WindowInfo *windowInfo;

	(void) child;
	(void) context_id;
	(void) num_contexts;

	self->privatePtr = (void *) &render_spu;

#ifdef CHROMIUM_THREADSAFE
	crDebug("Render SPU: thread-safe");
#endif

	crMemZero(&render_spu, sizeof(render_spu));

	render_spu.id = id;
	renderspuGatherConfiguration(&render_spu);
	if (render_spu.swap_master_url)
		swapsyncConnect();


	/* Get our special functions. */
	numSpecial = renderspuCreateFunctions( _cr_render_table );

	/* Get the OpenGL functions. */
	numFuncs = crLoadOpenGL( &render_spu.ws, _cr_render_table + numSpecial );
	if (numFuncs == 0) {
		crError("The render SPU was unable to load the native OpenGL library");
		return NULL;
	}

	numFuncs += numSpecial;

	render_spu.window_id = 0;
	render_spu.context_id = 0;
	render_spu.contextTable = crAllocHashtable();
	render_spu.windowTable = crAllocHashtable();

	/*
	 * Create the default window and context.  Their indexes are zero and
	 * a client can use them without calling CreateContext or WindowCreate.
	 */
	defaultWin = renderspuWindowCreate( NULL, visualBits );
	if (defaultWin < 0) {
		/* If we failed to get a default single buffered visual,
		 * let's try a double buffered one
		 */
		crDebug( "WindowCreate returned %d, trying CR_DOUBLE_BIT", defaultWin );
		visualBits = CR_RGB_BIT | CR_DOUBLE_BIT;
		defaultWin = renderspuWindowCreate( NULL, visualBits );
	}
	crDebug( "WindowCreate returned %d", defaultWin );
	CRASSERT(defaultWin == 0);

	defaultCtx = renderspuCreateContext( NULL, visualBits );
	CRASSERT(defaultCtx == 0);

	renderspuMakeCurrent( defaultWin, 0, defaultCtx );

	windowInfo = (WindowInfo *) crHashtableSearch(render_spu.windowTable, 0);
	CRASSERT(windowInfo);
	windowInfo->mapPending = GL_TRUE;

	/*
	 * Get the OpenGL extension functions.
	 * SIGH -- we have to wait until the very bitter end to load the 
	 * extensions, because the context has to be bound before
	 * wglGetProcAddress will work correctly.  No such issue with GLX though.
	 */
	numFuncs += crLoadOpenGLExtensions( &render_spu.ws, _cr_render_table + numFuncs );
	CRASSERT(numFuncs < 1000);

	render_spu.barrierHash = crAllocHashtable();

	render_spu.cursorX = 0;
	render_spu.cursorY = 0;
	render_spu.use_L2 = 0;

	render_spu.gather_conns = NULL;

	return &render_functions;
}


static void renderSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(render_spu.self) );
	crSPUCopyDispatchTable( &(render_spu.self), self );

	render_spu.server = (CRServer *)(self->server);
}


static int renderSPUCleanup(void)
{
	WindowInfo *window;
	ContextInfo *context;
	unsigned int i, num_elements;

	num_elements = crHashtableNumElements( render_spu.contextTable );
	for (i = 0; i < num_elements; i++) {
		context = (ContextInfo *) crHashtableSearch(render_spu.contextTable, i);
		renderspu_SystemDestroyContext( context );
		crHashtableDelete(render_spu.contextTable, i, GL_TRUE);
	}

	num_elements = crHashtableNumElements( render_spu.windowTable );
	for (i = 0; i < num_elements; i++) {
		window = (WindowInfo *) crHashtableSearch(render_spu.windowTable, i);
		renderspu_SystemDestroyWindow( window );
		crHashtableDelete(render_spu.windowTable, i, GL_TRUE);
	}

	crFreeHashtable(render_spu.windowTable);
	crFreeHashtable(render_spu.contextTable);

	crFreeHashtable(render_spu.barrierHash);

	return 1;
}


extern SPUOptions renderSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "render";
	*super = NULL;
	*init = renderSPUInit;
	*self = renderSPUSelfDispatch;
	*cleanup = renderSPUCleanup;
	*options = renderSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_IS_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
