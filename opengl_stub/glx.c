/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* opengl_stub/glx.c */

#include "chromium.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_mothership.h"
#include "cr_string.h"
#include "stub.h"

#define MULTISAMPLE 1

/** For optimizing glXMakeCurrent */
static Display *currentDisplay = NULL;
static GLXDrawable currentDrawable = 0;

static GLuint desiredVisual = CR_RGB_BIT;


/**
 * Set this to 1 if you want to build stub functions for the
 * GL_SGIX_pbuffer and GLX_SGIX_fbconfig extensions.
 * This used to be disabled, due to "messy compilation issues",
 * according to the earlier comment; but they're needed just
 * to resolve symbols for OpenInventor applications, and I
 * haven't found any reference to exactly what the "messy compilation
 * issues" are, so I'm re-enabling the code by default.
 */
#define GLX_EXTRAS 1

/**
 * Prototypes, in case they're not in glx.h or glxext.h
 * Unfortunately, there's some inconsistency between the extension
 * specs, and the SGI, NVIDIA, XFree86 and common glxext.h header
 * files.
 */
#if defined(GLX_GLXEXT_VERSION)
/* match glxext.h, XFree86, Mesa */
#define ATTRIB_TYPE const int
#else
#define ATTRIB_TYPE int
#endif

#if GLX_EXTRAS
GLXPbufferSGIX glXCreateGLXPbufferSGIX(Display *dpy, GLXFBConfigSGIX config, unsigned int width, unsigned int height, int *attrib_list);
int glXQueryGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf, int attribute, unsigned int *value);
GLXFBConfigSGIX *glXChooseFBConfigSGIX(Display *dpy, int screen, int *attrib_list, int *nelements);

void glXDestroyGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf);
void glXSelectEventSGIX (Display *dpy, GLXDrawable drawable, unsigned long mask);
void glXGetSelectedEventSGIX (Display *dpy, GLXDrawable drawable, unsigned long *mask);

