/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
#if 00 /*TEMPORARY*/
#include <unistd.h>
#include "cr_rand.h"
#endif

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Xatom.h>
#include <sys/time.h>
#include <stdio.h>

#include "cr_environment.h"
#include "cr_mothership.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_process.h"
#include "renderspu.h"


/*
 * Stuff from MwmUtils.h
 */
typedef struct
{
    unsigned long       flags;
    unsigned long       functions;
    unsigned long       decorations;
    long                inputMode;
    unsigned long       status;
} PropMotifWmHints;

#define PROP_MOTIF_WM_HINTS_ELEMENTS    5
#define MWM_HINTS_DECORATIONS   (1L << 1)


#define WINDOW_NAME window->title

static Bool WindowExistsFlag;

static int
WindowExistsErrorHandler( Display *dpy, XErrorEvent *xerr )
{
	if (xerr->error_code == BadWindow)
	{
		WindowExistsFlag = GL_FALSE;
	}
	return 0;
}

static GLboolean
WindowExists( Display *dpy, Window w )
{
	XWindowAttributes xwa;
	int (*oldXErrorHandler)(Display *, XErrorEvent *);

	WindowExistsFlag = GL_TRUE;
	oldXErrorHandler = XSetErrorHandler(WindowExistsErrorHandler);
	XGetWindowAttributes(dpy, w, &xwa); /* dummy request */
	XSetErrorHandler(oldXErrorHandler);
	return WindowExistsFlag;
}

static GLboolean
renderDestroyWindow( Display *dpy, Window w )
{
	XWindowAttributes xwa;
	int (*oldXErrorHandler)(Display *, XErrorEvent *);

	WindowExistsFlag = GL_TRUE;
	oldXErrorHandler = XSetErrorHandler(WindowExistsErrorHandler);
	XGetWindowAttributes(dpy, w, &xwa); /* dummy request */
	if (xwa.map_state == IsViewable) {
		XDestroyWindow (dpy, w); /* dummy request */
		XSync (dpy,0);
	}
	XSetErrorHandler(oldXErrorHandler);
	return WindowExistsFlag;
}

/*
 * Garbage collection function.
 * Loop over all known windows and check if corresponding X window still
 * exists.  If it doesn't, destroy the render SPU window.
 * XXX seems to blow up with threadtest.conf tests.
 */
void
renderspu_GCWindow(void)
{
	int i;
	WindowInfo *window;

	for (i = 0; i < (int)render_spu.window_id - 1; i++) {
		window = (WindowInfo *) crHashtableSearch(render_spu.windowTable, i);
		if (window->visual->dpy) {
			if (!WindowExists (window->visual->dpy, window->appWindow) ) {
				XSync(window->visual->dpy,0);
				if(WindowExists(window->visual->dpy, window->window)) {
					renderDestroyWindow(window->visual->dpy, window->window);
				}
			}
		}
	}
}

static Colormap 
GetLUTColormap( Display *dpy, XVisualInfo *vi )
{
	int a;	
	XColor col;
	Colormap cmap;

#if defined(__cplusplus) || defined(c_plusplus)
    int localclass = vi->c_class; /* C++ */
#else
    int localclass = vi->class; /* C */
#endif
	
	if ( localclass != DirectColor )
	{
		crError( "No support for non-DirectColor visuals with LUTs" );
	}

	cmap = XCreateColormap( dpy, RootWindow(dpy, vi->screen), 
			vi->visual, AllocAll );

	for (a=0; a<256; a++)
	{	
		col.red = render_spu.lut8[0][a]<<8;
		col.green = col.blue = 0;
		col.pixel = a<<16;
		col.flags = DoRed;
		XStoreColor(dpy, cmap, &col);
	}
	   
	for (a=0; a<256; a++)
	{	
		col.green = render_spu.lut8[1][a]<<8;
		col.red = col.blue = 0;
		col.pixel = a<<8;
		col.flags = DoGreen;
		XStoreColor(dpy, cmap, &col);
	}
   
	for (a=0; a<256; a++)
	{	
		col.blue = render_spu.lut8[2][a]<<8;
		col.red = col.green= 0;
		col.pixel = a;
		col.flags = DoBlue;
		XStoreColor(dpy, cmap, &col);
	}
   
	return cmap;
}

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


/**
 * Wrapper for glXGetConfig().
 */
static int
Attrib( const VisualInfo *visual, int attrib )
{
	int value = 0;
	render_spu.ws.glXGetConfig( visual->dpy, visual->visual, attrib, &value );
	return value;
}



/**
 * This is basically just a homegrown replacement for glXChooseVisual.
 * It seems that glXChooseVisual() is kind of flakey with ATI's driver.
 * Specifically, calling glXChooseVisual when the render SPU is hosted
 * by a CR Application node doesn't work reliably.  It _might_ be a
 * problem with older XFree86 libGL.so libraries which weren't built
 * with the -Bsymbolic flag.  But that wouldn't explain why calls to
 * glXGetConfig() seem OK!  Hmmm.
 */
