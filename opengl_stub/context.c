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
#include "api_templates.h"



/* To convert array indexes into an easily recognized numbers for debugging. */
#define MAGIC 500

#ifndef WINDOWS
extern int FindVisualInfo( Display *dpy, XVisualInfo *vis);
#endif


/*
 * This function should be called from MakeCurrent().  It'll detect if
 * we're in a multi-thread situation, and do the right thing for dispatch.
 */
#ifdef CHROMIUM_THREADSAFE
static void stubCheckMultithread( void )
{
	static unsigned long knownID;
	static GLboolean firstCall = GL_TRUE;

	if (stub.threadSafe)
		return;  /* nothing new, nothing to do */

	if (firstCall) {
		knownID = crThreadID();
		firstCall = GL_FALSE;
	}
	else if (knownID != crThreadID()) {
		/* going thread-safe now! */
		stub.threadSafe = GL_TRUE;
		memcpy(&glim, &stubThreadsafeDispatch, sizeof(SPUDispatchTable));
	}
}
#endif


/*
 * Install the given dispatch table as the table used for all gl* calls.
 */
static void stubSetDispatch( const SPUDispatchTable *table )
{
	CRASSERT(table);

#ifdef CHROMIUM_THREADSAFE
	/* always set the per-thread dispatch pointer */
	crSetTSD(&stub.dispatchTSD, (void *) table);
	if (stub.threadSafe) {
		/* Do nothing - the thread-safe dispatch functions will call GetTSD()
		 * to get a pointer to the dispatch table, and jump through it.
		 */
	}
	else 
#endif
	{
		/* Single thread mode - just install the caller's dispatch table */
		memcpy(&glim, table, sizeof(SPUDispatchTable));
	}
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
#ifdef WINDOWS
		char dpyName[20];
#else
		const char *dpyName = NULL;
#endif
		StubInit();
		memset(stub.Context, 0, sizeof(stub.Context));
		firstCall = GL_FALSE;

#ifdef WINDOWS
		sprintf(dpyName, "%d", hdc);
		stub.spuWindow = crCreateWindow( (const char *)dpyName, 0 );
#else
		dpyName = DisplayString(dpy);

		/* 
		 * Pull apart the context's requested visual information
		 * and select the correct CR_*_BIT's. The RenderSPU
		 * will do the right thing and select the appropriate
	  	 * visual at it's node.
		 *
		 * NOTE: We OR just in case an application has called
		 * glXChooseVisual to select it's desiredflags, and we honour 
		 * them!
		 */
		stub.desiredVisual |= FindVisualInfo( dpy, vis );

		stub.spuWindow = crCreateWindow( dpyName, stub.desiredVisual );
#endif
	}

	/* Find a free context slot */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (!stub.Context[i].inUse)
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
	stub.Context[i].inUse = GL_TRUE;
	stub.Context[i].type = UNDECIDED;
#ifndef WINDOWS
	stub.Context[i].dpy = dpy;
	stub.Context[i].visual = vis;
	stub.Context[i].direct = direct;
	stub.Context[i].share = share;
#endif
	stub.Context[i].currentDrawable = 0;

	/*
	fprintf(stderr, "*** stubCreateContext return %d\n", i+MAGIC);
	*/

	/* Add a magic number to the index and return it as the context
	 * identifier.
	 */
#ifdef WINDOWS
	return (HGLRC) (i + MAGIC);
#else
	return (GLXContext) (long) (i + MAGIC);
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
	stub.Context[i].hglrc = stub.wsInterface.wglCreateContext( hdc );
	return stub.Context[i].hglrc ? GL_TRUE : GL_FALSE;
