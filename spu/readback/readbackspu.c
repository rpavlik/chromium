/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "cr_applications.h"
#include "cr_url.h"

#include "readbackspu.h"

#define WINDOW_MAGIC 7000
#define CONTEXT_MAGIC 8000

/*
 * Allocate the color and depth buffers needed for the glDraw/ReadPixels
 * commands for the given window.
 */
static void AllocBuffers( WindowInfo *window )
{
	CRASSERT(window);
	CRASSERT(window->width >= 0);
	CRASSERT(window->height >= 0);

	if (window->colorBuffer)
		crFree(window->colorBuffer);
	
	if (readback_spu.gather_url)
		window->colorBuffer = (GLubyte *) crAlloc( window->width * window->height
																						 * 4 * sizeof(GLubyte) 
																						 + sizeof(CRMessageGather));
	else
		window->colorBuffer = (GLubyte *) crAlloc( window->width * window->height
																						 * 4 * sizeof(GLubyte) );

	/* XXX we might try GL_ABGR on NVIDIA - it might be a faster path */
	window->rgbaFormat = GL_RGBA;
	window->rgbFormat = GL_RGB;

	if (readback_spu.extract_depth)
	{
		GLint depthBytes;

		if (window->depthBuffer)
			crFree(window->depthBuffer);

		if (!window->depthType)
		{
			/* Determine best type for the depth buffer image */
			GLint zBits;
			readback_spu.super.GetIntegerv( GL_DEPTH_BITS, &zBits );
			if (zBits <= 16)
				window->depthType = GL_UNSIGNED_SHORT;
			else
				window->depthType = GL_FLOAT;
		}

		if (window->depthType == GL_UNSIGNED_SHORT)
		{
			depthBytes = sizeof(GLushort);
		}
		else
		{
			CRASSERT(window->depthType == GL_FLOAT);
			depthBytes = sizeof(GLfloat);
		}

		if (readback_spu.gather_url)
			window->depthBuffer = (GLfloat *) crAlloc( window->width * window->height
																							 * depthBytes
																							 + sizeof(CRMessageGather));
		else
			window->depthBuffer = (GLfloat *) crAlloc( window->width * window->height
																							 * depthBytes );
	}
}


/*
 * Determine the size of the given readback SPU window.
 * We may either have to query the super or child SPU window dims.
 * Reallocate the glReadPixels RGBA/depth buffers if the size changes.
 */
static void CheckWindowSize( WindowInfo *window )
{
	GLint newSize[2];
	GLint w = window - readback_spu.windows; /* pointer hack */
	CRMessage *msg;

	CRASSERT(w >= 0);
	CRASSERT(w < MAX_WINDOWS);

	/* XXX this code seems rather redundant with what's immediately below */
	if (readback_spu.gather_url)
	{
		/* ask downstream SPU (probably render) for its window size */
		readback_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 w, GL_INT, 2, newSize);
		if (newSize[0] == 0 && newSize[1] == 0)
		{
			/* something went wrong - recover - try viewport */
			GLint geometry[4];
			readback_spu.child.GetIntegerv( GL_VIEWPORT, geometry );
			newSize[0] = geometry[2];
			newSize[1] = geometry[3];
		}
		window->childWidth = newSize[0];
		window->childHeight = newSize[1];
	}

	newSize[0] = newSize[1] = 0;
	if (readback_spu.resizable)
	{
		/* ask downstream SPU (probably render) for its window size */
		readback_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 w, GL_INT, 2, newSize);
		if (newSize[0] == 0 && newSize[1] == 0)
		{
			/* something went wrong - recover - try viewport */
			GLint geometry[4];
			readback_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
			newSize[0] = geometry[2];
			newSize[1] = geometry[3];
		}
	}
	else
	{
		/* not resizable - ask render SPU for its window size */
		readback_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
					readback_spu.windows[0].renderWindow, GL_INT, 2, newSize);
	}

	if (newSize[0] != window->width || newSize[1] != window->height)
	{
		window->width = newSize[0];
		window->height = newSize[1];
		AllocBuffers(window);

		if (readback_spu.resizable)
		{
			/* update super/render SPU window size & viewport */
			CRASSERT(newSize[0] > 0);
			CRASSERT(newSize[1] > 0);
			readback_spu.super.WindowSize( w, newSize[0], newSize[1] );
			readback_spu.super.Viewport( 0, 0, newSize[0], newSize[1] );
			
			/* set child's viewport too */
			readback_spu.child.Viewport( 0, 0, newSize[0], newSize[1] );
		}

 		window->cppColor = 3;
 		if (readback_spu.extract_alpha)
 			window->cppColor++;

		msg = (CRMessage *)window->colorBuffer; 
		msg->header.type = CR_MESSAGE_GATHER;
		
 		if (readback_spu.extract_depth)
 		{
 			readback_spu.super.GetIntegerv( GL_DEPTH_BITS, &window->cppDepth );
 			window->cppDepth /= 8;
 			msg = (CRMessage *)window->depthBuffer; 
 			msg->header.type = CR_MESSAGE_GATHER;
 		}
	}
}


