/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_glstate.h"
#include "cr_error.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static int __getWindowSize( int *width_return, int *height_return )
{
#ifdef WINDOWS
	RECT r;

	if (!tilesort_spu.client_hwnd) {
		tilesort_spu.client_hwnd = WindowFromDC( tilesort_spu.client_hdc );
	}
	if (!tilesort_spu.client_hwnd) {
		if (tilesort_spu.fakeWindowWidth != 0) {
			*width_return = tilesort_spu.fakeWindowWidth;
			*height_return = tilesort_spu.fakeWindowHeight;
			return 1;
		}
		return 0;
	}

	GetClientRect( tilesort_spu.client_hwnd, &r );
	*width_return = r.right - r.left;
	*height_return = r.bottom - r.top;

#else /* X11 */
	Window root;
	int x, y;
	unsigned int width, height, border, depth;

	if (!tilesort_spu.glx_display || !tilesort_spu.glx_drawable) {
		if (tilesort_spu.fakeWindowWidth != 0) {
			*width_return = tilesort_spu.fakeWindowWidth;
			*height_return = tilesort_spu.fakeWindowHeight;
			return 1;
		}
		return 0;
	}

	if (!XGetGeometry( tilesort_spu.glx_display, 
										 tilesort_spu.glx_drawable,
										 &root, &x, &y, &width, &height, &border, &depth )) {
		crError( "XGetGeometry failed" );
	}

	*width_return  = width;
	*height_return = height;

#endif

	return 1;
}

void TILESORTSPU_APIENTRY tilesortspu_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	int w, h;

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

	if (!__getWindowSize( &w, &h ))
	{
		crError( "Couldn't get the window size in tilesortspu_Viewport!\nIf there is no Window, you can set some bogus\ndimensions in the configuration manager." );
	}

	if (tilesort_spu.scaleToMuralSize)
	{
		/* This is the usual case */
		tilesort_spu.widthScale = (float) (tilesort_spu.muralWidth) / (float) w;
		tilesort_spu.heightScale = (float) (tilesort_spu.muralHeight) / (float) h;
	}
	else {
		/* This is helpful for running Glean and other programs that are
		 * sensitive to viewport scaling.
		 */
		tilesort_spu.widthScale = 1.0;
		tilesort_spu.heightScale = 1.0;
	}

	/*
	printf("%s() %d, %d, %d, %d   muralWidth=%d muralHeight=%d  wScale=%f hScale=%f\n", __FUNCTION__, x, y, width, height, tilesort_spu.muralWidth, tilesort_spu.muralHeight, tilesort_spu.widthScale, tilesort_spu.heightScale);
	*/

	crStateViewport( (int) (x*tilesort_spu.widthScale + 0.5f), 
			 (int) (y*tilesort_spu.heightScale + 0.5f),
			 (int) (width*tilesort_spu.widthScale + 0.5f), 
			 (int) (height*tilesort_spu.heightScale + 0.5f) );
	tilesortspuSetBucketingBounds( (int) (x*tilesort_spu.widthScale + 0.5f),
			               (int) (y*tilesort_spu.heightScale + 0.5f),
			               (int) (width*tilesort_spu.widthScale + 0.5f), 
			               (int) (height*tilesort_spu.heightScale + 0.5f) );
}

void TILESORTSPU_APIENTRY tilesortspu_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
	if (tilesort_spu.scaleToMuralSize) {
		GLint newX = (GLint) (x * tilesort_spu.widthScale + 0.5F);
		GLint newY = (GLint) (y * tilesort_spu.heightScale + 0.5F);
		GLint newW = (GLint) (width * tilesort_spu.widthScale + 0.5F);
		GLint newH = (GLint) (height * tilesort_spu.heightScale + 0.5F);
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
	oldmode = t->mode;
	
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

	if (t->mode != oldmode)
		crStateMatrixMode(t->mode);
}
