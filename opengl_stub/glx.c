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

extern SPU *stub_spu;
extern void StubInit(void);

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
		if ( visual[i].class == TrueColor || visual[i].class == DirectColor )
		{
			best = &visual[i];
		}
	}

	if ( best ) {

		/* okay, select the RGB visual with the most depth */
		for ( i = 0; i < n; i++ )
		{
			if ( ( visual[i].class == TrueColor ||
						visual[i].class == DirectColor ) &&
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

XVisualInfo *glXChooseVisual( Display *dpy, int screen, int *attribList )
{
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
						crWarning( "glXChooseVisual: "
								"level=%d unsupported", level );
						return NULL;
					}
				}
				attrib++;
				break;

			case GLX_RGBA:
				wants_rgb = 1;
				break;

			case GLX_DOUBLEBUFFER:
				/* ignored? */
				/* wants_doublebuffer = 1; */
				break;

			case GLX_STEREO:
				crWarning( "glXChooseVisual: stereo "
						"unsupported" );
				return NULL;

			case GLX_AUX_BUFFERS:
				{
					int aux_buffers = attrib[1];
					if ( aux_buffers != 0 )
					{
						crWarning( "glXChooseVisual: "
								"aux_buffers=%d unsupported", aux_buffers );
						return NULL;
					}
				}
				attrib++;
				break;

			case GLX_RED_SIZE:
			case GLX_GREEN_SIZE:
			case GLX_BLUE_SIZE:
			case GLX_ALPHA_SIZE:
			case GLX_DEPTH_SIZE:
			case GLX_STENCIL_SIZE:
				/* ignored */
				attrib++;
				break;

			case GLX_ACCUM_RED_SIZE:
			case GLX_ACCUM_GREEN_SIZE:
			case GLX_ACCUM_BLUE_SIZE:
			case GLX_ACCUM_ALPHA_SIZE:
				/* unsupported */
				{
					int accum_size = attrib[1];
					if ( accum_size != 0 )
					{
						crWarning( "glXChooseVisual: "
								"accum_size=%d unsupported", accum_size );
						return NULL;
					}
				}
				attrib++;
				break;

			default:
				crWarning( "glXChooseVisual: bad "
						"attrib=0x%x", *attrib );
				return NULL;
		}
	}

	if ( !wants_rgb )
	{
		crWarning( "glXChooseVisual: didn't request"
				"RGB visual?" );
		return NULL;
	}

	return ReasonableVisual( dpy, screen );
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

#if 0
	void
glXCopyContext( Display *dpy, GLXContext src, GLXContext dst, GLuint mask )
{
	crError( "Unsupported GLX Call: glXCopyContext()" );
}
#endif

GLXContext glXCreateContext( Display *dpy, XVisualInfo *vis, GLXContext share,
		Bool direct )
{
	(void) dpy;
	(void) vis;
	(void) share;
	(void) direct;
#if 0
	// This is moving to glXMakeCurrent because the tilesort SPU needs
	// to know the drawable in order to do something intelligent with
	// the viewport calls.

	StubInit();
	stub_spu->dispatch_table.CreateContext();
#endif
	return (GLXContext) "foo";
}

GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *vis, Pixmap pixmap )
{
	(void) dpy;
	(void) vis;
	(void) pixmap;

	crError( "Unsupported GLX Call: glXCreateGLXPixmap()" );
	return (GLXPixmap) 0;
}

void glXDestroyContext( Display *dpy, GLXContext ctx )
{
	(void) dpy;
	(void) ctx;
	crError( "Unsupported GLX Call: glXDestroyContext()" );
}

void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pix )
{
	(void) dpy;
	(void) pix;
	crError( "Unsupported GLX Call: glXDestroyGLXPixmap()" );
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
			*value = 8;
			break;

		case GLX_DEPTH_SIZE:
			*value = 24;
			break;

		case GLX_STENCIL_SIZE:
			*value = 8;
			break;

		case GLX_ACCUM_RED_SIZE:
			*value = 0;
			break;

		case GLX_ACCUM_GREEN_SIZE:
			*value = 0;
			break;

		case GLX_ACCUM_BLUE_SIZE:
			*value = 0;
			break;

		case GLX_ACCUM_ALPHA_SIZE:
			*value = 0;
			break;

		default:
			return GLX_BAD_ATTRIBUTE;
	}

	return 0;
}

GLXContext glXGetCurrentContext( void )
{
	crError( "Unsupported GLX Call: glXGetCurrentContext()" );
	return (GLXContext) 0;
}

GLXDrawable glXGetCurrentDrawable( void )
{
	crError( "Unsupported GLX Call: glXGetCurrentDrawable()" );
	return (GLXDrawable) 0;
}

Bool glXIsDirect( Display *dpy, GLXContext ctx )
{
	(void) dpy;
	(void) ctx;
	return 1;
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
	static int first_time = 1;
	(void) dpy;
	(void) drawable;
	(void) ctx;
	if (first_time)
	{
		first_time = 0;
		StubInit();
		stub_spu->dispatch_table.CreateContext( (void *) dpy, (void *) drawable );
	}
	stub_spu->dispatch_table.MakeCurrent();
	return True;
}

Bool glXQueryExtension( Display *dpy, int *errorBase, int *eventBase )
{
	(void) dpy;
	(void) errorBase;
	(void) eventBase;
	return 1; // You BET we do...
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
	(void) dpy;
	(void) drawable;
	stub_spu->dispatch_table.SwapBuffers();
}

void glXUseXFont( Font font, int first, int count, int listBase )
{
	(void) font;
	(void) first;
	(void) count;
	(void) listBase;
	crError( "Unsupported GLX Call: glXUseXFont()" );
}

void glXWaitGL( void )
{
	static int first_call = 1;

	if ( first_call )
	{
		crDebug( "Ignoriing unsupported GLX call: glXWaitGL()" );
		first_call = 0;
	}
}

void glXWaitX( void )
{
	static int first_call = 1;

	if ( first_call )
	{
		crDebug( "Ignoriing unsupported GLX call: glXWaitX()" );
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
