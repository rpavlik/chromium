/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>

#include "cr_environment.h"
#include "cr_mothership.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "renderspu.h"

#define WINDOW_NAME render_spu.window_title

#define WindowExists(w)		( IsValidWindowPtr((w)) )

#define SetWContext(w, c)   ( SetWRefCon( (w), (unsigned long) (c) ) )
#define GetWContext(w)		( (ContextInfo *) GetWRefCon( ((w)->nativeWindow ? (w)->nativeWindow : (w)->window) ) )


/* Only one of these overall */
CGDirectDisplayID   display=NULL;
GDHandle			hDisplay=NULL;
CFDictionaryRef		old_dispMode=NULL;
GLboolean			disp_valid = GL_FALSE;

CFDictionaryRef		current_dispMode=NULL;

// where should this go?!
WindowGroupRef masterGroup = NULL;

//

GLboolean renderspu_SystemInitVisual( VisualInfo *visual )
{
	if( !disp_valid ) {
		display = CGMainDisplayID();
		if( !display ) {
			crWarning( "Couldn't open display" );
			return GL_FALSE;
		}

		if( DMGetGDeviceByDisplayID((DisplayIDType) display, &hDisplay, false) != noErr ) {
			crWarning("Unable to get GDevice");
			return GL_FALSE;
		}

		old_dispMode = CGDisplayCurrentMode( display );
	}

	return GL_TRUE;
}


GLboolean renderspuChoosePixelFormat( ContextInfo *context, AGLPixelFormat *pix ) {
	GLbitfield  visAttribs = context->visual->visAttribs;
	GLint		attribs[32];
	GLint		ind = 0;

#define ADD(s)		( attribs[ind++] = (s) )
#define ADDV(s,v)   ( ADD((s)), ADD((v)) )

	CRASSERT(render_spu.ws.aglChoosePixelFormat);

	ADD(AGL_RGBA);
/*	ADDV(AGL_RED_SIZE, 1);
	ADDV(AGL_GREEN_SIZE, 1);
	ADDV(AGL_BLUE_SIZE, 1); */

	if( render_spu.fullscreen )
		ADD(AGL_FULLSCREEN);

	if( visAttribs & CR_ALPHA_BIT )
		ADDV(AGL_ALPHA_SIZE, 1);
	
	if( visAttribs & CR_DOUBLE_BIT )
		ADD(AGL_DOUBLEBUFFER);

	if( visAttribs & CR_STEREO_BIT )
		ADD(AGL_STEREO);

	if( visAttribs & CR_DEPTH_BIT )
		ADDV(AGL_DEPTH_SIZE, 1);

	if( visAttribs & CR_STENCIL_BIT )
		ADDV(AGL_STENCIL_SIZE, 1);

	if( visAttribs & CR_ACCUM_BIT ) {
		ADDV(AGL_ACCUM_RED_SIZE, 1);
		ADDV(AGL_ACCUM_GREEN_SIZE, 1);
		ADDV(AGL_ACCUM_BLUE_SIZE, 1);
		if( visAttribs & CR_ALPHA_BIT )
			ADDV(AGL_ACCUM_ALPHA_SIZE, 1);
	}

	if( visAttribs & CR_MULTISAMPLE_BIT ) {
		ADDV(AGL_SAMPLE_BUFFERS_ARB, 1);
		ADDV(AGL_SAMPLES_ARB, 4);
	}

	if( visAttribs & CR_OVERLAY_BIT )
		ADDV(AGL_LEVEL, 1);

	ADD(AGL_NONE);

	if( render_spu.fullscreen )
		*pix = render_spu.ws.aglChoosePixelFormat( &hDisplay, 1, attribs );
	else
		*pix = render_spu.ws.aglChoosePixelFormat( NULL, 0, attribs );

	return (*pix != NULL);
}


void renderspuDestroyPixelFormat( ContextInfo *context, AGLPixelFormat *pix ) {
	render_spu.ws.aglDestroyPixelFormat( *pix );
	*pix = NULL;
}


GLboolean renderspu_SystemCreateContext( VisualInfo *visual, ContextInfo *context )
{
	AGLPixelFormat pix;

	CRASSERT(visual);
	CRASSERT(context);
	
	context->visual = visual;

	if( !renderspuChoosePixelFormat(context, &pix) ) {
		crError( "Render SPU: Unable to create pixel format" );
		return GL_FALSE;
	}

	context->context = render_spu.ws.aglCreateContext( pix, NULL );
	renderspuDestroyPixelFormat( context, &pix );

	if( !context->context ) {
		crError( "Render SPU: Could not create rendering context" ); 
		return GL_FALSE;
	}

	return GL_TRUE;
}


