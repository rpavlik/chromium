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

#define MAX_VISUALS 32
#define MAX_WINDOWS 32
#define MAX_CONTEXTS 32

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
	GLboolean inUse;
	int x, y;
	int width, height;
	VisualInfo *visual;
	GLboolean mapPending;
#ifndef WINDOWS
	Window window;
#endif
} WindowInfo;

typedef struct {
	GLboolean inUse;
	VisualInfo *visual;
	GLboolean everCurrent;
#ifdef WINDOWS
	HGLRC hRC;
#else
	GLXContext context;
#endif
} ContextInfo;

typedef struct {
	unsigned long id;
	int currentContext;
	int currentWindow;
#ifndef WINDOWS
	Display *dpy;
#endif
} ThreadInfo;

typedef struct {
	SPUDispatchTable self;
	int id;

	/* config options */
	char *window_title;
	int defaultX, defaultY;
	unsigned int defaultWidth, defaultHeight;
	int use_L2;
	int fullscreen, ontop;
	char        *display_string;
#ifndef WINDOWS
	int          try_direct;
	int          force_direct;
	int          sync;
#endif

	GLboolean drawCursor;
	GLint cursorX, cursorY;

	int numVisuals;
	VisualInfo visuals[MAX_VISUALS];

	WindowInfo windows[MAX_WINDOWS];

	ContextInfo contexts[MAX_CONTEXTS];

#ifndef CHROMIUM_THREADSAFE
	ThreadInfo singleThread;
#endif

	crOpenGLInterface ws;  /* Window System interface */

	CRHashTable *barrierHash;
} RenderSPU;

extern RenderSPU render_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _RenderTSD;
#define GET_THREAD(T)  ThreadInfo *T = (ThreadInfo *) crGetTSD(&_RenderTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(render_spu.singleThread)
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
extern void renderspu_SystemWindowPosition( WindowInfo *window, int x, int y );
extern void renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt );
extern void renderspu_SystemMakeCurrent( ThreadInfo *thread, WindowInfo *window, ContextInfo *context );
extern int renderspuCreateFunctions( SPUNamedFunctionTable table[] );

extern GLint RENDER_APIENTRY renderspuCreateWindow( const char *dpyName, GLint visBits );
extern GLint RENDER_APIENTRY renderspuCreateContext( const char *dpyname, GLint visBits );
extern void RENDER_APIENTRY renderspuMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx);
extern void RENDER_APIENTRY renderspuSwapBuffers( GLint window, GLint flags );

#endif /* CR_RENDERSPU_H */