/*
 * Read color/alpha/depth from the rendering window, send it to
 * the child SPU via glDrawPixels.
 * Do this for all the tiles (if running on a crserver) or the
 * whole window if running on the appfaker.
 * Do Greg's bounding box test only if running on the faker (whole window).
 */
static void read_and_send_tiles( WindowInfo *window )
{
	GLrecti extent0, outputwindow0;
	const GLrecti *extents;
	const GLrecti *outputwindow;
	int numExtents;
	int i;

	if (readback_spu.server && readback_spu.server->numExtents > 0)
	{
		/* we're running on the server, loop over tiles */
		numExtents = readback_spu.server->numExtents;
		extents = readback_spu.server->extents;
		outputwindow = readback_spu.server->outputwindow;
	}
	else
	{
		/* we're running on the appfaker */
		/* default to no bounding box - read/draw whole window */
		/* the != NULL check will override this later... */
		int x = 0, y = 0;
		int w = window->width;
		int h = window->height;

		numExtents = 1; /* this may get set to zero below! */

		/*
		 * Do bounding box cull check
		 */
		if (readback_spu.bbox != NULL)
		{
			CRContext *ctx = crStateGetCurrent();
			CRTransformState *t = &(ctx->transform);
			GLmatrix *m = &(t->transform);
			GLfloat xmax = 0, xmin = 0, ymax = 0, ymin = 0;

			if (readback_spu.bbox->xmin == readback_spu.bbox->xmax)
			{
				/* client can tell us to do nothing */
				numExtents = 0;   /* used to return here */
			}
			else
			{
				/* Check to make sure the transform is valid */
				if (!t->transformValid)
				{
					/* I'm pretty sure this is always the case, but I'll leave it. */
					crStateTransformUpdateTransform(t);
				}

				crTransformBBox( 
						readback_spu.bbox->xmin,
						readback_spu.bbox->ymin,
						readback_spu.bbox->zmin,
						readback_spu.bbox->xmax,
						readback_spu.bbox->ymax,
						readback_spu.bbox->zmax,
						m,
						&xmin, &ymin, NULL, 
						&xmax, &ymax, NULL );

				/* triv reject */
				if (xmin > 1.0f || ymin > 1.0f || xmax < -1.0f || ymax < -1.0f) 
				{
					numExtents = 0;   /* used to return here */
				}
				else
				{
					/* clamp */
					if (xmin < -1.0f) xmin = -1.0f;
					if (ymin < -1.0f) ymin = -1.0f;
					if (xmax > 1.0f) xmax = 1.0f;
					if (ymax > 1.0f) ymax = 1.0f;

					if (readback_spu.halfViewportWidth == 0)
					{
						/* we haven't computed it, and they haven't
						 * called glViewport, so set it to the full window */

						readback_spu.halfViewportWidth = (window->width / 2.0f);
						readback_spu.halfViewportHeight = (window->height / 2.0f);
						readback_spu.viewportCenterX = readback_spu.halfViewportWidth;
						readback_spu.viewportCenterY = readback_spu.halfViewportHeight;
					}
					x = (int) (readback_spu.halfViewportWidth*xmin + readback_spu.viewportCenterX);
					w = (int) (readback_spu.halfViewportWidth*xmax + readback_spu.viewportCenterX) - x;
					y = (int) (readback_spu.halfViewportHeight*ymin + readback_spu.viewportCenterY);
					h = (int) (readback_spu.halfViewportHeight*ymax + readback_spu.viewportCenterY) - y;
				}
			}
		}

		extent0.x1 = x;
		extent0.y1 = y;
		extent0.x2 = x + w;
		extent0.y2 = y + h;
		extents = &extent0;
		outputwindow0.x1 = x;
		outputwindow0.y1 = y;
		outputwindow0.x2 = x + w;
		outputwindow0.y2 = y + h;
		outputwindow = &outputwindow0;
	}

	/*
	 * NOTE: numExtents may be zero here if the bounding box test
	 * determined that nothing was drawn.  We can't just return though!
	 * we have to go through the barriers below so that we don't deadlock
	 * the server.
	 */

	/* One will typically use serverNode.Conf('only_swap_once', 1) to
	 * prevent extraneous glClear and SwapBuffer calls on the server.
	 */
	if (readback_spu.extract_depth) 
		readback_spu.child.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	else 
		readback_spu.child.Clear( GL_COLOR_BUFFER_BIT );

	/* wait for everyone to finish clearing */
	if (!readback_spu.gather_url)
		readback_spu.child.BarrierExec( CLEAR_BARRIER );

	/*
	 * Begin critical region.
	 * NOTE: we could put the SemaphoreP and SemaphoreV calls inside this
	 * loop to more tightly bracket the glDrawPixels calls.  However, by
	 * putting the mutex outside of the loop, we're more likely to pack more
	 * data into each buffer when doing image reassembly.
	 */
	readback_spu.child.SemaphoreP( MUTEX_SEMAPHORE );

	/*
	 * loop over extents (image regions)
	 */
	for (i = 0; i < numExtents; i++)
	{
		const int readx = outputwindow[i].x1;
		const int ready = outputwindow[i].y1;
		const int drawx = extents[i].x1;
		const int drawy = extents[i].y1;
		int w = outputwindow[i].x2 - outputwindow[i].x1;
		int h = outputwindow[i].y2 - outputwindow[i].y1;
		unsigned int shift;
		CRMessage *msg;

		if (readback_spu.gather_url)
		{
			msg = (CRMessage *)window->colorBuffer;
	 		msg->gather.offset = window->cppColor*(drawy * window->childWidth + drawx);

 			msg->gather.len    = window->cppColor*(w*h);
			if (readback_spu.extract_depth)
			{
				msg = (CRMessage *)window->depthBuffer;
		 		msg->gather.offset = window->cppDepth*(drawx * window->childWidth + drawx);
 				msg->gather.len    = window->cppDepth*(w*h);
			}
		}
		
		/* Clamp the image width and height to the readback SPU window's width
		 * and height.  We do this because there's nothing preventing someone
		 * from creating a tile larger than the rendering window.
		 * Our RGBA and depth buffers are the size of the window so trying to
		 * glReadPixels a larger size will cause a segfault.
		 */
		if (w > window->width)
			w = window->width;
		if (h > window->height)
			h = window->height;

		/*
		crDebug("readback from: %d, %d   to: %d, %d   size: %d x %d",
						readx, ready, drawx, drawy, w, h);
		*/

		/* Karl's gather code */
		shift = readback_spu.gather_url ? sizeof(CRMessageGather) : 0;

		/* Read RGB image, possibly alpha, possibly depth from framebuffer */
		if (readback_spu.extract_alpha)
		{
			readback_spu.super.ReadPixels( readx, ready, w, h,
					window->rgbaFormat, GL_UNSIGNED_BYTE,
					window->colorBuffer + shift );
		}
		else 
		{
			readback_spu.super.ReadPixels( readx, ready, w, h, 
					window->rgbFormat, GL_UNSIGNED_BYTE,
					window->colorBuffer + shift );
		}

		if (readback_spu.extract_depth)
		{
			readback_spu.super.ReadPixels( readx, ready, w, h,
																		 GL_DEPTH_COMPONENT, window->depthType,
																		 (GLubyte *) window->depthBuffer + shift);
		}

		/*
		 * Set the downstream viewport.  If we don't do this, and the
		 * downstream window is resized, the glRasterPos command doesn't
		 * seem to be reliable.  This is a problem both with Mesa and the
		 * NVIDIA drivers.  Technically, this may not be a driver bug at
		 * all since we're doing funny stuff.  Anyway, this fixes the problem.
		 * Note that the width and height are arbitrary since we only care
		 * about getting the origin right.  glDrawPixels, glClear, etc don't
		 * care what the viewport size is.  (BrianP)
		 */
		CRASSERT(window->width > 0);
		CRASSERT(window->height > 0);
		readback_spu.child.Viewport( 0, 0, window->width, window->height );

		/* Use the glBitmap trick to set the raster pos.
		 */
		readback_spu.child.RasterPos2i(0, 0);
		readback_spu.child.Bitmap(0, 0, 0, 0, (GLfloat)drawx, (GLfloat)drawy, NULL);

		/*
		 * OK, send color/depth images to child.
		 */
		if (readback_spu.extract_depth)
		{
			/* Draw the depth image into the depth buffer, setting the stencil
			 * to one wherever we pass the Z test, clearinging to zero where we fail.
			 */
			readback_spu.child.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
			readback_spu.child.StencilOp( GL_KEEP, GL_ZERO, GL_REPLACE );
			readback_spu.child.StencilFunc( GL_ALWAYS, 1, ~0 );
			readback_spu.child.Enable( GL_STENCIL_TEST );
			readback_spu.child.Enable( GL_DEPTH_TEST );
			readback_spu.child.DepthFunc( GL_LESS );
			readback_spu.child.DrawPixels( w, h,
																		 GL_DEPTH_COMPONENT, window->depthType,
																		 (GLubyte *) window->depthBuffer + shift );
			/* Now draw the RGBA image, only where the stencil is one, reset stencil
			 * to zero as we go (to avoid calling glClear(STENCIL_BUFFER_BIT)).
			 */
			readback_spu.child.Disable( GL_DEPTH_TEST );
			readback_spu.child.StencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
			readback_spu.child.StencilFunc( GL_EQUAL, 1, ~0 );
			readback_spu.child.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			if (readback_spu.visualize_depth) {
				/* draw depth as grayscale image */
				readback_spu.child.PixelTransferf(GL_RED_BIAS, 1.0);
				readback_spu.child.PixelTransferf(GL_GREEN_BIAS, 1.0);
				readback_spu.child.PixelTransferf(GL_BLUE_BIAS, 1.0);
				readback_spu.child.PixelTransferf(GL_RED_SCALE, -1.0);
				readback_spu.child.PixelTransferf(GL_GREEN_SCALE, -1.0);
				readback_spu.child.PixelTransferf(GL_BLUE_SCALE, -1.0);
				readback_spu.child.DrawPixels(w, h, 
																			GL_LUMINANCE, window->depthType,
																			(GLubyte *) window->depthBuffer + shift);
			}
			else {
				/* the usual case */
				readback_spu.child.DrawPixels( w, h,
						window->rgbFormat, GL_UNSIGNED_BYTE,
						window->colorBuffer );
			}
			readback_spu.child.Disable(GL_STENCIL_TEST);
		}
		else if (readback_spu.extract_alpha) {
			/* alpha compositing */
			readback_spu.child.BlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
			readback_spu.child.Enable( GL_BLEND );
			readback_spu.child.DrawPixels( w, h,
																		 window->rgbaFormat, GL_UNSIGNED_BYTE,
																		 window->colorBuffer + shift );
		}
		else
		{
			if (!readback_spu.gather_url)
			{
				/* just send color image */
				readback_spu.child.DrawPixels( w, h,
																			 window->rgbFormat, GL_UNSIGNED_BYTE,
																			 window->colorBuffer + shift );
			}
		}
	}

	/*
	 * End critical region.
	 */
	readback_spu.child.SemaphoreV( MUTEX_SEMAPHORE );
}


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
	}
	return thread;
}
#endif


