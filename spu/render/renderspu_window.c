/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#if defined WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#elif defined( IRIX ) || defined( IRIX64 ) || defined( Linux )
	#include <GL/glx.h>
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xmu/StdCmap.h>
	#include <X11/Xatom.h>
	#include <sys/time.h>
#else
	#error "I don't know how to make windows on this platform."
#endif

#include "cr_glwrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "cr_error.h"
#include "cr_string.h"
#include "renderspu.h"

#define WINDOW_NAME render_spu.window_title

#ifndef WINDOWS

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

#else

static LONG WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
static BOOL bSetupPixelFormat( HDC );

#endif

#ifndef WINDOWS

/* More WireGL holdover until I move this to Linux. */
	static int
Attrib( int attrib )
{
	int value = 0;
	render_spu.glXGetConfig( render_spu.dpy, render_spu.visual, attrib, &value );
	return value;
}

static void
makeAttribList( int *attribList, int red, int green, int blue, 
		int depth, int stencil )
{
	int idx = 0;

	attribList[idx++] = GLX_DOUBLEBUFFER;
	attribList[idx++] = GLX_RGBA;

	attribList[idx++] = GLX_RED_SIZE;
	attribList[idx++] = red;
	attribList[idx++] = GLX_GREEN_SIZE;
	attribList[idx++] = green;
	attribList[idx++] = GLX_BLUE_SIZE;
	attribList[idx++] = blue;

	if ( depth )
	{
		attribList[idx++] = GLX_DEPTH_SIZE;
		attribList[idx++] = depth;
	}

	if ( stencil )
	{
		attribList[idx++] = GLX_STENCIL_SIZE;
		attribList[idx++] = stencil;
	}

	attribList[idx++] = None;
}
#endif

