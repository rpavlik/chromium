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
#include "cr_threads.h"
#include "cr_spu.h"
#include "cr_hash.h"
#include "cr_server.h"

#define MAX_VISUALS 32

typedef struct {
	GLbitfield visAttribs;
	const char *displayName;
#ifdef WINDOWS
	HDC device_context;
	HWND hWnd;
#else
	Display *dpy;
	XVisualInfo *visual;
#endif
} VisualInfo;

typedef struct {
	int x, y;
	int width, height;
	VisualInfo *visual;
	GLboolean mapPending;
#ifndef WINDOWS
	Window window;
	Window nativeWindow;  /* for render_to_app_window */
#else
	HDC nativeWindow; /* for render_to_app_window */
#endif
} WindowInfo;

typedef struct {
	VisualInfo *visual;
	GLboolean everCurrent;
	int currentWindow;
#ifdef WINDOWS
	HGLRC hRC;
#else
	GLXContext context;
#endif
} ContextInfo;

typedef struct {
	SPUDispatchTable self;
	int id;

	/* config options */
	char *window_title;
	int defaultX, defaultY;
	unsigned int defaultWidth, defaultHeight;
	int use_L2;
	int fullscreen, ontop;
	char        display_string[100];
#ifndef WINDOWS
	int          try_direct;
	int          force_direct;
	int          sync;
#endif
	int render_to_app_window;
	int resizable;
	
	CRServer *server;
	int gather_port;
	int gather_userbuf_size;
	CRConnection **gather_conns;

	GLboolean drawCursor;
	GLint cursorX, cursorY;

	int numVisuals;
	VisualInfo visuals[MAX_VISUALS];

	CRHashTable *windowTable;
	CRHashTable *contextTable;

#ifndef CHROMIUM_THREADSAFE
	ContextInfo *currentContext;
#endif

	crOpenGLInterface ws;  /* Window System interface */

	CRHashTable *barrierHash;
} RenderSPU;

extern RenderSPU render_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _RenderTSD;
#define GET_CONTEXT(T)  ContextInfo *T = (ContextInfo *) crGetTSD(&_RenderTSD)
#else
#define GET_CONTEXT(T)  ContextInfo *T = render_spu->currentContext
#endif

extern void renderspuGatherConfiguration( RenderSPU *spu );
extern void renderspuMakeVisString( GLbitfield visAttribs, char *s );
extern VisualInfo *renderspuFindVisual(const char *displayName, GLbitfield visAttribs );
extern GLboolean renderspu_SystemInitVisual( VisualInfo *visual );
extern GLboolean renderspu_SystemCreateContext( VisualInfo *visual, ContextInfo *context );
extern void renderspu_SystemDestroyContext( ContextInfo *context );
extern GLboolean renderspu_SystemCreateWindow( VisualInfo *visual, GLboolean showIt, WindowInfo *window );
extern void renderspu_SystemDestroyWindow( WindowInfo *window );
extern void renderspu_SystemWindowSize( WindowInfo *window, int w, int h );
extern void renderspu_SystemGetWindowSize( WindowInfo *window, int *w, int *h );
extern void renderspu_SystemWindowPosition( WindowInfo *window, int x, int y );
extern void renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt );
extern void renderspu_SystemMakeCurrent( WindowInfo *window, GLint windowInfor, ContextInfo *context );
extern void renderspu_SystemSwapBuffers( WindowInfo *window, GLint flags );
extern int renderspuCreateFunctions( SPUNamedFunctionTable table[] );

extern GLint RENDER_APIENTRY renderspuCreateWindow( const char *dpyName, GLint visBits );
extern GLint RENDER_APIENTRY renderspuCreateContext( const char *dpyname, GLint visBits );
extern void RENDER_APIENTRY renderspuMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx);
extern void RENDER_APIENTRY renderspuSwapBuffers( GLint window, GLint flags );

#endif /* CR_RENDERSPU_H */
