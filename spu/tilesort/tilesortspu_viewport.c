/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_glstate.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	GET_THREAD(thread);
	CRCurrentState *c = &(thread->currentContext->State->current);
	WindowInfo *winInfo;

	if (c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION,
									"glViewport() called between glBegin/glEnd" );
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE,
									"glViewport(bad width or height" );
		return;
	}

	winInfo = thread->currentContext->currentWindow;
	CRASSERT(winInfo);

	/* get latest window dimensions */
	tilesortspuUpdateWindowInfo(winInfo);

	if (winInfo->lastWidth == 0 || winInfo->lastHeight == 0) 
	{
		crError( "Couldn't get the window size in tilesortspu_Viewport!\n"
						 "If there is no Window, set fake_window_dims for the "
						 "tilesort SPU." );
	}

	if (tilesort_spu.scaleToMuralSize)
	{
		/* This is the usual case */
		winInfo->widthScale = (float) winInfo->muralWidth / (float) winInfo->lastWidth;
		winInfo->heightScale = (float) winInfo->muralHeight / (float) winInfo->lastHeight;
	}
	else {
		/* This is helpful for running Glean and other programs that are
		 * sensitive to viewport scaling.
		 */
		winInfo->widthScale = 1.0;
		winInfo->heightScale = 1.0;
	}


	/*
	if (tilesort_spu.useDMX) {
		CRASSERT( winInfo->widthScale == 1.0 );
		CRASSERT( winInfo->heightScale == 1.0 );
	}
	*/

	/* Scale the viewport up to the mural's size */
	{
		GLint vpx, vpy, vpw, vph;

		vpx = (int) (x * winInfo->widthScale + 0.5f);
		vpy = (int) (y * winInfo->heightScale + 0.5f);
		vpw = (int) (width * winInfo->widthScale + 0.5f);
		vph = (int) (height * winInfo->heightScale + 0.5f);

		if (vpw == 0 && width > 0)
			vpw = 1;
		if (vph == 0 && height > 0)
			vph = 1;

		crStateViewport(vpx, vpy, vpw, vph);
		tilesortspuSetBucketingBounds(winInfo, vpx, vpy, vpw, vph);
	}
}

void TILESORTSPU_APIENTRY tilesortspu_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
	GET_THREAD(thread);
	WindowInfo *winInfo = thread->currentContext->currentWindow;

	if (tilesort_spu.scaleToMuralSize) {
		GLint newX = (GLint) (x * winInfo->widthScale + 0.5F);
		GLint newY = (GLint) (y * winInfo->heightScale + 0.5F);
		GLint newW = (GLint) (width * winInfo->widthScale + 0.5F);
		GLint newH = (GLint) (height * winInfo->heightScale + 0.5F);
		crStateScissor(newX, newY, newW, newH);
	}
	else {
    		crStateScissor(x, y, width, height);
	}
}

void TILESORTSPU_APIENTRY tilesortspu_PopAttrib( void )
{
	GET_CONTEXT(ctx);
	CRViewportState *v = &(ctx->viewport);
	CRTransformState *t = &(ctx->transform);
	GLint oldViewportX, oldViewportY, oldViewportW, oldViewportH;
	GLint oldScissorX, oldScissorY, oldScissorW, oldScissorH;
	GLenum oldmode;

	/* save current matrix mode */
	oldmode = t->matrixMode;
	
	/* save current viewport dims */
	oldViewportX = v->viewportX;
	oldViewportY = v->viewportY;
	oldViewportW = v->viewportW;
	oldViewportH = v->viewportH;

	/* save current scissor dims */
	oldScissorX = v->scissorX;
	oldScissorY = v->scissorY;
	oldScissorW = v->scissorW;
	oldScissorH = v->scissorH;

	crStatePopAttrib();

	/*
	 * If PopAttrib changed the something call the appropriate
	 * functions to be sure everything gets updated correctly
	 */
	if (v->viewportX != oldViewportX || v->viewportY != oldViewportY ||
		v->viewportW != oldViewportW || v->viewportH != oldViewportH)
	{
		 tilesortspu_Viewport( v->viewportX, v->viewportY,
													 v->viewportW, v->viewportH );
	}

	if (v->scissorX != oldScissorX || v->scissorY != oldScissorY ||
		v->scissorW != oldScissorW || v->scissorH != oldScissorH)
	{
		 tilesortspu_Scissor( v->scissorX, v->scissorY,
													 v->scissorW, v->scissorH );
	}

	if (t->matrixMode != oldmode)
		crStateMatrixMode(t->matrixMode);
}
