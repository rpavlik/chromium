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
#include "renderspu.h"
#include <stdio.h>

static SPUNamedFunctionTable render_table[1000];

SPUFunctions render_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	render_table /* THE ACTUAL FUNCTIONS */
};

RenderSPU render_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _RenderTSD;
#endif


SPUFunctions *renderSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id, unsigned int num_contexts )
{
	CRLimitsState limits[3];
	int numFuncs, numSpecial;
	GLint defaultWin, defaultCtx;
	/* Don't ask for ALPHA, if we don't have it, we fail immediately */
	const GLuint visualBits = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT | CR_STENCIL_BIT /*| CR_ALPHA_BIT*/;
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

	/* Get our special functions. */
	numSpecial = renderspuCreateFunctions( render_table );

	/* Get the OpenGL functions. */
	numFuncs = crLoadOpenGL( &render_spu.ws, render_table + numSpecial );
	if (numFuncs == 0) {
		crError("The render SPU was unable to load the native OpenGL library");
		return NULL;
	}

	numFuncs += numSpecial;

	render_spu.contextTable = crAllocHashtable();
	render_spu.windowTable = crAllocHashtable();

	/*
	 * Create the default window and context.  Their indexes are zero and
	 * a client can use them without calling CreateContext or WindowCreate.
	 */
	defaultWin = renderspuWindowCreate( NULL, visualBits );
	defaultCtx = renderspuCreateContext( NULL, visualBits );

	crDebug( "WindowCreate returned %d", defaultWin );
	CRASSERT(defaultWin == 0);
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
	numFuncs += crLoadOpenGLExtensions( &render_spu.ws, render_table + numFuncs );
	CRASSERT(numFuncs < 1000);

	/* Report OpenGL limits to the mothership.
	 * We use crSPUGetGLLimits() to query the real OpenGL limits via
	 * glGetString, glGetInteger, etc.
	 * Then, we intersect those limits with Chromium's OpenGL limits.
	 * Finally, we report the intersected limits to the mothership.
	 */
	crSPUGetGLLimits( render_table, &limits[0] );  /* OpenGL */
	crSPUInitGLLimits( &limits[1] );               /* Chromium */
	crSPUMergeGLLimits( 2, limits, &limits[2] );   /* intersection */
	crSPUReportGLLimits( &limits[2], render_spu.id );

	render_spu.barrierHash = crAllocHashtable();

	render_spu.cursorX = 0;
	render_spu.cursorY = 0;
	render_spu.use_L2 = 0;

	render_spu.gather_conns = NULL;

	return &render_functions;
}

void renderSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(render_spu.self) );
	crSPUCopyDispatchTable( &(render_spu.self), self );

	render_spu.server = (CRServer *)(self->server);
}

int renderSPUCleanup(void)
{
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