#else
	GLXContext shareCtx = 0;

	CRASSERT( i >= 0);
	CRASSERT( i < CR_MAX_CONTEXTS);

	/* sort out context sharing here */
	if (stub.Context[i].share) {
		int iShare = iShare = (int) (long) stub.Context[i].share - MAGIC;
		int j;
		CRASSERT(iShare >= 0);
		CRASSERT(iShare < CR_MAX_CONTEXTS);
		shareCtx = stub.Context[iShare].glxContext;
		/* now check that the share context is a native context */
		for (j = 0; j < CR_MAX_CONTEXTS; j++) {
			if (stub.Context[j].inUse && stub.Context[j].glxContext == shareCtx)
				break;
		}
		if (j == CR_MAX_CONTEXTS) {
			crWarning("glXCreateContext() is trying to share a non-existant GLX context.  Setting share context to zero.");
			shareCtx = 0;
		}
		else if (stub.Context[j].type != NATIVE) {
			crWarning("glXCreateContext() is trying to share a non-native GLX context.  Setting share context to zero.");
			shareCtx = 0;
		}
		else {
			/* sharing should work! */
		}
	}

	/*
	printf("***** calling native glXCreateContext(%p, %p, %d, %d)\n",
			   dpy, stub.Context[i].visual, (int) stub.Context[i].share, (int) stub.Context[i].direct);
	*/

	stub.Context[i].glxContext = stub.wsInterface.glXCreateContext( dpy, stub.Context[i].visual, shareCtx, stub.Context[i].direct );

	/*
	fprintf(stderr, "***** native glXCreateContext returned %d\n", (int) stub.Context[i].glxContext);
	*/

	return stub.Context[i].glxContext ? GL_TRUE : GL_FALSE;
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
		if (stub.Context[i].inUse && stub.Context[i].type == CHROMIUM) {
			if (stub.Context[i].currentDrawable == drawable) {
				return GL_TRUE;
			}
			else {
				return GL_FALSE;
			}
		}
	}

	/* If the user's specified a minimum window size for Chromium, see if
	 * this window satisfies that criterium.
	 */
	if (stub.minChromiumWindowWidth > 0 && stub.minChromiumWindowHeight > 0) {
#ifdef WINDOWS
		GetWindowSize( drawable, &w, &h );
#else
		GetWindowSize( dpy, drawable, &w, &h );
#endif
		if (w >= stub.minChromiumWindowWidth && h >= stub.minChromiumWindowHeight) {
			return GL_TRUE;
		}
		crDebug("Using native GL, app window doesn't meet minimum_window_size");
		return GL_FALSE;
	}
	else if (stub.matchWindowTitle) {
		/* If the user's specified a window title for Chromium, see if this
		 * window satisfies that criterium.
		 */
		GLboolean wildcard = GL_FALSE;
		char *titlePattern, *title;
		int len;
		/* check for leading '*' wildcard */
		if (stub.matchWindowTitle[0] == '*') {
			titlePattern = crStrdup( stub.matchWindowTitle + 1 );
			wildcard = GL_TRUE;
		}
		else {
			titlePattern = crStrdup( stub.matchWindowTitle );
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
		crDebug("Using native GL, app window title doesn't match match_window_title string");
		return GL_FALSE;
	}
	else {
		/* Window title and size don't matter */
		int i;
		CRASSERT(stub.minChromiumWindowWidth == 0);
		CRASSERT(stub.minChromiumWindowHeight == 0);
		CRASSERT(stub.matchWindowTitle == NULL);
		/* User hasn't specified a width/height or window title.
		 * We'll use chromium for this window (and context) if no other is.
		 */
		for (i = 0; i < CR_MAX_CONTEXTS; i++) {
			if (stub.Context[i].inUse && stub.Context[i].type == CHROMIUM) {
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
	const GLint i = (GLint) (long) context - MAGIC;
	int retVal;

	/*
	fprintf(stderr, "*** %s(context = %d, i = %d, draw = %d)\n",
					__FUNCTION__, (int) context, i, (int) drawable);
	*/

	CRASSERT(i >= 0);
	CRASSERT(i < CR_MAX_CONTEXTS);

#ifdef CHROMIUM_THREADSAFE
	stubCheckMultithread();
#endif

	if (stub.Context[i].type == UNDECIDED) {
		/* Here's where we really create contexts */
#ifdef CHROMIUM_THREADSAFE
		crLockMutex(&stub.mutex);
#endif
#ifdef WINDOWS
		if (UseChromium(drawable)) {
			char dpyName[20]; /* Enough to convert pointer */
			sprintf(dpyName, "%d", drawable);
#else
		XSync(dpy, 0); /* sync to force window creation on the server */
		if (UseChromium(dpy, drawable)) {
			const char *dpyName = DisplayString(dpy);
#endif
			/*fprintf(stderr,"---------UseChromium(%d) yes  visual 0x%x\n",
							i, stub.desiredVisual);*/
			CRASSERT(stub.spu);
			CRASSERT(stub.spu->dispatch_table.CreateContext);
			stub.Context[i].stubContext = stub.spu->dispatch_table.CreateContext( dpyName, stub.desiredVisual );
			stub.Context[i].type = CHROMIUM;
		}
		else {
			/*fprintf(stderr, "---------UseChromium(%d) no\n", i);*/
#ifdef WINDOWS
			if (!InstantiateNativeContext(drawable, i))
#else
			if (!InstantiateNativeContext(dpy, i))
#endif
			{
#ifdef CHROMIUM_THREADSAFE
				crUnlockMutex(&stub.mutex);
#endif
				return 0; /* false */
			}
			stub.Context[i].type = NATIVE;
		}
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&stub.mutex);
#endif
	}

	/* If the context is a native context, call the native wgl/glXMakeCurrent
	 * function.
	 */
	if (stub.Context[i].type == NATIVE) {
#ifdef WINDOWS
		retVal = (int) stub.wsInterface.wglMakeCurrent( drawable, stub.Context[i].hglrc );
#else
		retVal = (int) stub.wsInterface.glXMakeCurrent( dpy, drawable,
																							 stub.Context[i].glxContext );
#endif
	}
	else {
		/* let the pack SPU or tilesort SPU, etc handle this */
		GLint stubCtx = stub.Context[i].stubContext;
		CRASSERT(stub.Context[i].type == CHROMIUM);
		CRASSERT(stubCtx >= 0);

		if (stub.Context[i].currentDrawable && stub.Context[i].currentDrawable != drawable) {
			crWarning("Can't rebind context %d to a different window", i);
			retVal = 0;
		}
		else {
			stub.spu->dispatch_table.MakeCurrent( stub.spuWindow, (GLint) drawable, stubCtx );
			retVal = 1;
		}
	}

	stub.Context[i].currentDrawable = drawable;

	if (retVal) {
		/* Now, if we've transitions from Chromium to native rendering, or
		 * vice versa, we have to change all the OpenGL entrypoint pointers.
		 */

		if (stub.Context[i].type == NATIVE /*&& glim.Accum != stub.nativeDispatch.Accum*/) {
			/* Switch to native API */
			/*
			printf("  Switching to native API\n");
			*/
#if 0
			memcpy(&glim, &stub.nativeDispatch, sizeof(SPUDispatchTable));
#else
			stubSetDispatch(&stub.nativeDispatch);
#endif
		}
		else if (stub.Context[i].type == CHROMIUM /*&& glim.Accum != stub.spuDispatch.Accum*/) {
			/* Switch to stub (SPU) API */
			/*
			printf("  Switching to spu API\n");
			*/
#if 0
			memcpy(&glim, &stub.spuDispatch, sizeof(SPUDispatchTable));
#else
			stubSetDispatch(&stub.spuDispatch);
#endif
		}
		else {
			/* no API switch needed */
		}
	}

	/*
	fprintf(stderr, "*** %s() return %d\n", __FUNCTION__, retVal);
	*/

	stub.currentContext = i;

	return retVal;
}



#ifdef WINDOWS
BOOL stubDestroyContext( HGLRC context )
#else
void stubDestroyContext( Display *dpy, GLXContext context )
#endif
{
	const GLint i = (GLint) (long) context - MAGIC;

	/*
	fprintf(stderr, "*** %s(%d)\n", __FUNCTION__, (int) context);
	*/

	CRASSERT(i >= 0);
	CRASSERT(i < CR_MAX_CONTEXTS);
	CRASSERT(stub.Context[i].inUse);

	if (stub.Context[i].type == NATIVE) {
#ifdef WINDOWS
		stub.wsInterface.wglDeleteContext( stub.Context[i].hglrc );
#else
		stub.wsInterface.glXDestroyContext( dpy, stub.Context[i].glxContext );
#endif
	}
	else if (stub.Context[i].type == CHROMIUM) {
		/* Have pack SPU or tilesort SPU, etc. destroy the context */
		GLint stubCtx = stub.Context[i].stubContext;
		CRASSERT(stubCtx >= 0);
		stub.spu->dispatch_table.DestroyContext( stubCtx );
	}

	stub.Context[i].inUse = GL_FALSE;
	stub.Context[i].type = UNDECIDED;
	stub.Context[i].currentDrawable = 0;
#ifdef WINDOWS
	stub.Context[i].hglrc = 0;
#else
	stub.Context[i].glxContext = 0;
#endif
	stub.Context[i].stubContext = 0;

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
		if (stub.Context[i].inUse) {
			if (stub.Context[i].currentDrawable == drawable) {
				if (stub.Context[i].type == NATIVE) {
					/*
					printf("*** Swapping native window %d\n", (int) drawable);
					*/
#ifdef WINDOWS
					retVal = stub.wsInterface.wglSwapBuffers( drawable );
#else
					stub.wsInterface.glXSwapBuffers( dpy, drawable );
#endif
				}
				else if (stub.Context[i].type == CHROMIUM) {
					/* Let the SPU do the buffer swap */
					/*
					printf("*** Swapping chromium window %d\n", (int) drawable);
					*/
					if (stub.appDrawCursor) {
						int pos[2];
#ifdef WINDOWS
						GetCursorPosition(drawable, pos);
#else
						GetCursorPosition(dpy, drawable, pos);
#endif
						stub.spu->dispatch_table.ChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, pos);
					}

					stub.spu->dispatch_table.SwapBuffers( stub.spuWindow, 0 );
				}
				break; /* out of loop */
			}
		}
	}

#ifdef WINDOWS
	return retVal;
#endif
}


/*
 * Return size of window currently bound to the given context.
 */
void stubGetWindowSize( const ContextInfo *ctx,
												unsigned int *w, unsigned int *h )
{
#ifdef WINDOWS
	GetWindowSize(ctx->currentDrawable, w, h);
#else
	GetWindowSize(ctx->dpy, ctx->currentDrawable, w, h);
#endif
}
