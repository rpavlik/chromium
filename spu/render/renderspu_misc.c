/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_string.h"
#include "cr_error.h"
#include "renderspu.h"


/* used for debugging and giving info to the user */
void renderspuMakeVisString( GLbitfield visAttribs, char *s )
{
	s[0] = 0;

	if (visAttribs & CR_RGB_BIT)
		crStrcat(s, "RGB");
	if (visAttribs & CR_ALPHA_BIT)
		crStrcat(s, "A");
	if (visAttribs & CR_DOUBLE_BIT)
		crStrcat(s, ", Doublebuffer");
	if (visAttribs & CR_STEREO_BIT)
		crStrcat(s, ", Stereo");
	if (visAttribs & CR_DEPTH_BIT)
		crStrcat(s, ", Z");
	if (visAttribs & CR_STENCIL_BIT)
		crStrcat(s, ", Stencil");
	if (visAttribs & CR_ACCUM_BIT)
		crStrcat(s, ", Accum");
	if (visAttribs & CR_MULTISAMPLE_BIT)
		crStrcat(s, ", Multisample");
}


static GLint RENDER_APIENTRY renderspuCreateContext( void *dpy, GLint visual)
{
	static int numCalls = 0;
	(void) dpy;
	(void) visual;

#if 000
	/*
	 * Verify that we can satisfy the app's visual requirements.
	 */
	if (!(visual & CR_RGB_BIT))
		 crError("App requested color index visual!");

	if ((visual & CR_DEPTH_BIT) && !render_spu.depth_bits)
		 crError("App requested depth(Z) buffer but render SPU not configured for it.");

	if ((visual & CR_ALPHA_BIT) && !render_spu.alpha_bits)
		 crError("App requested alpha channel but render SPU not configured for it.");

	if ((visual & CR_STENCIL_BIT) && !render_spu.stencil_bits)
		 crError("App requested stencil buffer but render SPU not configured for it.");

	if ((visual & CR_ACCUM_BIT) && !render_spu.accum_bits)
		 crError("App requested accum buffer but render SPU not configured for it.");
#endif

	numCalls++;

	if ( numCalls == 1)
	{
		if ( visual != (GLint) render_spu.visAttribs )
		{
			/* This is the first call to MakeCurrent() and the window that we
			 * originally created does not have the visual attributes that the
			 * application wants.  Darn.  So we re-create the render SPU's window
			 * now with the new visual attributes.
			 * We only do this for the first call since if we have multiple
			 * contexts (with different visual attributes) rendering into the same
			 * window we don't want to keep destroying/recreating the window!
			 * We'll just hope that the visuals of the first context are a superset
			 * of what later contexts want.
			 */
			char s1[200], s2[200];
			renderspuMakeVisString(render_spu.visAttribs, s1);
			renderspuMakeVisString(visual, s2);
			crDebug("Reconfiguring SPU window:");
			crDebug("  from: %s (0x%x)", s1, render_spu.visAttribs);
			crDebug("    to: %s (0x%x)", s2, visual);
			render_spu.visAttribs = visual;
			if (!renderspuCreateWindow( render_spu.visAttribs, GL_TRUE ))
				return 0;
		}
		else
		{
			/* We map/display the window upon the first MakeCurrent call. */
			renderspuShowWindow(GL_TRUE);
			crDebug("First call to MakeCurrent, window visual is OK.");
		}
	}

	return 1;  /* non-zero indicates success, value not relevant otherwise */
}

static void RENDER_APIENTRY renderspuDestroyContext( void *dpy, GLint ctx )
{
	(void) dpy;
	(void) ctx;
}

static void RENDER_APIENTRY renderspuMakeCurrent(void *dpy, GLint drawable, GLint ctx)
{
	(void) dpy;
	(void) drawable;
	(void) ctx;
}


static void RENDER_APIENTRY renderspuChromiumParameteriCR(GLenum target, GLint value)
{
	(void) target;
	(void) value;
#if 0
	switch (target) {
	default:
		crWarning("Unhandled target in renderspuChromiumParameteriCR()");
		break;
	}
#endif
}

