/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_spu.h"
#include "cr_applications.h"

extern SPU *stub_spu;
extern void StubInit(void);

/* I *know* most of the parameters are unused, dammit. */
#pragma warning( disable: 4100 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int WINAPI wglChoosePixelFormat_prox( HDC hdc, CONST PIXELFORMATDESCRIPTOR *pfd )
{
	DWORD okayFlags;

	if ( pfd->nSize != sizeof(*pfd) || pfd->nVersion != 1 ) {
		crError( "wglChoosePixelFormat: bad pfd\n" );
		return 0;
	}

	okayFlags = ( PFD_DRAW_TO_WINDOW        |
			PFD_SUPPORT_GDI           |
			PFD_SUPPORT_OPENGL        |
			PFD_DOUBLEBUFFER          |
			PFD_DOUBLEBUFFER_DONTCARE |
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
		crError( "wglChoosePixelFormat: too much color precision requested\n" );
	}

	if ( pfd->cAccumBits      > 0 ||
			pfd->cAccumRedBits   > 0 ||
			pfd->cAccumGreenBits > 0 ||
			pfd->cAccumBlueBits  > 0 ||
			pfd->cAccumAlphaBits > 0 ) {
		crError( "wglChoosePixelFormat: asked for accumulation buffer, ignoring\n" );
	}

	if ( pfd->cDepthBits > 32 ) {
		crError( "wglChoosePixelFormat; asked for too many depth bits\n" );
	}

	if ( pfd->cStencilBits > 8 ) {
		crError( "wglChoosePixelFormat: asked for too many stencil bits\n" );
	}

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
	return 1;
}

BOOL WINAPI wglMakeCurrent_prox( HDC hdc, HGLRC hglrc )
{
	stub_spu->dispatch_table.MakeCurrent();

	return 1;
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
	crError( "wglShareLists: unsupported" );
	return 0;
}

HGLRC WINAPI wglCreateContext_prox( HDC hdc )
{
	static int already_has_a_context = 0;
	/* This should be early enough to initialize everything. 
	 * You have to create a context before you can make current 
	 * to it, right? */

	if (!already_has_a_context)
	{
		already_has_a_context = 1;
	}
	else
	{
		crError( "I'm sorry, I don't support multiple context creation right now.  If this is holding you up, file a bug." ); 
	}

	StubInit();
	stub_spu->dispatch_table.CreateContext( (void *) hdc, NULL );

	/* hack hack hack */
	return (HGLRC) 0x69696969;
}

BOOL WINAPI
wglSwapBuffers_prox( HDC hdc )
{
	stub_spu->dispatch_table.SwapBuffers();
	return 1;
}

BOOL WINAPI wglCopyContext_prox( HGLRC src, HGLRC dst, UINT mask )
{
	crError( "wglCopyContext: unsupported" );
	return 0;
}

HGLRC WINAPI wglCreateLayerContext_prox( HDC hdc, int layerPlane )
{
	crError( "wglCreateLayerContext: unsupported" );
	return 0;
}

PROC WINAPI wglGetProcAddress_prox( LPCSTR name )
{
	return (PROC) crGetProcAddress( name );
}

BOOL WINAPI wglUseFontBitmapsA_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase )
{
	crError( "wglUseFontBitmapsA: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontBitmapsW_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase )
{
	crError( "wglUseFontBitmapsW: unsupported" );
	return 0;
}

BOOL WINAPI wglDescribeLayerPlane_prox( HDC hdc, int pixelFormat, int layerPlane,
		UINT nBytes, LPLAYERPLANEDESCRIPTOR lpd )
{
	crError( "wglDescribeLayerPlane: unimplemented" );
	return 0;
}

int WINAPI wglSetLayerPaletteEntries_prox( HDC hdc, int layerPlane, int start,
		int entries, CONST COLORREF *cr )
{
	crError( "wglSetLayerPaletteEntries: unsupported" );
	return 0;
}

int WINAPI wglGetLayerPaletteEntries_prox( HDC hdc, int layerPlane, int start,
		int entries, COLORREF *cr )
{
	crError( "wglGetLayerPaletteEntries: unsupported" );
	return 0;
}

BOOL WINAPI wglRealizeLayerPalette_prox( HDC hdc, int layerPlane, BOOL realize )
{
	crError( "wglRealizeLayerPalette: unsupported" );
	return 0;
}

DWORD WINAPI wglSwapMultipleBuffers_prox( UINT a, CONST void *b )
{
	crError( "wglSwapMultipleBuffer: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontOutlinesA_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase,
		FLOAT deviation, FLOAT extrusion, int format,
		LPGLYPHMETRICSFLOAT gmf )
{
	crError( "wglUseFontOutlinesA: unsupported" );
	return 0;
}

BOOL WINAPI wglUseFontOutlinesW_prox( HDC hdc, DWORD first, DWORD count, DWORD listBase,
		FLOAT deviation, FLOAT extrusion, int format,
		LPGLYPHMETRICSFLOAT gmf )
{
	crError( "wglUseFontOutlinesW: unsupported" );
	return 0;
}

BOOL WINAPI wglSwapLayerBuffers_prox( HDC hdc, UINT planes )
{
	crError( "wglSwapLayerBuffers: unsupported" );
	return 0;
}