static XVisualInfo *
myChooseVisual(Display *dpy, int screen, const int attribs[])
{
	int rgba = 0, doubleBuffer = 0, stereo = 0;
	int redSize = 0, greenSize = 0, blueSize = 0, alphaSize = 0;
	int accRedSize = 0, accGreenSize = 0, accBlueSize = 0, accAlphaSize = 0;
	int depthSize = 0, stencilSize = 0, bufferSize = 0, level = 0, aux = 0;
	int multisample = 0, samples = 0;
	int i;
	XVisualInfo templateVis, *vis;
	long templateFlags;
	int count;

	for (i = 0; attribs[i]; i++) {
		switch (attribs[i]) {
		case GLX_USE_GL:
			/* ignored */
			break;
		case GLX_BUFFER_SIZE:
			i++;
			bufferSize = attribs[i];
			break;
		case GLX_LEVEL:
			i++;
			level = attribs[i];
			break;
		case GLX_AUX_BUFFERS:
			i++;
			aux = attribs[i];
			break;
		case GLX_SAMPLE_BUFFERS_SGIS:
			i++;
			multisample = attribs[i];
			break;
		case GLX_SAMPLES_SGIS:
			i++;
			samples = attribs[i];
			break;
		case GLX_RGBA:
			rgba = 1;
			break;
		case GLX_DOUBLEBUFFER:
			doubleBuffer = 1;
			break;
		case GLX_STEREO:
			stereo = 1;
			break;
		case GLX_RED_SIZE:
			i++;
			redSize = attribs[i];
			break;
		case GLX_GREEN_SIZE:
			i++;
			greenSize = attribs[i];
			break;
		case GLX_BLUE_SIZE:
			i++;
			blueSize = attribs[i];
			break;
		case GLX_ALPHA_SIZE:
			i++;
			alphaSize = attribs[i];
			break;
		case GLX_DEPTH_SIZE:
			i++;
			depthSize = attribs[i];
			break;
		case GLX_STENCIL_SIZE:
			i++;
			stencilSize = attribs[i];
			break;
		case GLX_ACCUM_RED_SIZE:
			i++;
			accRedSize = attribs[i];
			break;
		case GLX_ACCUM_GREEN_SIZE:
			i++;
			accGreenSize = attribs[i];
			break;
		case GLX_ACCUM_BLUE_SIZE:
			i++;
			accBlueSize = attribs[i];
			break;
		case GLX_ACCUM_ALPHA_SIZE:
			i++;
			accAlphaSize = attribs[i];
			break;
		default:
			crError("Unexpected case 0x%x!", attribs[i]);
		}
	}

	templateFlags = VisualScreenMask;
	templateVis.screen = screen;
	if (rgba) {
#if defined(__cplusplus) || defined(c_plusplus)
		templateVis.c_class = TrueColor;
#else
		templateVis.class = TrueColor;
#endif
		templateFlags |= VisualClassMask;
	}

	vis = XGetVisualInfo(dpy, templateFlags, &templateVis, &count);
	/* find first visual that's good enough */
	for (i = 0; i < count; i++) {
		int a;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_RGBA, &a);
		if (a != rgba)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_DOUBLEBUFFER, &a);
		if (a != doubleBuffer)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_STEREO, &a);
		if (a != stereo)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_LEVEL, &a);
		if (a != level)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_RED_SIZE, &a);
		if (a < redSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_GREEN_SIZE, &a);
		if (a < greenSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_BLUE_SIZE, &a);
		if (a < blueSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_ALPHA_SIZE, &a);
		if (a < alphaSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_DEPTH_SIZE, &a);
		if (a < depthSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_STENCIL_SIZE, &a);
		if (a < stencilSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_ACCUM_RED_SIZE, &a);
		if (a < accRedSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_ACCUM_GREEN_SIZE, &a);
		if (a < accGreenSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_ACCUM_BLUE_SIZE, &a);
		if (a < accBlueSize)
			continue;
		render_spu.ws.glXGetConfig(dpy, vis + i, GLX_ACCUM_ALPHA_SIZE, &a);
		if (a < accAlphaSize)
			continue;
 
		/* OK! */
		return vis + i;
	}

	/* no match */
	return NULL;
}


/**
 * Choose a GLX visual based upon a bitmask of CR_*_BIT attributes.
 */
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

	if (visAttribs & CR_MULTISAMPLE_BIT)
	{
		attribList[i++] = GLX_SAMPLE_BUFFERS_SGIS;
		attribList[i++] = 1;
		attribList[i++] = GLX_SAMPLES_SGIS;
		attribList[i++] = 4;
	}

	if (visAttribs & CR_OVERLAY_BIT)
	{
		attribList[i++] = GLX_LEVEL;
		attribList[i++] = 1;
	}

	if (render_spu.use_lut8)
	{
		/* 
		 * See if we have have GLX_EXT_visual_info so we
		 * can grab a Direct Color visual
		 */
#ifdef GLX_EXT_visual_info   
		if (crStrstr(render_spu.ws.glXQueryExtensionsString( dpy, screen ),
								"GLX_EXT_visual_info"))
		{
			attribList[i++] = GLX_X_VISUAL_TYPE_EXT;
			attribList[i++] = GLX_DIRECT_COLOR_EXT; 
		}
		else
		{
			render_spu.use_lut8 = 0;	
		}
#else
		render_spu.use_lut8 = 0;
#endif  
	}

	/* End the list */
	attribList[i++] = None;

	vis = myChooseVisual(dpy, screen, attribList);
	return vis;
}


/**
 * Find a visual with the specified attributes.  If we fail, turn off an
 * attribute (like multisample or stereo) and try again.
 */
