/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_applications.h"
#include "readbackspu.h"

#define WINDOW_MAGIC 7000
#define CONTEXT_MAGIC 8000


/*
 * Allocate a new ThreadInfo structure and bind this to the calling thread
 * with crSetTSD().
 */
#ifdef CHROMIUM_THREADSAFE
static ThreadInfo *readbackspuNewThread( unsigned long id )
{
	ThreadInfo *thread = crCalloc(sizeof(ThreadInfo));
	if (thread) {
		crSetTSD(&_ReadbackTSD, thread);
		thread->id = id;
		thread->currentContext = -1;
		thread->currentWindow = -1;
		thread->colorBuffer = NULL;
		thread->depthBuffer = NULL;
	}
	return thread;
}
#endif


static void READBACKSPU_APIENTRY readbackSwapBuffers( GLint window, GLint flags )
{
	GET_THREAD(thread);
	static int first_time = 1;
	static int geometry[4];
	GLfloat rastpos[2];

	if (first_time)
	{
		GLint zBits;

		first_time = 0;

		if ((readback_spu.server) && (readback_spu.server->numExtents))
		{
			/* no sense in reading the whole window if the tile 
			 * only convers part of it.. */
			geometry[2] = readback_spu.server->x2[0] - readback_spu.server->x1[0];
			geometry[3] = readback_spu.server->y2[0] - readback_spu.server->y1[0];
		}
		else
		{
			/* if the server is null, we are running on the 
			 * app node, not a network node, so just readout
			 * the whole shebang. if we dont have tiles, we're 
			 * likely not doing sort-first., so do it all 
			 *
			 * also, as i dont have the super nvidia drivers,
			 * im not sure if this works correctly with readback
			 * spus that are on the application node.....
			 * */
			readback_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
		}										 

		/* Determine best type for the depth buffer image */
		readback_spu.super.GetIntegerv( GL_DEPTH_BITS, &zBits );
		if (zBits <= 16)
			readback_spu.depthType = GL_UNSIGNED_SHORT;
		else
			readback_spu.depthType = GL_FLOAT;

		crDebug( "BC( %d, %d )", READBACK_BARRIER, readback_spu.barrierCount );
		readback_spu.child.BarrierCreate( READBACK_BARRIER, readback_spu.barrierCount );
	}

	if (!thread->colorBuffer) {
		CRASSERT(geometry[2]);
		CRASSERT(geometry[3]);
		thread->colorBuffer = (GLubyte *) crAlloc( geometry[2] * geometry[3]
																							 * 4 * sizeof(GLubyte) );
	}

	if (!thread->depthBuffer && readback_spu.extract_depth) {
		thread->depthBuffer = (GLfloat *) crAlloc( geometry[2] * geometry[3]
																							 * sizeof(GLfloat) );
	}

	/* Read RGB image, possibly alpha, possibly depth */
	if (readback_spu.extract_alpha)
		 readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3],
																		GL_RGBA, GL_UNSIGNED_BYTE,
																		thread->colorBuffer );
	else {
		 readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3],
																		GL_RGB, GL_UNSIGNED_BYTE,
																		thread->colorBuffer );
	}

	if (readback_spu.extract_depth)
		readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3],
																	 GL_DEPTH_COMPONENT, readback_spu.depthType,
																	 thread->depthBuffer );

	if ((flags & CR_SUPPRESS_SWAP_BIT) == 0) {
		/* only clear once */
		if (readback_spu.extract_depth) {
			readback_spu.child.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		}
		else {
			readback_spu.child.Clear( GL_COLOR_BUFFER_BIT );
		}
	}

	readback_spu.child.BarrierExec( READBACK_BARRIER );

	/* Move raster pos to (readback_spu.drawX, readback_spu.drawY) */
#if 0
	readback_spu.child.Bitmap( 0, 0, 0.0, 0.0,
														 (GLfloat) readback_spu.drawX,
														 (GLfloat) readback_spu.drawY, NULL );