static void DoReadback( WindowInfo *window )
{
	static int first_time = 1;
	GLint packAlignment, unpackAlignment;

	if (first_time || window->width < 1 || window->height < 1)
	{
		CheckWindowSize( window );
	}
	if (first_time)
	{
		/* one-time initializations */
		readback_spu.child.BarrierCreate(CLEAR_BARRIER, readback_spu.barrierSize);
		readback_spu.child.BarrierCreate(SWAP_BARRIER, readback_spu.barrierSize);
		readback_spu.child.SemaphoreCreate(MUTEX_SEMAPHORE, 1);
#if 000
		readback_spu.child.MatrixMode(GL_PROJECTION);
		readback_spu.child.LoadIdentity();
		readback_spu.child.Ortho( 0, window->width,
															0, window->height, -1, 1);
#endif
		first_time = 0;
	}
	else if (readback_spu.resizable)
	{
		/* check if window size changed, reallocate buffers if needed */
		CheckWindowSize( window );
	}

	/*
	 * Save pack/unpack alignments, and set to one.
	 */
	readback_spu.super.GetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);
	readback_spu.child.GetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
	readback_spu.super.PixelStorei(GL_PACK_ALIGNMENT, 1);
	readback_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	read_and_send_tiles(window);

	/*
	 * Restore pack/unpack alignments
	 */
	readback_spu.super.PixelStorei(GL_PACK_ALIGNMENT, packAlignment);
	readback_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
}


