/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/*
 * This file manages OpenGL rendering contexts in the faker library.
 * The big issue is switching between Chromium and native GL context
 * management.  This is where we support multiple client OpenGL
 * windows.  Typically, one window is handled by Chromium while any
 * other windows are handled by the native OpenGL library.
 */

#include <string.h>
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_applications.h"
#include "cr_mem.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "stub.h"


/*
 * If the application queries a visual's stencil size, etc we assume that the
 * application wants that feature.  This gets propogated down to the render
 * SPU so that it chooses an appropriate visual.
 * This bitmask of CR_ flags indicates what the user (probably) wants.
 */
GLuint DesiredVisual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;


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
	GLXContext share;
	XVisualInfo *visual;
	Bool direct;
	GLXContext glxContext;
	Window currentDrawable;
#endif
} ContextInfo;

static ContextInfo Context[CR_MAX_CONTEXTS];


static unsigned int MinChromiumWidth = 0, MinChromiumHeight = 0;
static char *ChromiumWindowTitle = NULL;


void stubMinimumChromiumWindowSize( int w, int h )
{
    MinChromiumWidth = w;
    MinChromiumHeight = h;
}

void stubMatchWindowTitle( const char *title )
{
    if (ChromiumWindowTitle) 
        crFree(ChromiumWindowTitle);
    if (title)
        ChromiumWindowTitle = crStrdup( title );
    else
        ChromiumWindowTitle = NULL;
}



#ifdef WINDOWS
HGLRC stubCreateContext( HDC hdc )
#else
GLXContext stubCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct )
#endif
{
	static GLboolean firstCall = GL_TRUE;
	int i;

	/*
	printf("***** %s(%p, %p, %d, %d) \n", __FUNCTION__, dpy, vis, (int) share, (int) direct);
	*/

	/* one-time init */
	if (firstCall) {
		StubInit();
		memset(Context, 0, sizeof(Context));
		firstCall = GL_FALSE;
	}

	/* Find a free context slot */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (!Context[i].inUse)
			break;  /* found a free slot */
	}
	if (i == CR_MAX_CONTEXTS) {
		/* too many contexts! */
#ifdef WINDOWS
		return (HGLRC) 0;
#else
		return (GLXContext) 0;
#endif
	}

	/* We'll create the context later, save this info */
	Context[i].inUse = GL_TRUE;
	Context[i].type = UNDECIDED;
#ifndef WINDOWS
	Context[i].visual = vis;
	Context[i].direct = direct;
	Context[i].share = share;
#endif
	Context[i].currentDrawable = 0;

	/*
	fprintf(stderr, "*** stubCreateContext return %d\n", i+500);
	*/

	/* Add a magic number to the index and return it as the context
	 * identifier.
	 */
#ifdef WINDOWS
	return (HGLRC) (i + 500);
#else
	return (GLXContext) (long) (i + 500);
#endif
}


/*
 * This creates a native GLX/WGL context for Context[i].
 */
#ifdef WINDOWS
static GLboolean InstantiateNativeContext( HDC hdc, int i )
#else
static GLboolean InstantiateNativeContext( Display *dpy, int i )
#endif
{
#ifdef WINDOWS
	CRASSERT( i >= 0);
	CRASSERT( i < CR_MAX_CONTEXTS);
	Context[i].hglrc = glinterface.wglCreateContext( hdc );
	return Context[i].hglrc ? GL_TRUE : GL_FALSE;
#else
	GLXContext shareCtx = 0;

	CRASSERT( i >= 0);
	CRASSERT( i < CR_MAX_CONTEXTS);

	/* sort out context sharing here */
	if (Context[i].share) {
		int iShare = iShare = (int) (long) Context[i].share - 500;
		int j;
		CRASSERT(iShare >= 0);
		CRASSERT(iShare < CR_MAX_CONTEXTS);
		shareCtx = Context[iShare].glxContext;
		/* now check that the share context is a native context */
		for (j = 0; j < CR_MAX_CONTEXTS; j++) {
			if (Context[j].inUse && Context[j].glxContext == shareCtx)
				break;
		}
		if (j == CR_MAX_CONTEXTS) {
			crWarning("glXCreateContext() is trying to share a non-existant GLX context.  Setting share context to zero.");
			shareCtx = 0;
		}
		else if (Context[j].type != NATIVE) {
			crWarning("glXCreateContext() is trying to share a non-native GLX context.  Setting share context to zero.");
			shareCtx = 0;
		}
		else {
			/* sharing should work! */
		}
	}

	/*
	printf("***** calling native glXCreateContext(%p, %p, %d, %d)\n",
			   dpy, Context[i].visual, (int) Context[i].share, (int) Context[i].direct);
	*/

	Context[i].glxContext = glinterface.glXCreateContext( dpy, Context[i].visual, shareCtx, Context[i].direct );

	/*
	fprintf(stderr, "***** native glXCreateContext returned %d\n", (int) Context[i].glxContext);
	*/

	return Context[i].glxContext ? GL_TRUE : GL_FALSE;
#endif
}