static XVisualInfo *
chooseVisualRetry( Display *dpy, int screen, GLbitfield visAttribs )
{
	while (1) {
		XVisualInfo *vis = chooseVisual(dpy, screen, visAttribs);
		if (vis)
			return vis;

		if (visAttribs & CR_MULTISAMPLE_BIT)
			visAttribs &= ~CR_MULTISAMPLE_BIT;
		else if (visAttribs & CR_OVERLAY_BIT)
			visAttribs &= ~CR_OVERLAY_BIT;
		else if (visAttribs & CR_STEREO_BIT)
			visAttribs &= ~CR_STEREO_BIT;
		else if (visAttribs & CR_ACCUM_BIT)
			visAttribs &= ~CR_ACCUM_BIT;
		else if (visAttribs & CR_ALPHA_BIT)
			visAttribs &= ~CR_ALPHA_BIT;
		else
			return NULL;
  }
}


/**
 * Get an FBconfig for the specified attributes
 */
#ifdef GLX_VERSION_1_3
static GLXFBConfig
chooseFBConfig( Display *dpy, int screen, GLbitfield visAttribs )
{
	GLXFBConfig *fbconfig;
	int attribs[1000], attrCount = 0, numConfigs;
	int major, minor;

	CRASSERT(visAttribs & CR_PBUFFER_BIT);

	/* Make sure pbuffers are supported */
	render_spu.ws.glXQueryVersion(dpy, &major, &minor);
	if (major * 100 + minor < 103) {
		crWarning("Render SPU: GLX %d.%d doesn't support pbuffers", major, minor);
		return 0;
	}

	attribs[attrCount++] = GLX_DRAWABLE_TYPE;
	attribs[attrCount++] = GLX_PBUFFER_BIT;

	if (visAttribs & CR_RGB_BIT) {
		attribs[attrCount++] = GLX_RENDER_TYPE;
		attribs[attrCount++] = GLX_RGBA_BIT;
		attribs[attrCount++] = GLX_RED_SIZE;
		attribs[attrCount++] = 1;
		attribs[attrCount++] = GLX_GREEN_SIZE;
		attribs[attrCount++] = 1;
		attribs[attrCount++] = GLX_BLUE_SIZE;
		attribs[attrCount++] = 1;
		if (visAttribs & CR_ALPHA_BIT) {
			attribs[attrCount++] = GLX_ALPHA_SIZE;
			attribs[attrCount++] = 1;
		}
	}

	if (visAttribs & CR_DEPTH_BIT) {
		attribs[attrCount++] = GLX_DEPTH_SIZE;
		attribs[attrCount++] = 1;
	}

	if (visAttribs & CR_DOUBLE_BIT) {
		attribs[attrCount++] = GLX_DOUBLEBUFFER;
		attribs[attrCount++] = True;
	}

	if (visAttribs & CR_STENCIL_BIT) {
		attribs[attrCount++] = GLX_STENCIL_SIZE;
		attribs[attrCount++] = 1;
	}

	if (visAttribs & CR_ACCUM_BIT) {
		attribs[attrCount++] = GLX_ACCUM_RED_SIZE;
		attribs[attrCount++] = 1;
		attribs[attrCount++] = GLX_ACCUM_GREEN_SIZE;
		attribs[attrCount++] = 1;
		attribs[attrCount++] = GLX_ACCUM_BLUE_SIZE;
		attribs[attrCount++] = 1;
		if (visAttribs & CR_ALPHA_BIT) {
			attribs[attrCount++] = GLX_ACCUM_ALPHA_SIZE;
			attribs[attrCount++] = 1;
		}
	}

	if (visAttribs & CR_MULTISAMPLE_BIT) {
		attribs[attrCount++] = GLX_SAMPLE_BUFFERS_SGIS;
		attribs[attrCount++] = 1;
		attribs[attrCount++] = GLX_SAMPLES_SGIS;
		attribs[attrCount++] = 4;
	}

	/* terminate */
	attribs[attrCount++] = 0;

	fbconfig = render_spu.ws.glXChooseFBConfig(dpy, screen, attribs, &numConfigs);
	if (fbconfig && numConfigs > 0)
		return fbconfig[0];
	else
		return 0;
}
#endif /* GLX_VERSION_1_3 */


GLboolean renderspu_SystemInitVisual( VisualInfo *visual )
{
	const char *dpyName;
	int screen;

	CRASSERT(visual);

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		/* A dummy visual - being non null is enough.  */
		visual->visual =(XVisualInfo *) "os";
		return GL_TRUE;
	}
#endif
	
	if (render_spu.display_string[0])
		dpyName = render_spu.display_string;
	else if (visual->displayName[0])
		dpyName = visual->displayName;
	else
		dpyName = NULL;

	crDebug("Render SPU: Opening display %s", dpyName);
	visual->dpy = XOpenDisplay(dpyName);  
	if (!visual->dpy)
	{
		crWarning( "Couldn't open X display named '%s'", dpyName );
		return GL_FALSE;
	}

	if ( !render_spu.ws.glXQueryExtension( visual->dpy, NULL, NULL ) )
	{
		crWarning( "Render SPU: Display %s doesn't support GLX", visual->displayName );
		return GL_FALSE;
	}

	screen = DefaultScreen(visual->dpy);

