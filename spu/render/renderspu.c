/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_applications.h"
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
VisualInfo *renderspuFindVisual(const char *displayName, GLbitfield visAttribs )
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
	{
		crWarning( "Couldn't create a visual, too many visuals already" );
		return NULL;
	}

	/* create a new visual */
	i = render_spu.numVisuals;
	render_spu.visuals[i].displayName = crStrdup(displayName);
	render_spu.visuals[i].visAttribs = visAttribs;
	if (renderspu_SystemInitVisual(&(render_spu.visuals[i]))) {
		render_spu.numVisuals++;
		return &(render_spu.visuals[i]);
	}
	else {
		crWarning( "Couldn't get a visual, renderspu_SystemInitVisual failed" );
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

	if (!dpyName || crStrlen(render_spu.display_string)>0)
		dpyName = render_spu.display_string;
	visual = renderspuFindVisual( dpyName, visBits );
	if (!visual)
		return -1;

	/* find free slot in contexts[] array */
	for (i = 0; i < MAX_CONTEXTS; i++) {
		if (!render_spu.contexts[i].inUse)
			break;
	}
	if (i == MAX_CONTEXTS)
		return -1;

	if (!renderspu_SystemCreateContext( visual, &(render_spu.contexts[i]) ))
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
	renderspu_SystemDestroyContext( context );
	context->inUse = GL_FALSE;
	context->everCurrent = GL_FALSE;
}

void RENDER_APIENTRY renderspuMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx)
{
	GET_THREAD(thread);

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

		if (!window->inUse)
		{
			crDebug("renderspuMakeCurrent: invalid window id: %d", crWindow);
			return;
		}
		if (!context->inUse)
		{
			crDebug("renderspuMakeCurrent: invalid context id: %d", ctx);
			return;
		}

		renderspu_SystemMakeCurrent( thread, window, nativeWindow, context );
		if (!context->everCurrent) {
			/* print OpenGL info */
			crDebug( "Render SPU: GL_VENDOR:   %s", render_spu.ws.glGetString( GL_VENDOR ) );
			crDebug( "Render SPU: GL_RENDERER: %s", render_spu.ws.glGetString( GL_RENDERER ) );
			crDebug( "Render SPU: GL_VERSION:  %s", render_spu.ws.glGetString( GL_VERSION ) );
			context->everCurrent = GL_TRUE;
		}
		if (crWindow == 0 && window->mapPending &&
				!render_spu.render_to_app_window) {
			/* Window[0] is special, it's the default window and normally hidden.
			 * If the mapPending flag is set, then we should now make the window
			 * visible.
			 */
			renderspu_SystemShowWindow( window, GL_TRUE );
			window->mapPending = GL_FALSE;
		}
	}
	else {
		thread->currentWindow = -1;
		thread->currentContext = -1;
		renderspu_SystemMakeCurrent( thread, NULL, 0, NULL );
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

	if (!dpyName || crStrlen(render_spu.display_string)>0)
		dpyName = render_spu.display_string;
	visual = renderspuFindVisual( dpyName, visBits );
	if (!visual)
	{
		crWarning( "Couldn't create a window, renderspuFindVisual returned NULL" );
		return -1;
	}

	/* find an empty slot in windows[] array */
	for (i = 0; i < MAX_WINDOWS; i++) {
		if (!render_spu.windows[i].inUse)
			break;
	}
	if (i == MAX_WINDOWS)
	{
		crWarning( "Couldn't create a window, i == MAX_WINDOWS" );
		return -1;
	}

	/* Have GLX/WGL create the window */
	if (render_spu.render_to_app_window)
		showIt = 0;
	else
		showIt = i > 0;
	if (!renderspu_SystemCreateWindow( visual, showIt, &(render_spu.windows[i]) ))
	{
		crWarning( "Couldn't create a window, renderspu_SystemCreateWindow failed" );
		return -1;
	}

	render_spu.windows[i].inUse = GL_TRUE;

	return i;
}

static void RENDER_APIENTRY renderspuDestroyWindow( GLint window )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	renderspu_SystemDestroyWindow( &(render_spu.windows[window]) );
	render_spu.windows[window].inUse = GL_FALSE;
}

static void RENDER_APIENTRY renderspuWindowSize( GLint window, GLint w, GLint h )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	CRASSERT(w > 0);
	CRASSERT(h > 0);
	renderspu_SystemWindowSize( &(render_spu.windows[window]), w, h );
}

static void RENDER_APIENTRY renderspuWindowPosition( GLint window, GLint x, GLint y )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	renderspu_SystemWindowPosition( &(render_spu.windows[window]), x, y );
}

/*
 * Set the current raster position to the given window coordinate.
 */