/*
 * Utility functions to get window size and titlebar text.
 */
#ifdef WINDOWS

static void GetWindowSize( HDC drawable, unsigned int *w, unsigned int *h )
{
	/* XXX to do */
	*w = *h = 0;
}

/* free the result with crFree(). */
static char *GetWindowTitle( HDC window, GLboolean recurseUp )
{
	/* XXX to do */
	return NULL;
}

static void GetCursorPosition( HDC window, int pos[2] )
{
	/* XXX to do */
	pos[0] = pos[1] = 0;
}

#else

static void GetWindowSize( Display *dpy, GLXDrawable drawable,
													 unsigned int *w, unsigned int *h )
{
	Window root;
	int x, y;
	unsigned int border, depth;
	if (!XGetGeometry(dpy, drawable, &root, &x, &y, w, h, &border, &depth))
		*w = *h = 0;
}

/* free the result with crFree(). */
static char *GetWindowTitle( Display *dpy, Window window, GLboolean recurseUp )
{
	while (1) {
		char *name;
		if (!XFetchName(dpy, window, &name))
			return NULL;
		if (name[0]) {
			char *s = crStrdup(name);
			XFree(name);
			return s;
		}
		else if (recurseUp) {
			/* This window has no name, try the parent */
			Status stat;
			Window root, parent, *children;
			unsigned int numChildren;
			stat = XQueryTree( dpy, window, &root, &parent,
												 &children, &numChildren );
			if (!stat || window == root)
				return NULL;
			if (children)
				XFree(children);
			window = parent;
		}
		else {
			XFree(name);
			return NULL;
		}
	}
}

static void GetCursorPosition( Display *dpy, Window win, int pos[2] )
{
	int rootX, rootY;
	Window root, child;
	unsigned int mask;
	Bool q;

	q = XQueryPointer(dpy, win, &root, &child,
										&rootX, &rootY, &pos[0], &pos[1], &mask);
	if (q) {
		unsigned int w, h;
		GetWindowSize( dpy, win, &w, &h );
		/* invert Y */
		pos[1] = (int) h - pos[1] - 1;
	}
	else {
		pos[0] = pos[1] = 0;
	}
}

#endif


/*
 * This function is called by MakeCurrent() and determines whether or
 * not a new rendering context should be bound to Chromium or the native
 * OpenGL.
 */