#ifdef GLX_VERSION_1_3
	if (visual->visAttribs & CR_PBUFFER_BIT)
	{
		{
			visual->fbconfig = chooseFBConfig(visual->dpy, screen, visual->visAttribs);
		}

		if (!visual->fbconfig) {
			char s[1000];
			renderspuMakeVisString( visual->visAttribs, s );
			crWarning( "Render SPU: Display %s doesn't have the necessary fbconfig: %s",
								 dpyName, s );
			XCloseDisplay(visual->dpy);
			return GL_FALSE;
		}
	}
	else
#endif /* GLX_VERSION_1_3 */
	{
		visual->visual = chooseVisualRetry(visual->dpy, screen, visual->visAttribs);
		if (!visual->visual) {
			char s[1000];
			renderspuMakeVisString( visual->visAttribs, s );
			crWarning( "Render SPU: Display %s doesn't have the necessary visual: %s",
								 dpyName, s );
			XCloseDisplay(visual->dpy);
			return GL_FALSE;
		}
	}

	if ( render_spu.sync )
	{
		crDebug( "Render SPU: Turning on XSynchronize" );
		XSynchronize( visual->dpy, True );
	}

	crDebug( "Render SPU: Looks like we have GLX" );

	if (visual->visual) {
			crDebug( "Render SPU: Chose visual id=0x%x: RGBA=(%d,%d,%d,%d) Z=%d"
					 " stencil=%d double=%d stereo=%d accum=(%d,%d,%d,%d)",
					 (int) visual->visual->visualid,
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
	}

	return GL_TRUE;
}


/*
 * Add a GLX window to a swap group for inter-machine SwapBuffer
 * synchronization.
 * Only supported on NVIDIA Quadro 3000G hardware.
 */
static void
JoinSwapGroup(Display *dpy, int screen, Window window,
							GLuint group, GLuint barrier)
{
	GLuint maxGroups, maxBarriers;
	const char *ext;
	Bool b;

	/*
	 * XXX maybe query glXGetClientString() instead???
	 */
	ext = render_spu.ws.glXQueryExtensionsString(dpy, screen);

	if (!crStrstr(ext, "GLX_NV_swap_group") ||
			!render_spu.ws.glXQueryMaxSwapGroupsNV ||
			!render_spu.ws.glXJoinSwapGroupNV ||
			!render_spu.ws.glXBindSwapBarrierNV) {
		crWarning("Render SPU: nv_swap_group is set but GLX_NV_swap_group is not supported on this system!");
		return;
	}

	b = render_spu.ws.glXQueryMaxSwapGroupsNV(dpy, screen,
																						&maxGroups, &maxBarriers);
	if (!b)
		crWarning("Render SPU: call to glXQueryMaxSwapGroupsNV() failed!");

	if (group >= maxGroups) {
		crWarning("Render SPU: nv_swap_group too large (%d > %d)",
						group, (int) maxGroups);
		return;
	}
	crDebug("Render SPU: max swap groups = %d, max barriers = %d",
					maxGroups, maxBarriers);

	/* add this window to the swap group */
	b = render_spu.ws.glXJoinSwapGroupNV(dpy, window, group);
	if (!b) {
		crWarning("Render SPU: call to glXJoinSwapGroupNV() failed!");
		return;
	}
	else {
		crDebug("Render SPU: call to glXJoinSwapGroupNV() worked!");
	}

	/* ... and bind window to barrier of same ID */
	b = render_spu.ws.glXBindSwapBarrierNV(dpy, group, barrier);
	if (!b) {
		crWarning("Render SPU: call to glXBindSwapBarrierNV(group=%d barrier=%d) failed!", group, barrier);
		return;
	}
	else {
		 crDebug("Render SPU: call to glXBindSwapBarrierNV(group=%d barrier=%d) worked!", group, barrier);
	}

	crDebug("Render SPU: window has joined swap group %d", group);
}



static GLboolean
createWindow( VisualInfo *visual, GLboolean showIt, WindowInfo *window )
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
	unsigned int vncWin;

	CRASSERT(visual);
	window->visual = visual;
	window->x = render_spu.defaultX;
	window->y = render_spu.defaultY;
	window->width  = render_spu.defaultWidth;
	window->height = render_spu.defaultHeight;
	window->nativeWindow = 0;

#ifdef USE_OSMESA
	if (render_spu.use_osmesa)
		return GL_TRUE;
