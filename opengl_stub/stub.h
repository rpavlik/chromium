/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STUB_H
#define CR_STUB_H

#include "cr_glwrapper.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "spu_dispatch_table.h"

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

extern void stubMatchWindowTitle( const char *title );
extern void StubInit(void);


extern SPUDispatchTable glim;
extern SPUDispatchTable stubThreadsafeDispatch;


/* "Global" variables for the stub library */
typedef struct {
	int appDrawCursor;
	SPU *spu;

	crOpenGLInterface wsInterface;
	SPUDispatchTable spuDispatch;
	SPUDispatchTable nativeDispatch;

	GLboolean haveNativeOpenGL;

#ifdef CHROMIUM_THREADSAFE
	CRtsd dispatchTSD;

	CRmutex mutex;
#endif

	GLuint desiredVisual;  /* Bitwise-or of CR_*_BIT flags */
	GLuint minChromiumWindowWidth;
	GLuint minChromiumWindowHeight;
	char *matchWindowTitle;

	GLint spuWindow;  /* returned by dispatch->CreateWindow() */

	GLboolean threadSafe;
} Stub;

extern Stub stub;


#endif /* CR_STUB_H */
