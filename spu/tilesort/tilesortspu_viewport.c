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
#else
	Window       root;
	int          x, y;
	unsigned int width, height, border, depth;
#endif


#ifdef WINDOWS
	if (!tilesort_spu.client_hwnd)
	{
		tilesort_spu.client_hwnd = WindowFromDC( tilesort_spu.client_hdc );
	}
	if (!tilesort_spu.client_hwnd)
	{
		goto try_fake;
	}
	GetClientRect( tilesort_spu.client_hwnd, &r );
	*width_return = r.right - r.left;
	*height_return = r.bottom - r.top;

#else

	if (!tilesort_spu.glx_display)
	{
		goto try_fake;
	}

	if ( !XGetGeometry( (Display *) tilesort_spu.glx_display, 
						(Drawable) tilesort_spu.glx_drawable,
						&root, &x, &y, &width, &height, &border, &depth ) )
	{
		crError( "XGetGeometry failed" );
	}

	*width_return  = width;
	*height_return = height;
#endif
	return 1;
try_fake:
	if (tilesort_spu.fakeWindowWidth != 0)
	{
		*width_return = tilesort_spu.fakeWindowWidth;
		*height_return = tilesort_spu.fakeWindowHeight;
		return 1;
	}
	return 0;
}

void TILESORTSPU_APIENTRY tilesortspu_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	int w,h;
	float widthscale, heightscale;

	if (!__getWindowSize( &w, &h ))
	{
		crError( "Couldn't get the window size in tilesortspu_Viewport!\nIf there is no Window, you can set some bogus\ndimensions in the configuration manager." );
	}
	widthscale = (float) (tilesort_spu.muralWidth) / (float) w;
	heightscale = (float) (tilesort_spu.muralHeight) / (float) h;
		
	/*crWarning( "Viewport: %d %d %f %f", w, h, widthscale, heightscale ); */
	crStateViewport( (int) (x*widthscale + 0.5f), 
			 (int) (y*heightscale + 0.5f),
			 (int) (width*widthscale + 0.5f), 
			 (int) (height*heightscale + 0.5f) );
	tilesortspuSetBucketingBounds( (int) (x*widthscale + 0.5f), 
			               (int) (y*heightscale + 0.5f),
			               (int) (width*widthscale + 0.5f), 
			               (int) (height*heightscale + 0.5f) );
}
