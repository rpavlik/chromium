/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/**
 * \mainpage OpenGL_stub 
 *
 * \section OpenGL_stubIntroduction Introduction
 *
 * Chromium consists of all the top-level files in the cr
 * directory.  The OpenGL_stub module basically takes care of API dispatch,
 * and OpenGL state management.
 *
 */

/**
 * This file manages OpenGL rendering contexts in the faker library.
 * The big issue is switching between Chromium and native GL context
 * management.  This is where we support multiple client OpenGL
 * windows.  Typically, one window is handled by Chromium while any
 * other windows are handled by the native OpenGL library.
 */

#include "chromium.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "stub.h"


#if !( defined(WINDOWS) || defined(DARWIN) )

/**
 * Get the display string for the given display pointer.
 * Never return just ":0.0".  In that case, prefix with our host name.
 */
static void
stubGetDisplayString( Display *dpy, char *nameResult, int maxResult )
{
	const char *dpyName = DisplayString(dpy);
	char host[1000];
#if 0
	if (dpyName[0] == ':')
	{
		crGetHostname(host, 1000);
	}
	else
#endif
	{
	  host[0] = 0;
	}
	if (crStrlen(host) + crStrlen(dpyName) >= maxResult - 1)
	{
		/* return null string */
		crWarning("Very long host / display name string in stubDisplayString!");
		nameResult[0] = 0;
	}
	else
	{
		/* return host concatenated with dpyName */
		crStrcpy(nameResult, host);
		crStrcat(nameResult, dpyName);
	}
}
#endif


/**
 * This function should be called from MakeCurrent().  It'll detect if
 * we're in a multi-thread situation, and do the right thing for dispatch.
 */
#ifdef CHROMIUM_THREADSAFE
static void
stubCheckMultithread( void )
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
		crSPUCopyDispatchTable(&glim, &stubThreadsafeDispatch);
	}
}
#endif


/**
 * Install the given dispatch table as the table used for all gl* calls.
 */
static void
stubSetDispatch( SPUDispatchTable *table )
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
		/* This conditional is an optimization to try to avoid unnecessary
		 * copying.  It seems to work with atlantis, multiwin, etc. but
		 * _could_ be a problem. (Brian)
		 */
		if (glim.copy_of != table->copy_of)
			crSPUCopyDispatchTable(&glim, table);
	}
}


/**
 * Create a new _Chromium_ window, not GLX, WGL or CGL.
 */
GLint
stubNewWindow( const char *dpyName, GLint visBits )
{
	WindowInfo *winInfo;
	GLint spuWin, size[2];
	
	spuWin = stub.spu->dispatch_table.WindowCreate( dpyName, visBits );
	if (spuWin < 0) {
		 return -1;
	}
		 
	winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!winInfo) {
		stub.spu->dispatch_table.WindowDestroy(spuWin);
		return -1;
	}

	winInfo->type = CHROMIUM;

	/* Ask the head SPU for the initial window size */
	size[0] = size[1] = 0;
	stub.spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, 0, GL_INT, 2, size);
	if (size[0] == 0 && size[1] == 0) {
		/* use some reasonable defaults */
		size[0] = size[1] = 512;
	}
	winInfo->width = size[0];
	winInfo->height = size[1];

	if (!dpyName)
		dpyName = "";

	crStrncpy(winInfo->dpyName, dpyName, MAX_DPY_NAME);
	winInfo->dpyName[MAX_DPY_NAME-1] = 0;

	/* Use spuWin as the hash table index and GLX/WGL handle*/
#ifdef WINDOWS
	winInfo->drawable = (HDC) spuWin;
#elif defined(DARWIN)
	winInfo->drawable = (WindowRef) spuWin;
#else
	winInfo->drawable = (GLXDrawable) spuWin;
#endif
	winInfo->spuWindow = spuWin;

	crHashtableAdd(stub.windowTable, (unsigned int) spuWin, winInfo);

	return spuWin;
}