static void RENDER_APIENTRY renderspuChromiumParameterfCR(GLenum target, GLfloat value)
{
	(void) target;
	(void) value;
#if 0
	switch (target) {
	default:
		crWarning("Unhandled target in renderspuChromiumParameterfCR()");
		break;
	}
#endif
}

static void RENDER_APIENTRY renderspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	switch (target) {
	case GL_CURSOR_POSITION_CR:
		if (type == GL_INT && count == 2) {
			render_spu.cursorX = ((GLint *) values)[0];
			render_spu.cursorY = ((GLint *) values)[1];
		}
		else {
			crWarning("Bad type or count for ChromiumParametervCR(GL_CURSOR_POSITION_CR)");
		}
		break;
	default:
#if 0
		crWarning("Unhandled target in renderspuChromiumParametervCR(0x%x)", (int) target);
#endif
		break;
	}
}

static void RENDER_APIENTRY renderspuGetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	(void) target;
	(void) index;
	(void) type;
	(void) count;
	(void) values;
}

static void RENDER_APIENTRY renderspuBarrierCreate( GLuint name, GLuint count )
{
	(void) name;
	(void) count;
}

static void RENDER_APIENTRY renderspuBarrierDestroy( GLuint name )
{
	(void) name;
}

static void RENDER_APIENTRY renderspuBarrierExec( GLuint name )
{
	(void) name;
}

static void RENDER_APIENTRY renderspuBoundsInfo( GLrecti *bounds, GLbyte *payload, GLint
 len, GLint num_opcodes )
{
	(void) bounds;
	(void) payload;
	(void) len;
	(void) num_opcodes;
}

static void RENDER_APIENTRY renderspuSemaphoreCreate( GLuint name, GLuint count )
{
	(void) name;
	(void) count;
}

static void RENDER_APIENTRY renderspuSemaphoreDestroy( GLuint name )
{
	(void) name;
}

static void RENDER_APIENTRY renderspuSemaphoreP( GLuint name )
{
	(void) name;
}

static void RENDER_APIENTRY renderspuSemaphoreV( GLuint name )
{
	(void) name;
}

static void RENDER_APIENTRY renderspuWriteback( GLint *writeback )
{
	(void) writeback;
}

/* We really only need this function in order to strip the
 * GL_SINGLE_CLIENT_BIT_CR bit from the bitmask so we don't generate
 * a GL error.
 */
void RENDER_APIENTRY renderspuClear( GLbitfield mask )
{
	render_spu.ClearFunc(mask & ~GL_SINGLE_CLIENT_BIT_CR);
}


#define FILLIN( NAME, FUNC ) \
  table[i].name = crStrdup(NAME); \
  table[i].fn = (SPUGenericFunction) FUNC; \
  i++;


/* These are the functions which the render SPU implements, not OpenGL.
 */
int renderspuCreateFunctions( SPUNamedFunctionTable table[] )
{
	int i = 0;
	FILLIN( "SwapBuffers", renderspuSwapBuffers );
	FILLIN( "CreateContext", renderspuCreateContext );
	FILLIN( "DestroyContext", renderspuDestroyContext );
	FILLIN( "MakeCurrent", renderspuMakeCurrent );
	FILLIN( "BarrierCreate", renderspuBarrierCreate );
	FILLIN( "BarrierDestroy", renderspuBarrierDestroy );
	FILLIN( "BarrierExec", renderspuBarrierExec );
	FILLIN( "BoundsInfo", renderspuBoundsInfo );
	FILLIN( "SemaphoreCreate", renderspuSemaphoreCreate );
	FILLIN( "SemaphoreDestroy", renderspuSemaphoreDestroy );
	FILLIN( "SemaphoreP", renderspuSemaphoreP );
	FILLIN( "SemaphoreV", renderspuSemaphoreV );
	FILLIN( "Writeback", renderspuWriteback );
	FILLIN( "ChromiumParameteriCR", renderspuChromiumParameteriCR );
	FILLIN( "ChromiumParameterfCR", renderspuChromiumParameterfCR );
	FILLIN( "ChromiumParametervCR", renderspuChromiumParametervCR );
	FILLIN( "GetChromiumParametervCR", renderspuGetChromiumParametervCR );
	FILLIN( "Clear", renderspuClear );
	return i;
}
