/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Xatom.h>
#include <sys/time.h>

#include "cr_glwrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "cr_error.h"
#include "cr_string.h"
#include "renderspu.h"

#define WINDOW_NAME render_spu.window_title


static Colormap 
GetShareableColormap( Display *dpy, XVisualInfo *vi )
{
	Status status;
	XStandardColormap *standardCmaps;
	Colormap cmap;
	int i, numCmaps;

#if defined(__cplusplus) || defined(c_plusplus)
    int localclass = vi->c_class; /* C++ */
#else
    int localclass = vi->class; /* C */
#endif
	if ( localclass != TrueColor )
	{
		crError( "No support for non-TrueColor visuals." );
	}

	status = XmuLookupStandardColormap( dpy, vi->screen, vi->visualid,
			vi->depth, XA_RGB_DEFAULT_MAP,
			False, True );

	if ( status == 1 )
	{
		status = XGetRGBColormaps( dpy, RootWindow( dpy, vi->screen), 
				&standardCmaps, &numCmaps, 
				XA_RGB_DEFAULT_MAP );
		if ( status == 1 )
		{
			for (i = 0 ; i < numCmaps ; i++)
			{
				if (standardCmaps[i].visualid == vi->visualid)
				{
					cmap = standardCmaps[i].colormap;
					XFree( standardCmaps);
					return cmap;
				}
			}
		}
	}

	cmap = XCreateColormap( dpy, RootWindow(dpy, vi->screen), 
			vi->visual, AllocNone );
	return cmap;
}


static int
WaitForMapNotify( Display *display, XEvent *event, char *arg )
{
	(void)display;
	return ( event->type == MapNotify && event->xmap.window == (Window)arg );
}


static int
Attrib( const VisualInfo *visual, int attrib )
{
	int value = 0;
	render_spu.ws.glXGetConfig( visual->dpy, visual->visual, attrib, &value );
	return value;
}


static XVisualInfo *
chooseVisual( Display *dpy, int screen, GLbitfield visAttribs )
{
	XVisualInfo *vis;
	int attribList[100];
	int i = 0;

	CRASSERT(visAttribs & CR_RGB_BIT);  /* anybody need color index */

	attribList[i++] = GLX_RGBA;
	attribList[i++] = GLX_RED_SIZE;
	attribList[i++] = 1;
	attribList[i++] = GLX_GREEN_SIZE;
	attribList[i++] = 1;
	attribList[i++] = GLX_BLUE_SIZE;
	attribList[i++] = 1;

	if (visAttribs & CR_ALPHA_BIT)
	{
		attribList[i++] = GLX_ALPHA_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_DOUBLE_BIT)
		attribList[i++] = GLX_DOUBLEBUFFER;

	if (visAttribs & CR_STEREO_BIT)
		attribList[i++] = GLX_STEREO;

	if (visAttribs & CR_DEPTH_BIT)
	{
		attribList[i++] = GLX_DEPTH_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_STENCIL_BIT)
	{
		attribList[i++] = GLX_STENCIL_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_ACCUM_BIT)
	{
		attribList[i++] = GLX_ACCUM_RED_SIZE;
		attribList[i++] = 1;
		attribList[i++] = GLX_ACCUM_GREEN_SIZE;
		attribList[i++] = 1;
		attribList[i++] = GLX_ACCUM_BLUE_SIZE;
		attribList[i++] = 1;
		if (visAttribs & CR_ALPHA_BIT)
		{
			attribList[i++] = GLX_ACCUM_ALPHA_SIZE;
			attribList[i++] = 1;
		}
	}

	/* XXX add multisample support eventually */

	attribList[i++] = None;

	vis = render_spu.ws.glXChooseVisual( dpy, screen, attribList );
	return vis;
}