GLXFBConfigSGIX glXGetFBConfigFromVisualSGIX(Display *dpy, XVisualInfo *vis);
XVisualInfo *glXGetVisualFromFBConfigSGIX(Display *dpy, GLXFBConfig config);
GLXContext glXCreateContextWithConfigSGIX(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
GLXPixmap glXCreateGLXPixmapWithConfigSGIX(Display *dpy, GLXFBConfig config, Pixmap pixmap);
int glXGetFBConfigAttribSGIX(Display *dpy, GLXFBConfig config, int attribute, int *value);

/*
 * GLX 1.3 functions
 */
GLXFBConfig *glXChooseFBConfig(Display *dpy, int screen, ATTRIB_TYPE *attrib_list, int *nelements);
GLXPbuffer glXCreatePbuffer(Display *dpy, GLXFBConfig config, ATTRIB_TYPE *attrib_list);
GLXPixmap glXCreatePixmap(Display *dpy, GLXFBConfig config, Pixmap pixmap, ATTRIB_TYPE *attrib_list);
GLXWindow glXCreateWindow(Display *dpy, GLXFBConfig config, Window win, ATTRIB_TYPE *attrib_list);
GLXContext glXCreateNewContext(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
void glXDestroyPbuffer(Display *dpy, GLXPbuffer pbuf);
void glXDestroyPixmap(Display *dpy, GLXPixmap pixmap);
void glXDestroyWindow(Display *dpy, GLXWindow win);
GLXDrawable glXGetCurrentReadDrawable(void);
int glXGetFBConfigAttrib(Display *dpy, GLXFBConfig config, int attribute, int *value);
GLXFBConfig *glXGetFBConfigs(Display *dpy, int screen, int *nelements);
void glXGetSelectedEvent(Display *dpy, GLXDrawable draw, unsigned long *event_mask);
XVisualInfo *glXGetVisualFromFBConfig(Display *dpy, GLXFBConfig config);
Bool glXMakeContextCurrent(Display *display, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
int glXQueryContext(Display *dpy, GLXContext ctx, int attribute, int *value);
void glXQueryDrawable(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
void glXSelectEvent(Display *dpy, GLXDrawable draw, unsigned long event_mask);

 

#endif /* GLX_EXTRAS */



/**
 * Return string for a GLX error code
 */
static const char *glx_error_string(int err)
{
	static const char *glxErrors[] = {
		"none",
		"GLX_BAD_SCREEN",
		"GLX_BAD_ATTRIBUTE",
		"GLX_NO_EXTENSION",
		"GLX_BAD_VISUAL",
		"GLX_BAD_CONTEXT",
		"GLX_BAD_VALUE",
		"GLX_BAD_ENUM"
	};
	if (err > 0 && err < 8) {
		return glxErrors[err];
	}
	else {
		static char tmp[100];
		sprintf(tmp, "0x%x", err);
		return tmp;
	}
}


/**
 * This function is used to satisfy an application's calls to glXChooseVisual
 * when the display server many not even support GLX.
 */
static XVisualInfo *ReasonableVisual( Display *dpy, int screen )
{
	int i, n;
	XVisualInfo vis_template, *visual, *best;

	/* find the set of all visuals for this display/screen */
	vis_template.screen = screen;
	visual = XGetVisualInfo( dpy, VisualScreenMask, &vis_template, &n );
	if ( visual == NULL || n < 1 )
	{
		crWarning( "glXChooseVisual: "
				"XGetVisualInfo() found no matches?" );
		return NULL;
	}

	/* now see if we can find a TrueColor/DirectColor visual */
	best = NULL;
	for ( i = 0; i < n; i++ )
	{
#if defined(__cplusplus) || defined(c_plusplus)
	int localclass = visual[i].c_class;  /* C++ */
#else
	int localclass = visual[i].class;
#endif

		if ( localclass == TrueColor || localclass == DirectColor )
		{
			best = &visual[i];
			break;
		}
	}

	if ( best ) {

		/* okay, select the RGB visual with the most depth */
		for ( i = 0; i < n; i++ )
		{
#if defined(__cplusplus) || defined(c_plusplus)
	    int localclass = visual[i].c_class;  /* C++ */
#else
	    int localclass = visual[i].class;
#endif
			if ( ( localclass == TrueColor ||
						localclass == DirectColor ) &&
					visual[i].depth > best->depth &&
					visual[i].bits_per_rgb > best->bits_per_rgb )
			{
				best = &visual[i];
			}
		}
	}
	else
	{
		/* no RGB visuals, select the deepest colorindex visual */
		/* NOTE: this code is likely never ever executed */
		/* we don't have to check for TrueColor/DirectColor, since
		 * we know none of those exist */
		best = &visual[0];
		for ( i = 1; i < n; i++ )
		{
			if ( visual[i].depth > best->depth )
			{
				best = &visual[i];
			}
		}
	}

	vis_template.screen = screen;
	vis_template.visualid = best->visualid;

	XFree( visual );

	visual = XGetVisualInfo( dpy, VisualScreenMask | VisualIDMask,
			&vis_template, &n );

	if ( visual == NULL || n != 1 )
	{
		crError( "glXChooseVisual: XGetVisualInfo( visualid=%ld ) failed!",
				vis_template.visualid );
		return NULL;
	}

	return visual;
}

/**
 * Query the GLX attributes for the given visual and return a bitmask of
 * the CR_*_BIT flags which describes the visual's capabilities.
 */
static GLuint
FindVisualInfo( Display *dpy, XVisualInfo *vInfo )
{
	int visBits = 0;
	GLint doubleBuffer = 0, stereo = 0;
	GLint alphaSize = 0, depthSize = 0, stencilSize = 0;
	GLint accumRedSize = 0, accumGreenSize = 0, accumBlueSize = 0;
	GLint sampleBuffers = 0, samples = 0, level = 0;

	/* Should we check more attributes? */
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &doubleBuffer);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_STEREO, &stereo);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &alphaSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &depthSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &stencilSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &accumRedSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &accumGreenSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &accumBlueSize);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_SAMPLE_BUFFERS_SGIS, &sampleBuffers);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_SAMPLES_SGIS, &samples);
	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_LEVEL, &level);

	visBits |= CR_RGB_BIT;	/* assuming we always want this... */

	if (alphaSize > 0)
		visBits |= CR_ALPHA_BIT;
	if (depthSize > 0)
		visBits |= CR_DEPTH_BIT;
	if (stencilSize > 0)
		visBits |= CR_STENCIL_BIT;
	if (accumRedSize > 0 || accumGreenSize > 0 || accumBlueSize > 0)
		visBits |= CR_ACCUM_BIT;
	if (doubleBuffer)
		visBits |= CR_DOUBLE_BIT;
	if (stereo)
		visBits |= CR_STEREO_BIT;