#endif

	dpy = visual->dpy;

	if ( render_spu.use_L2 )
	{
		crWarning( "Render SPU: Going fullscreen because we think we're using Lightning-2." );
		render_spu.fullscreen = 1;
	}

	/*
	 * Query screen size if we're going full-screen
	 */
	if ( render_spu.fullscreen )
	{
		XWindowAttributes xwa;
		Window root_window;

		/* disable the screensaver */
		XSetScreenSaver( dpy, 0, 0, PreferBlanking, AllowExposures );
		crDebug( "Render SPU: Just turned off the screensaver" );

		/* Figure out how big the screen is, and make the window that size */
		root_window = DefaultRootWindow( dpy );
		XGetWindowAttributes( dpy, root_window, &xwa );

		crDebug( "Render SPU: root window size: %d x %d", xwa.width, xwa.height );

		window->x = 0;
		window->y = 0;
		window->width  = xwa.width;
		window->height = xwa.height;
	}

	/*
	 * Get a colormap.
	 */
	if (render_spu.use_lut8)
		cmap = GetLUTColormap( dpy, visual->visual );
	else
		cmap = GetShareableColormap( dpy, visual->visual );
	if ( !cmap ) {
		crError( "Render SPU: Unable to get a colormap!" );
		return GL_FALSE;
	}

	/* destroy existing window if there is one */
	if (window->window) {
		XDestroyWindow(dpy, window->window);
	}

	/*
	 * Create the window
	 *
	 * POSSIBLE OPTIMIZATION:
	 * If we're using the render_to_app_window or render_to_crut_window option
	 * (or DMX) we may never actually use this X window.  So we could perhaps
	 * delay its creation until glXMakeCurrent is called.  With NVIDIA's OpenGL
	 * driver, creating a lot of GLX windows can eat up a lot of VRAM and lead
	 * to slow, software-fallback rendering.  Apps that use render_to_app_window,
	 * etc should set the default window size very small to avoid this.
	 * See dmx.conf for example.
	 */
	swa.colormap     = cmap;
	swa.border_pixel = 0;
	swa.event_mask   = ExposureMask | StructureNotifyMask;

	flags = CWBorderPixel | CWColormap | CWEventMask;

	/* 
	 * We pass the VNC's desktop windowID via an environment variable.
	 * If we don't find one, we're not on a 3D-capable vncviewer, or
	 * if we do find one, then create the renderspu subwindow as a
	 * child of the vncviewer's desktop window. 
	 *
	 * This is purely for the replicateSPU.
	 *
	 * NOTE: This is crufty, and will do for now. FIXME.
	 */
	vncWin = crStrToInt( crGetenv("CRVNCWINDOW") );

	if (vncWin) {
		window->window = XCreateWindow(dpy, (Window) vncWin,
																	 window->x, window->y,
																	 window->width, window->height,
																	 0, visual->visual->depth, InputOutput,
																	 visual->visual->visual, flags, &swa);
	}
	else {
		window->window = XCreateWindow(dpy,
																	 RootWindow(dpy, visual->visual->screen),
																	 window->x, window->y,
																	 window->width, window->height,
																	 0, visual->visual->depth, InputOutput,
																	 visual->visual->visual, flags, &swa);
	}

	if (!window->window) {
		crWarning( "Render SPU: unable to create window" );
		return GL_FALSE;
	}

	crDebug( "Render SPU: Created window on display %s, Xvisual 0x%x",
					 DisplayString(visual->dpy),
					 (int) visual->visual->visual->visualid  /* yikes */
					 );

	if (render_spu.fullscreen || render_spu.borderless)
	{
		/* Disable border/decorations with an MWM property (observed by most
		 * modern window managers.
		 */
		PropMotifWmHints motif_hints;
		Atom prop, proptype;

		/* setup the property */
		motif_hints.flags = MWM_HINTS_DECORATIONS;
		motif_hints.decorations = 0; /* Turn off all decorations */

		/* get the atom for the property */
		prop = XInternAtom( dpy, "_MOTIF_WM_HINTS", True );
		if (prop) {
			/* not sure this is correct, seems to work, XA_WM_HINTS didn't work */
			proptype = prop;
			XChangeProperty( dpy, window->window,        /* display, window */
											 prop, proptype,              /* property, type */
											 32,                          /* format: 32-bit datums */
											 PropModeReplace,                /* mode */
											 (unsigned char *) &motif_hints, /* data */
											 PROP_MOTIF_WM_HINTS_ELEMENTS    /* nelements */
											 );
		}
	}


	/* Make a clear cursor to get rid of the monitor cursor */
	if ( render_spu.fullscreen )
	{
		Pixmap pixmap;
		Cursor cursor;
		XColor colour;
		char clearByte = 0;
        
		/* AdB - Only bother to create a 1x1 cursor (byte) */
		pixmap = XCreatePixmapFromBitmapData(dpy, window->window, &clearByte,
																				 1, 1, 1, 0, 1);
		if(!pixmap){
			crWarning("Unable to create clear cursor pixmap");
			return GL_FALSE;
		}
        
		cursor = XCreatePixmapCursor(dpy, pixmap, pixmap, &colour, &colour, 0, 0);
		if(!cursor){
			crWarning("Unable to create clear cursor from zero byte pixmap");
			return GL_FALSE;
		}
		XDefineCursor(dpy, window->window, cursor);
		XFreePixmap(dpy, pixmap);
	}
    
	hints.x = window->x;
	hints.y = window->y;
	hints.width = window->width;
	hints.height = window->height;
	hints.min_width = hints.width;
	hints.min_height = hints.height;
	hints.max_width = hints.width;
	hints.max_height = hints.height;
	if (render_spu.resizable)
		hints.flags = USPosition | USSize;
	else
		hints.flags = USPosition | USSize | PMinSize | PMaxSize;
	XSetStandardProperties( dpy, window->window,
			WINDOW_NAME, WINDOW_NAME,
			None, NULL, 0, &hints );

	/* New item!  This is needed so that the sgimouse server can find
	 * the crDebug window. 
	 */
	name = WINDOW_NAME;
	XStringListToTextProperty( &name, 1, &text_prop );
	XSetWMName( dpy, window->window, &text_prop );

	/* Set window name, resource class */
	class_hints = XAllocClassHint( );
	class_hints->res_name = crStrdup( "foo" );
	class_hints->res_class = crStrdup( "Chromium" );
	XSetClassHint( dpy, window->window, class_hints );
	crFree( class_hints->res_name );
	crFree( class_hints->res_class );
	XFree( class_hints );

	if (showIt) {
		XMapWindow( dpy, window->window );
		XIfEvent( dpy, &event, WaitForMapNotify, 
							(char *) window->window );
	}
	window->visible = showIt;

	if ((window->visual->visAttribs & CR_DOUBLE_BIT) && render_spu.nvSwapGroup) {
		/* NOTE:
		 * If this SPU creates N windows we don't want to gang the N windows
		 * together!
		 * By adding the window ID to the nvSwapGroup ID we can be sure each
		 * app window is in a separate swap group while all the back-end windows
		 * which form a mural are in the same swap group.
		 */
		GLuint group = 0; /*render_spu.nvSwapGroup + window->id;*/
		GLuint barrier = 0;
		JoinSwapGroup(dpy, visual->visual->screen, window->window, group, barrier);
	}

	/*
	 * End GLX code
	 */
	crDebug( "Render SPU: actual window x, y, width, height: %d, %d, %d, %d",
					 window->x, window->y, window->width, window->height );

	XSync(dpy, 0);

	return GL_TRUE;
}