static void READBACKSPU_APIENTRY readbackspuFlush( void )
{
	/* find current context's window */
	WindowInfo *window;
	GET_THREAD(thread);
	if (!thread || thread->currentWindow < 0)
		return;  /* invalid window */
	window = &(readback_spu.windows[thread->currentWindow]);
	if (!window->inUse)
		return;  /* invalid window */

	DoReadback( window );

	/*
	 * XXX I'm not sure we need to sync on glFlush, but let's be safe for now.
	 */
	readback_spu.child.BarrierExec( SWAP_BARRIER );
}



static void READBACKSPU_APIENTRY readbackspuSwapBuffers( GLint window, GLint flags )
{
	unsigned short port = 3000;
	char url[4098];
	
	(void) flags;

	/* setup OOB gather connections, if necessary */
	if (readback_spu.gather_url)
	{
		if (readback_spu.gather_conn == NULL)
		{
			crParseURL(readback_spu.gather_url, url, url, &port, 3000);

			readback_spu.child.ChromiumParametervCR(GL_GATHER_CONNECT_CR, GL_INT, 1, &port);
			readback_spu.child.Flush();

			readback_spu.gather_conn = crNetConnectToServer(readback_spu.gather_url, port, 
							readback_spu.gather_mtu, 1);

			if (!readback_spu.gather_conn)
			{
				crError("Problem setting up gather connection");
			}
		}
	}
	
	DoReadback( &(readback_spu.windows[window]) );

	/*
	 * Everyone syncs up here before calling SwapBuffers().
	 */
	readback_spu.child.BarrierExec( SWAP_BARRIER );

	if (!readback_spu.gather_url)
	{
		/* Note: we don't pass the CR_SUPPRESS_SWAP_BIT flag here. */
		readback_spu.child.SwapBuffers( readback_spu.windows[window].childWindow,
																		flags & ~CR_SUPPRESS_SWAP_BIT );
		readback_spu.child.Finish();
	}

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

	/* create a state tracker (to record matrix operations) for this context */
	readback_spu.contexts[i].tracker = crStateCreateContext( NULL );

	return i;
}