void renderspu_SystemDestroyContext( ContextInfo *context )
{
	if( !context )
		return;

	render_spu.ws.aglDestroyContext( context->context );
	context->visual = NULL;
	context->context = 0;
}


/*
 * This attempts to fade the screen.  If it can't, it won't.
 */
void TransitionToDisplayMode( CGDirectDisplayID _display, CFDictionaryRef _dispMode ) {
	CGDisplayFadeReservationToken   token;
	CGDisplayFadeInterval			interval = 0.0f;

	/* fade out */
	if( interval > 0.0f ) {
		CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval, &token);
		CGDisplayFade( token, interval, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0f, 0.0f, 0.0f, TRUE );
	}

	CGDisplaySwitchToMode( _display, _dispMode );

	/* fade in */
	if( interval > 0.0f ) {
		CGDisplayFade( token, interval, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0f, 0.0f, 0.0f, FALSE );
		CGReleaseDisplayFadeReservation( token );
	}

}

void renderspuFullscreen( WindowInfo *window, GLboolean fullscreen ) {
	CFDictionaryRef _curr = CGDisplayCurrentMode(display);
	OSStatus err;

	if( fullscreen ) {
		crDebug("Going Fullscreen");

		if( !CGDisplayIsCaptured(display) )
			err = CGDisplayCapture(display);

		if( current_dispMode && current_dispMode != _curr )
			TransitionToDisplayMode( display, current_dispMode );
	} else {
		crDebug("Reverting from Fullscreen");

		if( old_dispMode != _curr )
			TransitionToDisplayMode( display, old_dispMode );

		CGDisplayRelease(display);
	}
}


GLboolean renderspuWindowAttachContext( WindowInfo *wi, WindowRef window, ContextInfo *context ) {
	GLboolean   b;
	if( render_spu.fullscreen ) {
/*#if DEBUG
		aglEnable( context->context, AGL_FS_CAPTURE_SINGLE );
#else
		aglDisable( context->context, AGL_FS_CAPTURE_SINGLE );
#endif*/

		renderspuFullscreen( wi, GL_TRUE );

		b = render_spu.ws.aglSetCurrentContext( context->context );
		if( !b ) {
			crDebug("SetCurrentContext Failed");
			return GL_FALSE;
		}

#if 0
		/* just be sure we set hDisplay in the PixelFormat */
		b = render_spu.ws.aglSetFullScreen( context->context, 0, 0, 0, 0 );
#else
		b = render_spu.ws.aglSetFullScreen( context->context, wi->width, wi->height, 0, 0 );
#endif
		if( !b )
			crDebug("SetFullScreen Failed");
	} else {
		CGrafPtr	save;
		CGrafPtr	port;

		GetPort( &save );
		port = GetWindowPort( window );
		SetPort( port );

		b = render_spu.ws.aglSetDrawable( context->context, port );
		if( b )
			b = render_spu.ws.aglSetCurrentContext( context->context );

		SetPort( save );
/*		SetWContext( window, context ); */
	}
	
	SetWContext( window, context );

	return b;
}


/* Get Dictionary Double (from Apple GLCarbonAGLFullscreen sample) */
static double getDictDouble( CFDictionaryRef refDict, CFStringRef key ) {
	double double_value;

	CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(refDict, key);
	if( !number_value ) // if can't get a number for the dictionary
		return -1;

	if( !CFNumberGetValue(number_value, kCFNumberDoubleType, &double_value) ) // or if can't convert it
		return -1;

	return double_value;
}

// window event handler
static pascal OSStatus windowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (userData)
	WindowRef			window = NULL;
    OSStatus			result = eventNotHandledErr;
    UInt32 				class = GetEventClass (event);
    UInt32 				kind = GetEventKind (event);

	switch (class) {
	case kEventClassWindow:
		GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);

		switch (kind) {
		case kEventWindowClose: // called when window is being closed (close box)
			HideWindow( window );
			SetWRefCon( window, NULL );

			crWarning( "caught kEventWindowClose -- quitting." );
			exit(0);
			break;
		}
		break;
	}

    return result;
}


