/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "renderspu.h"

/*
 * Allocate a new ThreadInfo structure and bind this to the calling thread
 * with crSetTSD().
 */
#ifdef CHROMIUM_THREADSAFE
static ThreadInfo *renderspuNewThread( unsigned long id )
{
	ThreadInfo *thread = crCalloc(sizeof(ThreadInfo));
	if (thread) {
		crSetTSD(&_RenderTSD, thread);
		thread->id = id;
		thread->currentContext = -1;
		thread->currentWindow = -1;
	}
	return thread;
}
#endif


/*
 * Visual functions
 */

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


/*
 * Find a VisualInfo which matches the given display name and attribute
 * bitmask, or return a pointer to a new visual.
 */
VisualInfo *renderspu_FindVisual(const char *displayName, GLbitfield visAttribs )
{
#ifndef WINDOWS
	int i;

	if (!displayName)
		displayName = "";

	/* first, try to find a match */
	for (i = 0; i < render_spu.numVisuals; i++) {
		if (crStrcmp(displayName, render_spu.visuals[i].displayName) == 0
			&& visAttribs == render_spu.visuals[i].visAttribs) {
			return &(render_spu.visuals[i]);
		}
	}

	if (render_spu.numVisuals >= MAX_VISUALS)
		return NULL;

	/* create a new visual */
	i = render_spu.numVisuals;
	render_spu.visuals[i].displayName = crStrdup(displayName);
	render_spu.visuals[i].visAttribs = visAttribs;
	if (renderspu_InitVisual(&(render_spu.visuals[i]))) {
		render_spu.numVisuals++;
		return &(render_spu.visuals[i]);
	}
	else {
		return NULL;
	}
#else
	return &(render_spu.visuals[0]);
#endif
}

/*
 * Context functions
 */

GLint RENDER_APIENTRY renderspuCreateContext( const char *dpyName, GLint visBits )
{
	VisualInfo *visual;
	int i;

	if (!dpyName)
		dpyName = render_spu.display_string;
	visual = renderspu_FindVisual( dpyName, visBits );
	if (!visual)
		return -1;

	/* find free slot in contexts[] array */
	for (i = 0; i < MAX_CONTEXTS; i++) {
		if (!render_spu.contexts[i].inUse)
			break;
	}
	if (i == MAX_CONTEXTS)
		return -1;

	if (!renderspu_CreateContext( visual, &(render_spu.contexts[i]) ))
		return -1;

	render_spu.contexts[i].inUse = GL_TRUE;

	/*
	printf("%s return %d\n", __FUNCTION__, i);
	*/

	return i;
}


static void RENDER_APIENTRY renderspuDestroyContext( GLint ctx )
{
	ContextInfo *context;

	CRASSERT(ctx);

	context = &(render_spu.contexts[ctx]);
	renderspu_DestroyContext( context );
	context->inUse = GL_FALSE;
	context->everCurrent = GL_FALSE;
}

void RENDER_APIENTRY renderspuMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx)
{
	GET_THREAD(thread);

	/*
	printf("***** %s(%d, %d, %d)\n", __FUNCTION__, crWindow, nativeWindow, ctx);
	*/

#ifdef CHROMIUM_THREADSAFE
	if (!thread) {
		thread = renderspuNewThread( crThreadID() );
	}
#endif

	CRASSERT(thread);

	if (crWindow >= 0 && ctx >= 0) {
		WindowInfo *window;
		ContextInfo *context;
		thread->currentWindow = crWindow;
		thread->currentContext = ctx;
		window = &(render_spu.windows[crWindow]);
		context = &(render_spu.contexts[ctx]);
		renderspu_MakeCurrent( thread, window, context );
		if (!context->everCurrent) {
#ifdef WINDOWS
			/* XXX */
#else
			/* print OpenGL info */
			crDebug( "Render SPU: GL_VENDOR:   %s", render_spu.ws.glGetString( GL_VENDOR ) );
			crDebug( "Render SPU: GL_RENDERER: %s", render_spu.ws.glGetString( GL_RENDERER ) );
			crDebug( "Render SPU: GL_VERSION:  %s", render_spu.ws.glGetString( GL_VERSION ) );
#endif
			context->everCurrent = GL_TRUE;
		}
		if (crWindow == 0 && window->mapPending) {
			/* Window[0] is special, it's the default window and normally hidden.
			 * If the mapPending flag is set, then we should now make the window
			 * visible.
			 */
			renderspu_ShowWindow( window, GL_TRUE );
			window->mapPending = GL_FALSE;
		}
	}
	else {
		thread->currentWindow = -1;
		thread->currentContext = -1;
		renderspu_MakeCurrent( thread, NULL, NULL );
	}
}