static void READBACKSPU_APIENTRY readbackspuDestroyContext( GLint ctx )
{
	/*	readback_spu.child.BarrierCreate( DESTROY_CONTEXT_BARRIER, 0 );*/
	CRASSERT(ctx >= 0);
	CRASSERT(ctx < MAX_CONTEXTS);
	readback_spu.super.DestroyContext(readback_spu.contexts[ctx].renderContext);
	readback_spu.contexts[ctx].inUse = GL_FALSE;
	crStateDestroyContext( readback_spu.contexts[ctx].tracker );
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

		/* Initialize child's projection matrix so that glRasterPos2i(0,0)
		 * corresponds to window coordinate (0,0).
		 */
		readback_spu.child.MatrixMode(GL_PROJECTION);
		readback_spu.child.LoadIdentity();
		readback_spu.child.Ortho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );

		/* state tracker (for matrices) */
		crStateMakeCurrent( readback_spu.contexts[ctx].tracker );
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
	readback_spu.windows[i].width = -1; /* unknown */
	readback_spu.windows[i].height = -1; /* unknown */
	readback_spu.windows[i].colorBuffer = NULL;
	readback_spu.windows[i].depthBuffer = NULL;

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
	/* no-op */
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

static void READBACKSPU_APIENTRY readbackspuMatrixMode( GLenum mode )
{
	crStateMatrixMode( mode );
	readback_spu.super.MatrixMode( mode );
}

