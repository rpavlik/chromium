/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_logo.h"
#include "renderspu.h"

#include <stdio.h>

extern SPUNamedFunctionTable render_table[];

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
	(void) child;
	(void) super;
	(void) context_id;
	(void) num_contexts;

	render_spu.id = id;
	render_spu.dispatch = NULL;
	renderspuGatherConfiguration();
	renderspuLoadSystemGL( );
	renderspuCreateWindow();
	
	/* SIGH -- we have to wait until the very bitter end to load the 
	 * extensions, because the context has to be created first. */

	renderspuLoadSystemExtensions();

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
	render_spu.dispatch = self;
	(void) raw_bytes;
	/*
	render_spu.dispatch->ClearColor( 1, 0, 0, 1 );
	render_spu.dispatch->Clear( GL_COLOR_BUFFER_BIT );
	render_spu.dispatch->ClearColor( 0, 0, 0, 1 );

	render_spu.dispatch->TexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	render_spu.dispatch->TexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	render_spu.dispatch->TexImage2D( GL_TEXTURE_2D, 0, 4, CR_LOGO_H_WIDTH, CR_LOGO_H_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw_bytes );

	render_spu.dispatch->PushMatrix();
	render_spu.dispatch->LoadIdentity();
	render_spu.dispatch->Translatef( -5, -5, 0 );
	render_spu.dispatch->MatrixMode( GL_PROJECTION );
	render_spu.dispatch->PushMatrix();
	render_spu.dispatch->LoadIdentity();
	render_spu.dispatch->Ortho( 0, render_spu.actual_window_width, render_spu.actual_window_height, 0, -1, 1 );

	render_spu.dispatch->Color3f( 1, 1, 1 );
	render_spu.dispatch->Enable( GL_TEXTURE_2D );
	render_spu.dispatch->Enable( GL_BLEND );
	render_spu.dispatch->BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	render_spu.dispatch->Begin( GL_QUAD_STRIP );
	render_spu.dispatch->TexCoord2f( 0, 1 ); render_spu.dispatch->Vertex2i( render_spu.actual_window_width-CR_LOGO_H_WIDTH, render_spu.actual_window_height );
	render_spu.dispatch->TexCoord2f( 1, 1 ); render_spu.dispatch->Vertex2i( render_spu.actual_window_width,   render_spu.actual_window_height );
	render_spu.dispatch->TexCoord2f( 0, 0 ); render_spu.dispatch->Vertex2i( render_spu.actual_window_width-CR_LOGO_H_WIDTH, render_spu.actual_window_height-CR_LOGO_H_HEIGHT );
	render_spu.dispatch->TexCoord2f( 1, 0 ); render_spu.dispatch->Vertex2i( render_spu.actual_window_width,   render_spu.actual_window_height-CR_LOGO_H_HEIGHT );
	render_spu.dispatch->End();

	render_spu.dispatch->Disable( GL_BLEND );
	render_spu.dispatch->Disable( GL_TEXTURE_2D );

	render_spu.dispatch->PopMatrix();
	render_spu.dispatch->MatrixMode( GL_MODELVIEW );
	render_spu.dispatch->SwapBuffers();
	*/
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