/**
 * Given a Windows HDC or GLX Drawable, return the corresponding
 * WindowInfo structure.  Create a new one if needed.
 */
#ifdef WINDOWS
WindowInfo *
stubGetWindowInfo( HDC drawable )
#elif defined(DARWIN)
WindowInfo *
stubGetWindowInfo( WindowRef drawable )
#else
WindowInfo *
stubGetWindowInfo( Display *dpy, GLXDrawable drawable )
#endif
{
	WindowInfo *winInfo = (WindowInfo *) crHashtableSearch(stub.windowTable, (unsigned int) drawable);
	if (!winInfo) {
		winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
		if (!winInfo)
			return NULL;
#if !( defined(WINDOWS) || defined(DARWIN) )
		crStrncpy(winInfo->dpyName, DisplayString(dpy), MAX_DPY_NAME);
		winInfo->dpyName[MAX_DPY_NAME-1] = 0;
#endif
		winInfo->drawable = drawable;
		winInfo->type = UNDECIDED;
		winInfo->spuWindow = -1;
#if !( defined(WINDOWS) || defined(DARWIN) )
		winInfo->dpy = dpy;
#endif
		crHashtableAdd(stub.windowTable, (unsigned int) drawable, winInfo);
	}
	return winInfo;
}


/**
 * Allocate a new ContextInfo object, initialize it, put it into the
 * context hash table.  If type==CHROMIUM, call the head SPU's
 * CreateContext() function too.
 */
ContextInfo *
stubNewContext( const char *dpyName, GLint visBits, ContextType type )
{
	GLint spuContext = -1;
	ContextInfo *context;

	if (type == CHROMIUM) {
		spuContext = stub.spu->dispatch_table.CreateContext(dpyName, visBits);
		if (spuContext < 0)
			return NULL;
	}

	context = crCalloc(sizeof(ContextInfo));
	if (!context) {
		stub.spu->dispatch_table.DestroyContext(spuContext);
		return NULL;
	}

	if (!dpyName)
		dpyName = "";

	context->id = stub.freeContextNumber++;
	context->type = type;
	context->spuContext = spuContext;
	context->visBits = visBits;
	context->currentDrawable = NULL;
	crStrncpy(context->dpyName, dpyName, MAX_DPY_NAME);
	context->dpyName[MAX_DPY_NAME-1] = 0;

	crHashtableAdd(stub.contextTable, context->id, (void *) context);

	return context;
}


/**
 * Called via glXCreateCurrent() , wglCreateCurrent() or CGLCreatContext().
 * Allocate a ContextInfo object and initialize its type to UNDECIDED.
 * Later, in MakeCurrent, we'll decide (by examining the window size and
 * title) whether to use a Chromium context or native GLX/WGL/CGL context.
 */
#ifdef WINDOWS
HGLRC
stubCreateContext( HDC hdc )
#elif defined(DARWIN)
CGLError
stubCreateContext( CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx )
#else
GLXContext
stubCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct )
#endif
{
	char dpyName[MAX_DPY_NAME];
	ContextInfo *context;

	CRASSERT(stub.contextTable);

	/* 
	 * Pull apart the context's requested visual information
	 * and select the correct CR_*_BIT's. The RenderSPU
	 * will do the right thing and select the appropriate
	 * visual at its node.
	 *
	 * NOTE: We OR just in case an application has called
	 * glXChooseVisual to select its desiredflags, and we honour 
	 * them!
	 *
	 * NOTE: We can only.... do this with a native renderer...
	 */
	
#ifdef WINDOWS
	sprintf(dpyName, "%d", hdc);
	if (stub.haveNativeOpenGL)
		stub.desiredVisual |= FindVisualInfo( hdc );
#elif defined(DARWIN)
	dpyName[0] = '\0';
	if( stub.haveNativeOpenGL )
		stub.desiredVisual |= FindVisualInfo( pix );
#else
	stubGetDisplayString(dpy, dpyName, MAX_DPY_NAME);
	if (stub.haveNativeOpenGL) {
		int foo, bar;
		if (stub.wsInterface.glXQueryExtension(dpy, &foo, &bar)) {
			stub.desiredVisual |= FindVisualInfo( dpy, vis );
		}
	}
#endif

	context = stubNewContext(dpyName, stub.desiredVisual, UNDECIDED);
	if (!context)
		return 0;

#ifndef WINDOWS
#ifndef DARWIN
	context->dpy = dpy;
	context->visual = vis;
	context->direct = direct;
#endif
	context->share = (ContextInfo *) crHashtableSearch(stub.contextTable, (unsigned long) share);
#endif

#ifdef WINDOWS
	return (HGLRC) context->id;
#elif defined(DARWIN)
	if( stub.haveNativeOpenGL )
		stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFADisplayMask, &context->disp_mask );
	else
		context->disp_mask = 0;
	*ctx = (CGLContextObj) context->id;
	return noErr;