#if MULTISAMPLE
	if (sampleBuffers > 0 && samples > 0)
		visBits |= CR_MULTISAMPLE_BIT;
#endif
	if (level > 0)
		visBits |= CR_OVERLAY_BIT;

#if 0
	/* Pack the visual ID number in the high 16 bits of the bitfield */
	visBits |= ((int) vInfo->visual->visualid) << CR_NATIVE_VISUAL_SHIFT;

	/* Make sure there's no overlapping use of the bits */
	CRASSERT((CR_NATIVE_VISUAL_MASK & CR_ALL_VISUAL_BITS) == 0);
#endif

	return visBits;
}


XVisualInfo *
glXChooseVisual( Display *dpy, int screen, int *attribList )
{
	XVisualInfo *vis;
	int *attrib, wants_rgb = 0, attribCopy[1000], copy = 0;
	int foo, bar;
	int visBits;

	stubInit();

	visBits = desiredVisual;

	for (attrib = attribList; *attrib != None; attrib++)
	{
		/* Note: we build a copy of the attribute list as we go so that
		 * we're able to modify it later if necessary.
		 */
		attribCopy[copy++] = *attrib;

		switch (*attrib)
		{
			case GLX_USE_GL:
				/* ignored, this is mandatory */
				break;

			case GLX_BUFFER_SIZE:
				/* this is for color-index visuals, which we don't support */
				attrib++;
				break;

			case GLX_LEVEL:
				if (attrib[1] > 0)
					visBits |= CR_OVERLAY_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_RGBA:
				visBits |= CR_RGB_BIT;
				wants_rgb = 1;
				break;

			case GLX_DOUBLEBUFFER:
				visBits |= CR_DOUBLE_BIT;
				break;

			case GLX_STEREO:
				visBits |= CR_STEREO_BIT;
				/*
				crWarning( "glXChooseVisual: stereo unsupported" );
				return NULL;
				*/
				break;

			case GLX_AUX_BUFFERS:
				{
					int aux_buffers = attrib[1];
					if (aux_buffers != 0)
					{
						crWarning("glXChooseVisual: aux_buffers=%d unsupported",
											aux_buffers);
						return NULL;
					}
				}
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_RED_SIZE:
			case GLX_GREEN_SIZE:
			case GLX_BLUE_SIZE:
				if (attrib[1] > 0)
					visBits |= CR_RGB_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_ALPHA_SIZE:
				if (attrib[1] > 0)
					visBits |= CR_ALPHA_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_DEPTH_SIZE:
				if (attrib[1] > 0)
					visBits |= CR_DEPTH_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_STENCIL_SIZE:
				if (attrib[1] > 0)
					visBits |= CR_STENCIL_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_ACCUM_RED_SIZE:
			case GLX_ACCUM_GREEN_SIZE:
			case GLX_ACCUM_BLUE_SIZE:
			case GLX_ACCUM_ALPHA_SIZE:
				if (attrib[1] > 0)
					visBits |= CR_ACCUM_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;

			case GLX_SAMPLE_BUFFERS_SGIS: /* aka GLX_SAMPLES_ARB */
#if MULTISAMPLE
				if (attrib[1] > 0)
					visBits |= CR_MULTISAMPLE_BIT;
				attrib++;
				attribCopy[copy++] = *attrib;
				break;
#endif
			case GLX_SAMPLES_SGIS: /* aka GLX_SAMPLES_ARB */
#if MULTISAMPLE
				/* just ignore value for now, we'll try to get 4 samples/pixel */
				attrib++;
				attribCopy[copy++] = *attrib;
				break;
#endif

			default:
				crWarning( "glXChooseVisual: bad attrib=0x%x", *attrib );
				return NULL;
		}
	}

	desiredVisual |= visBits;

	attribCopy[copy++] = None;

	if ( !wants_rgb && !(desiredVisual & CR_OVERLAY_BIT) )
	{
		/* normal layer, color index mode not supported */
		crWarning( "glXChooseVisual: didn't request RGB visual?" );
		return NULL;
	}

	/* try to satisfy this request with the native glXChooseVisual() */
	if (stub.haveNativeOpenGL &&
			stub.wsInterface.glXQueryExtension(dpy, &foo, &bar))
	{
		vis = stub.wsInterface.glXChooseVisual(dpy, screen, attribList);
		if (vis)
		{
			crDebug("faker native glXChooseVisual returning visual 0x%x",
							(int) vis->visualid);
			/* successful glXChooseVisual, so clear ours */
			desiredVisual = FindVisualInfo(dpy, vis);
		}
		else if (desiredVisual & CR_STEREO_BIT) {
			/* Try getting a monoscopic visual instead of stereo */
			int i;
			/* Replace GLX_STEREO with GLX_USE_GL to turn off stereo.
			 * XXX this algorithm isn't ideal since GLX_STEREO=6 and 6 could appear
			 * at various places in the attrib list (but that's unlikely).
			 */
			for (i = 0; i < copy; i++) {
				if (attribCopy[i] == GLX_STEREO) {
					attribCopy[i] = GLX_USE_GL;
				}
			}
			vis = stub.wsInterface.glXChooseVisual(dpy, screen, attribCopy);
			if (vis) {
				crDebug("Replacing request for stereo visual with non-stereo visual.");
			}
		}
	}
	else {
		/* we don't have the GLX extension, so try ordinary X visuals */
		vis = ReasonableVisual( dpy, screen );
		if (vis)
		{
			crDebug("faker glXChooseVisual returning visual 0x%x",
							(int) vis->visualid);
		}
	}

	/*crDebug("glXChooseVisual returning 0x%x", (int) vis->visual->visualid);*/
	return vis;
}