static GLboolean
createPBuffer( VisualInfo *visual, WindowInfo *window )
{
	window->visual = visual;
	window->x = 0;
	window->y = 0;
	window->nativeWindow = 0;

	CRASSERT(window->width > 0);
	CRASSERT(window->height > 0);

#ifdef GLX_VERSION_1_3

	CRASSERT(visual->fbconfig);

#if SEPIA3
	if (render_spu.sepiaPam)
	{
		window->window = SepiaCreateDrawable(render_spu.sepiaPam, visual->dpy,
																				 DefaultScreen(visual->dpy),
																				 visual->fbconfig,
																				 window->width, window->height,
																				 visual->sepiaBuffers);
		crDebug("Render SPU: SepiaCreateDrawable returned %d",
						(int) window->window);
	}
	else
#endif /* SEPIA3 */
	{
		int attribs[100], i = 0;
		attribs[i++] = GLX_PRESERVED_CONTENTS;
		attribs[i++] = True;
		attribs[i++] = GLX_PBUFFER_WIDTH;
		attribs[i++] = window->width;
		attribs[i++] = GLX_PBUFFER_HEIGHT;
		attribs[i++] = window->height;
		attribs[i++] = 0; /* terminator */
		window->window = render_spu.ws.glXCreatePbuffer(visual->dpy,
																										visual->fbconfig, attribs);
	}
	if (window->window) {
		return GL_TRUE;
	}
#endif /* GLX_VERSION_1_3 */
	return GL_FALSE;
}


GLboolean
renderspu_SystemCreateWindow( VisualInfo *visual, GLboolean showIt,
															WindowInfo *window )
{
	if (visual->visAttribs & CR_PBUFFER_BIT) {
		window->width = render_spu.defaultWidth;
		window->height = render_spu.defaultHeight;
		return createPBuffer(visual, window);
	}
	else {
		return createWindow(visual, showIt, window);
	}
}


void renderspu_SystemDestroyWindow( WindowInfo *window )
{
	CRASSERT(window);
	CRASSERT(window->visual);

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) 
	{
		crFree(window->buffer);
		window->buffer = NULL;
	}
	else
#endif
	{
		if (window->visual->visAttribs & CR_PBUFFER_BIT) {
#ifdef GLX_VERSION_1_3
			render_spu.ws.glXDestroyPbuffer(window->visual->dpy, window->window);
#endif
		}
		else {
			/* The value window->nativeWindow will only be non-NULL if the
			 * render_to_app_window option is set to true.  In this case, we
			 * don't want to do anything, since we're not responsible for this
			 * window.  I know...personal responsibility and all...
			 */
			if (!window->nativeWindow) {
				XDestroyWindow(window->visual->dpy, window->window);
				XSync(window->visual->dpy, 0);
			}
		}
	}
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

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		context->context = (GLXContext) render_spu.OSMesaCreateContext(OSMESA_RGB, 0);
		if (context->context)
			return GL_TRUE;
		else
			return GL_FALSE;
	}
#endif

#ifdef  GLX_VERSION_1_3
	if (visual->visAttribs & CR_PBUFFER_BIT) {
		context->context = render_spu.ws.glXCreateNewContext( visual->dpy, 
																													visual->fbconfig,
																													GLX_RGBA_TYPE,
																													NULL,
																													render_spu.try_direct);
	}
	else
#endif
	{
		 context->context = render_spu.ws.glXCreateContext( visual->dpy, 
																												visual->visual,
																												NULL,
																												render_spu.try_direct);
	}
	if (!context->context) {
		crError( "Render SPU: Couldn't create rendering context" ); 
		return GL_FALSE;
	}

	is_direct = render_spu.ws.glXIsDirect( visual->dpy, context->context );
	if (visual->visual)
		crDebug( "Render SPU: Created %s context on display %s, Xvisual 0x%x",
					 is_direct ? "DIRECT" : "INDIRECT",
					 DisplayString(visual->dpy),
					 (int) visual->visual->visual->visualid );

	if ( render_spu.force_direct && !is_direct )
	{
		crError( "Render SPU: Direct rendering not possible." );
		return GL_FALSE;
	}

	return GL_TRUE;
}


