/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_CreateContext(void *arg1, void *arg2)
{
	crDebug( "In tilesortspu_CreateContext" );
#ifdef WINDOWS
	tilesort_spu.client_hdc = (HDC) arg1;
	tilesort_spu.client_hwnd = NULL;
	(void) arg2;
#else
	tilesort_spu.glx_display = (Display *) arg1;
	tilesort_spu.glx_drawable = (Drawable) arg2;
#endif
}

void TILESORTSPU_APIENTRY tilesortspu_MakeCurrent(void)
{
	//crDebug( "In tilesortspu_MakeCurrent" );
}
