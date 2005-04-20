/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>

#include <stdio.h>

#include "cr_environment.h"
#include "cr_mothership.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "renderspu.h"

#define WINDOW_NAME window->title

#define renderspuSetWindowContext(w, c) \
	( SetWRefCon( (w), (unsigned long) (c) ) )
#define renderspuGetWindowContext(w) \
	( (ContextInfo *) GetWRefCon( ((w)->nativeWindow ? (w)->nativeWindow : (w)->window) ) )

/* functions not found in headers */

GrafPtr           UMAGetWindowPort( WindowRef inWindowRef );
CGSConnectionID   _CGSDefaultConnection(void);
OSStatus         CGSGetWindowBounds( CGSConnectionID cid, CGSWindowID wid,
                                     float *bounds );


// XXX TODO: get all of this into VisualInfo
CGDirectDisplayID   display=NULL; /* Only one of these overall */
GDHandle			hDisplay=NULL;
CFDictionaryRef		old_displayMode=NULL;
GLboolean			display_valid = GL_FALSE;

CFDictionaryRef		current_displayMode=NULL;

// where should this go?!
WindowGroupRef masterGroup = NULL;

//

GLboolean
renderspu_SystemInitVisual( VisualInfo *visual )
{
	if( visual->visAttribs & CR_PBUFFER_BIT )
		crWarning("Render SPU: PBuffers not support on Darwin/AGL yet.");

	if( !display_valid ) {
		display = CGMainDisplayID();

		if( !display ) {
			crWarning( "Render SPU: Couldn't open display" );
			return GL_FALSE;
		}

		if( DMGetGDeviceByDisplayID((DisplayIDType) display, &hDisplay, false) != noErr ) {
			crWarning("Render SPU: Unable to get GDevice");
			return GL_FALSE;
		}

		old_displayMode = CGDisplayCurrentMode( display );
		display_valid = GL_TRUE;
	}

	return GL_TRUE;
}


GLboolean
renderspuChoosePixelFormat( ContextInfo *context, AGLPixelFormat *pix )
{
	GLbitfield  visAttribs = context->visual->visAttribs;
	GLint		attribs[32];
	GLint		ind = 0;

#define ATTR_ADD(s)		( attribs[ind++] = (s) )
#define ATTR_ADDV(s,v)  ( ATTR_ADD((s)), ATTR_ADD((v)) )

	CRASSERT(render_spu.ws.aglChoosePixelFormat);

	ATTR_ADD(AGL_RGBA);
/*	ATTR_ADDV(AGL_RED_SIZE, 1);
	ATTR_ADDV(AGL_GREEN_SIZE, 1);
	ATTR_ADDV(AGL_BLUE_SIZE, 1); */

	if( render_spu.fullscreen )
		ATTR_ADD(AGL_FULLSCREEN);

	if( visAttribs & CR_ALPHA_BIT )
		ATTR_ADDV(AGL_ALPHA_SIZE, 1);
	
	if( visAttribs & CR_DOUBLE_BIT )
		ATTR_ADD(AGL_DOUBLEBUFFER);

	if( visAttribs & CR_STEREO_BIT )
		ATTR_ADD(AGL_STEREO);

	if( visAttribs & CR_DEPTH_BIT )
		ATTR_ADDV(AGL_DEPTH_SIZE, 1);

	if( visAttribs & CR_STENCIL_BIT )
		ATTR_ADDV(AGL_STENCIL_SIZE, 1);

	if( visAttribs & CR_ACCUM_BIT ) {
		ATTR_ADDV(AGL_ACCUM_RED_SIZE, 1);
		ATTR_ADDV(AGL_ACCUM_GREEN_SIZE, 1);
		ATTR_ADDV(AGL_ACCUM_BLUE_SIZE, 1);
		if( visAttribs & CR_ALPHA_BIT )
			ATTR_ADDV(AGL_ACCUM_ALPHA_SIZE, 1);
	}

	if( visAttribs & CR_MULTISAMPLE_BIT ) {
		ATTR_ADDV(AGL_SAMPLE_BUFFERS_ARB, 1);
		ATTR_ADDV(AGL_SAMPLES_ARB, 4);
	}

	if( visAttribs & CR_OVERLAY_BIT )
		ATTR_ADDV(AGL_LEVEL, 1);

	ATTR_ADD(AGL_NONE);

	if( render_spu.fullscreen )
		*pix = render_spu.ws.aglChoosePixelFormat( &hDisplay, 1, attribs );
	else
		*pix = render_spu.ws.aglChoosePixelFormat( NULL, 0, attribs );

	return (*pix != NULL);
}


void
renderspuDestroyPixelFormat( ContextInfo *context, AGLPixelFormat *pix )
{
	render_spu.ws.aglDestroyPixelFormat( *pix );
	*pix = NULL;
}


