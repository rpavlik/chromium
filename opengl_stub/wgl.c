/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_spu.h"
#include "cr_environment.h"
#include "stub.h"

/* I *know* most of the parameters are unused, dammit. */
#pragma warning( disable: 4100 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

GLuint FindVisualInfo( HDC hdc )
{
	PIXELFORMATDESCRIPTOR pfd; 
	int iPixelFormat;
	GLuint b = 0;

	iPixelFormat = GetPixelFormat( hdc );

	DescribePixelFormat( hdc, iPixelFormat, sizeof(pfd), &pfd );

	if (pfd.cDepthBits > 0)
		b |= CR_DEPTH_BIT;
	if (pfd.cAccumBits > 0)
		b |= CR_ACCUM_BIT;
	if (pfd.cColorBits > 8)
		b |= CR_RGB_BIT;
	if (pfd.cStencilBits > 0)
		b |= CR_STENCIL_BIT;
	if (pfd.cAlphaBits > 0)
		b |= CR_ALPHA_BIT;
	if (pfd.dwFlags & PFD_DOUBLEBUFFER)
		b |= CR_DOUBLE_BIT;
	if (pfd.dwFlags & PFD_STEREO)
		b |= CR_STEREO_BIT;

	return b;
}

int WINAPI wglChoosePixelFormat_prox( HDC hdc, CONST PIXELFORMATDESCRIPTOR *pfd )
{
	DWORD okayFlags;

	stubInit();

	/* 
	 * NOTE!!!
	 * Here we're telling the renderspu not to use the GDI
	 * equivalent's of ChoosePixelFormat/DescribePixelFormat etc
	 * There are subtle differences in the use of these calls.
	 */
	crSetenv("CR_WGL_DO_NOT_USE_GDI", "yes");

	if ( pfd->nSize != sizeof(*pfd) || pfd->nVersion != 1 ) {
		crError( "wglChoosePixelFormat: bad pfd\n" );
		return 0;
	}

	okayFlags = ( PFD_DRAW_TO_WINDOW        |
			PFD_SUPPORT_GDI           |
			PFD_SUPPORT_OPENGL        |
			PFD_DOUBLEBUFFER          |
			PFD_DOUBLEBUFFER_DONTCARE |
			PFD_STEREO	          |
			PFD_STEREO_DONTCARE       |
			PFD_DEPTH_DONTCARE        );
	if ( pfd->dwFlags & ~okayFlags ) {
		crError( "wglChoosePixelFormat: only support flags=0x%x, but you gave me flags=0x%x", okayFlags, pfd->dwFlags );
	}

	if ( pfd->iPixelType != PFD_TYPE_RGBA ) {
		crError( "wglChoosePixelFormat: only support RGBA\n" );
	}

	if ( pfd->cColorBits > 32 ||
			pfd->cRedBits   > 8  ||
			pfd->cGreenBits > 8  ||
			pfd->cBlueBits  > 8  ||
			pfd->cAlphaBits > 8 ) {
		crWarning( "wglChoosePixelFormat: too much color precision requested\n" );
	}

	if ( pfd->dwFlags & PFD_DOUBLEBUFFER )
		stub.desiredVisual |= CR_DOUBLE_BIT;

	if ( pfd->dwFlags & PFD_STEREO )
		stub.desiredVisual |= CR_STEREO_BIT;

	if ( pfd->cColorBits > 8)
		stub.desiredVisual |= CR_RGB_BIT;

	if ( pfd->cAccumBits      > 0 ||
			pfd->cAccumRedBits   > 0 ||
			pfd->cAccumGreenBits > 0 ||
			pfd->cAccumBlueBits  > 0 ||
			pfd->cAccumAlphaBits > 0 ) {
		crWarning( "wglChoosePixelFormat: asked for accumulation buffer, ignoring\n" );
	}

	if ( pfd->cAccumBits > 0 )
		stub.desiredVisual |= CR_ACCUM_BIT;

	if ( pfd->cDepthBits > 32 ) {
		crError( "wglChoosePixelFormat; asked for too many depth bits\n" );
	}
	
	if ( pfd->cDepthBits > 0 )
		stub.desiredVisual |= CR_DEPTH_BIT;

	if ( pfd->cStencilBits > 8 ) {
		crError( "wglChoosePixelFormat: asked for too many stencil bits\n" );
	}

	if ( pfd->cStencilBits > 0 )
		stub.desiredVisual |= CR_STENCIL_BIT;

	if ( pfd->cAuxBuffers > 0 ) {
		crError( "wglChoosePixelFormat: asked for aux buffers\n" );
	}

	if ( pfd->iLayerType != PFD_MAIN_PLANE ) {
		crError( "wglChoosePixelFormat: asked for a strange layer\n" );
	}

	return 1;
}

BOOL WINAPI wglSetPixelFormat_prox( HDC hdc, int pixelFormat, 
		CONST PIXELFORMATDESCRIPTOR *pdf )
{
	if ( pixelFormat != 1 ) {
		crError( "wglSetPixelFormat: pixelFormat=%d?\n", pixelFormat );
	}

	return 1;
}

BOOL WINAPI wglDeleteContext_prox( HGLRC hglrc )
{
	stubDestroyContext( (GLint) hglrc );
	return 1;
}

BOOL WINAPI wglMakeCurrent_prox( HDC hdc, HGLRC hglrc )
{
	ContextInfo *context;
	WindowInfo *window;

	context = (ContextInfo *) crHashtableSearch(stub.contextTable, (int) hglrc);
	window = stubGetWindowInfo(hdc);

	return stubMakeCurrent( window, context );
}

