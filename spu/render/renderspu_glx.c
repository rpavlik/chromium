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



/* This stuff is from WireGL, and hasn't been changed yet since 
 * I'm developing on Windows. */

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
Attrib( int attrib )
{
	int value = 0;
	render_spu.ws.glXGetConfig( render_spu.dpy, render_spu.visual, attrib, &value );
	return value;
}


static XVisualInfo *chooseVisual( Display *dpy, GLbitfield visAttribs )
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
		attribList[i++] = 8;  /* XXX also try 1-bit stencil */
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

	vis = render_spu.ws.glXChooseVisual( dpy, DefaultScreen(dpy), attribList );
	return vis;
}



/*
 * VERY IMPORTANT: this function may be called more than once
 * (twice, actually) in order to get a window with different visual
 * attributes the second time around.  Make sure we don't re-open the
 * display or anything silly like that.
 */
GLboolean renderspuCreateWindow( GLbitfield visAttribs, GLboolean showIt )
{
	Colormap             cmap;
	XSetWindowAttributes swa;
	XSizeHints           hints = {0};
	XEvent               event;
	XTextProperty        text_prop;
	XClassHint          *class_hints = NULL;
	char                *name;
	Bool                 is_direct;
	unsigned long        flags;

	int actual_window_x = render_spu.window_x;
	int actual_window_y = render_spu.window_y;

	render_spu.actual_window_width  = render_spu.window_width;
	render_spu.actual_window_height = render_spu.window_height;

	if ( render_spu.use_L2 )
	{
		crWarning( "Going fullscreen because we think we're using Lightning-2." );
		render_spu.fullscreen = 1;
	}

	/*
	 * Open the display, check for GLX support
	 */
	if ( !render_spu.dpy )
	{
		render_spu.dpy = XOpenDisplay( render_spu.display_string );
		if ( !render_spu.dpy )
		{
			crError( "Couldn't open display \"%s\"", 
							 XDisplayName( render_spu.display_string ) );
			return GL_FALSE;
		}

		render_spu.display_string = 
			XDisplayName( DisplayString( render_spu.dpy ) );

		crDebug( "Opened the display: %s", 
						 render_spu.display_string );

		if ( render_spu.sync )
		{
			crDebug( "Turning on XSynchronize" );
			XSynchronize( render_spu.dpy, True );
		}

		if ( !render_spu.ws.glXQueryExtension( render_spu.dpy, NULL, NULL ) )
		{
			crError( "Display %s doesn't support GLX", 
							 render_spu.display_string );
			return GL_FALSE;
		}

		crDebug( "Looks like we have GLX" );
	}

	/*
	 * Query screen size if we're going full-screen
	 */
	if ( render_spu.fullscreen )
	{
		XWindowAttributes xwa;
		Window root_window;

		/* disable the screensaver */
		XSetScreenSaver( render_spu.dpy, 0, 0, PreferBlanking,
				AllowExposures );
		crDebug( "Just turned off the screensaver" );

		/* Figure out how big the screen is, and make the window that size */

		root_window = DefaultRootWindow( render_spu.dpy );
		XGetWindowAttributes( render_spu.dpy, root_window, &xwa );

		render_spu.actual_window_width  = xwa.width;
		render_spu.actual_window_height = xwa.height;

		crDebug( "root window=%dx%d",	xwa.width, xwa.height );

		actual_window_x = 0;
		actual_window_y = 0;
	}

	/*
	 * Get the GLX visual.
	 */
	if ( render_spu.visual )
	{
		XFree( render_spu.visual );
		render_spu.visual = NULL;
	}
	render_spu.visual = chooseVisual( render_spu.dpy, visAttribs );
	if ( !render_spu.visual )
	{
		char s[1000];
		renderspuMakeVisString( visAttribs, s );
		crError( "Display %s doesn't have the necessary visual: %s",
						 render_spu.display_string, s );
		return GL_FALSE;
	}

	crDebug( "Chose visual id=%ld: RGBA=(%d,%d,%d,%d) Z=%d stencil=%d double=%d"
					 " stereo=%d accum=(%d,%d,%d,%d)",
					 render_spu.visual->visualid,
					 Attrib( GLX_RED_SIZE ), Attrib( GLX_GREEN_SIZE ),
					 Attrib( GLX_BLUE_SIZE ), Attrib( GLX_ALPHA_SIZE ),
					 Attrib( GLX_DEPTH_SIZE ), Attrib( GLX_STENCIL_SIZE ),
					 Attrib( GLX_DOUBLEBUFFER ), Attrib( GLX_STEREO ),
					 Attrib( GLX_ACCUM_RED_SIZE ), Attrib( GLX_ACCUM_GREEN_SIZE ),
					 Attrib( GLX_ACCUM_BLUE_SIZE ), Attrib( GLX_ACCUM_ALPHA_SIZE )
					 );

	/*
	 * Get a colormap.
	 */
	cmap = GetShareableColormap( render_spu.dpy, render_spu.visual );
	if ( !cmap ) {
		crError( "Unable to get a colormap!" );
		return GL_FALSE;
	}

	/*
	 * Create the GLX rendering context
	 */
	if ( render_spu.context )
	{
		/* free old context */
		render_spu.ws.glXDestroyContext( render_spu.dpy, render_spu.context );
	}

	render_spu.context = render_spu.ws.glXCreateContext( render_spu.dpy, 
			render_spu.visual,
			NULL, render_spu.try_direct );

	if ( render_spu.context == NULL ) {
		crError( "Couldn't create rendering context" ); 
		return GL_FALSE;
	}

	is_direct = render_spu.ws.glXIsDirect( render_spu.dpy, render_spu.context );

	crDebug( "Created a context (%s)",
			is_direct ? "direct" : "indirect" );

	if ( render_spu.force_direct && !is_direct )
	{
		crError( "Direct rendering not possible." );
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

	if ( render_spu.window )
	{
		/* destroy the old one */
		XDestroyWindow( render_spu.dpy, render_spu.window );
	}
	render_spu.window = 
		XCreateWindow( render_spu.dpy,
				RootWindow( render_spu.dpy, 
					render_spu.visual->screen ),
				actual_window_x,
				actual_window_y,
				render_spu.actual_window_width, 
				render_spu.actual_window_height,
				0, render_spu.visual->depth, InputOutput,
				render_spu.visual->visual,
				flags, &swa );

	if (!render_spu.window) {
		crWarning( "renderspu: unable to create window" );
		return GL_FALSE;
	}

	crDebug( "actual_window_x,y: %d, %d", actual_window_x, actual_window_y);


	if ( render_spu.fullscreen )
	{
		/* Make a clear cursor to get rid of the monitor cursor */
		Pixmap pixmap;
		Cursor cursor;
		XColor color;
		char   clear_bits[32];

		memset( clear_bits, 0, sizeof(clear_bits) );

		pixmap = XCreatePixmapFromBitmapData( render_spu.dpy, 
				render_spu.window,
				clear_bits, 16, 16, 1, 0, 1 );
		cursor = XCreatePixmapCursor( render_spu.dpy, pixmap, pixmap,
				&color, &color, 8, 8 );
		XDefineCursor( render_spu.dpy, render_spu.window, cursor );

		XFreePixmap( render_spu.dpy, pixmap );
	}

	crDebug( "Created the window" );
	hints.x = actual_window_x;
	hints.y = actual_window_y;
	hints.width = render_spu.actual_window_width;
	hints.height = render_spu.actual_window_height;
	hints.min_width = hints.width;
	hints.min_height = hints.height;
	hints.max_width = hints.width;
	hints.max_height = hints.height;
	hints.flags = USPosition | USSize | PMinSize | PMaxSize;
	XSetStandardProperties( render_spu.dpy, render_spu.window,
			WINDOW_NAME, WINDOW_NAME,
			None, NULL, 0, &hints );
#if 1
	/* New item!  This is needed so that the sgimouse server can find
	 * the crDebug window. 
	 */
	name = WINDOW_NAME;
	XStringListToTextProperty( &name, 1, &text_prop );
	XSetWMName( render_spu.dpy, render_spu.window, &text_prop );
#endif
	class_hints = XAllocClassHint( );
	class_hints->res_name = crStrdup( "foo" );
	class_hints->res_class = crStrdup( "Chromium" );
	XSetClassHint( render_spu.dpy, render_spu.window, class_hints );
	free( class_hints->res_name );
	free( class_hints->res_class );
	XFree( class_hints );

	crDebug( "About to make current to the context" );

	if (showIt) {
		XMapWindow( render_spu.dpy, render_spu.window );
		XIfEvent( render_spu.dpy, &event, WaitForMapNotify, 
							(char *) render_spu.window );
	}

#if 0
	/* Note: There is a nasty bug somewhere in render_spu.ws.glXMakeCurrent() for
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

	if ( !render_spu.ws.glXMakeCurrent( render_spu.dpy, render_spu.window, 
				render_spu.context ) )
	{
		crError( "Error making current" );
		return GL_FALSE;
	}
	crDebug( "Made current to the context" );

#if 1
        CRASSERT(render_spu.ws.glGetString);
	crDebug( "GL_VENDOR:   %s", render_spu.ws.glGetString( GL_VENDOR ) );
	crDebug( "GL_RENDERER: %s", render_spu.ws.glGetString( GL_RENDERER ) );
	crDebug( "GL_VERSION:  %s", render_spu.ws.glGetString( GL_VERSION ) );
#endif

	/*
	 * End GLX code
	 */

	crDebug( "actual_window_width, height = %d, %d\n",
			 render_spu.actual_window_width, render_spu.actual_window_height);
	crDebug( "window_x, y, width, height = %d, %d, %d, %d\n",
			 render_spu.window_x, render_spu.window_y,
			 render_spu.window_width, render_spu.window_height);

	return GL_TRUE;
}



/* Either show or hide the render SPU's window. */
void renderspuShowWindow( GLboolean showIt )
{
	if ( render_spu.dpy && render_spu.window )
	{
		if (showIt)
		{
			XEvent event;
			XMapWindow( render_spu.dpy, render_spu.window );
			XIfEvent( render_spu.dpy, &event, WaitForMapNotify, 
								(char *) render_spu.window );
		}
		else
		{
			XUnmapWindow( render_spu.dpy, render_spu.window );
		}
	}
}