/**
 **  There is a problem with glXCopyContext.
 ** IRIX and Mesa both define glXCopyContext
 ** to have the mask argument being a 
 ** GLuint.  XFree 4 and oss.sgi.com
 ** define it to be an unsigned long.
 ** Solution: We don't support
 ** glXCopyContext anyway so we'll just
 ** #ifdef out the code.
 */
void
glXCopyContext( Display *dpy, GLXContext src, GLXContext dst, 
#if defined(AIX) || defined(PLAYSTATION2)
GLuint mask )
#elif defined(SunOS)
unsigned int mask )
#else
unsigned long mask )
#endif
{
	(void) dpy;
	(void) src;
	(void) dst;
	(void) mask;
	crWarning( "Unsupported GLX Call: glXCopyContext()" );
}


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



GLXContext
glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct)
{
	char dpyName[MAX_DPY_NAME];
	ContextInfo *context;

	stubInit();

	CRASSERT(stub.contextTable);

	stubGetDisplayString(dpy, dpyName, MAX_DPY_NAME);
	if (stub.haveNativeOpenGL) {
		int foo, bar;
		if (stub.wsInterface.glXQueryExtension(dpy, &foo, &bar)) {
			int visBits = FindVisualInfo( dpy, vis );
			crDebug("FindVisualInfo(0x%x) = 0x%x", (int)vis->visual->visualid, visBits);
			desiredVisual |= visBits;
			if (stub.force_pbuffers) {
				crInfo("App faker: Forcing use of Pbuffers");
				desiredVisual |= CR_PBUFFER_BIT;
			}
		}
	}

	context = stubNewContext(dpyName, desiredVisual, UNDECIDED);
	if (!context)
		return 0;

	context->dpy = dpy;
	context->visual = vis;
	context->direct = direct;
	context->share = (ContextInfo *) crHashtableSearch(stub.contextTable, (unsigned long) share);

	return (GLXContext) context->id;
}


