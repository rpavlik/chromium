/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "cr_glwrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "cr_error.h"
#include "cr_string.h"
#include "renderspu.h"

#define WINDOW_NAME render_spu.window_title


void renderspuKillWindow( void )
{
	render_spu.ws.wglMakeCurrent( GetDC( render_spu.hWnd ), NULL );
	/*wglDeleteContext( render_spu.hRC ); */
	DestroyWindow( render_spu.hWnd );
	render_spu.hWnd = NULL;
}


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

	/* set depth buffer */
	/* pfd.cDepthBits = (BYTE) render_spu.depth_bits; */
        /* set stencil buffer */
	/* pfd.cStencilBits = (BYTE) render_spu.stencil_bits; */

	/*  Commented out by Ian. 
	 *pfd.cColorBits = GetDeviceCaps(hdc,BITSPIXEL); */
	ppfd = &pfd;

	/* calling the wgl functions directly if the SPU was loaded by the 
	 * application (i.e., if the app didn't create a window and get 
	 * faked out) seems to not work. */
	if (getenv( "__CR_LAUNCHED_FROM_APP_FAKER" ) != NULL)
	{
		pixelformat = render_spu.ws.wglChoosePixelFormat( hdc, ppfd );
		/* doing this twice is normal Win32 magic */
		pixelformat = render_spu.ws.wglChoosePixelFormat( hdc, ppfd );
		if ( pixelformat == 0 ) 
		{
			crError( "render_spu.ws.wglChoosePixelFormat failed" );
		}
		if ( !render_spu.ws.wglSetPixelFormat( hdc, pixelformat, ppfd ) ) 
		{
			crError( "render_spu.ws.wglSetPixelFormat failed" );
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


/*
 * VERY IMPORTANT: this function may be called more than once
 * (twice, actually) in order to get a window with different visual
 * attributes the second time around.  Make sure we don't re-open the
 * display or anything silly like that.
 * XXX this needs updating!!! (ask Brian)
 */
GLboolean renderspuCreateWindow( GLbitfield visAttribs, GLboolean showIt )
{
	HDESK     desktop;
	HINSTANCE hinstance;
	WNDCLASS  wc;
	DWORD     window_style;
	int       window_plus_caption_width;
	int       window_plus_caption_height;

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
	 * Begin Windows / WGL code
	 */

	hinstance = GetModuleHandle( NULL );
	if (!hinstance)
	{
		crError( "Couldn't get a handle to my module." );
		return GL_FALSE;
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
		return GL_FALSE;
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
			return GL_FALSE;
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
		return GL_FALSE;
	}

	crDebug( "Showing the window" );
	/* NO ERROR CODE FOR SHOWWINDOW */
	if (showIt)
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
		return GL_FALSE;
	}

	render_spu.hRC = render_spu.ws.wglCreateContext( render_spu.device_context );
	if (!render_spu.hRC)
	{
		crError( "Couldn't create the context for the window!" );
		return GL_FALSE;
	}
	crDebug( "Created the context: 0x%x", render_spu.hRC );
	if (!render_spu.ws.wglMakeCurrent( render_spu.device_context, render_spu.hRC ))
	{
		crError( "Couldn't make current to the context!" );
		return GL_FALSE;
	}
	crDebug( "Made Current" );

	/*
	 * End Windows / WGL code
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
	if (showIt)
		ShowWindow( render_spu.hWnd, SW_SHOWNORMAL );
	else
		; /* XXX to do */
}