static void SetRasterPos( GLint winX, GLint winY )
{
	GLfloat fx, fy;

	/* Push current matrix mode and viewport attributes */
	render_spu.self.PushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

	/* Setup projection parameters */
	render_spu.self.MatrixMode( GL_PROJECTION );
	render_spu.self.PushMatrix();
	render_spu.self.LoadIdentity();
	render_spu.self.MatrixMode( GL_MODELVIEW );
	render_spu.self.PushMatrix();
	render_spu.self.LoadIdentity();

	render_spu.self.Viewport( winX - 1, winY - 1, 2, 2 );

	/* set the raster (window) position */
	/* huh ? */
	fx = (GLfloat) (winX - (int) winX);
	fy = (GLfloat) (winY - (int) winY);
	render_spu.self.RasterPos4f( fx, fy, 0.0, 1.0 );

	/* restore matrices, viewport and matrix mode */
	render_spu.self.PopMatrix();
	render_spu.self.MatrixMode( GL_PROJECTION );
	render_spu.self.PopMatrix();

	render_spu.self.PopAttrib();
}


/*
 * Draw the mouse pointer bitmap at (x,y) in window coords.
 */
static void DrawCursor( GLint x, GLint y )
{
#define POINTER_WIDTH   32
#define POINTER_HEIGHT  32
	/* Somebody artistic could probably do better here */
	static const char *pointerImage[POINTER_HEIGHT] =
	{
		"XX..............................",
		"XXXX............................",
		".XXXXX..........................",
		".XXXXXXX........................",
		"..XXXXXXXX......................",
		"..XXXXXXXXXX....................",
		"...XXXXXXXXXXX..................",
		"...XXXXXXXXXXXXX................",
		"....XXXXXXXXXXXXXX..............",
		"....XXXXXXXXXXXXXXXX............",
		".....XXXXXXXXXXXXXXXXX..........",
		".....XXXXXXXXXXXXXXXXXXX........",
		"......XXXXXXXXXXXXXXXXXXXX......",
		"......XXXXXXXXXXXXXXXXXXXXXX....",
		".......XXXXXXXXXXXXXXXXXXXXXXX..",
		".......XXXXXXXXXXXXXXXXXXXXXXXX.",
		"........XXXXXXXXXXXXX...........",
		"........XXXXXXXX.XXXXX..........",
		".........XXXXXX...XXXXX.........",
		".........XXXXX.....XXXXX........",
		"..........XXX.......XXXXX.......",
		"..........XX.........XXXXX......",
		"......................XXXXX.....",
		".......................XXXXX....",
		"........................XXX.....",
		".........................X......",
		"................................",
		"................................",
		"................................",
		"................................",
		"................................",
		"................................"

	};
	static GLubyte pointerBitmap[POINTER_HEIGHT][POINTER_WIDTH / 8];
	static GLboolean firstCall = GL_TRUE;
	GLboolean lighting, depthTest, scissorTest;

	if (firstCall) {
		/* Convert pointerImage into pointerBitmap */
		GLint i, j;
		for (i = 0; i < POINTER_HEIGHT; i++) {
			for (j = 0; j < POINTER_WIDTH; j++) {
				if (pointerImage[POINTER_HEIGHT - i - 1][j] == 'X') {
					GLubyte bit = 128 >> (j & 0x7);
					pointerBitmap[i][j / 8] |= bit;
				}
			}
		}
		firstCall = GL_FALSE;
	}

	render_spu.self.GetBooleanv(GL_LIGHTING, &lighting);
	render_spu.self.GetBooleanv(GL_DEPTH_TEST, &depthTest);
	render_spu.self.GetBooleanv(GL_SCISSOR_TEST, &scissorTest);
	render_spu.self.Disable(GL_LIGHTING);
	render_spu.self.Disable(GL_DEPTH_TEST);
	render_spu.self.Disable(GL_SCISSOR_TEST);
	render_spu.self.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	render_spu.self.Color3f(1, 1, 1);
	SetRasterPos(x, y);
	render_spu.self.Bitmap(POINTER_WIDTH, POINTER_HEIGHT, 1.0, 31.0, 0, 0,
								(const GLubyte *) pointerBitmap);

	if (lighting)
	   render_spu.self.Enable(GL_LIGHTING);
	if (depthTest)
	   render_spu.self.Enable(GL_DEPTH_TEST);
	if (scissorTest)
		render_spu.self.Enable(GL_SCISSOR_TEST);
}

void RENDER_APIENTRY renderspuSwapBuffers( GLint window, GLint flags )
{
	WindowInfo *w;
	if (window < 0 || window >= MAX_WINDOWS)
	{
		crDebug("renderspuSwapBuffers: window id %d out of range", window);
		return;
	}
	w = &(render_spu.windows[window]);
	if (!w->inUse)
	{
		crDebug("renderspuSwapBuffers: invalid window id: %d", window);
		return;
	}

	if (flags & CR_SUPPRESS_SWAP_BIT)
	  return;

	if (render_spu.drawCursor)
		DrawCursor( render_spu.cursorX, render_spu.cursorY );

	renderspu_SystemSwapBuffers( window, flags );
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
	switch (target) {
	case GL_WINDOW_SIZE_CR:
	   {
		  CRASSERT(type == GL_INT);
		  CRASSERT(count == 2);
		  CRASSERT(values);
		  if (index >= 0 && index < MAX_WINDOWS) {
			 GLint w, h, *size = (GLint *) values;
			 WindowInfo *window = &(render_spu.windows[index]);
			 renderspu_SystemGetWindowSize(window, &w, &h);
			 size[0] = w;
			 size[1] = h;
		  }
	   }
	   break;
	default:
	   ; /* nothing - silence compiler */
	}
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
