/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* opengl_stub/glx.c */

#include <GL/glx.h>

#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_applications.h"
#include "cr_mothership.h"
#include "stub.h"

#ifndef GLX_SAMPLE_BUFFERS_SGIS
#define GLX_SAMPLE_BUFFERS_SGIS    0x186a0 /*100000*/
#endif
#ifndef GLX_SAMPLES_SGIS
#define GLX_SAMPLES_SGIS           0x186a1 /*100001*/
#endif
#ifndef GLX_VISUAL_CAVEAT_EXT
#define GLX_VISUAL_CAVEAT_EXT       0x20  /* visual_rating extension type */
#endif


/* For optimizing glXMakeCurrent */
static Display *currentDisplay = NULL;
static GLXDrawable currentDrawable = 0;
static GLXContext currentContext = 0;

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

int FindVisualInfo( Display *dpy, XVisualInfo *vInfo )
{
	int desiredVisual = 0;
	GLint doubleBuffer, stereo;
	GLint alphaSize, depthSize, stencilSize;
	GLint accumRedSize, accumGreenSize, accumBlueSize;
	/* Should check more ??? */

   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &doubleBuffer);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_STEREO, &stereo);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &alphaSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &depthSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &stencilSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &accumRedSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &accumRedSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &accumRedSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &accumGreenSize);
   	stub.wsInterface.glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &accumBlueSize);

	desiredVisual |= CR_RGB_BIT;	/* assuming we always want this... */

	if (alphaSize > 0)
		desiredVisual |= CR_ALPHA_BIT;
	if (depthSize > 0)
		desiredVisual |= CR_DEPTH_BIT;
	if (stencilSize > 0)
		desiredVisual |= CR_STENCIL_BIT;
	if (accumRedSize > 0 || accumGreenSize > 0 || accumBlueSize > 0)
		desiredVisual |= CR_ACCUM_BIT;
	if (doubleBuffer)
		desiredVisual |= CR_DOUBLE_BIT;
	if (stereo)
		desiredVisual |= CR_STEREO_BIT;
	/* Should we be checking more ... ??? */

	return desiredVisual;
}

XVisualInfo *glXChooseVisual( Display *dpy, int screen, int *attribList )
{
	XVisualInfo *vis;
	int *attrib, wants_rgb;

	wants_rgb = 0;

	for ( attrib = attribList; *attrib != None; attrib++ )
	{
		switch ( *attrib )
		{
			case GLX_USE_GL:
				/* ignored, this is mandatory */
				break;

			case GLX_BUFFER_SIZE:
				/* this is for color-index visuals, which we don't support */
				attrib++;
				break;

			case GLX_LEVEL:
				{
					int level = attrib[1];
					if ( level != 0 )
					{
						crWarning( "glXChooseVisual: GLX_LEVEL = %d unsupported", level );
						return NULL;
					}
				}
				attrib++;
				break;

			case GLX_RGBA:
				stub.desiredVisual |= CR_RGB_BIT;
				wants_rgb = 1;
				break;

			case GLX_DOUBLEBUFFER:
				stub.desiredVisual |= CR_DOUBLE_BIT;
				/* wants_doublebuffer = 1; */
				break;

			case GLX_STEREO:
				stub.desiredVisual |= CR_STEREO_BIT;
				crWarning( "glXChooseVisual: stereo unsupported" );
				return NULL;

			case GLX_AUX_BUFFERS:
				{
					int aux_buffers = attrib[1];
					if ( aux_buffers != 0 )
					{
						crWarning( "glXChooseVisual: aux_buffers=%d unsupported",
											 aux_buffers );
						return NULL;
					}
				}
				attrib++;
				break;

			case GLX_RED_SIZE:
			case GLX_GREEN_SIZE:
			case GLX_BLUE_SIZE:
				if (attrib[1] > 0)
					stub.desiredVisual |= CR_RGB_BIT;
				attrib++;
				break;

			case GLX_ALPHA_SIZE:
				if (attrib[1] > 0)
					stub.desiredVisual |= CR_ALPHA_BIT;
				attrib++;
				break;

			case GLX_DEPTH_SIZE:
				if (attrib[1] > 0)
					stub.desiredVisual |= CR_DEPTH_BIT;
				attrib++;
				break;

			case GLX_STENCIL_SIZE:
				if (attrib[1] > 0)
					stub.desiredVisual |= CR_STENCIL_BIT;
				attrib++;
				break;

			case GLX_ACCUM_RED_SIZE:
			case GLX_ACCUM_GREEN_SIZE:
			case GLX_ACCUM_BLUE_SIZE:
			case GLX_ACCUM_ALPHA_SIZE:
				if (attrib[1] > 0)
					stub.desiredVisual |= CR_ACCUM_BIT;
				attrib++;
				break;

			case GLX_SAMPLE_BUFFERS_SGIS:
				if (attrib[1] > 0) {
					return NULL;  /* don't handle multisample yet */
					/* eventually... */
					/*stub.desiredVisual |= CR_MULTISAMPLE_BIT;*/
				}
				attrib++;
				break;
			case GLX_SAMPLES_SGIS:
				if (attrib[1] > 0) {
					return NULL;  /* don't handle multisample yet */
					/* eventually... */
					/*stub.desiredVisual |= CR_MULTISAMPLE_BIT;*/
				}
				attrib++;
				break;

			default:
				crWarning( "glXChooseVisual: bad attrib=0x%x", *attrib );
				return NULL;
		}
	}

	if ( !wants_rgb )
	{
		crWarning( "glXChooseVisual: didn't request RGB visual?" );
		return NULL;
	}

	vis = ReasonableVisual( dpy, screen );
	return vis;
}