static void READBACKSPU_APIENTRY readbackspuLoadMatrixf( GLfloat *m )
{
	crStateLoadMatrixf( m );
	readback_spu.super.LoadMatrixf( m );
}

static void READBACKSPU_APIENTRY readbackspuLoadMatrixd( GLdouble *m )
{
	crStateLoadMatrixd( m );
	readback_spu.super.LoadMatrixd( m );
}

static void READBACKSPU_APIENTRY readbackspuMultMatrixf( GLfloat *m )
{
	crStateMultMatrixf( m );
	readback_spu.super.MultMatrixf( m );
}

static void READBACKSPU_APIENTRY readbackspuMultMatrixd( GLdouble *m )
{
	crStateMultMatrixd( m );
	readback_spu.super.MultMatrixd( m );
}

static void READBACKSPU_APIENTRY readbackspuLoadIdentity( )
{
	crStateLoadIdentity(  );
	readback_spu.super.LoadIdentity( );
}

static void READBACKSPU_APIENTRY readbackspuRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
	crStateRotatef( angle, x, y, z );
	readback_spu.super.Rotatef( angle, x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
	crStateRotated( angle, x, y, z );
	readback_spu.super.Rotated( angle, x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuScalef( GLfloat x, GLfloat y, GLfloat z )
{
	crStateScalef( x, y, z );
	readback_spu.super.Scalef( x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuScaled( GLdouble x, GLdouble y, GLdouble z )
{
	crStateScaled( x, y, z );
	readback_spu.super.Scaled( x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
	crStateTranslatef( x, y, z );
	readback_spu.super.Translatef( x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuTranslated( GLdouble x, GLdouble y, GLdouble z )
{
	crStateTranslated( x, y, z );
	readback_spu.super.Translated( x, y, z );
}

static void READBACKSPU_APIENTRY readbackspuFrustum( GLdouble left, 
		GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, 
		GLdouble zFar)
{
	crStateFrustum( left, right, bottom, top, zNear, zFar );
	readback_spu.super.Frustum( left, right, bottom, top, zNear, zFar );
}

static void READBACKSPU_APIENTRY readbackspuViewport( GLint x, 
		GLint y, GLint w, GLint h )
{
	readback_spu.super.Viewport( x, y, w, h );
	readback_spu.halfViewportWidth = 0.5F * w;
	readback_spu.halfViewportHeight = 0.5F * h;
	readback_spu.viewportCenterX = x + readback_spu.halfViewportWidth;
	readback_spu.viewportCenterY = y + readback_spu.halfViewportHeight;
}

static void READBACKSPU_APIENTRY readbackspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count, GLvoid *values)
{
	switch( target )
	{
		case GL_OBJECT_BBOX_CR:
			readback_spu.bbox = values;
			break;
		default:
			readback_spu.child.ChromiumParametervCR( target, type, count, values );
			break;
	}
}


static void READBACKSPU_APIENTRY readbackspuChromiumParameteriCR(GLenum target,  GLint value)
{
	
	switch( target )
	{
		case GL_READBACK_BARRIER_SIZE_CR:
			readback_spu.barrierSize = value;
			break;
		case GL_GATHER_POST_SWAPBUFFERS_CR:
				if ((!readback_spu.server) || (readback_spu.server->numExtents < 0))
					crError("bleh! trying to do GATHER on appfaker.");	

				if (readback_spu.gather_url)
				{
					GLint draw_parm[7];
					CRMessage *msg;
					static int first_time = 1;

					GLrecti *outputwindow = readback_spu.server->outputwindow;
					int w = outputwindow[0].x2 - outputwindow[0].x1;
					int h = outputwindow[0].y2 - outputwindow[0].y1;

					/* only swap 1 tiled-rgb ATM */
					draw_parm[0] = 0;
					draw_parm[1] = 0;
					draw_parm[2] = readback_spu.windows[value].childWidth;
					draw_parm[3] = readback_spu.windows[value].childHeight;
					draw_parm[4] = GL_RGB;
					draw_parm[5] = GL_UNSIGNED_BYTE;
					draw_parm[6] = value;

					if (!first_time)
					{
						crNetGetMessage(readback_spu.gather_conn, &msg);
						if (msg->header.type != CR_MESSAGE_OOB)
							crError("Expecting MESSAGE_OOB for sync after gather");
						crNetFree(readback_spu.gather_conn, msg);
					}
					else
						first_time = 0;
	
					/* send the framebuffer */
					crNetSend(readback_spu.gather_conn, NULL, readback_spu.windows[value].colorBuffer, 
							sizeof(CRMessageGather)+readback_spu.windows[value].cppColor*w*h);

					/* inform the child that their is a frame on the way */
					readback_spu.child.ChromiumParametervCR(GL_GATHER_DRAWPIXELS_CR, GL_INT, 7, draw_parm);
					readback_spu.child.Flush();
				}
				break;

		default:
			readback_spu.child.ChromiumParameteriCR( target, value );
			break;
	}
}


SPUNamedFunctionTable readback_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) readbackspuSwapBuffers },
	{ "CreateContext", (SPUGenericFunction) readbackspuCreateContext },
	{ "DestroyContext", (SPUGenericFunction) readbackspuDestroyContext },
	{ "MakeCurrent", (SPUGenericFunction) readbackspuMakeCurrent },
	{ "crCreateWindow", (SPUGenericFunction) readbackspuCreateWindow },
	{ "DestroyWindow", (SPUGenericFunction) readbackspuDestroyWindow },
	{ "WindowSize", (SPUGenericFunction) readbackspuWindowSize },
	{ "BarrierCreate", (SPUGenericFunction) readbackspuBarrierCreate },
	{ "BarrierDestroy", (SPUGenericFunction) readbackspuBarrierDestroy },
	{ "BarrierExec", (SPUGenericFunction) readbackspuBarrierExec },
	{ "LoadMatrixf", (SPUGenericFunction) readbackspuLoadMatrixf },
	{ "LoadMatrixd", (SPUGenericFunction) readbackspuLoadMatrixd },
	{ "MultMatrixf", (SPUGenericFunction) readbackspuMultMatrixf },
	{ "MultMatrixd", (SPUGenericFunction) readbackspuMultMatrixd },
	{ "MatrixMode", (SPUGenericFunction) readbackspuMatrixMode },
	{ "LoadIdentity", (SPUGenericFunction) readbackspuLoadIdentity },
	{ "Rotatef", (SPUGenericFunction) readbackspuRotatef },
	{ "Rotated", (SPUGenericFunction) readbackspuRotated },
	{ "Translatef", (SPUGenericFunction) readbackspuTranslatef },
	{ "Translated", (SPUGenericFunction) readbackspuTranslated },
	{ "Scalef", (SPUGenericFunction) readbackspuScalef },
	{ "Scaled", (SPUGenericFunction) readbackspuScaled },
	{ "Frustum", (SPUGenericFunction) readbackspuFrustum },
	{ "Viewport", (SPUGenericFunction) readbackspuViewport },
	{ "Flush", (SPUGenericFunction) readbackspuFlush },
	
	{ "ChromiumParametervCR", (SPUGenericFunction) readbackspuChromiumParametervCR },
	{ "ChromiumParameteriCR", (SPUGenericFunction) readbackspuChromiumParameteriCR },
	{ NULL, NULL }
};