void glXDestroyContext( Display *dpy, GLXContext ctx )
{
	(void) dpy;
	stubDestroyContext( (unsigned long) ctx );
}


Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
	ContextInfo *context;
	WindowInfo *window;
	Bool retVal;

	/*crDebug("glXMakeCurrent(%p, 0x%x, 0x%x)", (void *) dpy, (int) drawable, (int) ctx);*/

	context = (ContextInfo *) crHashtableSearch(stub.contextTable, (unsigned long) ctx);
	window = stubGetWindowInfo(dpy, drawable);

	if (context && context->type == UNDECIDED) {
		XSync(dpy, 0); /* sync to force window creation on the server */
	}

	currentDisplay = dpy;
	currentDrawable = drawable;

	retVal = stubMakeCurrent(window, context);
	return retVal;
}


GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *vis, Pixmap pixmap )
{
	(void) dpy;
	(void) vis;
	(void) pixmap;

	stubInit();
	crWarning( "Unsupported GLX Call: glXCreateGLXPixmap()" );
	return (GLXPixmap) 0;
}

void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pix )
{
	(void) dpy;
	(void) pix;
	crWarning( "Unsupported GLX Call: glXDestroyGLXPixmap()" );
}

int glXGetConfig( Display *dpy, XVisualInfo *vis, int attrib, int *value )
{
	int visBits = desiredVisual;
	(void) dpy;
	(void) vis;

	stubInit();

	/* try to satisfy this request with the native glXGetConfig() */
	if (stub.haveNativeOpenGL)
	{
		int foo, bar;
		int return_val;

		if (stub.wsInterface.glXQueryExtension(dpy, &foo, &bar))
		{
			return_val = stub.wsInterface.glXGetConfig( dpy, vis, attrib, value );
			if (return_val)
			{
				crDebug("faker native glXGetConfig returned %s",
								glx_error_string(return_val));
			}
			return return_val;
		}
	}

	/*
	 * If the GLX application chooses its visual via a bunch of calls to
	 * glXGetConfig, instead of by calling glXChooseVisual, we need to keep
	 * track of which attributes are queried to help satisfy context creation
	 * later.
	 */
	switch ( attrib ) {

		case GLX_USE_GL:
			*value = 1;
			break;

		case GLX_BUFFER_SIZE:
			*value = 32;
			break;

		case GLX_LEVEL:
			*value = (visBits & CR_OVERLAY_BIT) ? 1 : 0;
			break;

		case GLX_RGBA:
			*value = 1;
			break;

		case GLX_DOUBLEBUFFER:
			*value = 1;
			break;

		case GLX_STEREO:
			*value = 1;
			break;

		case GLX_AUX_BUFFERS:
			*value = 0;
			break;

		case GLX_RED_SIZE:
			*value = 8;
			break;

		case GLX_GREEN_SIZE:
			*value = 8;
			break;

		case GLX_BLUE_SIZE:
			*value = 8;
			break;

		case GLX_ALPHA_SIZE:
			*value = (visBits & CR_ALPHA_BIT) ? 8 : 0;
			break;

		case GLX_DEPTH_SIZE:
			*value = 16;
			break;

		case GLX_STENCIL_SIZE:
			visBits |= CR_STENCIL_BIT;
			*value = 8;
			break;

		case GLX_ACCUM_RED_SIZE:
			visBits |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_GREEN_SIZE:
			visBits |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_BLUE_SIZE:
			visBits |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_ALPHA_SIZE:
			visBits |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_SAMPLE_BUFFERS_SGIS:
#if MULTISAMPLE
			visBits |= CR_MULTISAMPLE_BIT;
			*value = 0;  /* fix someday */
			break;
#endif

		case GLX_SAMPLES_SGIS:
#if MULTISAMPLE
			visBits |= CR_MULTISAMPLE_BIT;
			*value = 0;  /* fix someday */
			break;
#endif

		case GLX_VISUAL_CAVEAT_EXT:
			*value = GLX_NONE_EXT;
			break;
#ifdef SunOS
		/*
		  I don't think this is even a valid attribute for glxGetConfig. 
		  No idea why this gets called under SunOS but we simply ignore it
		  -- jw
		*/
		case GLX_X_VISUAL_TYPE:
		  crWarning ("Ignoring Unsuported GLX Call: glxGetConfig with attrib 0x%x", attrib);
		  break;
#endif 

		case GLX_TRANSPARENT_TYPE:
			*value = GLX_NONE_EXT;
			break;
		case GLX_TRANSPARENT_INDEX_VALUE:
			*value = 0;
			break;
		case GLX_TRANSPARENT_RED_VALUE:
			*value = 0;
			break;
		case GLX_TRANSPARENT_GREEN_VALUE:
			*value = 0;
			break;
		case GLX_TRANSPARENT_BLUE_VALUE:
			*value = 0;
			break;
		case GLX_TRANSPARENT_ALPHA_VALUE:
			*value = 0;
			break;
		default:
			crWarning( "Unsupported GLX Call: glXGetConfig with attrib 0x%x", attrib );
			return GLX_BAD_ATTRIBUTE;
		
	}

	desiredVisual |= visBits;

	return 0;
}