GLboolean renderspu_SystemInitVisual( VisualInfo *visual )
{
	int screen;

	CRASSERT(visual);
	if (visual->displayName[0] == 0)
		visual->dpy = XOpenDisplay(NULL);
	else
		visual->dpy = XOpenDisplay(visual->displayName);

	if (!visual->dpy)
	{
		crWarning( "Couldn't initialize the visual because visual->dpy was NULL" );
		return GL_FALSE;
	}

	screen = DefaultScreen(visual->dpy);
	visual->visual = chooseVisual(visual->dpy, screen, visual->visAttribs);
	if (!visual->visual) {
		char s[1000];
		renderspuMakeVisString( visual->visAttribs, s );
		crWarning( "Render SPU: Display %s doesn't have the necessary visual: %s",
						 render_spu.display_string, s );
		XCloseDisplay(visual->dpy);
		return GL_FALSE;
	}

	if ( render_spu.sync )
	{
		crDebug( "Render SPU: Turning on XSynchronize" );
		XSynchronize( visual->dpy, True );
	}

	if ( !render_spu.ws.glXQueryExtension( visual->dpy, NULL, NULL ) )
	{
		crWarning( "Render SPU: Display %s doesn't support GLX", visual->displayName );
		return GL_FALSE;
	}

	crDebug( "Render SPU: Looks like we have GLX" );

	crDebug( "Render SPU: Chose visual id=%ld: RGBA=(%d,%d,%d,%d) Z=%d stencil=%d"
					 " double=%d stereo=%d accum=(%d,%d,%d,%d)",
					 visual->visual->visualid,
					 Attrib( visual, GLX_RED_SIZE ),
					 Attrib( visual, GLX_GREEN_SIZE ),
					 Attrib( visual, GLX_BLUE_SIZE ),
					 Attrib( visual, GLX_ALPHA_SIZE ),
					 Attrib( visual, GLX_DEPTH_SIZE ),
					 Attrib( visual, GLX_STENCIL_SIZE ),
					 Attrib( visual, GLX_DOUBLEBUFFER ),
					 Attrib( visual, GLX_STEREO ),
					 Attrib( visual, GLX_ACCUM_RED_SIZE ),
					 Attrib( visual, GLX_ACCUM_GREEN_SIZE ),
					 Attrib( visual, GLX_ACCUM_BLUE_SIZE ),
					 Attrib( visual, GLX_ACCUM_ALPHA_SIZE )
					 );

	return GL_TRUE;
}