#ifdef WINDOWS
static GLboolean UseChromium( HDC drawable )
#else
static GLboolean UseChromium( Display *dpy, GLXDrawable drawable )
#endif
{
	unsigned int w, h;
	int i;

	/* Can only have one chromium window at this time */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (Context[i].inUse && Context[i].type == CHROMIUM) {
			if (Context[i].currentDrawable == drawable)
				return GL_TRUE;
			else
				return GL_FALSE;
		}
	}

	/* If the user's specified a minimum window size for Chromium, see if
	 * this window satisfies that criterium.
	 */
	if (MinChromiumWidth > 0 && MinChromiumHeight > 0) {
#ifdef WINDOWS
		GetWindowSize( drawable, &w, &h );
#else
		GetWindowSize( dpy, drawable, &w, &h );
#endif
		if (w >= MinChromiumWidth && h >=MinChromiumHeight)
			return GL_TRUE;
	}
	else if (ChromiumWindowTitle) {
		/* If the user's specified a window title for Chromium, see if this
		 * window satisfies that criterium.
		 */
		GLboolean wildcard = GL_FALSE;
		char *titlePattern, *title;
		int len;
		/* check for leading '*' wildcard */
		if (ChromiumWindowTitle[0] == '*') {
			titlePattern = crStrdup( ChromiumWindowTitle + 1 );
			wildcard = GL_TRUE;
		}
		else {
			titlePattern = crStrdup( ChromiumWindowTitle );
		}
		/* check for trailing '*' wildcard */
		len = crStrlen(titlePattern);
		if (len > 0 && titlePattern[len - 1] == '*') {
			titlePattern[len - 1] = '\0'; /* terminate here */
			wildcard = GL_TRUE;
		}

#ifdef WINDOWS
		title = GetWindowTitle( drawable, GL_TRUE );
#else
		title = GetWindowTitle( dpy, drawable, GL_TRUE );
#endif
		if (title) {
			if (wildcard) {
				if (crStrstr(title, titlePattern)) {
					crFree(titlePattern);
					crFree(title);
					return GL_TRUE;
				}
			}
			else if (crStrcmp(title, titlePattern) == 0) {
				crFree(titlePattern);
				crFree(title);
				return GL_TRUE;
			}
			crFree(title);
		}
		crFree(titlePattern);
	}
	else {
		int i;
		CRASSERT(MinChromiumWidth == 0);
		CRASSERT(MinChromiumHeight == 0);
		CRASSERT(ChromiumWindowTitle == NULL);
		/* User hasn't specified a width/height or window title.
		 * We'll use chromium for this window (and context) if no other is.
		 */
		for (i = 0; i < CR_MAX_CONTEXTS; i++) {
			if (Context[i].inUse && Context[i].type == CHROMIUM) {
				return GL_FALSE;
			}

		}
		return GL_TRUE;  /* use Chromium! */
	}
	return GL_FALSE; /* never get here, silence warning */
}



