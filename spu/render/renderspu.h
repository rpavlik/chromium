/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_RENDERSPU_H
#define CR_RENDERSPU_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define RENDER_APIENTRY __stdcall
#else
#include <GL/glx.h>
#define RENDER_APIENTRY
#endif

#include "cr_spu.h"

void renderspuGatherConfiguration( void );
GLboolean renderspuCreateWindow( GLbitfield visAttribs, GLboolean showIt );
void renderspuShowWindow( GLboolean showIt );
void renderspuMakeVisString( GLbitfield visAttribs, char *s );
int renderspuCreateFunctions( SPUNamedFunctionTable table[] );

void RENDER_APIENTRY renderspuSwapBuffers( void );
void RENDER_APIENTRY renderspuClear( GLbitfield mask );



typedef struct {
	SPUDispatchTable self;
	int id;
	char *window_title;
	int window_x, window_y;
	unsigned int window_width, window_height;
	unsigned int actual_window_width, actual_window_height;
	int use_L2;
	int fullscreen, ontop;

	GLboolean drawCursor;
	GLint cursorX, cursorY;

	crOpenGLInterface ws;  /* Window System interface */
	ClearFunc_t ClearFunc;

#if 0
	/* XXX these will be going away */
	int depth_bits, stencil_bits, accum_bits, alpha_bits;
#endif

	GLbitfield visAttribs;
#ifdef WINDOWS
	HWND         hWnd;
	HGLRC        hRC;
	HDC          device_context;
#else
	Display     *dpy;
	XVisualInfo *visual;
	GLXContext   context;
	Window       window;
	char        *display_string;
	int          try_direct;
	int          force_direct;
	int          sync;
#endif
} RenderSPU;

extern RenderSPU render_spu;

#endif /* CR_RENDERSPU_H */