/*
 * Window functions
 */

GLint RENDER_APIENTRY renderspuCreateWindow( const char *dpyName, GLint visBits )
{
	VisualInfo *visual;
	GLboolean showIt;
	int i;

	if (!dpyName)
		dpyName = render_spu.display_string;
	visual = renderspu_FindVisual( dpyName, visBits );
	if (!visual)
		return -1;

	/* find an empty slot in windows[] array */
	for (i = 0; i < MAX_WINDOWS; i++) {
		if (!render_spu.windows[i].inUse)
			break;
	}
	if (i == MAX_WINDOWS)
		return -1;

	/* Have GLX/WGL create the window */
	showIt = i > 0;
	if (!renderspu_CreateWindow( visual, showIt, &(render_spu.windows[i]) ))
		return -1;

	render_spu.windows[i].inUse = GL_TRUE;

	return i;
}

static void RENDER_APIENTRY renderspuDestroyWindow( GLint window )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	renderspu_DestroyWindow( &(render_spu.windows[window]) );
	render_spu.windows[window].inUse = GL_FALSE;
}

static void RENDER_APIENTRY renderspuWindowSize( GLint window, GLint w, GLint h )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	renderspu_WindowSize( &(render_spu.windows[window]), w, h );
}

static void RENDER_APIENTRY renderspuWindowPosition( GLint window, GLint x, GLint y )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	renderspu_WindowPosition( &(render_spu.windows[window]), x, y );
}


/*
 * Barrier functions
 * Normally, we'll have a crserver somewhere that handles the barrier calls.
 * However, if we're running the render SPU on the client node, then we
 * should handle barriers here.
 */

typedef struct {
	CRbarrier barrier;
	GLuint count;
} Barrier;


static void RENDER_APIENTRY renderspuBarrierCreate( GLuint name, GLuint count )
{
	Barrier *b = (Barrier *) crHashtableSearch( render_spu.barrierHash, name );
	if (b) {
		/* HACK -- this allows everybody to create a barrier, and all
           but the first creation are ignored, assuming the count
           match. */
		if ( b->count != count ) {
			crError( "Barrier name=%u created with count=%u, but already "
					 "exists with count=%u", name, count, b->count );
		}
	}
	else {
		b = (Barrier *) crAlloc( sizeof(Barrier) );
		b->count = count;
		crInitBarrier( &b->barrier, count );
		crHashtableAdd( render_spu.barrierHash, name, b );
	}
}

static void RENDER_APIENTRY renderspuBarrierDestroy( GLuint name )
{
	crHashtableDelete( render_spu.barrierHash, name );
}

static void RENDER_APIENTRY renderspuBarrierExec( GLuint name )
{
	Barrier *b = (Barrier *) crHashtableSearch( render_spu.barrierHash, name );
	if (b) {
		crWaitBarrier( &(b->barrier) );
	}
	else {
		crWarning("Bad barrier name in renderspuBarrierExec()");
	}
}


/*
 * Semaphore functions
 * XXX we should probably implement these too, for the same reason as
 * barriers (see above).
 */

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


/*
 * Misc functions
 */

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


static void RENDER_APIENTRY renderspuBoundsInfo( GLrecti *bounds, GLbyte *payload, GLint
 len, GLint num_opcodes )
{
	(void) bounds;
	(void) payload;
	(void) len;
	(void) num_opcodes;
}

static void RENDER_APIENTRY renderspuWriteback( GLint *writeback )
{
	(void) writeback;
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
	FILLIN( "crCreateWindow", renderspuCreateWindow );
	FILLIN( "DestroyWindow", renderspuDestroyWindow );
	FILLIN( "WindowSize", renderspuWindowSize );
	FILLIN( "WindowPosition", renderspuWindowPosition );
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
	return i;
}