#else
	return (GLXContext) context->id;
#endif
}


#ifdef DARWIN

#define SET_ATTR(l,i,a)		( (l)[(i)++] = (a) )
#define SET_ATTR_V(l,i,a,v) ( SET_ATTR(l,i,a), SET_ATTR(l,i,v) )

void stubSetPFA( ContextInfo *ctx, CGLPixelFormatAttribute *attribs, int size, GLint *num ) {
	GLuint visual = ctx->visBits;
	int i = 0;

	CRASSERT(visual & CR_RGB_BIT);

	SET_ATTR_V(attribs, i, kCGLPFAColorSize, 8);

	if( visual & CR_DEPTH_BIT )
		SET_ATTR_V(attribs, i, kCGLPFADepthSize, 16);

	if( visual & CR_ACCUM_BIT )
		SET_ATTR_V(attribs, i, kCGLPFAAccumSize, 1);

	if( visual & CR_STENCIL_BIT )
		SET_ATTR_V(attribs, i, kCGLPFAStencilSize, 1);

	if( visual & CR_ALPHA_BIT )
		SET_ATTR_V(attribs, i, kCGLPFAAlphaSize, 1);

	if( visual & CR_DOUBLE_BIT )
		SET_ATTR(attribs, i, kCGLPFADoubleBuffer);

	if( visual & CR_STEREO_BIT )
		SET_ATTR(attribs, i, kCGLPFAStereo);

/*	SET_ATTR_V(attribs, i, kCGLPFASampleBuffers, 1);
	SET_ATTR_V(attribs, i, kCGLPFASamples, 0);
	SET_ATTR_V(attribs, i, kCGLPFADisplayMask, 0);	*/
	SET_ATTR(attribs, i, kCGLPFABackingStore);
	SET_ATTR(attribs, i, kCGLPFAWindow);
	SET_ATTR_V(attribs, i, kCGLPFADisplayMask, ctx->disp_mask);

	SET_ATTR(attribs, i, NULL);

	*num = i;
}

#endif

/**
 * This creates a native GLX/WGL context.
 */