GLboolean renderspu_SystemCreateWindow( VisualInfo *visual, GLboolean showIt, WindowInfo *window )
{
	Display             *dpy;
	Colormap             cmap;
	XSetWindowAttributes swa;
	XSizeHints           hints = {0};
	XEvent               event;
	XTextProperty        text_prop;
	XClassHint          *class_hints = NULL;
	char                *name;
	unsigned long        flags;

	CRASSERT(visual);
	window->visual = visual;
	window->x = render_spu.defaultX;
	window->y = render_spu.defaultY;
	window->width  = render_spu.defaultWidth;
	window->height = render_spu.defaultHeight;

	dpy = visual->dpy;

	if ( render_spu.use_L2 )
	{
		crWarning( "Render SPU: Going fullscreen because we think we're using Lightning-2." );
		render_spu.fullscreen = 1;
	}

	/*
	 * Open the display, check for GLX support
	 */

	/*
	 * Query screen size if we're going full-screen
	 */
	if ( render_spu.fullscreen )
	{
		XWindowAttributes xwa;
		Window root_window;

		/* disable the screensaver */
		XSetScreenSaver( dpy, 0, 0, PreferBlanking,
				AllowExposures );
		crDebug( "Render SPU: Just turned off the screensaver" );

		/* Figure out how big the screen is, and make the window that size */

		root_window = DefaultRootWindow( dpy );
		XGetWindowAttributes( dpy, root_window, &xwa );

		crDebug( "Render SPU: root window=%dx%d",
						 xwa.width, xwa.height );

		window->x = 0;
		window->y = 0;
		window->width  = xwa.width;
		window->height = xwa.height;
	}

	/*
	 * Get a colormap.
	 */
	cmap = GetShareableColormap( dpy, visual->visual );
	if ( !cmap ) {
		crError( "Render SPU: Unable to get a colormap!" );
		return GL_FALSE;
	}

	/*
	 * Create the window
	 */
	swa.colormap     = cmap;
	swa.border_pixel = 0;
	swa.event_mask   = ExposureMask | StructureNotifyMask;

	flags = CWBorderPixel | CWColormap | CWEventMask;

	if (render_spu.fullscreen)
	{
		swa.override_redirect = True;
		flags |= CWOverrideRedirect;
	}

	if ( window->window ) {
		/* destroy the old one */
		XDestroyWindow( dpy, window->window );
	}
	window->window = XCreateWindow( dpy,
																	RootWindow( dpy, 
																							visual->visual->screen ),
																	window->x,
																	window->y,
																	window->width,
																	window->height,
																	0, visual->visual->depth,
																	InputOutput,
																	visual->visual->visual,
																	flags, &swa );

	if (!window->window) {
		crWarning( "Render SPU: unable to create window" );
		return GL_FALSE;
	}

	if ( render_spu.fullscreen )
	{
		/* Make a clear cursor to get rid of the monitor cursor */
		Pixmap pixmap;
		Cursor cursor;
		XColor color;
		char   clear_bits[32];

		memset( clear_bits, 0, sizeof(clear_bits) );

		pixmap = XCreatePixmapFromBitmapData( dpy, 
				window->window,
				clear_bits, 16, 16, 1, 0, 1 );
		cursor = XCreatePixmapCursor( dpy, pixmap, pixmap,
				&color, &color, 8, 8 );
		XDefineCursor( dpy, window->window, cursor );

		XFreePixmap( dpy, pixmap );
	}

	crDebug( "Render SPU: Created the window on display %s",
			 visual->displayName ? visual->displayName : "(default)" );
	hints.x = window->x;
	hints.y = window->y;
	hints.width = window->width;
	hints.height = window->height;
	hints.min_width = hints.width;
	hints.min_height = hints.height;
	hints.max_width = hints.width;
	hints.max_height = hints.height;
	hints.flags = USPosition | USSize | PMinSize | PMaxSize;
	XSetStandardProperties( dpy, window->window,
			WINDOW_NAME, WINDOW_NAME,
			None, NULL, 0, &hints );
#if 1
	/* New item!  This is needed so that the sgimouse server can find
	 * the crDebug window. 
	 */
	name = WINDOW_NAME;
	XStringListToTextProperty( &name, 1, &text_prop );
	XSetWMName( dpy, window->window, &text_prop );
#endif
	class_hints = XAllocClassHint( );
	class_hints->res_name = crStrdup( "foo" );
	class_hints->res_class = crStrdup( "Chromium" );
	XSetClassHint( dpy, window->window, class_hints );
	free( class_hints->res_name );
	free( class_hints->res_class );
	XFree( class_hints );

	crDebug( "Render SPU: About to make current to the context" );

	if (showIt) {
		XMapWindow( dpy, window->window );
		XIfEvent( dpy, &event, WaitForMapNotify, 
							(char *) window->window );
	}

#if 0
	/* Note: There is a nasty bug somewhere in glXMakeCurrent() for
		 the 0.9.5 version of the NVIDIA OpenGL drivers, and has been
		 observed under both RedHat 6.2 (running a 2.2.16-3 kernel) and
		 RedHat 7.0 (running the stock 2.2.16-22 kernel).  The bug
		 manifests itself as a kernel wedge requiring the machine to be
		 power-cycled (hard reset).  In both cases we had built the
		 NVIDIA kernel module from source, because there was no
		 appropriate binary release. */
	crDebug( "*** You are screwed, this will "
			"almost certainly wedge the machine." );
#endif

#if 0
	if ( !render_spu.ws.glXMakeCurrent( dpy, window->window, 
				window->context ) )
	{
		crError( "Error making current" );
		return GL_FALSE;
	}
	crDebug( "Made current to the context" );
#endif

#if 0
	CRASSERT(render_spu.ws.glGetString);
	crDebug( "GL_VENDOR:   %s", render_spu.ws.glGetString( GL_VENDOR ) );
	crDebug( "GL_RENDERER: %s", render_spu.ws.glGetString( GL_RENDERER ) );
	crDebug( "GL_VERSION:  %s", render_spu.ws.glGetString( GL_VERSION ) );
#endif

	/*
	 * End GLX code
	 */

	crDebug( "Render SPU: actual window x, y, width, height: %d, %d, %d, %d",
					 window->x, window->y, window->width, window->height );

	return GL_TRUE;
}