HGLRC WINAPI wglGetCurrentContext_prox( void )
{
	return (HGLRC) NULL; /*__wiregl_globals.context; */
}

HDC WINAPI wglGetCurrentDC_prox( void )
{
	return (HDC) NULL; /*__wiregl_globals.client_hdc; */
}

int WINAPI wglGetPixelFormat_prox( HDC hdc )
{
	/* this is what we call our generic pixelformat, regardless of the HDC */
	return 1;
}

int WINAPI wglDescribePixelFormat_prox( HDC hdc, int pixelFormat, UINT nBytes,
		LPPIXELFORMATDESCRIPTOR pfd )
{
/*	if ( pixelFormat != 1 ) { 
 *		crError( "wglDescribePixelFormat: pixelFormat=%d?\n", pixelFormat ); 
 *		return 0; 
 *	} */

	if ( nBytes != sizeof(*pfd) ) {
		crWarning( "wglDescribePixelFormat: nBytes=%u?\n", nBytes );
		return 1; /* There's only one, baby */
	}

	pfd->nSize           = sizeof(*pfd);
	pfd->nVersion        = 1;
	pfd->dwFlags         = ( PFD_DRAW_TO_WINDOW |
		 		 PFD_SUPPORT_GDI    |
				 PFD_SUPPORT_OPENGL |
				 PFD_DOUBLEBUFFER );
	pfd->iPixelType      = PFD_TYPE_RGBA;
	pfd->cColorBits      = 32;
	pfd->cRedBits        = 8;
	pfd->cRedShift       = 24;
	pfd->cGreenBits      = 8;
	pfd->cGreenShift     = 16;
	pfd->cBlueBits       = 8;
	pfd->cBlueShift      = 8;
	pfd->cAlphaBits      = 8;
	pfd->cAlphaShift     = 0;
	pfd->cAccumBits      = 0;
	pfd->cAccumRedBits   = 0;
	pfd->cAccumGreenBits = 0;
	pfd->cAccumBlueBits  = 0;
	pfd->cAccumAlphaBits = 0;
	pfd->cDepthBits      = 32;
	pfd->cStencilBits    = 8;
	pfd->cAuxBuffers     = 0;
	pfd->iLayerType      = PFD_MAIN_PLANE;
	pfd->bReserved       = 0;
	pfd->dwLayerMask     = 0;
	pfd->dwVisibleMask   = 0;
	pfd->dwDamageMask    = 0;

	/* the max PFD index */
	return 1;
}

BOOL WINAPI wglShareLists_prox( HGLRC hglrc1, HGLRC hglrc2 )
{
	crWarning( "wglShareLists: unsupported" );
	return 0;
}

HGLRC WINAPI wglCreateContext_prox( HDC hdc )
{
	stubInit();
	return stubCreateContext( hdc );
}

BOOL WINAPI
wglSwapBuffers_prox( HDC hdc )
{
	const WindowInfo *window = stubGetWindowInfo(hdc);
	stubSwapBuffers( window, 0 );
	return 1;
}

BOOL WINAPI wglCopyContext_prox( HGLRC src, HGLRC dst, UINT mask )
{
	crWarning( "wglCopyContext: unsupported" );
	return 0;
}

HGLRC WINAPI wglCreateLayerContext_prox( HDC hdc, int layerPlane )
{
	stubInit();
	crWarning( "wglCreateLayerContext: unsupported" );
	return 0;
}

PROC WINAPI wglGetProcAddress_prox( LPCSTR name )
{
	return (PROC) crGetProcAddress( name );
}

BOOL WINAPI wglUseFontBitmapsA_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase )
{
	crWarning( "wglUseFontBitmapsA: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontBitmapsW_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase )
{
	crWarning( "wglUseFontBitmapsW: unsupported" );
	return 0;
}

BOOL WINAPI wglDescribeLayerPlane_prox( HDC hdc, int pixelFormat, int layerPlane,
		UINT nBytes, LPLAYERPLANEDESCRIPTOR lpd )
{
	crWarning( "wglDescribeLayerPlane: unimplemented" );
	return 0;
}

int WINAPI wglSetLayerPaletteEntries_prox( HDC hdc, int layerPlane, int start,
		int entries, CONST COLORREF *cr )
{
	crWarning( "wglSetLayerPaletteEntries: unsupported" );
	return 0;
}

int WINAPI wglGetLayerPaletteEntries_prox( HDC hdc, int layerPlane, int start,
		int entries, COLORREF *cr )
{
	crWarning( "wglGetLayerPaletteEntries: unsupported" );
	return 0;
}

BOOL WINAPI wglRealizeLayerPalette_prox( HDC hdc, int layerPlane, BOOL realize )
{
	crWarning( "wglRealizeLayerPalette: unsupported" );
	return 0;
}

DWORD WINAPI wglSwapMultipleBuffers_prox( UINT a, CONST void *b )
{
	crWarning( "wglSwapMultipleBuffer: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontOutlinesA_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase,
		FLOAT deviation, FLOAT extrusion, int format,
		LPGLYPHMETRICSFLOAT gmf )
{
	crWarning( "wglUseFontOutlinesA: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontOutlinesW_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase,
		FLOAT deviation, FLOAT extrusion, int format,
		LPGLYPHMETRICSFLOAT gmf )
{
	crWarning( "wglUseFontOutlinesW: unsupported" );
	return 0;
}

BOOL WINAPI wglSwapLayerBuffers_prox( HDC hdc, UINT planes )
{
	crWarning( "wglSwapLayerBuffers: unsupported" );
	return 0;
}