static GLboolean
InstantiateNativeContext( WindowInfo *window, ContextInfo *context )
{
#ifdef WINDOWS
	context->hglrc = stub.wsInterface.wglCreateContext( window->drawable );
	return context->hglrc ? GL_TRUE : GL_FALSE;
#elif defined(DARWIN)
	CGLContextObj shareCtx = NULL;
	CGLPixelFormatObj pix;
	long npix;

	CGLPixelFormatAttribute attribs[16];
	GLint ind = 0;

	if( context->share ) {
		if( context->cglc != context->share->cglc ) {
			crWarning("CGLCreateContext() is trying to share a non-existant "
					  "CGL context.  Setting share context to zero.");
			shareCtx = 0;
		}
		else
			shareCtx = context->cglc;
	}

	/* XXX \todo We need to get this to work (?) */

	stubSetPFA( context, attribs, 16, &ind );

	stub.wsInterface.CGLChoosePixelFormat( attribs, &pix, &npix );
	stub.wsInterface.CGLCreateContext( pix, shareCtx, &context->cglc );
	if( !context->cglc )
		crError("InstantiateNativeContext: Couldn't Create the context!");

	stub.wsInterface.CGLDestroyPixelFormat( pix );

	if( context->parambits ) {
		/* Set the delayed parameters */
		if( context->parambits & VISBIT_SWAP_RECT )
			stub.wsInterface.CGLSetParameter( context->cglc, kCGLCPSwapRectangle, context->swap_rect );

		if( context->parambits & VISBIT_SWAP_INTERVAL )
			stub.wsInterface.CGLSetParameter( context->cglc, kCGLCPSwapInterval, &(context->swap_interval) );

		if( context->parambits & VISBIT_CLIENT_STORAGE )
			stub.wsInterface.CGLSetParameter( context->cglc, kCGLCPClientStorage, &(context->client_storage) );
		
		context->parambits = 0;
	}
	
	return context->cglc ? GL_TRUE : GL_FALSE;
#else
	GLXContext shareCtx = 0;

	/* sort out context sharing here */
	if (context->share) {
			if (context->glxContext != context->share->glxContext) {
					crWarning("glXCreateContext() is trying to share a non-existant "
										"GLX context.  Setting share context to zero.");
					shareCtx = 0;
			}
			else {
					shareCtx = context->glxContext;
			}
	}

	context->glxContext = stub.wsInterface.glXCreateContext( window->dpy,
				 context->visual, shareCtx, context->direct );

	return context->glxContext ? GL_TRUE : GL_FALSE;
#endif
}


/**
 * Utility functions to get window size and titlebar text.
 */
#ifdef WINDOWS

void
stubGetWindowGeometry( const WindowInfo *window, int *x, int *y,
											 unsigned int *w, unsigned int *h )
{
	RECT rect;
	HWND hwnd;

	if (!window->drawable) {
		*w = *h = 0;
		return;
	}

	hwnd = WindowFromDC( window->drawable );

	if (!hwnd) {
		*w = 0;
		*h = 0;
	}
	else {
		GetClientRect( hwnd, &rect );
		*x = rect.left;
		*y = rect.top;
		*w = rect.right - rect.left;
		*h = rect.bottom - rect.top;
	}
}

static void
GetWindowTitle( const WindowInfo *window, char *title )
{
	HWND hwnd;
	/* XXX - we don't handle recurseUp */
	hwnd = WindowFromDC( window->drawable );
	if (hwnd)
		GetWindowText(hwnd, title, 100);
	else
		title[0] = 0;
}

static void
GetCursorPosition( const WindowInfo *window, int pos[2] )
{
	RECT rect;
	POINT point;
	GLint size[2], x, y;
	unsigned int NativeHeight, NativeWidth, ChromiumHeight, ChromiumWidth;
	float WidthRatio, HeightRatio;
	static int DebugFlag = 0;
	
	// apparently the "window" parameter passed to this 
	// function contains the native window information
	HWND NATIVEhwnd = WindowFromDC( window->drawable );	

	// get the native window's height and width
	stubGetWindowGeometry(window, &x, &y, &NativeWidth, &NativeHeight);

	// get the spu window's height and width
	stub.spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, window->spuWindow, GL_INT, 2, size);
	ChromiumWidth = size[0];
	ChromiumHeight = size[1];

	// get the ratio of the size of the native window to the cr window
	WidthRatio = (float)ChromiumWidth / (float)NativeWidth;
	HeightRatio = (float)ChromiumHeight / (float)NativeHeight;
	
	// output some debug information at the beginning
	if(DebugFlag)
	{
		DebugFlag = 0;
		crDebug("Native Window Handle = %d", NATIVEhwnd);
		crDebug("Native Width = %i", NativeWidth);
		crDebug("Native Height = %i", NativeHeight);
		crDebug("Chromium Width = %i", ChromiumWidth);
		crDebug("Chromium Height = %i", ChromiumHeight);
	}
		
	if (NATIVEhwnd) 
	{ 
		GetClientRect( NATIVEhwnd, &rect );
		GetCursorPos (&point);
		
		// make sure these coordinates are relative to the native window,
		// not the whole desktop
		ScreenToClient(NATIVEhwnd, &point);

		// calculate the new position of the virtual cursor
		pos[0] = (int)(point.x * WidthRatio);
		pos[1] = (int)((NativeHeight - point.y) * HeightRatio);
	}
	else 
	{
		pos[0] = 0;
		pos[1] = 0;
	}
}