void renderspu_SystemDestroyContext( ContextInfo *context )
{
#ifdef USE_OSMESA
	if (render_spu.use_osmesa) 
	{
		render_spu.OSMesaDestroyContext( (OSMesaContext) context->context );
	}
	else
#endif
	{
#if 0
		/* XXX disable for now - causes segfaults w/ NVIDIA's driver */
		render_spu.ws.glXDestroyContext( context->visual->dpy, context->context );
#endif
	}
	context->visual = NULL;
	context->context = 0;
}


#ifdef USE_OSMESA
static void check_buffer_size( WindowInfo *window )
{
	if (window->width != window->in_buffer_width
	    || window->height != window->in_buffer_height
	    || ! window->buffer) {
		crFree(window->buffer);

		window->buffer = crCalloc(window->width * window->height 
															* 4 * sizeof (GLubyte));
		
		window->in_buffer_width = window->width;
		window->in_buffer_height = window->height;

		crDebug("Render SPU: dimensions changed to %d x %d", window->width, window->height);
	}
}
#endif


void renderspu_SystemMakeCurrent( WindowInfo *window, GLint nativeWindow, ContextInfo *context )
{
	CRConnection* conn;
	char response[8096];
	Bool b;

	CRASSERT(render_spu.ws.glXMakeCurrent);
	window->appWindow = nativeWindow;

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		check_buffer_size(window);
		render_spu.OSMesaMakeCurrent( (OSMesaContext) context->context, 
																	window->buffer, GL_UNSIGNED_BYTE,
																	window->width, window->height);
		return;
	}
#endif

	if (window && context) {
		if (window->visual != context->visual) {
			crDebug("Render SPU: MakeCurrent visual mismatch (0x%x != 0x%x); remaking window.",
							window->visual->visAttribs, context->visual->visAttribs);
			/*
			 * XXX have to revisit this issue!!!
			 *
			 * But for now we destroy the current window
			 * and re-create it with the context's visual abilities
			 */
#ifndef SOLARIS_9_X_BUG
		  /*
		    I'm having some really weird issues if I destroy this window 
		    when I'm using the version of sunX that comes with Solaris 9.
		    Subsiquent glX calls return GLXBadCurrentWindow error. 

		    This is an issue even when running Linux version and using 
		    the Solaris 9 sunX as a display.
		    -- jw

				jw: we might have to call glXMakeCurrent(dpy, 0, 0) to unbind
        the context from the window before destroying it. -Brian
		  */
			render_spu.ws.glXMakeCurrent(window->visual->dpy, 0, 0);
			renderspu_SystemDestroyWindow( window );
#endif 
			renderspu_SystemCreateWindow( context->visual, window->visible, window );
			/*
			crError("In renderspu_SystemMakeCurrent() window and context"
							" weren't created with same visual!");
			*/
		}

		CRASSERT(context->context);

		if (render_spu.render_to_crut_window) {
			if (render_spu.crut_drawable == 0) {

				conn = crMothershipConnect();
				if (!conn)
				{
					crError("Couldn't connect to the mothership to get CRUT drawable-- I have no idea what to do!");
				}

				crMothershipGetParam( conn, "crut_drawable", response );
				render_spu.crut_drawable = crStrToInt(response);
				crMothershipDisconnect(conn);

				crDebug("Received a drawable: %i", render_spu.crut_drawable);
				if (!render_spu.crut_drawable) {
					crDebug("Crut drawable is invalid\n");
					/* Continue with nativeWindow = 0; we should render to the
					* little window instead
					*/
				}
			}

			nativeWindow = render_spu.crut_drawable;
			window->nativeWindow = render_spu.crut_drawable;
		}

		if ((render_spu.render_to_crut_window || render_spu.render_to_app_window)
				&& nativeWindow)
		{
			/* The render_to_app_window option is set and we've got a nativeWindow
			 * handle, save the handle for later calls to swapbuffers().
			 */
			if (WindowExists(window->visual->dpy, nativeWindow))
			{
				window->nativeWindow = (Window) nativeWindow;
				b = render_spu.ws.glXMakeCurrent( window->visual->dpy,
																					(Window) nativeWindow,
																					context->context );
				/* don't CRASSERT(b) - it causes a problem with CRUT */
			}
			else
			{
				crWarning("render SPU's render_to_app_window option is set but the application window ID 0x%x is invalid on the display named %s", (unsigned int) nativeWindow, DisplayString(window->visual->dpy));
				CRASSERT(window->window);
				b = render_spu.ws.glXMakeCurrent( window->visual->dpy,
																			window->window, context->context );
				CRASSERT(b);
			}
		}
		else
		{
			/* This is the normal case - rendering to the render SPU's own window */
			CRASSERT(window->window);
			b = render_spu.ws.glXMakeCurrent( window->visual->dpy,
																		window->window, context->context );
			CRASSERT(b);
		}

		/* XXX this is a total hack to work around an NVIDIA driver bug */
		if (render_spu.self.GetFloatv && context->haveWindowPosARB) {
			GLfloat f[4];
			render_spu.self.GetFloatv(GL_CURRENT_RASTER_POSITION, f);
			if (!window->everCurrent || f[1] < 0.0) {
				crDebug("Resetting raster pos");
				render_spu.self.WindowPos2iARB(0, 0);
			}
		}

	}