GLboolean
renderspu_SystemCreateContext( VisualInfo *visual, ContextInfo *context )
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


void
renderspu_SystemDestroyContext( ContextInfo *context )
{
	if( !context )
		return;

	render_spu.ws.aglSetDrawable( context->context, NULL );
	render_spu.ws.aglSetCurrentContext( NULL );
	if( context->context ) {
		render_spu.ws.aglDestroyContext( context->context );
		context->context = NULL;
	}

	context->visual = NULL;
}


/*
 * This attempts to fade the screen.  If it can't, it won't.
 */
void
renderspuTransitionToDisplayMode( CGDirectDisplayID _display,
                                  CFDictionaryRef _displayMode )
{
	CGDisplayFadeReservationToken   token;
	CGDisplayFadeInterval           interval = 0.0f;

	/* fade out */
	if( interval > 0.0f ) {
		CGAcquireDisplayFadeReservation( kCGMaxDisplayReservationInterval,
		                                 &token );
		CGDisplayFade( token, interval, kCGDisplayBlendNormal,
		               kCGDisplayBlendSolidColor, 0.0f, 0.0f, 0.0f, TRUE );
	}

	CGDisplaySwitchToMode( _display, _displayMode );

	/* fade in */
	if( interval > 0.0f ) {
		CGDisplayFade( token, interval, kCGDisplayBlendSolidColor,
		               kCGDisplayBlendNormal, 0.0f, 0.0f, 0.0f, FALSE );
		CGReleaseDisplayFadeReservation( token );
	}
}


void
renderspuFullscreen( WindowInfo *window, GLboolean fullscreen )
{
	CFDictionaryRef displayMode = CGDisplayCurrentMode(display);

	if( fullscreen ) {
		crDebug("Render SPU: Going Fullscreen");

		if( !CGDisplayIsCaptured(display) )
			CGDisplayCapture(display);

		if( current_displayMode && current_displayMode != displayMode )
			renderspuTransitionToDisplayMode( display, current_displayMode );
	} else {
		crDebug("Render SPU: Reverting from Fullscreen");

		if( old_displayMode && old_displayMode != displayMode )
			renderspuTransitionToDisplayMode( display, old_displayMode );

		if( CGDisplayIsCaptured(display) )
			CGDisplayRelease(display);
	}
}


GLboolean
renderspuWindowAttachContext( WindowInfo *wi, WindowRef window,
                              ContextInfo *context )
{
	GLboolean   result;

	if( !context || !wi )
		return render_spu.ws.aglSetCurrentContext( NULL );

	if( render_spu.fullscreen ) {
		renderspuFullscreen( wi, GL_TRUE );

		result = render_spu.ws.aglSetCurrentContext( context->context );
		if( !result ) {
			crDebug("Render SPU: SetCurrentContext Failed");
			return GL_FALSE;
		}

#if 0
		/* just be sure we set hDisplay in the PixelFormat */
		result = render_spu.ws.aglSetFullScreen( context->context, 0, 0, 0, 0 );
#else
		result = render_spu.ws.aglSetFullScreen( context->context, wi->width,
		                                         wi->height, 0, 0 );
#endif
		if( !result ) {
			crDebug("Render SPU: SetFullScreen Failed");
			return GL_FALSE;
		}
	} else {
		AGLDrawable drawable;

		drawable = (AGLDrawable) GetWindowPort( window );

		/* Isn't this a bit 'expensive' to do every time? */
		result = render_spu.ws.aglSetDrawable( context->context, drawable );
		if( !result ) {
			crDebug("Render SPU: SetDrawable Failed");
			return GL_FALSE;
		}

		result = render_spu.ws.aglSetCurrentContext( context->context );
		if( !result ) {
			crDebug("Render SPU: SetCurrentContext Failed");
			return GL_FALSE;
		}

		result = render_spu.ws.aglUpdateContext( context->context );
		if( !result ) {
			crDebug("Render SPU: UpdateContext Failed");
			return GL_FALSE;
		}
	}

	renderspuSetWindowContext( window, context );

	return result;
}


/* Get Dictionary Double (from Apple GLCarbonAGLFullscreen sample) */
static double getDictDouble( CFDictionaryRef refDict, CFStringRef key ) {
	double double_value;

	CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(refDict, key);

	// if can't get a number for the dictionary
	if( !number_value ) 
		return -1;

	// or if can't convert it
	if( !CFNumberGetValue(number_value, kCFNumberDoubleType, &double_value) )
		return -1;

	return double_value;
}