GLXContext glXGetCurrentContext( void )
{
	if (stub.currentContext)
		return (GLXContext) stub.currentContext->id;
	else
		return (GLXContext) NULL;
}

GLXDrawable glXGetCurrentDrawable( void )
{
	return currentDrawable;
}

Display *glXGetCurrentDisplay( void )
{
	return currentDisplay;
}

Bool glXIsDirect( Display *dpy, GLXContext ctx )
{
	(void) dpy;
	(void) ctx;
	return True;
}

Bool glXQueryExtension( Display *dpy, int *errorBase, int *eventBase )
{
	(void) dpy;
	(void) errorBase;
	(void) eventBase;
	return 1; /* You BET we do... */
}

Bool glXQueryVersion( Display *dpy, int *major, int *minor )
{
	(void) dpy;
	*major = 1;
	*minor = 3;

	return 1;
}

void glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
	const WindowInfo *window = stubGetWindowInfo(dpy, drawable);
	stubSwapBuffers( window, 0 );
}

void glXUseXFont( Font font, int first, int count, int listBase )
{
	if (stub.currentContext->type == CHROMIUM)
	{
		Display *dpy = stub.wsInterface.glXGetCurrentDisplay();
		if (dpy) {
			stubUseXFont( dpy, font, first, count, listBase );
		}
		else {
			dpy = XOpenDisplay(NULL);
			if (!dpy)
				return;
			stubUseXFont( dpy, font, first, count, listBase );
			XCloseDisplay(dpy);
		}
	} else
		stub.wsInterface.glXUseXFont( font, first, count, listBase );
}

void glXWaitGL( void )
{
	static int first_call = 1;

	if ( first_call )
	{
		crDebug( "Ignoring unsupported GLX call: glXWaitGL()" );
		first_call = 0;
	}
}

void glXWaitX( void )
{
	static int first_call = 1;

	if ( first_call )
	{
		crDebug( "Ignoring unsupported GLX call: glXWaitX()" );
		first_call = 0;
	}
}

const char *glXQueryExtensionsString( Display *dpy, int screen )
{
	/* XXX maybe also advertise GLX_SGIS_multisample? */
	static const char *retval = "GLX_ARB_multisample";
	(void) dpy;
	(void) screen;

	return retval;
}

const char *glXGetClientString( Display *dpy, int name )
{
	const char *retval;
	(void) dpy;
	(void) name;

	switch ( name ) {

		case GLX_VENDOR:
			retval  = "Chromium";
			break;

		case GLX_VERSION:
			retval  = "1.2 Chromium";
			break;

		case GLX_EXTENSIONS:
			retval  = "";
			break;

		default:
			retval  = NULL;
	}

	return retval;
}

const char *glXQueryServerString( Display *dpy, int screen, int name )
{
	const char *retval;
	(void) dpy;
	(void) screen;

	switch ( name ) {

		case GLX_VENDOR:
			retval  = "Chromium";
			break;

		case GLX_VERSION:
			retval  = "1.2 Chromium";
			break;

		case GLX_EXTENSIONS:
			retval  = "";
			break;

		default:
			retval  = NULL;
	}

	return retval;
}