#ifdef WINDOWS
BOOL WINAPI stubMakeCurrent( HDC drawable, HGLRC context )
#else
Bool stubMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext context )
#endif
{
	const GLint i = (GLint) (long) context - 500; /* See magic number above */
	int retVal;

	/*
	fprintf(stderr, "*** %s(context = %d, i = %d, draw = %d)\n", __FUNCTION__, (int) context, i, (int) drawable);
	*/

	CRASSERT(i >= 0);
	CRASSERT(i < CR_MAX_CONTEXTS);

	if (Context[i].type == UNDECIDED) {
		/* Here's where we really create contexts */
#ifdef WINDOWS
		if (UseChromium(drawable)) {
#else
		if (UseChromium(dpy, drawable)) {
#endif
			 /*fprintf(stderr,"---------UseChromium(%d) yes  visual 0x%x\n", i, DesiredVisual);*/
#ifdef WINDOWS
			Context[i].stubContext = stub_spu->dispatch_table.CreateContext( drawable, DesiredVisual );
#else
			Context[i].stubContext = stub_spu->dispatch_table.CreateContext( dpy, DesiredVisual );
#endif
			Context[i].type = CHROMIUM;
		}
		else {
			/*fprintf(stderr, "---------UseChromium(%d) no\n", i);*/
#ifdef WINDOWS
			if (!InstantiateNativeContext(drawable, i))
#else
			if (!InstantiateNativeContext(dpy, i))
#endif
				return 0; /* false */
			Context[i].type = NATIVE;
		}
	}

	/* If the context is a native context, call the native wgl/glXMakeCurrent
	 * function.
	 */
	if (Context[i].type == NATIVE) {
#ifdef WINDOWS
		retVal = (int) glinterface.wglMakeCurrent( drawable, Context[i].hglrc );
#else
		retVal = (int) glinterface.glXMakeCurrent( dpy, drawable,
																							 Context[i].glxContext );
#endif
	}
	else {
		/* let the pack SPU or tilesort SPU, etc handle this */
		GLint stubCtx = Context[i].stubContext;
		CRASSERT(Context[i].type == CHROMIUM);
		CRASSERT(stubCtx >= 0);

		if (Context[i].currentDrawable && Context[i].currentDrawable != drawable) {
			crWarning("Can't rebind context %d to a different window", i);
			retVal = 0;
		}
		else {
#ifdef WINDOWS
			stub_spu->dispatch_table.MakeCurrent( NULL, (GLint) drawable, stubCtx );
#else
			stub_spu->dispatch_table.MakeCurrent( dpy, (GLint) drawable, stubCtx );
#endif
			retVal = 1;
		}
	}

	Context[i].currentDrawable = drawable;

	if (retVal) {
		/* Now, if we've transitions from Chromium to native rendering, or
		 * vice versa, we have to change all the OpenGL entrypoint pointers.
		 */

		if (Context[i].type == NATIVE && glim.Accum != glnative.Accum) {
			/* Switch to native API */
			/*
			printf("  Switching to native API\n");
			*/
			memcpy(&glim, &glnative, sizeof(SPUDispatchTable));
		}
		else if (Context[i].type == CHROMIUM && glim.Accum != glstub.Accum) {
			/* Switch to stub (SPU) API */
			/*
			printf("  Switching to spu API\n");
			*/
			memcpy(&glim, &glstub, sizeof(SPUDispatchTable));
		}
		else {
			/* no API switch needed */
		}
	}

	/*
	fprintf(stderr, "*** %s() return %d\n", __FUNCTION__, retVal);
	*/

	return retVal;
}



#ifdef WINDOWS
BOOL stubDestroyContext( HGLRC context )
#else
void stubDestroyContext( Display *dpy, GLXContext context )
#endif
{
	const GLint i = (GLint) (long) context - 500; /* See magic number above */

	/*
	fprintf(stderr, "*** %s(%d)\n", __FUNCTION__, (int) context);
	*/

	CRASSERT(i >= 0);
	CRASSERT(i < CR_MAX_CONTEXTS);
	CRASSERT(Context[i].inUse);

	if (Context[i].type == NATIVE) {
#ifdef WINDOWS
		glinterface.wglDeleteContext( Context[i].hglrc );
#else
		glinterface.glXDestroyContext( dpy, Context[i].glxContext );
#endif
	}
	else if (Context[i].type == CHROMIUM) {
		/* Have pack SPU or tilesort SPU, etc. destroy the context */
		GLint stubCtx = Context[i].stubContext;
		CRASSERT(stubCtx >= 0);
#ifdef WINDOWS
		stub_spu->dispatch_table.DestroyContext( NULL, stubCtx );
#else
		stub_spu->dispatch_table.DestroyContext( dpy, stubCtx );
#endif
	}

	Context[i].inUse = GL_FALSE;
	Context[i].type = UNDECIDED;
	Context[i].currentDrawable = 0;
#ifdef WINDOWS
	Context[i].hglrc = 0;
#else
	Context[i].glxContext = 0;
#endif
	Context[i].stubContext = 0;

#ifdef WINDOWS
	return 1; /* true */
#else
	return;
#endif
}



#ifdef WINDOWS
BOOL stubSwapBuffers( HDC drawable )
#else
void stubSwapBuffers( Display *dpy, GLXDrawable drawable )
#endif
{
#ifdef WINDOWS
	BOOL retVal = GL_FALSE;
#endif
	int i;

	/* Determine if this window is being rendered natively or through
	 * Chromium.
	 */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (Context[i].inUse) {
			if (Context[i].currentDrawable == drawable) {
				if (Context[i].type == NATIVE) {
					/*
					printf("*** Swapping native window %d\n", (int) drawable);
					*/
#ifdef WINDOWS
					retVal = glinterface.wglSwapBuffers( drawable );
#else
					glinterface.glXSwapBuffers( dpy, drawable );
#endif
				}
				else if (Context[i].type == CHROMIUM) {
					/* Let the SPU do the buffer swap */
					/*
					printf("*** Swapping chromium window %d\n", (int) drawable);
					*/
					if (crAppDrawCursor) {
						int pos[2];
#ifdef WINDOWS
						GetCursorPosition(drawable, pos);
#else
						GetCursorPosition(dpy, drawable, pos);
#endif
						stub_spu->dispatch_table.ChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, pos);
					}

					stub_spu->dispatch_table.SwapBuffers();
				}
				break; /* out of loop */
			}
		}
	}

#ifdef WINDOWS
	return retVal;
#endif
}