void renderspu_SystemDestroyWindow( WindowInfo *window )
{
	CRASSERT(window);
	CRASSERT(window->visual);
	XDestroyWindow(window->visual->dpy, window->window);
	window->visual = NULL;
	window->window = 0;
	window->width = window->height = 0;
}


GLboolean renderspu_SystemCreateContext( VisualInfo *visual, ContextInfo *context )
{
	Bool is_direct;

	CRASSERT(visual);
	CRASSERT(context);

	context->visual = visual;

	context->context = render_spu.ws.glXCreateContext( visual->dpy, 
																										 visual->visual,
																										 NULL,
																										 render_spu.try_direct );
	if (!context->context) {
		crError( "Render SPU: Couldn't create rendering context" ); 
		return GL_FALSE;
	}

	is_direct = render_spu.ws.glXIsDirect( visual->dpy, context->context );
	crDebug( "Render SPU: Created a context (%s)",
			is_direct ? "direct" : "indirect" );

	if ( render_spu.force_direct && !is_direct )
	{
		crError( "Render SPU: Direct rendering not possible." );
		return GL_FALSE;
	}

	return GL_TRUE;
}


void renderspu_SystemDestroyContext( ContextInfo *context )
{
	render_spu.ws.glXDestroyContext( context->visual->dpy, context->context );
	context->visual = NULL;
	context->context = 0;
}


void renderspu_SystemMakeCurrent( ThreadInfo *thread, WindowInfo *window, ContextInfo *context )
{
	CRASSERT(render_spu.ws.glXMakeCurrent);

	if (window && context) {
		if (window->visual != context->visual) {
			/*
			 * XXX have to revisit this issue!!!
			 *
			 * But for now we destroy the current window
			 * and re-create it with the context's visual abilities
			 */
			renderspu_SystemDestroyWindow( window );
			renderspu_SystemCreateWindow( context->visual, GL_FALSE, window );
			/*
			crError("In renderspu_SystemMakeCurrent() window and context"
							" weren't created with same visual!");
			*/
		}

		CRASSERT(window->window);
		CRASSERT(context->context);

		if (thread)
			thread->dpy = window->visual->dpy;

		render_spu.ws.glXMakeCurrent( window->visual->dpy,
					window->window, context->context );
	
	} else if (thread && thread->dpy) {
		render_spu.ws.glXMakeCurrent( thread->dpy, 0, 0 );
	}
}


void renderspu_SystemWindowSize( WindowInfo *window, int w, int h )
{
	CRASSERT(window);
	CRASSERT(window->visual);
	XResizeWindow(window->visual->dpy, window->window, w, h);
}


void renderspu_SystemWindowPosition( WindowInfo *window, int x, int y )
{
	CRASSERT(window);
	CRASSERT(window->visual);
	XMoveWindow(window->visual->dpy, window->window, x, y);
}


/* Either show or hide the render SPU's window. */
void renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt )
{
	if ( window->visual->dpy && window->window )
	{
		if (showIt)
		{
			XEvent event;
			XMapWindow( window->visual->dpy, window->window );
			XIfEvent( window->visual->dpy, &event, WaitForMapNotify, 
								(char *) window->window );
		}
		else
		{
			XUnmapWindow( window->visual->dpy, window->window );
		}
	}
}
