/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STUB_H
#define CR_STUB_H

#include "cr_glwrapper.h"


#ifdef WINDOWS

extern HGLRC stubCreateContext( HDC hdc );
extern BOOL WINAPI stubMakeCurrent( HDC drawable, HGLRC context );
extern BOOL stubDestroyContext( HGLRC context );
extern BOOL stubSwapBuffers( HDC hdc );

#else

extern GLXContext stubCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct );
extern Bool stubMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext context );
extern void stubDestroyContext( Display *dpy, GLXContext context );
extern void stubSwapBuffers( Display *dpy, GLXDrawable drawable );

extern void stubUseXFont( Display *dpy, Font font, int first, int count, int listbase );

#endif

extern void stubMinimumChromiumWindowSize( int w, int h );
extern void stubMatchWindowTitle( const char *title );

extern void FakerInit( SPU *fns );
extern void StubInit(void);

extern int crAppDrawCursor;
extern SPU *stub_spu;

extern crOpenGLInterface glinterface;
extern SPUDispatchTable glim;
extern SPUDispatchTable glstub;
extern SPUDispatchTable glnative;

extern GLuint DesiredVisual;  /* Bitwise-or of VIS_* flags */

#endif /* CR_STUB_H */