#elif defined(DARWIN)

void
stubGetWindowGeometry( const WindowInfo *window, int *x, int *y, unsigned int *w, unsigned int *h )
{
	Rect rect;

#if 1
	GetWindowBounds( window->drawable, kWindowStructureRgn, &rect );
#else
	GetWindowPortBounds( window->drawable, &rect );
#endif

	*x = rect.left;
	*y = rect.top;
	*w = rect.right  - rect.left;
	*h = rect.bottom - rect.top;
}


static void
GetWindowTitle( const WindowInfo *window, char *title )
{
	/* we dont have a DC, just a window :)
	 * XXX figure this out plz
	 */
	if( window->drawable ) {
		GetWTitle( window->drawable, title );
	} else
		title[0] = 0;
}

static void
GetCursorPosition( const WindowInfo *window, int pos[2] )
{
	GrafPtr save, port;
	Point pt;
	Rect rect = {0,0,0,0};

	WindowRef win = FrontWindow();

	GetPort( &save );
	port = GetWindowPort( win );
	GetPortBounds( port, &rect );
	SetPort( port );
	GetMouse( &pt );	// this gets local
	SetPort( save );

	pos[0] = pt.h;
	pos[1] = (rect.bottom - rect.top) - pt.v;
}

#else

void
stubGetWindowGeometry( const WindowInfo *window, int *x, int *y,
											 unsigned int *w, unsigned int *h )
{
	Window root, child;
	unsigned int border, depth;
	if (!window
			|| !window->dpy
			|| !window->drawable
			|| !XGetGeometry(window->dpy, window->drawable, &root,
											 x, y, w, h, &border, &depth)
			|| !XTranslateCoordinates(window->dpy, window->drawable, root,
																*x, *y, x, y, &child)) {
		*x = *y = 0;
		*w = *h = 0;
	}
}