GLboolean renderspu_SystemCreateWindow( VisualInfo *visual, GLboolean showIt, WindowInfo *window )
{
//	CGDirectDisplayID   dpy;
	WindowAttributes	winAttr = 0L;
	WindowClass			winClass;
	Rect				rect;

	CRASSERT(visual);
	CRASSERT(window);

	window->visual  = visual;
	window->x		= render_spu.defaultX;
	window->y		= render_spu.defaultY;
	window->width   = render_spu.defaultWidth;
	window->height  = render_spu.defaultHeight;
	window->nativeWindow = NULL;

	if ( render_spu.use_L2 ) {
		crWarning( "Render SPU: Going fullscreen because we think we're using Lightning-2." );
		render_spu.fullscreen = 1;
	}

	/*
	 * Query screen size if we're going full-screen
	 */
	if( render_spu.fullscreen )
	{
		window->x = 0;
		window->y = 0;
/*		window->width  = CGDisplayPixelsWide( dpy );
		window->height = /CGDisplayPixelsHigh( dpy ); */

		winAttr |= kWindowNoShadowAttribute;
		winClass = kAltPlainWindowClass;
	} else {
		winAttr |= kWindowStandardFloatingAttributes | kWindowMetalAttribute | kWindowStandardHandlerAttribute;
		winClass = kDocumentWindowClass;
	}

	if( window->window && WindowExists(window->window) )
		/* destroy the old one */
		DisposeWindow( window->window );

	OSStatus stat;

	rect.left   = window->x;
	rect.right  = window->x + window->width;
	rect.top    = window->y;
	rect.bottom = window->y + window->height;

	stat = CreateNewWindow( winClass, winAttr, &rect, &window->window );

	if( stat != noErr ) {
		crError( "Render SPU: unable to create window" );
		return GL_FALSE;
	}

	if( !masterGroup ) {
		WindowGroupAttributes grpAttr = 0L;
		stat = CreateWindowGroup( grpAttr, &masterGroup );
	}

	SetWindowGroup( window->window, masterGroup );
	SetWindowGroupLevel( masterGroup, 1010 ); /* this sets it above the screensaver */

	if( render_spu.fullscreen ) {
		CGRefreshRate refresh;
		CFDictionaryRef disp_mode;

		refresh = getDictDouble( old_dispMode,  kCGDisplayRefreshRate );

		/* note: returns a display mode with the specified property,
		 *       or the current display mode if no matching display mode is found
		 */
		disp_mode = CGDisplayBestModeForParametersAndRefreshRate( display, CGDisplayBitsPerPixel(display), window->width, window->height, refresh, NULL );

		if( !current_dispMode || disp_mode != current_dispMode )
			current_dispMode = disp_mode;
	} else {
		/* Even though there are still issues with the windows themselves, install the event handlers */
		EventHandlerRef ref;
		EventTypeSpec list[] = { { kEventClassWindow, kEventWindowClose } };
		window->event_handler = NewEventHandlerUPP(windowEvtHndlr); 

		InstallWindowEventHandler( window->window, window->event_handler, GetEventTypeCount(list), list, (void*)window->window, &ref );
	}

	if( showIt )
		renderspu_SystemShowWindow( window, GL_TRUE );

	crDebug( "Render SPU: actual window x, y, width, height: %d, %d, %d, %d",
			 window->x, window->y, window->width, window->height );

	return GL_TRUE;
}


void renderspu_SystemDestroyWindow( WindowInfo *window )
{
	CRASSERT(window);
	CRASSERT(window->visual);

	if( render_spu.fullscreen )
		renderspuFullscreen( window, GL_FALSE );

	/*
	 * We need to destroy the context for the fullscreen windows
	 * since AGL seems to only like having one context with the screen.
	 */
	if( render_spu.fullscreen )
		renderspu_SystemDestroyContext( GetWContext(window) );

	if( !window->nativeWindow )
		DisposeWindow( window->window );

	window->visual = NULL;
	window->window = NULL;
	window->width = window->height = 0;
}


void renderspu_SystemWindowSize( WindowInfo *window, int w, int h )
{
	CRASSERT(window);
	CRASSERT(window->window);
	SizeWindow( window->window, w, h, true );
}