// window event handler
static pascal OSStatus
windowEvtHndlr( EventHandlerCallRef myHandler, EventRef event, void* userData )
{
#pragma unused (userData)
	WindowRef   window = NULL;
	Rect        rectPort = { 0, 0, 0, 0 };
	OSStatus    result = eventNotHandledErr;
	UInt32      class = GetEventClass( event );
	UInt32      kind = GetEventKind( event );

	GetEventParameter(event, kEventParamDirectObject, typeWindowRef,
	                  NULL, sizeof(WindowRef), NULL, &window);
	if( window )
		GetWindowPortBounds( window, &rectPort );

	switch (class) {
	case kEventClassWindow:
		switch (kind) {
		case kEventWindowActivated:
		case kEventWindowDrawContent:
			// draw func
			break;

		case kEventWindowClose:
			HideWindow( window );
			SetWRefCon( window, NULL );

			crWarning( "Render SPU: caught kEventWindowClose -- quitting." );
//			exit(0);
			break;

		case kEventWindowShown:
			// build gl
			if( window == FrontWindow() )
				SetUserFocusWindow( window );
			InvalWindowRect( window, &rectPort );
			break;

		case kEventWindowBoundsChanged:
			// resize
			// update
			break;

		case kEventWindowZoomed: //  zoom button
			//
			break;
		}
		break;
	}

	return result;
}


GLboolean
renderspu_SystemCreateWindow( VisualInfo *visual, GLboolean showIt,
                              WindowInfo *window )
{
//	CGDirectDisplayID   dpy;
	WindowAttributes	winAttr = kWindowNoAttributes;
	WindowClass			winClass = 0;
	Rect				windowRect;
	OSStatus			stat;

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
		window->y += 20; // It -should- be offsetting from the title bar...
		winAttr |= kWindowCollapseBoxAttribute | kWindowCloseBoxAttribute;
		winClass = kDocumentWindowClass;
	}

	if( window->window && IsValidWindowPtr(window->window) )
		/* destroy the old one */
		DisposeWindow( window->window );

	SetRect( &windowRect, window->x, window->y, window->x + window->width,
	         window->y + window->height );

	stat = CreateNewWindow( winClass, winAttr, &windowRect, &window->window );

	if( stat != noErr ) {
		crError( "Render SPU: unable to create window" );
		return GL_FALSE;
	}

	{
		CFStringRef title_string;

		title_string =
		    CFStringCreateWithCStringNoCopy(NULL, WINDOW_NAME,
		                                    kCFStringEncodingMacRoman, NULL);
		SetWindowTitleWithCFString( window->window, title_string );
		CFRelease( title_string );
	}

	if( !masterGroup ) {
		WindowGroupAttributes grpAttr = 0L;
		stat = CreateWindowGroup( grpAttr, &masterGroup );
	}

	SetWindowGroup( window->window, masterGroup );
//	SetWindowGroupLevel( masterGroup, 1010 ); /* this sets it above the screensaver */

	if( render_spu.fullscreen ) {
		CGRefreshRate   refresh;
		CFDictionaryRef displayMode;
		size_t          depth;

		depth = CGDisplayBitsPerPixel( display );
		refresh = getDictDouble( old_displayMode,  kCGDisplayRefreshRate );

		/* note: returns a display mode with the specified property, or the
		         current display mode if no matching display mode is found
		 */
		displayMode = CGDisplayBestModeForParametersAndRefreshRate( display,
		    depth, window->width, window->height, refresh, NULL );

		if( displayMode && displayMode != current_displayMode )
			current_displayMode = displayMode;
	} else {
		/* Even though there are still issues with the windows themselves,
		   install the event handlers */
		EventTypeSpec event_list[] = { {kEventClassWindow, kEventWindowClose} };

		window->event_handler = NewEventHandlerUPP( windowEvtHndlr ); 

		InstallWindowEventHandler( window->window, window->event_handler,
								   GetEventTypeCount(event_list), event_list,
								   NULL, NULL );
	}

	if( showIt )
		renderspu_SystemShowWindow( window, GL_TRUE );

	crDebug( "Render SPU: actual window (x, y, width, height): %d, %d, %d, %d",
			 window->x, window->y, window->width, window->height );

	return GL_TRUE;
}


void
renderspu_SystemDestroyWindow( WindowInfo *window )
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
		renderspu_SystemDestroyContext( renderspuGetWindowContext(window) );

	if( !window->nativeWindow )
		DisposeWindow( window->window );

	window->visual = NULL;
	window->window = NULL;
	window->width = window->height = 0;
}


void
renderspu_SystemWindowSize( WindowInfo *window, GLint w, GLint h )
{
	CRASSERT(window);
	CRASSERT(window->window);

	SizeWindow( window->window, w, h, true );
}