static char *
GetWindowTitleHelper( Display *dpy, Window window, GLboolean recurseUp )
{
	while (1) {
		char *name;
		if (!XFetchName(dpy, window, &name))
			return NULL;
		if (name[0]) {
			return name;
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

static void
GetWindowTitle( const WindowInfo *window, char *title )
{
	char *t = GetWindowTitleHelper(window->dpy, window->drawable, GL_TRUE);
	if (t) {
		crStrcpy(title, t);
		XFree(t);
	}
	else {
		title[0] = 0;
	}
}


/**
 *Return current cursor position in local window coords.
 */
static void
GetCursorPosition( const WindowInfo *window, int pos[2] )
{
	int rootX, rootY;
	Window root, child;
	unsigned int mask;
	int x, y;
	Bool q = XQueryPointer(window->dpy, window->drawable, &root, &child,
												 &rootX, &rootY, &pos[0], &pos[1], &mask);
	if (q) {
		unsigned int w, h;
		stubGetWindowGeometry( window, &x, &y, &w, &h );
		/* invert Y */
		pos[1] = (int) h - pos[1] - 1;
	}
	else {
		pos[0] = pos[1] = 0;
	}
}

#endif


/**
 * This function is called by MakeCurrent() and determines whether or
 * not a new rendering context should be bound to Chromium or the native
 * OpenGL.
 * \return  GL_FALSE if native OpenGL should be used, or GL_TRUE if Chromium
 *          should be used.
 */
static GLboolean
stubCheckUseChromium( WindowInfo *window )
{
	int x, y;
	unsigned int w, h;

#ifdef DARWIN
	if( !window )
		return GL_FALSE;

	if( !crGetenv("CR_FORCE_CHROMIUM") )
		return GL_FALSE;
#endif

	/* If the provided window is CHROMIUM, we're clearly intended
	 * to create a CHROMIUM context.
	 */
	if (window->type == CHROMIUM)
		return GL_TRUE;

	if (stub.ignoreFreeglutMenus) {
		const char *glutMenuTitle = "freeglut menu";
		char title[1000];
		GetWindowTitle(window, title);
		if (crStrcmp(title, glutMenuTitle) == 0) {
			crDebug("GL faker: Ignoring freeglut menu window");
			return GL_FALSE;
		}
	}

	/*  If the user's specified a window count for Chromium, see if
		*  this window satisfies that criterium.
		*/
	stub.matchChromiumWindowCounter++;
	if (stub.matchChromiumWindowCount > 0) {
		if (stub.matchChromiumWindowCounter != stub.matchChromiumWindowCount) {
			crDebug("Using native GL, app window doesn't meet match_window_count");
			return GL_FALSE;
		}
	}

	/* If the user's specified a window list to ignore, see if this
	 * window satisfies that criterium.
	 */
	if (stub.matchChromiumWindowID) {
		GLuint i;

		for (i = 0; i <= stub.numIgnoreWindowID; i++) {
			if (stub.matchChromiumWindowID[i] == stub.matchChromiumWindowCounter) {
				crDebug("Ignore window ID %d, using native GL", stub.matchChromiumWindowID[i]);
				return GL_FALSE;
			}
		}
	}

	/* If the user's specified a minimum window size for Chromium, see if
	 * this window satisfies that criterium. 
	 */
	if (stub.minChromiumWindowWidth > 0 && 
	    stub.minChromiumWindowHeight > 0) {
		stubGetWindowGeometry( window, &x, &y, &w, &h );
		if (w >= stub.minChromiumWindowWidth && 
		    h >= stub.minChromiumWindowHeight) {

			/* Check for maximum sized window now too */
			if (stub.maxChromiumWindowWidth && 
			    stub.maxChromiumWindowHeight) {
				if (w < stub.maxChromiumWindowWidth &&
				    h < stub.maxChromiumWindowHeight)
					return GL_TRUE;
				else 
					return GL_FALSE;
			}

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
		char title[1000];
		char *titlePattern;
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

		GetWindowTitle( window, title );
		if (title[0]) {
			if (wildcard) {
				if (crStrstr(title, titlePattern)) {
					crFree(titlePattern);
					return GL_TRUE;
				}
			}
			else if (crStrcmp(title, titlePattern) == 0) {
				crFree(titlePattern);
				return GL_TRUE;
			}
		}
		crFree(titlePattern);
		crDebug("Using native GL, app window title doesn't match match_window_title string (\"%s\" != \"%s\")", title, stub.matchWindowTitle);
		return GL_FALSE;
	}

	/* Window title and size don't matter */
	CRASSERT(stub.minChromiumWindowWidth == 0);
	CRASSERT(stub.minChromiumWindowHeight == 0);
	CRASSERT(stub.matchWindowTitle == NULL);

	/* User hasn't specified a width/height or window title.
	 * We'll use chromium for this window (and context) if no other is.
	 */

	return GL_TRUE;  /* use Chromium! */
}


GLboolean
#ifdef DARWIN
/*
 * 'have_drawable' is a little work-around for CGLSetCurrentContext,
 *  which doesn't come with a drawable
 */
stubMakeCurrent( WindowInfo *window, ContextInfo *context, GLenum drawable_type )
#else
stubMakeCurrent( WindowInfo *window, ContextInfo *context )
#endif
{
	GLboolean retVal;

	/*
	 * Get WindowInfo and ContextInfo pointers.
	 */
#ifdef DARWIN
	if( !context || (drawable_type==DRAW_HAVE && !window) ) {
#else
	if (!context || !window) {
#endif
		if (stub.currentContext)
			stub.currentContext->currentDrawable = NULL;
		if (context)
			context->currentDrawable = NULL;
		stub.currentContext = NULL;
		return GL_TRUE;  /* OK */
	}

#ifdef CHROMIUM_THREADSAFE
	stubCheckMultithread();
#endif


	if (context->type == UNDECIDED) {
		/* Here's where we really create contexts */
#ifdef CHROMIUM_THREADSAFE
		crLockMutex(&stub.mutex);
#endif

#ifdef DARWIN
		if( drawable_type==DRAW_HAVE && stubCheckUseChromium(window) ) {
#else
		if(stubCheckUseChromium(window)) {
#endif
			/*
			 * Create a Chromium context.
			 */
			CRASSERT(stub.spu);
			CRASSERT(stub.spu->dispatch_table.CreateContext);
			context->type = CHROMIUM;

			/* For Darwin, the window should be created first. (shouldn't it?) */
#ifdef DARWIN
			if( window->spuWindow == -1 )
				window->spuWindow = stub.spu->dispatch_table.WindowCreate( window->dpyName, context->visBits );
			context->spuContext = stub.spu->dispatch_table.CreateContext( context->dpyName, context->visBits );
#else
			context->spuContext = stub.spu->dispatch_table.CreateContext( context->dpyName, context->visBits );
			if (window->spuWindow == -1) 
				window->spuWindow = stub.spu->dispatch_table.WindowCreate( window->dpyName, context->visBits );
#endif
		}
		else {
			/*
			 * Create a native OpenGL context.
			 */
			if (!InstantiateNativeContext(window, context))
			{
#ifdef CHROMIUM_THREADSAFE
				crUnlockMutex(&stub.mutex);
#endif
				return 0; /* false */
			}
			context->type = NATIVE;
		}

#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&stub.mutex);
#endif
	}


	if (context->type == NATIVE) {
		/*
		 * Native OpenGL MakeCurrent().
		 */
#ifdef WINDOWS
		retVal = (GLboolean) stub.wsInterface.wglMakeCurrent( window->drawable, context->hglrc );
#elif defined(DARWIN)
		if( drawable_type == DRAW_SET_CURRENT )
			retVal = (stub.wsInterface.CGLSetCurrentContext(context->cglc) == noErr);
		else
			retVal = (stub.wsInterface.CGLSetSurface(context->cglc, context->surf_a, context->surf_b, context->surf_c) == noErr);
#else
		retVal = (GLboolean) stub.wsInterface.glXMakeCurrent( window->dpy, window->drawable, context->glxContext );
#endif
	}
	else {
		/*
		 * SPU chain MakeCurrent().
		 */
		CRASSERT(context->type == CHROMIUM);
		CRASSERT(context->spuContext >= 0);

		if (context->currentDrawable && context->currentDrawable != window)
			crWarning("Rebinding context %p to a different window", context);

		if (window->type == NATIVE) {
			crWarning("Can't rebind a chromium context to a native window\n");
			retVal = 0;
		}
		else {
			if (window->spuWindow == -1)
				window->spuWindow = stub.spu->dispatch_table.WindowCreate( window->dpyName, context->visBits );

			if (window->spuWindow != (GLint)window->drawable)
				 stub.spu->dispatch_table.MakeCurrent( window->spuWindow, (GLint) window->drawable, context->spuContext );
			else
				 stub.spu->dispatch_table.MakeCurrent( window->spuWindow, 0, /* native window handle */ context->spuContext );

			retVal = 1;
		}
	}

#ifdef DARWIN
	if( drawable_type == DRAW_HAVE )
#endif
	window->type = context->type;
	context->currentDrawable = window;
	stub.currentContext = context;

	if (retVal) {
		/* Now, if we've transitions from Chromium to native rendering, or
		 * vice versa, we have to change all the OpenGL entrypoint pointers.
		 */
		if (context->type == NATIVE) {
			/* Switch to native API */
			/*printf("  Switching to native API\n");*/
			stubSetDispatch(&stub.nativeDispatch);
		}
		else if (context->type == CHROMIUM) {
			/* Switch to stub (SPU) API */
			/*printf("  Switching to spu API\n");*/
			stubSetDispatch(&stub.spuDispatch);
		}
		else {
			/* no API switch needed */
		}
	}
#ifdef DARWIN
	if( drawable_type == DRAW_HAVE )
#endif
	if (!window->width && window->type == CHROMIUM) {
		/* Now call Viewport to setup initial parameters */
		int x, y;
		unsigned int winW, winH;

		stubGetWindowGeometry( window, &x, &y, &winW, &winH );

		/* If we're not using GLX/WGL (no app window) we'll always get
		 * a width and height of zero here.  In that case, skip the viewport
		 * call since we're probably using a tilesort SPU with fake_window_dims
		 * which the tilesort SPU will use for the viewport.
		 */
		window->width = winW;
		window->height = winH;
		if (stub.trackWindowSize)
			stub.spuDispatch.WindowSize( window->spuWindow, winW, winH );
		if (winW > 0 && winH > 0)
			stub.spu->dispatch_table.Viewport( 0, 0, winW, winH );
	}

	return retVal;
}



void
stubDestroyContext( unsigned long contextId )
{
	ContextInfo *context;

	context = (ContextInfo *) crHashtableSearch(stub.contextTable, contextId);

	CRASSERT(context);

	if (context->type == NATIVE) {
#ifdef WINDOWS
		stub.wsInterface.wglDeleteContext( context->hglrc );
#elif defined(DARWIN)
		stub.wsInterface.CGLDestroyContext( context->cglc );
#else
		stub.wsInterface.glXDestroyContext( context->dpy, context->glxContext );
#endif
	}
	else if (context->type == CHROMIUM) {
		/* Have pack SPU or tilesort SPU, etc. destroy the context */
		CRASSERT(context->spuContext >= 0);
		stub.spu->dispatch_table.DestroyContext( context->spuContext );
	}

	if (stub.currentContext == context) {
		stub.currentContext = NULL;
	}

	crMemZero(context, sizeof(ContextInfo));  /* just to be safe */
	crHashtableDelete(stub.contextTable, contextId, crFree);
}


#ifdef DARWIN
void
stubSwapContextBuffers( const ContextInfo *context, GLint flags )
{
	if( !context )
		return;

	if( context->type == NATIVE ) {
		stub.wsInterface.CGLFlushDrawable( context->cglc );
	} else if( context->type == CHROMIUM ) {
		if( context->currentDrawable )
			stubSwapBuffers( context->currentDrawable, 0 );
		else
			crDebug("stubSwapContextBuffers: Not sure what to do with chromium context buffers.");
	} else {
		crDebug("Calling SwapContextBuffers on a window we haven't seen before (no-op).");
	}
}
#endif


void
stubSwapBuffers( const WindowInfo *window, GLint flags )
{
	if (!window)
		return;

	/* Determine if this window is being rendered natively or through
	 * Chromium.
	 */

	if (window->type == NATIVE) {
		/*printf("*** Swapping native window %d\n", (int) drawable);*/
#ifdef WINDOWS
		(void) stub.wsInterface.wglSwapBuffers( window->drawable );
#elif defined(DARWIN)
		/* ...is this ok? */
/*		stub.wsInterface.CGLFlushDrawable( context->cglc ); */
		crDebug("stubSwapBuffers: unable to swap (no context!)");
#else
		stub.wsInterface.glXSwapBuffers( window->dpy, window->drawable );
#endif
	}
	else if (window->type == CHROMIUM) {
		/* Let the SPU do the buffer swap */
		/*printf("*** Swapping chromium window %d\n", (int) drawable);*/
		if (stub.appDrawCursor) {
			int pos[2];
			GetCursorPosition(window, pos);
			stub.spu->dispatch_table.ChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, pos);
		}
		stub.spu->dispatch_table.SwapBuffers( window->spuWindow, flags );
	}
	else {
		crDebug("Calling SwapBuffers on a window we haven't seen before (no-op).");
	}
}