void renderspu_SystemGetWindowSize( WindowInfo *window, GLint *w, GLint *h )
{
	GrafPtr save;
	Rect r;

	CRASSERT(window);
	CRASSERT(window->window);

	GetPort( &save );
	SetPortWindowPort( window->window );
	GetWindowPortBounds( window->window, &r );
	SetPort( save );

	*w = (GLint)( r.right - r.left );
	*h = (GLint)( r.bottom - r.top );
}


void renderspu_SystemWindowPosition( WindowInfo *window, int x, int y )
{
	CRASSERT(window);

	MoveWindow( window->window, x, y, true );
}


/* Either show or hide the render SPU's window. */
void renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt )
{
	if( render_spu.fullscreen )
		renderspuFullscreen( window, showIt );

	if( WindowExists(window->window) ) {
		if( showIt ) {
			ShowWindow( window->window );
			SelectWindow( window->window );
		} else {
			HideWindow( window->window );
		}
	}

	window->visible = showIt;
}


void renderspu_SystemMakeCurrent( WindowInfo *window, GLint nativeWindow, ContextInfo *context )
{
//	CRConnection* conn;
	Boolean b;
//	char response[8096];

	CRASSERT(render_spu.ws.aglSetCurrentContext);

	if( window && context ) {
		if( window->visual != context->visual ) {
			crDebug("Render SPU:  MakeCurrent visual mismatch (0x%x != 0x%x); remaking window.",
					window->visual->visAttribs, context->visual->visAttribs);
			/*
			 * XXX have to revisit this issue!!!
			 *
			 * But for now we destroy the current window
			 * and re-create it with the context's visual abilities
			 */
			renderspu_SystemDestroyWindow( window );
			renderspu_SystemCreateWindow( context->visual, window->visible, window );
		}

		CRASSERT(context->context);

		/* Thanks to GLUT, this doesn't do anything.. yet */
/*		if( render_spu.render_to_crut_window ) {
			conn = crMothershipConnect();
			if( !conn )
				crError("Couldn't connect to the mothership to get CRUT drawable-- I have no idea what to do!");

			crMothershipGetParam( conn, "crut_drawable", response );
			render_spu.crut_drawable = crStrToInt(response);
			crMothershipDisconnect( conn );

			crDebug( "Received a drawable: %i", render_spu.crut_drawable );
			if( !render_spu.crut_drawable )
				crError("Crut drawable is invalid\n");

			nativeWindow = render_spu.crut_drawable;
			window->nativeWindow = (WindowRef)render_spu.crut_drawable;
		}

		if( (render_spu.render_to_crut_window || render_spu.render_to_app_window) && nativeWindow )*/
		if( render_spu.render_to_app_window && nativeWindow )
		{
			/* The render_to_app_window option is set and we've got a nativeWindow
			 * handle, save the handle for later calls to swapbuffers().
			 */
			if( WindowExists((WindowRef)nativeWindow) ) {
				window->nativeWindow = (WindowRef) nativeWindow;

				b = renderspuWindowAttachContext( window, window->nativeWindow, context );
				/* don't CRASSERT(b) - it causes a problem with CRUT */
			} else {
				crWarning("render SPU's render_to_app_window option is set"
						  "but the application window ID 0x%x is invalid", (unsigned int) nativeWindow);
				CRASSERT(window->window);
				b = renderspuWindowAttachContext( window, window->window, context );
				CRASSERT(b);
			}
		}
		else
		{
			/* This is the normal case - rendering to the render SPU's own window */
			CRASSERT(window->window);
			b = renderspuWindowAttachContext( window, window->window, context );
			CRASSERT(b);
		}

		/* XXX this is a total hack to work around an NVIDIA driver bug */
		if( render_spu.self.GetFloatv && context->haveWindowPosARB ) {
			GLfloat f[4];
			render_spu.self.GetFloatv(GL_CURRENT_RASTER_POSITION, f);
			if (!window->everCurrent || f[1] < 0.0) {
				crDebug("Resetting raster pos");
				render_spu.self.WindowPos2iARB(0, 0);
			}
		}
	}
}


void renderspu_SystemSwapBuffers( WindowInfo *w, GLint flags )
{
	ContextInfo *context;

	CRASSERT(w);

	context = GetWContext( w );

	if( !context )
		crError( "SwapBuffers: got a null context from the window" );

	render_spu.ws.aglSwapBuffers( context->context );
}