CR_GLXFuncPtr glXGetProcAddressARB( const GLubyte *name )
{
	return (CR_GLXFuncPtr) crGetProcAddress( (const char *) name );
}

CR_GLXFuncPtr glXGetProcAddress( const GLubyte *name )
{
	return (CR_GLXFuncPtr) crGetProcAddress( (const char *) name );
}


#if GLX_EXTRAS

GLXPbufferSGIX glXCreateGLXPbufferSGIX(Display *dpy, GLXFBConfigSGIX config,
																			 unsigned int width, unsigned int height,
																			 int *attrib_list)
{
	(void) dpy;
	(void) config;
	(void) width;
	(void) height;
	(void) attrib_list;
	crWarning("glXCreateGLXPbufferSGIX not implemented by Chromium");
	return 0;
}

void glXDestroyGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf)
{
	(void) dpy;
	(void) pbuf;
	crWarning("glXDestroyGLXPbufferSGIX not implemented by Chromium");
}

void glXSelectEventSGIX(Display *dpy, GLXDrawable drawable, unsigned long mask)
{
	(void) dpy;
	(void) drawable;
	(void) mask;
}

void glXGetSelectedEventSGIX(Display *dpy, GLXDrawable drawable, unsigned long *mask)
{
	(void) dpy;
	(void) drawable;
	(void) mask;
}

int glXQueryGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf,
													 int attribute, unsigned int *value)
{
	(void) dpy;
	(void) pbuf;
	(void) attribute;
	(void) value;
	crWarning("glXQueryGLXPbufferSGIX not implemented by Chromium");
	return 0;
}

int glXGetFBConfigAttribSGIX(Display *dpy, GLXFBConfig config,
														 int attribute, int *value)
{
	(void) dpy;
	(void) config;
	(void) attribute;
	(void) value;
	crWarning("glXGetFBConfigAttribSGIX not implemented by Chromium");
	return 0;
}

GLXFBConfigSGIX *glXChooseFBConfigSGIX(Display *dpy, int screen,
					 int *attrib_list, int *nelements)
{
	(void) dpy;
	(void) screen;
	(void) attrib_list;
	(void) nelements;
	crWarning("glXChooseFBConfigSGIX not implemented by Chromium");
	return NULL;
}

GLXPixmap glXCreateGLXPixmapWithConfigSGIX(Display *dpy,
																					 GLXFBConfig config,
																					 Pixmap pixmap)
{
	(void) dpy;
	(void) config;
	(void) pixmap;
	crWarning("glXCreateGLXPixmapWithConfigSGIX not implemented by Chromium");
	return 0;	}

GLXContext glXCreateContextWithConfigSGIX(Display *dpy, GLXFBConfig config,
																					int render_type,
																					GLXContext share_list,
																					Bool direct)
{
	(void) dpy;
	(void) config;
	(void) render_type;
	(void) share_list;
	(void) direct;
	crWarning("glXCreateContextWithConfigSGIX not implemented by Chromium");
	return NULL;
}

XVisualInfo *glXGetVisualFromFBConfigSGIX(Display *dpy,
																					GLXFBConfig config)
{
	(void) dpy;
	(void) config;
	crWarning("glXGetVisualFromFBConfigSGIX not implemented by Chromium");
	return NULL;
}

GLXFBConfigSGIX glXGetFBConfigFromVisualSGIX(Display *dpy, XVisualInfo *vis)
{
	(void) dpy;
	(void) vis;
	crWarning("glXGetFBConfigFromVisualSGIX not implemented by Chromium");
	return NULL;
}

/*
 * GLX 1.3 functions
 */
GLXFBConfig *glXChooseFBConfig(Display *dpy, int screen, ATTRIB_TYPE *attrib_list, int *nelements)
{
	(void) dpy;
	(void) screen;
	(void) attrib_list;
	(void) nelements;
	crWarning("glXChooseFBConfig not implemented by Chromium");
	return NULL;
}