#endif
	/* presumable if we're not a tile, we can start drawing back
	 * at (0, 0), or wherever the raster happens to be.
	 */
	if ((readback_spu.server) && (readback_spu.server->numExtents) &&
				(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
	{		
		rastpos[0] = (float)2.0*((float)readback_spu.server->x1[0] /
				(float)readback_spu.server->muralWidth)-(float)1.0;
		rastpos[1] = (float)2.0*((float)readback_spu.server->y1[0] / 
				(float)readback_spu.server->muralHeight)-(float)1.0;
		readback_spu.child.RasterPos2f(rastpos[0], rastpos[1]);
	}
	
	if (readback_spu.extract_depth)
	{
		/* Draw the depth image into the depth buffer, setting the stencil
		 * to one wherever we pass the Z test.
		 */
		readback_spu.child.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		readback_spu.child.StencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
		readback_spu.child.StencilFunc( GL_ALWAYS, 1, ~0 );
		readback_spu.child.Enable( GL_STENCIL_TEST );
		readback_spu.child.Enable( GL_DEPTH_TEST );
		readback_spu.child.DepthFunc( GL_LESS );
		readback_spu.child.Clear( GL_STENCIL_BUFFER_BIT );
		readback_spu.child.DrawPixels( geometry[2], geometry[3],
																	 GL_DEPTH_COMPONENT, readback_spu.depthType,
																	 thread->depthBuffer );
		if ((readback_spu.server) && (readback_spu.server->numExtents) &&
					(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
		{		
			rastpos[0] = (float)2.0*((float)(readback_spu.server->x1[0]) /
					(float)readback_spu.server->muralWidth)-(float)1.0;
			rastpos[1] = (float)2.0*((float)(readback_spu.server->y1[0]) / 
					(float)readback_spu.server->muralHeight)-(float)1.0;
			readback_spu.child.RasterPos2f(rastpos[0], rastpos[1]);
		}

		/* Now draw the RGBA image, only where the stencil is one */
		readback_spu.child.Disable( GL_DEPTH_TEST );
		readback_spu.child.StencilFunc( GL_EQUAL, 1, ~0 );
		readback_spu.child.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		if (readback_spu.visualize_depth) {
			readback_spu.child.PixelTransferf(GL_RED_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_GREEN_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_BLUE_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_RED_SCALE, -1.0);
			readback_spu.child.PixelTransferf(GL_GREEN_SCALE, -1.0);
			readback_spu.child.PixelTransferf(GL_BLUE_SCALE, -1.0);
			readback_spu.child.DrawPixels( geometry[2], geometry[3],
																		 GL_LUMINANCE, readback_spu.depthType,
																		 thread->depthBuffer );
		}
		else {
			/* the usual case */
			readback_spu.child.DrawPixels( geometry[2], geometry[3],
																		 GL_RGB, GL_UNSIGNED_BYTE,
																		 thread->colorBuffer );
		}
		readback_spu.child.Disable(GL_STENCIL_TEST);
	}
	else if (readback_spu.extract_alpha) {
		/* alpha compositing */
		readback_spu.child.BlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
		readback_spu.child.Enable( GL_BLEND );
		readback_spu.child.DrawPixels( geometry[2], geometry[3],
																	 GL_RGBA, GL_UNSIGNED_BYTE,
																	 thread->colorBuffer );
	}
	else
	{
		readback_spu.child.DrawPixels( geometry[2], geometry[3],
																	 GL_RGB, GL_UNSIGNED_BYTE,
																	 thread->colorBuffer );
	}

#if 0
	/* Move raster pos back to (0, 0) */
	readback_spu.child.Bitmap( 0, 0, 0.0, 0.0,
															(GLfloat) -readback_spu.drawX,
															(GLfloat) -readback_spu.drawY, NULL );
#endif
		
	if ((readback_spu.server) && (readback_spu.server->numExtents) &&
				(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
	{		
		readback_spu.child.RasterPos2f(-1., -1.);
	}

	readback_spu.child.BarrierExec( READBACK_BARRIER);

	if ((flags & CR_SUPPRESS_SWAP_BIT) == 0)
		readback_spu.child.SwapBuffers( readback_spu.windows[window].childWindow, 0 );

	if (readback_spu.local_visualization)
	{
		readback_spu.super.SwapBuffers( readback_spu.windows[window].renderWindow, 0 );
	}
}


static GLint READBACKSPU_APIENTRY readbackspuCreateContext( const char *dpyName, GLint visual)
{
	GLint childVisual = visual;
	int i;

	CRASSERT(readback_spu.child.BarrierCreate);

	/* find empty slot in contexts[] array */
	for (i = 0; i < MAX_CONTEXTS; i++) {
		if (!readback_spu.contexts[i].inUse)
			break;
	}
	if (i == MAX_CONTEXTS) {
		crWarning("ran out of contexts in readbackspuCreateContext");
		return -1;
	}

	/* If doing z-compositing, need stencil buffer */
	if (readback_spu.extract_depth)
		childVisual |= CR_STENCIL_BIT;
	if (readback_spu.extract_alpha)
		childVisual |= CR_ALPHA_BIT;

	readback_spu.contexts[i].inUse = GL_TRUE;
	readback_spu.contexts[i].renderContext = readback_spu.super.CreateContext(dpyName, visual);
	readback_spu.contexts[i].childContext = readback_spu.child.CreateContext(dpyName, childVisual);

	/*
	printf("%s return %d\n", __FUNCTION__, i);
	*/

	return i;
}


static void READBACKSPU_APIENTRY readbackspuDestroyContext( GLint ctx )
{
	/*	readback_spu.child.BarrierCreate( DESTROY_CONTEXT_BARRIER, 0 );*/
	CRASSERT(ctx >= 0);
	CRASSERT(ctx < MAX_CONTEXTS);
	readback_spu.super.DestroyContext(readback_spu.contexts[ctx].renderContext);
	readback_spu.contexts[ctx].inUse = GL_FALSE;
}


static void READBACKSPU_APIENTRY readbackspuMakeCurrent(GLint window, GLint nativeWindow, GLint ctx)
{
	GET_THREAD(thread);

#ifdef CHROMIUM_THREADSAFE
	if (!thread) {
		thread = readbackspuNewThread( crThreadID() );
	}
#endif

	CRASSERT(thread);

	/*
	printf("%s(t=%p w=%d c=%d\n", __FUNCTION__, thread, window, ctx);
	*/

	if (window >= 0 && ctx >= 0) {
		thread->currentWindow = window;
		thread->currentContext = ctx;
		CRASSERT(readback_spu.windows[window].inUse);
		CRASSERT(readback_spu.contexts[ctx].inUse);
		readback_spu.super.MakeCurrent(readback_spu.windows[window].renderWindow,
																	 nativeWindow,
																	 readback_spu.contexts[ctx].renderContext);
		readback_spu.child.MakeCurrent(readback_spu.windows[window].childWindow,
																	 nativeWindow,
																	 readback_spu.contexts[ctx].childContext);
	}
	else {
		thread->currentWindow = -1;
		thread->currentContext = -1;
	}
}


static GLint READBACKSPU_APIENTRY readbackspuCreateWindow( const char *dpyName, GLint visBits )
{
	GLint childVisual = visBits;
	int i;

	/*
	printf("%s(%s, %d) \n", __FUNCTION__, dpyName, visBits);
	*/

	/* find empty slot in windows[] array */
	for (i = 0; i < MAX_WINDOWS; i++) {
		if (!readback_spu.windows[i].inUse)
			break;
	}
	if (i == MAX_WINDOWS) {
		crWarning("Ran out of windows in readbackspuCreateWindow");
		return -1;
	}

	/* If doing z-compositing, need stencil buffer */
	if (readback_spu.extract_depth)
		childVisual |= CR_STENCIL_BIT;
	if (readback_spu.extract_alpha)
		childVisual |= CR_ALPHA_BIT;

	readback_spu.windows[i].inUse = GL_TRUE;
	readback_spu.windows[i].renderWindow = readback_spu.super.crCreateWindow( dpyName, visBits );
	readback_spu.windows[i].childWindow = 0;
	/*
	printf("********* %s return %d\n", __FUNCTION__, i);
	*/
	return i;
}

static void READBACKSPU_APIENTRY readbackspuDestroyWindow( GLint window )
{
	CRASSERT(window >= 0);
	CRASSERT(window < MAX_WINDOWS);
	readback_spu.super.DestroyWindow( readback_spu.windows[window].renderWindow );
	readback_spu.windows[window].inUse = GL_FALSE;
}

static void READBACKSPU_APIENTRY readbackspuWindowSize( GLint window, GLint w, GLint h )
{
	CRASSERT(window == readback_spu.renderWindow);
	readback_spu.super.WindowSize( readback_spu.renderWindow, w, h );
	readback_spu.child.WindowSize( readback_spu.childWindow, w, h );
}

/* don't implement WindowPosition() */



static void READBACKSPU_APIENTRY readbackspuBarrierCreate( GLuint name, GLuint count )
{
	(void) name;
	/* We'll propogate this value downstream to the server when we create
	 * private readback SPU barriers.
	 */
	// readback_spu.barrierCount = count;
	
	// this is totally wrong -- the barrierCount should be zero.
}

static void READBACKSPU_APIENTRY readbackspuBarrierDestroy( GLuint name )
{
	(void) name;
	/* no-op */
}

static void READBACKSPU_APIENTRY readbackspuBarrierExec( GLuint name )
{
	(void) name;
	/* no-op */
}

SPUNamedFunctionTable readback_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) readbackSwapBuffers },
	{ "CreateContext", (SPUGenericFunction) readbackspuCreateContext },
	{ "DestroyContext", (SPUGenericFunction) readbackspuDestroyContext },
	{ "MakeCurrent", (SPUGenericFunction) readbackspuMakeCurrent },
	{ "crCreateWindow", (SPUGenericFunction) readbackspuCreateWindow },
	{ "DestroyWindow", (SPUGenericFunction) readbackspuDestroyWindow },
	{ "WindowSize", (SPUGenericFunction) readbackspuWindowSize },
	{ "BarrierCreate", (SPUGenericFunction) readbackspuBarrierCreate },
	{ "BarrierDestroy", (SPUGenericFunction) readbackspuBarrierDestroy },
	{ "BarrierExec", (SPUGenericFunction) readbackspuBarrierExec },
	{ NULL, NULL }
};
