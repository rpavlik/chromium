/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

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

SPUFunctions *renderSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id, unsigned int num_contexts )
{
	CRLimitsState limits[3];
	int numFuncs, numSpecial;

	(void) child;
	(void) super;
	(void) context_id;
	(void) num_contexts;

	render_spu.id = id;
	renderspuGatherConfiguration();

	/* Get our special functions. */
	numSpecial = renderspuCreateFunctions( render_table );

	/* Get the OpenGL functions. */
	numFuncs = crLoadOpenGL( &render_spu.ws, render_table + numSpecial );
	if (numFuncs == 0) {
		crError("The render SPU was unable to load the native OpenGL library");
		return NULL;
	}

	/* Get pointer to the real OpenGL glClear function */
	render_spu.ClearFunc = (ClearFunc_t)
		 crSPUFindFunction(render_table + numSpecial, "Clear");
	CRASSERT(render_spu.ClearFunc);

	numFuncs += numSpecial;

	/* Need to create the window and call glX/wglMakeCurrent here */
	(void) renderspuCreateWindow( render_spu.visAttribs, GL_FALSE );
	
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

	return &render_functions;
}

void renderSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(render_spu.self) );
	crSPUCopyDispatchTable( &(render_spu.self), self );
}

int renderSPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "render";
	*super = NULL;
	*init = renderSPUInit;
	*self = renderSPUSelfDispatch;
	*cleanup = renderSPUCleanup;
	
	return 1;
}