/* There is a problem with glXCopyContext.
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
#if defined(AIX)
GLuint mask )
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



/*
 * For now we're ignoring all the parameters.
 */
GLXContext glXCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share, Bool direct )
{
	return stubCreateContext( dpy, vis, share, direct);
}


void glXDestroyContext( Display *dpy, GLXContext ctx )
{
	stubDestroyContext( dpy, ctx );
}


Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
	Bool retVal;

	if (drawable == 0 && ctx == 0) {
		/* Glean is one app that calls glXMakeCurrent(dpy, 0, 0).
		 * We can safely ignore it.
		 */
		currentDrawable = 0;
		currentContext = 0;
		return True;
	}

	/*
	if (currentDrawable && currentDrawable != drawable)
		crWarning("Multiple drawables not fully supported!");
	*/

	retVal = stubMakeCurrent( dpy, drawable, ctx );

	if (retVal) {
		currentDisplay = dpy;
		currentDrawable = drawable;
		currentContext = ctx;
	}

	return retVal;
}


GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *vis, Pixmap pixmap )
{
	(void) dpy;
	(void) vis;
	(void) pixmap;

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
	(void) dpy;
	(void) vis;

	switch ( attrib ) {

		case GLX_USE_GL:
			*value = 1;
			break;

		case GLX_BUFFER_SIZE:
			*value = 32;
			break;

		case GLX_LEVEL:
			*value = 0;
			break;

		case GLX_RGBA:
			*value = 1;
			break;

		case GLX_DOUBLEBUFFER:
			*value = 1;
			break;

		case GLX_STEREO:
			*value = 0;
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
			*value = (stub.desiredVisual & CR_ALPHA_BIT) ? 8 : 0;
			break;

		case GLX_DEPTH_SIZE:
			*value = 16;
			break;

		case GLX_STENCIL_SIZE:
			stub.desiredVisual |= CR_STENCIL_BIT;
			*value = 8;
			break;

		case GLX_ACCUM_RED_SIZE:
			stub.desiredVisual |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_GREEN_SIZE:
			stub.desiredVisual |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_BLUE_SIZE:
			stub.desiredVisual |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_ACCUM_ALPHA_SIZE:
			stub.desiredVisual |= CR_ACCUM_BIT;
			*value = 16;
			break;

		case GLX_SAMPLE_BUFFERS_SGIS:
			stub.desiredVisual |= CR_MULTISAMPLE_BIT;
			*value = 0;  /* fix someday */
			break;

		case GLX_SAMPLES_SGIS:
			stub.desiredVisual |= CR_MULTISAMPLE_BIT;
			*value = 0;  /* fix someday */
			break;


		case GLX_VISUAL_CAVEAT_EXT:
			*value = GLX_NONE_EXT;
			break;

		default:
			crWarning( "Unsupported GLX Call: glXGetConfig with attrib 0x%x", attrib );
			return GLX_BAD_ATTRIBUTE;
	}

	return 0;
}

GLXContext glXGetCurrentContext( void )
{
	return currentContext;
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
	stubSwapBuffers( dpy, drawable );
}

void glXUseXFont( Font font, int first, int count, int listBase )
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
	static const char *retval = "";
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
			retval  = "1.0 Chromium";
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
			retval  = "1.0 Chromium";
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