#if 0
	/* XXX disabled for now due to problem with threadtest.conf */
	renderspu_GCWindow();
#endif
}


void renderspu_SystemWindowSize( WindowInfo *window, int w, int h )
{
#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		window->width = w;
		window->height = h;
		check_buffer_size(window);
		return;
	}
#endif

	CRASSERT(window);
	CRASSERT(window->visual);
	if (window->visual->visAttribs & CR_PBUFFER_BIT)
	{
#ifdef CHROMIUM_THREADSAFE
		ContextInfo *currentContext = (ContextInfo *) crGetTSD(&_RenderTSD);
#else
		ContextInfo *currentContext = render_spu.currentContext;
#endif
		/* Can't resize pbuffers, so destroy it and make a new one */
		render_spu.ws.glXDestroyPbuffer(window->visual->dpy, window->window);
		window->width = w;
		window->height = h;
		crDebug("Creating new %d x %d PBuffer", w, h);
		if (!createPBuffer(window->visual, window)) {
			crWarning("Render SPU: Unable to create the PBuffer!");
		}
		else if (currentContext && currentContext->currentWindow == window) {
			/* Determine if we need to bind the current context to the new pbuffer */
			render_spu.ws.glXMakeCurrent(window->visual->dpy,
																	 window->window, currentContext->context );
		}
	}
	else
	{
		/*
		 * This is ugly, but it seems to be the only thing that works.
		 * Basically, XResizeWindow() doesn't seem to always take effect immediately.
		 * Even after an XSync(), the GetWindowAttributes() call will sometimes
		 * return the old window size.  So, we use a loop to repeat the window
		 * resize until it seems to take effect.
		 */
		int attempt;
		XResizeWindow(window->visual->dpy, window->window, w, h);
		XSync(window->visual->dpy, 0);
		for (attempt = 0; attempt < 3; attempt++) { /* try three times max */
			XWindowAttributes attribs;
			/* Now, query the window size */
			XGetWindowAttributes(window->visual->dpy, window->window, &attribs);
			if (attribs.width == w && attribs.height == h)
				break;
			/* sleep for a millisecond and try again */
			crMsleep(1);
		}
	}
}


void renderspu_SystemGetWindowSize( WindowInfo *window, int *w, int *h )
{
	Window root;
	int x, y;
	unsigned int width, height, bw, d;

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		*w = window->width;
		*h = window->height;
		return;
	}
#endif

	CRASSERT(window);
	CRASSERT(window->visual);
	CRASSERT(window->window);
	if (window->visual->visAttribs & CR_PBUFFER_BIT)
	{
		*w = window->width;
		*h = window->height;
	}
	else
	{
		XGetGeometry(window->visual->dpy, window->window, &root,
								 &x, &y, &width, &height, &bw, &d);
		*w = (int) width;
		*h = (int) height;
	}
}


void renderspu_SystemWindowPosition( WindowInfo *window, int x, int y )
{
#ifdef USE_OSMESA
	if (render_spu.use_osmesa)
		return;
#endif

	CRASSERT(window);
	CRASSERT(window->visual);
	if ((window->visual->visAttribs & CR_PBUFFER_BIT) == 0)
	{
		XMoveWindow(window->visual->dpy, window->window, x, y);
		XSync(window->visual->dpy, 0);
	}
}


/* Either show or hide the render SPU's window. */
void renderspu_SystemShowWindow( WindowInfo *window, GLboolean showIt )
{
#ifdef USE_OSMESA
	if (render_spu.use_osmesa)
		return;
#endif

	if (window->visual->dpy && window->window &&
			(window->visual->visAttribs & CR_PBUFFER_BIT) == 0)
	{
		if (showIt)
		{
			XMapWindow( window->visual->dpy, window->window );
			XSync(window->visual->dpy, 0);
		}
		else
		{
			XUnmapWindow( window->visual->dpy, window->window );
			XSync(window->visual->dpy, 0);
		}
		window->visible = showIt;
	}
}


/** Experimental **/
static void
MarkWindow(WindowInfo *w)
{
	static GC gc = 0;
	if (!gc) {
		XGCValues gcValues;
		gc = XCreateGC(w->visual->dpy, w->nativeWindow, 0, &gcValues);
	}
	XDrawLine(w->visual->dpy, w->nativeWindow, gc, 0, 0, w->width, w->height);
}


void renderspu_SystemSwapBuffers( WindowInfo *w, GLint flags )
{
	CRASSERT(w);

#if 00 /*TEMPORARY - FOR TESTING SWAP LOCK*/
	if (1) {
		/* random delay */
		int k = crRandInt(1000 * 100, 750*1000);
		static int first = 1;
		if (first) {
			 crRandAutoSeed();
			 first = 0;
		}
		usleep(k);
	}
#endif

	{
		/* render_to_app_window:
		 * w->nativeWindow will only be non-zero if the
		 * render_spu.render_to_app_window option is true and
		 * MakeCurrent() recorded the nativeWindow handle in the WindowInfo
		 * structure.
		 */
		if (w->nativeWindow) {
#if 0
			MarkWindow(w);
#else
			(void) MarkWindow;
#endif
			render_spu.ws.glXSwapBuffers( w->visual->dpy, w->nativeWindow );
		}
		else {
			render_spu.ws.glXSwapBuffers( w->visual->dpy, w->window );
		}
	}

}
