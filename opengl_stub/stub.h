/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STUB_H
#define CR_STUB_H

#include "chromium.h"
#include "cr_process.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "spu_dispatch_table.h"


/* When we first create a rendering context we can't be sure whether
 * it'll be handled by Chromium or as a native GLX/WGL context.  So in
 * CreateContext() we'll mark the ContextInfo object as UNDECIDED then
 * switch it to either NATIVE or CHROMIUM the first time MakeCurrent()
 * is called.  In MakeCurrent() we can use a criterium like window size
 * or window title to decide between CHROMIUM and NATIVE.
 */
typedef enum
{
	UNDECIDED,
	CHROMIUM,
	NATIVE
} ContextType;


typedef struct
{
	GLboolean inUse;
	GLint stubContext;
	ContextType type;
#ifdef WINDOWS
	HGLRC hglrc;
	HDC currentDrawable;
#else
	Display *dpy;
	GLXContext share;
	XVisualInfo *visual;
	Bool direct;
	GLXContext glxContext;
	Window currentDrawable;
#endif
} ContextInfo;


/* "Global" variables for the stub library */
typedef struct {
	/* the first SPU in the SPU chain on this app node */
	SPU *spu;

	/* OpenGL/SPU dispatch tables */
	crOpenGLInterface wsInterface;
	SPUDispatchTable spuDispatch;
	SPUDispatchTable nativeDispatch;
	GLboolean haveNativeOpenGL;

	/* config options */
	int appDrawCursor;
	GLuint minChromiumWindowWidth;
	GLuint minChromiumWindowHeight;
	char *matchWindowTitle;
	int trackWindowSize;

	/* thread safety stuff */
	GLboolean threadSafe;
#ifdef CHROMIUM_THREADSAFE
	CRtsd dispatchTSD;
	CRmutex mutex;
#endif

	CRpid mothershipPID;

	/* visual/context/window management */
	GLuint desiredVisual;  /* Bitwise-or of CR_*_BIT flags */
	GLint spuWindow;       /* returned by dispatch->WindowCreate() */
  	GLuint spuWindowWidth, spuWindowHeight;
	GLint currentContext;  /* index into Context[] array, or -1 for none */
	ContextInfo Context[CR_MAX_CONTEXTS];

} Stub;


extern Stub stub;
extern SPUDispatchTable glim;
extern SPUDispatchTable stubThreadsafeDispatch;


#ifdef WINDOWS

/* WGL versions */
extern HGLRC stubCreateContext( HDC hdc );
extern BOOL WINAPI stubMakeCurrent( HDC drawable, HGLRC context );
extern BOOL stubDestroyContext( HGLRC context );
extern BOOL stubSwapBuffers( HDC hdc );

#else

/* GLX versions */
extern GLXContext stubCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct );
extern Bool stubMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext context );
extern void stubDestroyContext( Display *dpy, GLXContext context );
extern void stubSwapBuffers( Display *dpy, GLXDrawable drawable );

extern void stubUseXFont( Display *dpy, Font font, int first, int count, int listbase );

#endif

extern void stubGetWindowSize( const ContextInfo *ctx, unsigned int *w, unsigned int *h );
extern void stubMatchWindowTitle( const char *title );
extern void StubInit(void);



#endif /* CR_STUB_H */