void
renderspu_SystemGetWindowGeometry( WindowInfo *window, GLint *x, GLint *y,
                                   GLint *w, GLint *h )
{
	GrafPtr save;
	Rect r;

	CRASSERT(window);
	CRASSERT(window->window);

	GetPort( &save );
	SetPortWindowPort( window->window );
	GetWindowPortBounds( window->window, &r );
	SetPort( save );

	*x = (int) r.left;
	*y = (int) r.top;
	*w = (int) (r.right - r.left);
	*h = (int) (r.bottom - r.top);
}


void
renderspu_SystemGetMaxWindowSize( WindowInfo *window, GLint *w, GLint *h )
{
	/* XXX fix this */
	(void) window;
	*w = 1600;
	*h = 1200;
}


void
renderspu_SystemWindowPosition( WindowInfo *window, GLint x, GLint y )
{
	CRASSERT(window);

	MoveWindow( window->window, x, y, true );
}


/* Either show or hide the render SPU's window. */
void
renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt )
{
	if( render_spu.fullscreen )
		renderspuFullscreen( window, showIt );

	if( IsValidWindowPtr(window->window) ) {
		if( showIt ) {
			TransitionWindow( window->window, kWindowZoomTransitionEffect,
			                  kWindowShowTransitionAction, NULL );
			ShowWindow( window->window );
			SelectWindow( window->window );
		} else {
			HideWindow( window->window );
		}
	}

	window->visible = showIt;
}


GLboolean
WindowExists( CGSWindowID window )
{
	OSStatus err;
	float bounds[4];

	err = CGSGetWindowBounds( _CGSDefaultConnection(), window, bounds );

	return err == noErr;
}


void
renderspu_SystemMakeCurrent( WindowInfo *window, GLint nativeWindow,
                             ContextInfo *context )
{
//	CRConnection* conn;
	Boolean result;
//	char response[8096];

	CRASSERT(render_spu.ws.aglSetCurrentContext);
	crDebug( "renderspu_SystemMakeCurrent( %x, %i, %x )", window, nativeWindow,
	                                                      context );

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
			renderspu_SystemCreateWindow( context->visual, window->visible,
			                              window );
		}

		CRASSERT(context->context);

#if 0
		/* Thanks to CGS being used on the appfaker and Carbon in here,
		 * this will not work.. yet
		 */
		if( render_spu.render_to_crut_window ) {
			if( render_spu.crut_drawable == 0 ) {
				conn = crMothershipConnect();
				if( !conn )
					crError( "Couldn't connect to the mothership to get CRUT"
					         " drawable -- I have no idea what to do!" );

				crMothershipGetParam( conn, "crut_drawable", response );
				render_spu.crut_drawable = crStrToInt(response);  // Getting CGSWindow
				crMothershipDisconnect( conn );

				crDebug( "Render SPU: using CRUT drawable 0x%x",
				         render_spu.crut_drawable );
				if( !render_spu.crut_drawable )
					crDebug( "Render SPU: Crut drawable 0 is invalid" );
			}

			nativeWindow = render_spu.crut_drawable;
		}

		if( (render_spu.render_to_crut_window ||
		     render_spu.render_to_app_window) && nativeWindow )
		{
			/* The render_to_app_window option is set and we've got a native
			 * window handle, save the handle for later calls to swapbuffers().
			 */
			if( WindowExists(nativeWindow) ) {
				window->nativeWindow = (WindowRef) nativeWindow;

				renderspuWindowAttachContext( window, window->nativeWindow,
				                              context );
				/* don't CRASSERT(result) - it causes a problem with CRUT */
			} else {
				crWarning("Render SPU: render_to_app/crut_window option is set,"
				          "but the application window ID (%i) is invalid",
				          nativeWindow);
				CRASSERT(window->window);

				result = renderspuWindowAttachContext( window, window->window,
				                                       context );
				CRASSERT(result);
			}
		}
		else
#endif
		{
			// This is the normal case: rendering to the render SPU's own window
			CRASSERT(window->window);
			result = renderspuWindowAttachContext( window, window->window,
			                                       context );
			CRASSERT(result);
		}

		/* XXX this is a total hack to work around an NVIDIA driver bug */
		if( render_spu.self.GetFloatv && context->haveWindowPosARB ) {
			GLfloat f[4];
			render_spu.self.GetFloatv(GL_CURRENT_RASTER_POSITION, f);
			if (!window->everCurrent || f[1] < 0.0) {
				crDebug("Render SPU: Resetting raster pos");
				render_spu.self.WindowPos2iARB(0, 0);
			}
		}
	} else {
		renderspuWindowAttachContext( 0, 0, 0 );
	}
}


void
renderspu_SystemSwapBuffers( WindowInfo *window, GLint flags )
{
	ContextInfo *context;

	CRASSERT(window);

	context = renderspuGetWindowContext( window );

	if( !context )
		crError( "Render SPU: SwapBuffers got a null context from the window" );

	render_spu.ws.aglSwapBuffers( context->context );
}

