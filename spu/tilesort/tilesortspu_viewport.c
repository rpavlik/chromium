#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"

static int __getWindowSize( int *width_return, int *height_return )
{
#ifdef WINDOWS
	RECT r;
	if (!tilesort_spu.client_hwnd)
	{
		tilesort_spu.client_hwnd = WindowFromDC( tilesort_spu.client_hdc );
	}
	if (!tilesort_spu.client_hwnd)
	{
		return 0;
	}
	GetClientRect( tilesort_spu.client_hwnd, &r );
	*width_return = r.right - r.left;
	*height_return = r.bottom - r.top;

#else

	Window       root;
	int          x, y;
	unsigned int width, height, border, depth;

	if (!tilesort_spu.glx_display)
	{
		return 0;
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
}

void TILESORTSPU_APIENTRY tilesortspu_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
}

void TILESORTSPU_APIENTRY tilesortspu_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
}