void renderspuCreateWindow( void )
{
#ifndef WINDOWS
	Colormap             cmap;
	XSetWindowAttributes swa;
	XSizeHints           hints = {0};
	XEvent               event;
	XTextProperty        text_prop;
	XClassHint          *class_hints = NULL;
	char                *name;
	int                  attribList[16];
	Bool                 is_direct;
	unsigned long        flags;
#else
	HDESK     desktop;
	HINSTANCE hinstance;
	WNDCLASS  wc;
	DWORD     window_style;
	int       window_plus_caption_width;
	int       window_plus_caption_height;
#endif

	int actual_window_x = render_spu.window_x;
	int actual_window_y = render_spu.window_y;

	render_spu.actual_window_width  = render_spu.window_width;
	render_spu.actual_window_height = render_spu.window_height;

	if ( render_spu.use_L2 )
	{
		crWarning( "Going fullscreen because we think we're using Lightning-2." );
		render_spu.fullscreen = 1;
	}

#ifndef WINDOWS
	/* WireGL Holdover */
	if ( render_spu.display_string == NULL )
	{
		render_spu.display_string = ":0.0";
	}

	render_spu.dpy = XOpenDisplay( render_spu.display_string );
	if ( !render_spu.dpy )
	{
		crError( "Couldn't open display \"%s\"", 
				XDisplayName( render_spu.display_string ) );
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

	if ( !render_spu.glXQueryExtension( render_spu.dpy, NULL, NULL ) )
	{
		crError( "Display %s doesn't support GLX", 
				render_spu.display_string );
	}

	crDebug( "Looks like we have GLX" );

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

		crDebug( "root window=%dx%d",
				xwa.width, xwa.height );

		actual_window_x = 0;
		actual_window_y = 0;
	}

	makeAttribList( attribList, 8, 8, 8, 
			render_spu.depth_bits, render_spu.stencil_bits );

	/* choose visual seems to be somewhat busted, so work it a bit */
	render_spu.visual = 
		render_spu.glXChooseVisual( render_spu.dpy, 
				DefaultScreen( render_spu.dpy ), 
				attribList );
	if ( !render_spu.visual )
	{
		makeAttribList( attribList, 5, 6, 5,
				render_spu.depth_bits, render_spu.stencil_bits );
		render_spu.visual = 
			render_spu.glXChooseVisual( render_spu.dpy, 
					DefaultScreen( render_spu.dpy), 
					attribList );
	}

	if ( !render_spu.visual )
	{
		makeAttribList( attribList, 4, 4, 4,
				render_spu.depth_bits, render_spu.stencil_bits );
		render_spu.visual = 
			render_spu.glXChooseVisual( render_spu.dpy, 
					DefaultScreen( render_spu.dpy ), 
					attribList );
	}

	if ( !render_spu.visual )
	{
		makeAttribList( attribList, 1, 1, 1,
				render_spu.depth_bits, render_spu.stencil_bits );
		render_spu.visual = 
			render_spu.glXChooseVisual( render_spu.dpy, 
					DefaultScreen( render_spu.dpy ), 
					attribList );
	}

	if ( !render_spu.visual && render_spu.depth_bits > 24 ) {
		/* MWE -- why does Francois end up here? */
		render_spu.depth_bits = 24;

		makeAttribList( attribList, 8, 8, 8, 
				render_spu.depth_bits, render_spu.stencil_bits );

		render_spu.visual = 
			render_spu.glXChooseVisual( render_spu.dpy, 
					DefaultScreen( render_spu.dpy ), 
					attribList );
	}

	if ( !render_spu.visual )
	{
		crError( "Display %s doesn't have the necessary visual "
				"(RGB=%d, Z=%d stencil=%d double=%d)",
				render_spu.display_string, 8, 
				render_spu.depth_bits, render_spu.stencil_bits,
				1 );
	}

	crDebug( "Chose a visual (id=%ld)", render_spu.visual->visualid );
	crDebug( "Visual: RGBA=<%d,%d,%d,%d> "
			"Z=%d stencil=%d double=%d", Attrib( GLX_RED_SIZE ),
			Attrib( GLX_GREEN_SIZE ), Attrib( GLX_BLUE_SIZE ),
			Attrib( GLX_ALPHA_SIZE ), Attrib( GLX_DEPTH_SIZE ),
			Attrib( GLX_STENCIL_SIZE ), Attrib( GLX_DOUBLEBUFFER ) );

	cmap = GetShareableColormap( render_spu.dpy, render_spu.visual );
	crDebug( "Chose a colormap" );

	crDebug( "Creating the OpenGL context" );

	render_spu.context = render_spu.glXCreateContext( render_spu.dpy, 
			render_spu.visual,
			NULL, render_spu.try_direct );

	if ( render_spu.context == NULL )
		crError( "Couldn't create rendering context" ); 

	is_direct = render_spu.glXIsDirect( render_spu.dpy, render_spu.context );

	crDebug( "Created a context (%s)",
			is_direct ? "direct" : "indirect" );

	if ( render_spu.force_direct && !is_direct )
	{
		crError( "Direct rendering not possible." );
	}

	swa.colormap     = cmap;
	swa.border_pixel = 0;
	swa.event_mask   = ExposureMask | StructureNotifyMask;

	flags = CWBorderPixel | CWColormap | CWEventMask;

	if (render_spu.fullscreen)
	{
		swa.override_redirect = True;
		flags |= CWOverrideRedirect;
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

	crDebug( "actual_window_x: %d", 
			actual_window_x);
	crDebug( "actual_window_y: %d", 
			actual_window_y);


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

	XMapWindow( render_spu.dpy, render_spu.window );
	XIfEvent( render_spu.dpy, &event, WaitForMapNotify, 
			(char *) render_spu.window );

#if 0
	/* Note: There is a nasty bug somewhere in render_spu.glXMakeCurrent() for
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

	if ( !render_spu.glXMakeCurrent( render_spu.dpy, render_spu.window, 
				render_spu.context ) )
	{
		crError( "Error making current" );
	}
	crDebug( "Made current to the context" );

	crDebug( "GL_VENDOR:   %s", render_spu.glGetString( GL_VENDOR ) );
	crDebug( "GL_RENDERER: %s", render_spu.glGetString( GL_RENDERER ) );
	crDebug( "GL_VERSION:  %s", render_spu.glGetString( GL_VERSION ) );

#else
	hinstance = GetModuleHandle( NULL );
	if (!hinstance)
	{
		crError( "Couldn't get a handle to my module." );
	}
	crDebug( "Got the module handle: 0x%x", hinstance );

	/* If we were launched from a service, telnet, or rsh, we need to
	 * get the input desktop.  */

	desktop = OpenInputDesktop( 0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE );

	if ( !desktop )
	{
		crError( "Couldn't aquire input desktop" );
	}
	crDebug( "Got the desktop: 0x%x", desktop );

	if ( !SetThreadDesktop( desktop ) )
	{
		/* If this function fails, it's probably because 
		 * it's already been called (i.e., the render SPU 
		 * is bolted to an application?) */

		/*crError( "Couldn't set thread to input desktop" ); */
	}
	crDebug( "Set the thread desktop -- this might have failed." );

	if ( !GetClassInfo(hinstance, WINDOW_NAME, &wc) ) 
	{
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = (WNDPROC) MainWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hinstance;
		wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WINDOW_NAME;

		if ( !RegisterClass( &wc ) )
		{
			crError( "Couldn't register window class -- you're not trying "
					"to do multi-pipe stuff on windows, are you?\n\nNote --"
					"This error message is from 1997 and probably doesn't make"
					"any sense any more, but it's nostalgic for Humper." );
		}
		crDebug( "Registered the class" );
	}
	crDebug( "Got the class information" );

	/* Full screen window should be a popup (undecorated) window */
#if 1
	window_style = ( render_spu.fullscreen ? WS_POPUP : WS_CAPTION );
#else
	window_style = ( WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	window_style |= WS_SYSMENU;
#endif

	crDebug( "Fullscreen: %s\n", render_spu.fullscreen ? "yes" : "no");

	if ( render_spu.fullscreen )
	{
#if 0

		int smCxFixedFrame = GetSystemMetrics( SM_CXFIXEDFRAME );
		int smCyFixedFrame = GetSystemMetrics( SM_CXFIXEDFRAME ) + 1;
		int smCyCaption = GetSystemMetrics( SM_CYCAPTION );

		render_spu.actual_window_width = GetSystemMetrics( SM_CXSCREEN ) ;
		render_spu.actual_window_height = GetSystemMetrics( SM_CYSCREEN ) ;

		crDebug( "Window Dims: %d, %d\n", 
				render_spu.actual_window_width,
				render_spu.actual_window_height);

		actual_window_x = render_spu.window_x - smCxFixedFrame - 1;
		actual_window_y = render_spu.window_y - smCyFixedFrame - smCyCaption;

		window_plus_caption_width = render_spu.actual_window_width +
			2 * smCxFixedFrame;
		window_plus_caption_height = render_spu.actual_window_height + 
			2 * smCyFixedFrame + smCyCaption;

#else
		/* Since it's undecorated, we don't have to do anything fancy
		 * with these parameters. */

		render_spu.actual_window_width = GetSystemMetrics( SM_CXSCREEN ) ;
		render_spu.actual_window_height = GetSystemMetrics( SM_CYSCREEN ) ;
		actual_window_x = 0;
		actual_window_y = 0;
		window_plus_caption_width = render_spu.actual_window_width;
		window_plus_caption_height = render_spu.actual_window_height;

#endif
	}
	else
	{
		/* CreateWindow takes the size of the entire window, so we add
		 * in the size necessary for the frame and the caption. */

		int smCxFixedFrame, smCyFixedFrame, smCyCaption;
		smCxFixedFrame = GetSystemMetrics( SM_CXFIXEDFRAME );
		crDebug( "Got the X fixed frame" );
		smCyFixedFrame = GetSystemMetrics( SM_CYFIXEDFRAME );
		crDebug( "Got the Y fixed frame" );
		smCyCaption = GetSystemMetrics( SM_CYCAPTION );
		crDebug( "Got the Caption " );

		window_plus_caption_width = render_spu.actual_window_width +
			2 * smCxFixedFrame;
		window_plus_caption_height = render_spu.actual_window_height + 
			2 * smCyFixedFrame + smCyCaption;

		actual_window_x = render_spu.window_x - smCxFixedFrame;
		actual_window_y = render_spu.window_y - smCyFixedFrame - smCyCaption;
	}

	crDebug( "Creating the window: (%d,%d), (%d,%d)", render_spu.window_x, render_spu.window_y, window_plus_caption_width, window_plus_caption_height );
	render_spu.hWnd = CreateWindow( WINDOW_NAME, WINDOW_NAME,
			window_style,
			actual_window_x, actual_window_y,
			window_plus_caption_width,
			window_plus_caption_height,
			NULL, NULL, hinstance, &render_spu );

	if ( !render_spu.hWnd )
	{
		crError( "Create Window failed!  That's almost certainly terrible." );
	}

	crDebug( "Showing the window" );
	/* NO ERROR CODE FOR SHOWWINDOW */
	ShowWindow( render_spu.hWnd, SW_SHOWNORMAL );
	SetForegroundWindow( render_spu.hWnd );
	if ( render_spu.fullscreen )
	{
		SetWindowPos( render_spu.hWnd, HWND_TOPMOST, 
				actual_window_x, actual_window_y,
				window_plus_caption_width, 
				window_plus_caption_height,
				SWP_SHOWWINDOW | SWP_NOSENDCHANGING | 
				SWP_NOREDRAW | SWP_NOACTIVATE );
		ShowCursor( FALSE );
	}
	render_spu.device_context = GetDC( render_spu.hWnd );
	crDebug( " Got the DC: 0x%x", render_spu.device_context );
	if ( !bSetupPixelFormat( render_spu.device_context ) )
	{
		crError( "Couldn't set up the device context!  Yikes!" );
	}

	render_spu.hRC = render_spu.wglCreateContext( render_spu.device_context );
	if (!render_spu.hRC)
	{
		crError( "Couldn't create the context for the window!" );
	}
	crDebug( "Created the context: 0x%x", render_spu.hRC );
	if (!render_spu.wglMakeCurrent( render_spu.device_context, render_spu.hRC ))
	{
		crError( "Couldn't make current to the context!" );
	}
	crDebug( "Made Current" );

#endif
}

void renderspuKillWindow( void )
{
#ifdef WINDOWS
	render_spu.wglMakeCurrent( GetDC( render_spu.hWnd ), NULL );
	/*wglDeleteContext( render_spu.hRC ); */
	DestroyWindow( render_spu.hWnd );
	render_spu.hWnd = NULL;
#endif
}

#ifdef WINDOWS

	static LONG WINAPI
MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	/* int w,h; */

	switch ( uMsg ) {

		case WM_SIZE:
			/* w = LOWORD( lParam ); 
			 * h = HIWORD( lParam ); */

			/* glViewport( 0, 0, w, h ); */
#if 0
			glViewport( -render_spu.mural_x, -render_spu.mural_y, 
					render_spu.mural_width, render_spu.mural_height );
			glScissor( -render_spu.mural_x, -render_spu.mural_y, 
					render_spu.mural_width, render_spu.mural_height );
#endif
			break;

		case WM_CLOSE:
			crWarning( "caught WM_CLOSE -- quitting." );
			exit( 0 );
			break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

	static BOOL
bSetupPixelFormat( HDC hdc )
{
	PIXELFORMATDESCRIPTOR *ppfd; 
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),  /*  size of this pfd */
		1,                              /* version number */
		PFD_DRAW_TO_WINDOW |            /* support window */
			PFD_SUPPORT_OPENGL |            /* support OpenGL */
			PFD_DOUBLEBUFFER,               /* double buffered */
		PFD_TYPE_RGBA,                  /* RGBA type */
		24,                             /* 24-bit color depth */
		0, 0, 0, 0, 0, 0,               /* color bits ignored */
		0,                              /* no alpha buffer */
		0,                              /* shift bit ignored */
		0,                              /* no accumulation buffer */
		0, 0, 0, 0,                     /* accum bits ignored */
		0,                              /* set depth buffer	 */
		0,                              /* set stencil buffer */
		0,                              /* no auxiliary buffer */
		PFD_MAIN_PLANE,                 /* main layer */
		0,                              /* reserved */
		0, 0, 0                         /* layer masks ignored */
	}; 
	int pixelformat;

	pfd.cDepthBits = (BYTE) render_spu.depth_bits;           /* set depth buffer	 */
	pfd.cStencilBits = (BYTE) render_spu.stencil_bits;       /* set stencil buffer */

	/*  Commented out by Ian. 
	 *pfd.cColorBits = GetDeviceCaps(hdc,BITSPIXEL); */
	ppfd = &pfd;

	/* calling the wgl functions directly if the SPU was loaded by the 
	 * application (i.e., if the app didn't create a window and get 
	 * faked out) seems to not work. */
	if (getenv( "__CR_LAUNCHED_FROM_APP_FAKER" ) != NULL)
	{
		pixelformat = render_spu.wglChoosePixelFormat( hdc, ppfd );
		/* doing this twice is normal Win32 magic */
		pixelformat = render_spu.wglChoosePixelFormat( hdc, ppfd );
		if ( pixelformat == 0 ) 
		{
			crError( "render_spu.wglChoosePixelFormat failed" );
		}
		if ( !render_spu.wglSetPixelFormat( hdc, pixelformat, ppfd ) ) 
		{
			crError( "render_spu.wglSetPixelFormat failed" );
		}
	}
	else
	{
		/* Okay, we were loaded manually.  Call the GDI functions. */
		pixelformat = ChoosePixelFormat( hdc, ppfd );
		/* doing this twice is normal Win32 magic */
		pixelformat = ChoosePixelFormat( hdc, ppfd );
		if ( pixelformat == 0 ) 
		{
			crError( "ChoosePixelFormat failed" );
		}
		if ( !SetPixelFormat( hdc, pixelformat, ppfd ) ) 
		{
			crError( "SetPixelFormat failed" );
		}
	}

	return TRUE;
}
#endif