GLXContext glXCreateNewContext(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct)
{
	(void) dpy;
	(void) config;
	(void) render_type;
	(void) share_list;
	(void) direct;
	crWarning("glXCreateNewContext not implemented by Chromium");
	return NULL;
}

GLXPbuffer glXCreatePbuffer(Display *dpy, GLXFBConfig config, ATTRIB_TYPE *attrib_list)
{
	(void) dpy;
	(void) config;
	(void) attrib_list;
	crWarning("glXCreatePbuffer not implemented by Chromium");
	return 0;
}

GLXPixmap glXCreatePixmap(Display *dpy, GLXFBConfig config, Pixmap pixmap, ATTRIB_TYPE *attrib_list)
{
	(void) dpy;
	(void) config;
	(void) pixmap;
	(void) attrib_list;
	crWarning("glXCreatePixmap not implemented by Chromium");
	return 0;
}

GLXWindow glXCreateWindow(Display *dpy, GLXFBConfig config, Window win, ATTRIB_TYPE *attrib_list)
{
	(void) dpy;
	(void) config;
	(void) win;
	(void) attrib_list;
	crWarning("glXCreateWindow not implemented by Chromium");
	return 0;
}

void glXDestroyPbuffer(Display *dpy, GLXPbuffer pbuf)
{
	(void) dpy;
	(void) pbuf;
	crWarning("glXDestroyPbuffer not implemented by Chromium");
}
void glXDestroyPixmap(Display *dpy, GLXPixmap pixmap)
{
	(void) dpy;
	(void) pixmap;
	crWarning("glXDestroyPixmap not implemented by Chromium");
}

void glXDestroyWindow(Display *dpy, GLXWindow win)
{
	(void) dpy;
	(void) win;
	crWarning("glXDestroyWindow not implemented by Chromium");
}

GLXDrawable glXGetCurrentReadDrawable(void)
{
	crWarning("glXGetCurrentReadDrawable not implemented by Chromium");
	return 0;
}

int glXGetFBConfigAttrib(Display *dpy, GLXFBConfig config, int attribute, int *value)
{
	(void) dpy;
	(void) config;
	(void) attribute;
	(void) value;
	crWarning("glXGetFBConfigAttrib not implemented by Chromium");
	return 0;
}

GLXFBConfig *glXGetFBConfigs(Display *dpy, int screen, int *nelements)
{
	(void) dpy;
	(void) screen;
	(void) nelements;
	crWarning("glXGetFBConfigs not implemented by Chromium");
	return NULL;
}

void glXGetSelectedEvent(Display *dpy, GLXDrawable draw, unsigned long *event_mask)
{
	(void) dpy;
	(void) draw;
	(void) event_mask;
	crWarning("glXGetSelectedEvent not implemented by Chromium");
}

XVisualInfo *glXGetVisualFromFBConfig(Display *dpy, GLXFBConfig config)
{
	(void) dpy;
	(void) config;
	crWarning("glXGetVisualFromFBConfig not implemented by Chromium");
	return NULL;
}

Bool glXMakeContextCurrent(Display *display, GLXDrawable draw, GLXDrawable read, GLXContext ctx)
{
	(void) display;
	(void) draw;
	(void) read;
	(void) ctx;
	crWarning("glXMakeContextCurrent not implemented by Chromium");
	return 0;
}

int glXQueryContext(Display *dpy, GLXContext ctx, int attribute, int *value)
{
	(void) dpy;
	(void) ctx;
	(void) attribute;
	(void) value;
	crWarning("glXQueryContext not implemented by Chromium");
	return 0;
}

void glXQueryDrawable(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value)
{
	(void) dpy;
	(void) draw;
	(void) attribute;
	(void) value;
	crWarning("glXQueryDrawable not implemented by Chromium");
}

void glXSelectEvent(Display *dpy, GLXDrawable draw, unsigned long event_mask)
{
	(void) dpy;
	(void) draw;
	(void) event_mask;
	crWarning("glXSelectEvent not implemented by Chromium");
}


#endif /* GLX_EXTRAS */

