/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include <float.h>  /* for FLT_MAX */

#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "cr_url.h"

#include "readbackspu.h"

#define WINDOW_MAGIC 7000
#define CONTEXT_MAGIC 8000
#define MTRANSFORM(x, y, z, w, m, vx, vy, vz) \
    x = m[0]*vx + m[4]*vy + m[8]*vz  + m[12]; \
    y = m[1]*vx + m[5]*vy + m[9]*vz  + m[13]; \
    z = m[2]*vx + m[6]*vy + m[10]*vz + m[14]; \
    w = m[3]*vx + m[7]*vy + m[11]*vz + m[15]

#define I_TRANSFORM(num, m, vx, vy, vz) \
    MTRANSFORM (x[num], y[num], z[num], w[num], m, vx, vy, vz)

#define CLAMP(a, b, c) \
    if (a < b) a = b; if (a > c) a = c

/****************************************************************
 *
 * These functions below are used to do bounding box calculations
 *
 ****************************************************************/
static void
clipCoords(GLfloat modl[16], GLfloat proj[16],
					 GLfloat * x1, GLfloat * y1, GLfloat * z1,
					 GLfloat * x2, GLfloat * y2, GLfloat * z2)
{
	static GLfloat m[16];
	int i;

	GLfloat x[8], y[8], z[8], w[8];
	GLfloat vx1, vy1, vz1;
	GLfloat vx2, vy2, vz2;

	GLfloat xmin = FLT_MAX, ymin = FLT_MAX, zmin = FLT_MAX;
	GLfloat xmax = -FLT_MAX, ymax = -FLT_MAX, zmax = -FLT_MAX;

	m[0] =
		proj[0] * modl[0] + proj[4] * modl[1] + proj[8] * modl[2] +
		proj[12] * modl[3];
	m[1] =
		proj[1] * modl[0] + proj[5] * modl[1] + proj[9] * modl[2] +
		proj[13] * modl[3];
	m[2] =
		proj[2] * modl[0] + proj[6] * modl[1] + proj[10] * modl[2] +
		proj[14] * modl[3];
	m[3] =
		proj[3] * modl[0] + proj[7] * modl[1] + proj[11] * modl[2] +
		proj[15] * modl[3];
	m[4] =
		proj[0] * modl[4] + proj[4] * modl[5] + proj[8] * modl[6] +
		proj[12] * modl[7];
	m[5] =
		proj[1] * modl[4] + proj[5] * modl[5] + proj[9] * modl[6] +
		proj[13] * modl[7];
	m[6] =
		proj[2] * modl[4] + proj[6] * modl[5] + proj[10] * modl[6] +
		proj[14] * modl[7];
	m[7] =
		proj[3] * modl[4] + proj[7] * modl[5] + proj[11] * modl[6] +
		proj[15] * modl[7];
	m[8] =
		proj[0] * modl[8] + proj[4] * modl[9] + proj[8] * modl[10] +
		proj[12] * modl[11];
	m[9] =
		proj[1] * modl[8] + proj[5] * modl[9] + proj[9] * modl[10] +
		proj[13] * modl[11];
	m[10] =
		proj[2] * modl[8] + proj[6] * modl[9] + proj[10] * modl[10] +
		proj[14] * modl[11];
	m[11] =
		proj[3] * modl[8] + proj[7] * modl[9] + proj[11] * modl[10] +
		proj[15] * modl[11];
	m[12] =
		proj[0] * modl[12] + proj[4] * modl[13] + proj[8] * modl[14] +
		proj[12] * modl[15];
	m[13] =
		proj[1] * modl[12] + proj[5] * modl[13] + proj[9] * modl[14] +
		proj[13] * modl[15];
	m[14] =
		proj[2] * modl[12] + proj[6] * modl[13] + proj[10] * modl[14] +
		proj[14] * modl[15];
	m[15] =
		proj[3] * modl[12] + proj[7] * modl[13] + proj[11] * modl[14] +
		proj[15] * modl[15];

	/* Tranform the point by m */
	vx1 = *x1;
	vy1 = *y1;
	vz1 = *z1;
	vx2 = *x2;
	vy2 = *y2;
	vz2 = *z2;

	I_TRANSFORM(0, m, vx1, vy1, vz1);
	I_TRANSFORM(1, m, vx1, vy1, vz2);
	I_TRANSFORM(2, m, vx1, vy2, vz1);
	I_TRANSFORM(3, m, vx1, vy2, vz2);
	I_TRANSFORM(4, m, vx2, vy1, vz1);
	I_TRANSFORM(5, m, vx2, vy1, vz2);
	I_TRANSFORM(6, m, vx2, vy2, vz1);
	I_TRANSFORM(7, m, vx2, vy2, vz2);

	for (i = 0; i < 8; i++)
	{
		x[i] /= w[i];
		y[i] /= w[i];
		z[i] /= w[i];

		if (x[i] > xmax)
			xmax = x[i];
		if (y[i] > ymax)
			ymax = y[i];
		if (z[i] > zmax)
			zmax = z[i];
		if (x[i] < xmin)
			xmin = x[i];
		if (y[i] < ymin)
			ymin = y[i];
		if (z[i] < zmin)
			zmin = z[i];
	}

	*x1 = xmin;
	*y1 = ymin;
	*z1 = zmin;
	*x2 = xmax;
	*y2 = ymax;
	*z2 = zmax;
}

/*******************************************************
 * Get the clipped window based on the projection and 
 * model matrices and the bounding box supplied by the
 * application.
 *******************************************************/
static int
getClippedWindow(GLfloat modl[16], GLfloat proj[16],
								 int *xstart, int *ystart, int *xend, int *yend)
{
	GLfloat viewport[4];
	GLfloat x1, x2, y1, y2, z1, z2;
	int win_height, win_width;

	if (readback_spu.bbox != NULL)
	{
		x1 = readback_spu.bbox->xmin;
		y1 = readback_spu.bbox->ymin;
		z1 = readback_spu.bbox->zmin;
		x2 = readback_spu.bbox->xmax;
		y2 = readback_spu.bbox->ymax;
		z2 = readback_spu.bbox->zmax;
	}
	else
	{															/* no bounding box defined */
		crDebug("No BBox");
		return 0;
	}

	clipCoords(modl, proj, &x1, &y1, &z1, &x2, &y2, &z2);
	/* Sanity check... */
	if (x2 < x1 || y2 < y1 || z2 < z1)
	{
		crWarning("Damnit!!!!, we screwed up the clipping somehow...");
		return 0;
	}

	/* can we remove this get to speed things up? */
	readback_spu.super.GetFloatv(GL_VIEWPORT, viewport);
	(*xstart) = (int) ((x1 + 1.0f) * (viewport[2] / 2.0f) + viewport[0]);
	(*ystart) = (int) ((y1 + 1.0f) * (viewport[3] / 2.0f) + viewport[1]);
	(*xend) = (int) ((x2 + 1.0f) * (viewport[2] / 2.0f) + viewport[0]);
	(*yend) = (int) ((y2 + 1.0f) * (viewport[3] / 2.0f) + viewport[1]);

	win_width = (int) viewport[2];
	win_height = (int) viewport[3];

	CLAMP((*xstart), 0, win_width);
	CLAMP((*xend), 0, win_width);
	CLAMP((*ystart), 0, win_height);
	CLAMP((*yend), 0, win_height);
	return 1;
}

/*
 * Allocate the color and depth buffers needed for the glDraw/ReadPixels
 * commands for the given window.
 */
static void
AllocBuffers(WindowInfo * window)
{
	GLint colorBytes = 0, depthBytes = 0;
	CRASSERT(window);
	CRASSERT(window->width >= 0);
	CRASSERT(window->height >= 0);

	if (window->colorBuffer)
		crFree(window->colorBuffer);

	if (readback_spu.gather_url)
		window->colorBuffer = (GLubyte *) crAlloc(window->width * window->height
																							* 4 * sizeof(GLubyte)
																							+ sizeof(CRMessageGather));
	else
		window->colorBuffer = (GLubyte *) crAlloc(window->width * window->height
																							* 4 * sizeof(GLubyte));

	/* XXX we might try GL_ABGR on NVIDIA - it might be a faster path */
	window->rgbaFormat = GL_RGBA;
	window->rgbFormat = GL_RGB;
	colorBytes = readback_spu.extract_alpha ? 4 : 3;

	if (readback_spu.extract_depth)
	{
		if (window->depthBuffer)
			crFree(window->depthBuffer);

		if (!window->depthType)
		{
			/* Determine best type for the depth buffer image */
			GLint zBits;
			readback_spu.super.GetIntegerv(GL_DEPTH_BITS, &zBits);
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
			window->depthBuffer = (GLfloat *) crAlloc(window->width * window->height
																								* depthBytes
																								+ sizeof(CRMessageGather));
		else
			window->depthBuffer = (GLfloat *) crAlloc(window->width * window->height
																								* depthBytes);
	}

	crDebug("Readback SPU: %d bytes per pixel (%d RGB[A] + %d Z)",
					colorBytes + depthBytes, colorBytes, depthBytes);
	crDebug("Readback SPU: %d bytes per %dx%d image",
					(colorBytes + depthBytes) * window->width * window->height,
					window->width, window->height);
}



/*
 * Determine the size of the given readback SPU window.
 * We may either have to query the super or child SPU window dims.
 * Reallocate the glReadPixels RGBA/depth buffers if the size changes.
 */
static void
CheckWindowSize(WindowInfo * window)
{
	GLint newSize[2];
	CRMessage *msg;

	newSize[0] = newSize[1] = 0;
	if (readback_spu.gather_url || readback_spu.resizable)
	{
		/* ask downstream SPU (probably render) for its window size */
		readback_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 window->childWindow,
																							 GL_INT, 2, newSize);
		if (newSize[0] == 0 && newSize[1] == 0)
		{
			/* something went wrong - recover - try viewport */
			GLint geometry[4];
			readback_spu.child.GetIntegerv(GL_VIEWPORT, geometry);
			newSize[0] = geometry[2];
			newSize[1] = geometry[3];
		}
		window->childWidth = newSize[0];
		window->childHeight = newSize[1];
	}
	else
	{
		/* not resizable - ask render SPU for its window size */
		readback_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 window->renderWindow, GL_INT,
																							 2, newSize);
	}

	if (newSize[0] != window->width || newSize[1] != window->height)
	{
		/* The window size has changed (or first-time init) */
		window->width = newSize[0];
		window->height = newSize[1];
		AllocBuffers(window);

		if (readback_spu.resizable)
		{
			/* update super/render SPU window size & viewport */
			CRASSERT(newSize[0] > 0);
			CRASSERT(newSize[1] > 0);
			readback_spu.super.WindowSize(window->renderWindow,
																		newSize[0], newSize[1]);
			readback_spu.super.Viewport(0, 0, newSize[0], newSize[1]);
			/* set child's viewport too */
			readback_spu.child.Viewport(0, 0, newSize[0], newSize[1]);
		}

		if (readback_spu.extract_alpha)
			window->bytesPerColor = 4 * sizeof(GLubyte);
		else
			window->bytesPerColor = 3 * sizeof(GLubyte);

		msg = (CRMessage *) window->colorBuffer;
		msg->header.type = CR_MESSAGE_GATHER;

		if (readback_spu.extract_depth)
		{
			GLint zBits;
			readback_spu.super.GetIntegerv(GL_DEPTH_BITS, &zBits);
			if (zBits > 16)
				window->bytesPerDepth = 4;
			else if (zBits > 8)
				window->bytesPerDepth = 2;
			else
				window->bytesPerDepth = 1;
			CRASSERT(window->depthBuffer);
			msg = (CRMessage *) window->depthBuffer;
			msg->header.type = CR_MESSAGE_GATHER;
		}
	}
}


/*
 * This is the guts of the readback operation.  Here, we call glReadPixels
 * to read a region of the color and depth buffers from the parent (render
 * SPU) window.  Then, we call glDrawPixels (and some other GL functions)
 * on the child/downstream SPU to composite those regions.
 * Input:  window - the window we're processing
 *         w, h - size of image region to process
 *         readx, ready - glReadPixels source location
 *         drawx, drawy - glDrawPixels dest location.
 */
static void
CompositeTile(WindowInfo * window, int w, int h,
							int readx, int ready, int drawx, int drawy)
{
	unsigned int shift;
	CRMessage *msg;

	if (readback_spu.gather_url)
	{
		msg = (CRMessage *) window->colorBuffer;
		msg->gather.offset =
			window->bytesPerColor * (drawy * window->childWidth + drawx);

		msg->gather.len = window->bytesPerColor * (w * h);
		if (readback_spu.extract_depth)
		{
			msg = (CRMessage *) window->depthBuffer;
			msg->gather.offset =
				window->bytesPerDepth * (drawx * window->childWidth + drawx);
			msg->gather.len = window->bytesPerDepth * (w * h);
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
		readback_spu.super.ReadPixels(readx, ready, w, h,
																	window->rgbaFormat, GL_UNSIGNED_BYTE,
																	window->colorBuffer + shift);
	}
	else
	{
		readback_spu.super.ReadPixels(readx, ready, w, h,
																	window->rgbFormat, GL_UNSIGNED_BYTE,
																	window->colorBuffer + shift);
	}

	if (readback_spu.extract_depth)
	{
		readback_spu.super.ReadPixels(readx, ready, w, h,
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
	readback_spu.child.Viewport(0, 0, window->width, window->height);

	/* Use the glBitmap trick to set the raster pos.
	 */
	readback_spu.child.RasterPos2i(0, 0);
	readback_spu.child.Bitmap(0, 0, 0, 0, (GLfloat) drawx, (GLfloat) drawy,
														NULL);

	/*
	 * OK, send color/depth images to child.
	 */
	if (readback_spu.extract_depth)
	{
		/* Draw the depth image into the depth buffer, setting the stencil
		 * to one wherever we pass the Z test, clearinging to zero where we fail.
		 */
		readback_spu.child.ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		readback_spu.child.StencilOp(GL_KEEP, GL_ZERO, GL_REPLACE);
		readback_spu.child.StencilFunc(GL_ALWAYS, 1, ~0);
		readback_spu.child.Enable(GL_STENCIL_TEST);
		readback_spu.child.Enable(GL_DEPTH_TEST);
		readback_spu.child.DepthFunc(GL_LEQUAL);
		readback_spu.child.DrawPixels(w, h,
																	GL_DEPTH_COMPONENT, window->depthType,
																	(GLubyte *) window->depthBuffer + shift);
		/* Now draw the RGBA image, only where the stencil is one, reset stencil
		 * to zero as we go (to avoid calling glClear(STENCIL_BUFFER_BIT)).
		 */
		readback_spu.child.Disable(GL_DEPTH_TEST);
		readback_spu.child.StencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
		readback_spu.child.StencilFunc(GL_EQUAL, 1, ~0);
		readback_spu.child.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		if (readback_spu.visualize_depth)
		{
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
		else
		{
			/* the usual case */
			readback_spu.child.DrawPixels(w, h,
																		window->rgbFormat, GL_UNSIGNED_BYTE,
																		window->colorBuffer + shift);
		}
		readback_spu.child.Disable(GL_STENCIL_TEST);
	}
	else if (readback_spu.extract_alpha)
	{
		/* alpha compositing */
		readback_spu.child.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		readback_spu.child.Enable(GL_BLEND);
		readback_spu.child.DrawPixels(w, h,
																	window->rgbaFormat, GL_UNSIGNED_BYTE,
																	window->colorBuffer + shift);
	}
	else
	{
		if (!readback_spu.gather_url)
		{
			/* just send color image */
			readback_spu.child.DrawPixels(w, h,
																		window->rgbFormat, GL_UNSIGNED_BYTE,
																		window->colorBuffer + shift);
		}
	}
}


/*
 * Get the mural information for the given window.
 */
static const CRMuralInfo *
getMuralInfo(int window)
{
	if (readback_spu.server)
		return (CRMuralInfo *) crHashtableSearch(readback_spu.server->muralTable,
																						 window);
	else
		return NULL;
}


/*
 * Do readback/composite for a window.
 * This involves:
 *   - computing the image regions (tiles) to process
 *   - bounding box testing (only if running on the faker / whole window)
 *   - doing glClear
 *   - semaphore-based synchronization
 *   - call ProcessTile() for each region
 */
static void
ProcessTiles(WindowInfo * window)
{
	const CRMuralInfo *mural;
	CRExtent extent0;
	const CRExtent *extents;
	int numExtents;
	int i;

	/* XXX DMX verify window->renderWindow is correct! */
	mural = getMuralInfo(window->renderWindow);

	if (mural && mural->numExtents > 0)
	{
		/* we're running on the server, loop over tiles */
		numExtents = mural->numExtents;
		extents = mural->extents;
		/*outputwindow = mural->outputwindow;*/
	}
	else
	{
		/* we're running on the appfaker */
		/* default to no bounding box - read/draw whole window */
		/* the != NULL check will override this later... */
		int x = 0, y = 0;
		int w = window->width;
		int h = window->height;

		numExtents = 1;							/* this may get set to zero below! */

		/*
		 * Do bounding box cull check
		 */
		if (readback_spu.bbox != NULL)
		{
			if (readback_spu.bbox->xmin == readback_spu.bbox->xmax)
			{
				/* client can tell us to do nothing */
				numExtents = 0;					/* used to return here */
			}
			else
			{
				int read_start_x, read_start_y, read_end_x, read_end_y;
				GLfloat *proj = readback_spu.proj;
				GLfloat *modl = readback_spu.modl;
				getClippedWindow(modl, proj,
												 &read_start_x, &read_start_y,
												 &read_end_x, &read_end_y);
				x = read_start_x;
				y = read_start_y;
				w = read_end_x - read_start_x;
				h = read_end_y - read_start_y;
			}
		}

		extent0.imagewindow.x1 = x;
		extent0.imagewindow.y1 = y;
		extent0.imagewindow.x2 = x + w;
		extent0.imagewindow.y2 = y + h;
		extent0.outputwindow.x1 = x;
		extent0.outputwindow.y1 = y;
		extent0.outputwindow.x2 = x + w;
		extent0.outputwindow.y2 = y + h;
		extents = &extent0;
		/*outputwindow = &outputwindow0;*/
	}

	/*
	 * NOTE: numExtents may be zero here if the bounding box test
	 * determined that nothing was drawn.  We can't just return though!
	 * We have to go through the barriers below so that we don't deadlock
	 * the server.
	 */

	/* One will typically use serverNode.Conf('only_swap_once', 1) to
	 * prevent extraneous glClear and SwapBuffer calls on the server.
	 * NOTE: we don't have to clear the color buffer when doing Z compositing.
	 * The reason is we're using GL_LEQUAL as the depth test so the first
	 * image drawn will pass the depth test for all pixels, thus painting the
	 * whole color buffer. (neat!)
	 */
	if (readback_spu.extract_depth)
		readback_spu.child.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		readback_spu.child.Clear(GL_COLOR_BUFFER_BIT);

	/* wait for everyone to finish clearing */
	if (!readback_spu.gather_url)
		readback_spu.child.BarrierExecCR(CLEAR_BARRIER);

	/*
	 * Begin critical region.
	 * NOTE: we could put the SemaphoreP and SemaphoreV calls inside the
	 * tile loop  to more tightly bracket the glDrawPixels calls.  However, by
	 * putting the mutex outside of the loop, we're more likely to pack more
	 * data into each buffer when doing image reassembly.
	 */
	readback_spu.child.SemaphorePCR(MUTEX_SEMAPHORE);

	/*
	 * loop over extents (image regions)
	 */
	for (i = 0; i < numExtents; i++)
	{
#if 0
		const int readx = outputwindow[i].x1;
		const int ready = outputwindow[i].y1;
		const int drawx = extents[i].x1;
		const int drawy = extents[i].y1;
		int w = outputwindow[i].x2 - outputwindow[i].x1;
		int h = outputwindow[i].y2 - outputwindow[i].y1;
#else
		const int readx = extents[i].outputwindow.x1;
		const int ready = extents[i].outputwindow.y1;
		const int drawx = extents[i].imagewindow.x1;
		const int drawy = extents[i].imagewindow.y1;
		const int w = extents[i].outputwindow.x2 - extents[i].outputwindow.x1;
		const int h = extents[i].outputwindow.y2 - extents[i].outputwindow.y1;
#endif
		CompositeTile(window, w, h, readx, ready, drawx, drawy);
	}

	/*
	 * End critical region.
	 */
	readback_spu.child.SemaphoreVCR(MUTEX_SEMAPHORE);
}


static void
DoReadback(WindowInfo * window)
{
	static int first_time = 1;
	GLint packAlignment, unpackAlignment;

	/* setup OOB gather connections, if necessary */
	if (readback_spu.gather_url)
	{
		unsigned short port = 3000;
		char url[4098];
		if (readback_spu.gather_conn == NULL)
		{
			crParseURL(readback_spu.gather_url, url, url, &port, 3000);

			readback_spu.child.ChromiumParametervCR(GL_GATHER_CONNECT_CR,
																							GL_INT, 1, &port);
			readback_spu.child.Flush();

			readback_spu.gather_conn = crNetConnectToServer(readback_spu.gather_url,
																											port,
																											readback_spu.gather_mtu,
																											1);

			if (!readback_spu.gather_conn)
			{
				crError("Problem setting up gather connection");
			}
		}
	}

	if (first_time || window->width < 1 || window->height < 1)
	{
		CheckWindowSize(window);
	}

	if (first_time)
	{
		/* one-time initializations */
		readback_spu.child.BarrierCreateCR(CLEAR_BARRIER,
																			 readback_spu.barrierSize);
		readback_spu.child.BarrierCreateCR(SWAP_BARRIER,
																			 readback_spu.barrierSize);
		readback_spu.child.SemaphoreCreateCR(MUTEX_SEMAPHORE, 1);
		first_time = 0;
	}
	else if (readback_spu.resizable)
	{
		/* check if window size changed, reallocate buffers if needed */
		CheckWindowSize(window);
	}

	/*
	 * Save pack/unpack alignments, and set to one.
	 */
	readback_spu.super.GetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);
	readback_spu.child.GetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
	readback_spu.super.PixelStorei(GL_PACK_ALIGNMENT, 1);
	readback_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	ProcessTiles(window);

	/*
	 * Restore pack/unpack alignments
	 */
	readback_spu.super.PixelStorei(GL_PACK_ALIGNMENT, packAlignment);
	readback_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
}


static void READBACKSPU_APIENTRY
readbackspuFlush(void)
{
	WindowInfo *window;
	GET_CONTEXT(context);
	CRASSERT(context);						/* we shouldn't be flushing without a context */
	window = context->currentWindow;
	if (!window)
		return;

	DoReadback(window);

	/*
	 * XXX I'm not sure we need to sync on glFlush, but let's be safe for now.
	 */
	readback_spu.child.BarrierExecCR(SWAP_BARRIER);
}



static void READBACKSPU_APIENTRY
readbackspuSwapBuffers(GLint win, GLint flags)
{
	WindowInfo *window;

	window = (WindowInfo *) crHashtableSearch(readback_spu.windowTable, win);
	CRASSERT(window);

	DoReadback(window);

	/*
	 * Everyone syncs up here before calling SwapBuffers().
	 */
	readback_spu.child.BarrierExecCR(SWAP_BARRIER);

	if (!readback_spu.gather_url)
	{
		readback_spu.child.SwapBuffers(window->childWindow, flags);
		readback_spu.child.Finish();
	}

	if (readback_spu.local_visualization)
	{
		readback_spu.super.SwapBuffers(window->renderWindow, 0);
	}
}


static GLint READBACKSPU_APIENTRY
readbackspuCreateContext(const char *dpyName, GLint visual)
{
	static GLint freeID = 0;
	ContextInfo *context;
	GLint childVisual = visual;

	CRASSERT(readback_spu.child.BarrierCreateCR);

	context = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!context)
	{
		crWarning("readback SPU: create context failed.");
		return -1;
	}

	/* If doing z-compositing, need stencil buffer */
	if (readback_spu.extract_depth)
		childVisual |= CR_STENCIL_BIT;
	if (readback_spu.extract_alpha)
		childVisual |= CR_ALPHA_BIT;

	context->renderContext = readback_spu.super.CreateContext(dpyName, visual);
	context->childContext =
		readback_spu.child.CreateContext(dpyName, childVisual);

	/* put into hash table */
	crHashtableAdd(readback_spu.contextTable, freeID, context);
	freeID++;
	return freeID - 1;
}


static void READBACKSPU_APIENTRY
readbackspuDestroyContext(GLint ctx)
{
	ContextInfo *context;
	context = (ContextInfo *) crHashtableSearch(readback_spu.contextTable, ctx);
	CRASSERT(context);
	readback_spu.super.DestroyContext(context->renderContext);
	crHashtableDelete(readback_spu.contextTable, ctx, crFree);
}


static void READBACKSPU_APIENTRY
readbackspuMakeCurrent(GLint win, GLint nativeWindow, GLint ctx)
{
	ContextInfo *context;
	WindowInfo *window;

	context = (ContextInfo *) crHashtableSearch(readback_spu.contextTable, ctx);
	window = (WindowInfo *) crHashtableSearch(readback_spu.windowTable, win);

	if (context && window)
	{
#ifdef CHROMIUM_THREADSAFE
		crSetTSD(&_ReadbackTSD, context);
#else
		readback_spu.currentContext = context;
#endif
		CRASSERT(window);
		context->currentWindow = window;
		readback_spu.super.MakeCurrent(window->renderWindow,
																	 nativeWindow, context->renderContext);
		readback_spu.child.MakeCurrent(window->childWindow,
																	 nativeWindow, context->childContext);

		/* Initialize child's projection matrix so that glRasterPos2i(0,0)
		 * corresponds to window coordinate (0,0).
		 */
		readback_spu.child.MatrixMode(GL_PROJECTION);
		readback_spu.child.LoadIdentity();
		readback_spu.child.Ortho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	}
	else
	{
#ifdef CHROMIUM_THREADSAFE
		crSetTSD(&_ReadbackTSD, NULL);
#else
		readback_spu.currentContext = NULL;
#endif
	}
}


static GLint READBACKSPU_APIENTRY
readbackspuWindowCreate(const char *dpyName, GLint visBits)
{
	WindowInfo *window;
	GLint childVisual = visBits;
	static GLint freeID = 1;			/* skip default window 0 */

	/* If doing z-compositing, need stencil buffer */
	if (readback_spu.extract_depth)
		childVisual |= CR_STENCIL_BIT;
	if (readback_spu.extract_alpha)
		childVisual |= CR_ALPHA_BIT;

	/* allocate window */
	window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!window)
	{
		crWarning("readback SPU: unable to allocate window.");
		return -1;
	}

	/* init window */
	window->index = freeID;
	window->renderWindow = readback_spu.super.WindowCreate(dpyName, visBits);
	window->childWindow = 0;      /* use the default window! */
	window->width = -1;						/* unknown */
	window->height = -1;					/* unknown */
	window->colorBuffer = NULL;
	window->depthBuffer = NULL;

	/* put into hash table */
	crHashtableAdd(readback_spu.windowTable, window->index, window);
	freeID++;

	return freeID - 1;
}

static void READBACKSPU_APIENTRY
readbackspuWindowDestroy(GLint win)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(readback_spu.windowTable, win);
	CRASSERT(window);
	readback_spu.super.WindowDestroy(window->renderWindow);
	crHashtableDelete(readback_spu.windowTable, win, crFree);
}

static void READBACKSPU_APIENTRY
readbackspuWindowSize(GLint win, GLint w, GLint h)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(readback_spu.windowTable, win);
	CRASSERT(window);
	readback_spu.super.WindowSize(window->renderWindow, w, h);
	readback_spu.child.WindowSize(window->childWindow, w, h);
}

/* don't implement WindowPosition() */


/*
 * The render SPU implements barriers for the case of multi-threaded
 * rendering.  We don't want to block on barriers when using Readback.
 * The Readback SPU has its own barriers.
 */

static void READBACKSPU_APIENTRY
readbackspuBarrierCreateCR(GLuint name, GLuint count)
{
	(void) name;
	(void) count;
	/* no-op */
}

static void READBACKSPU_APIENTRY
readbackspuBarrierDestroyCR(GLuint name)
{
	(void) name;
	/* no-op */
}

static void READBACKSPU_APIENTRY
readbackspuBarrierExecCR(GLuint name)
{
	(void) name;
	/* no-op */
}

static void READBACKSPU_APIENTRY
readbackspuClearColor(GLclampf red,
											GLclampf green, GLclampf blue, GLclampf alpha)
{
	readback_spu.super.ClearColor(red, green, blue, alpha);
	readback_spu.child.ClearColor(red, green, blue, alpha);
}




static void READBACKSPU_APIENTRY
readbackspuViewport(GLint x, GLint y, GLint w, GLint h)
{
	readback_spu.super.Viewport(x, y, w, h);
	readback_spu.halfViewportWidth = 0.5F * w;
	readback_spu.halfViewportHeight = 0.5F * h;
	readback_spu.viewportCenterX = x + readback_spu.halfViewportWidth;
	readback_spu.viewportCenterY = y + readback_spu.halfViewportHeight;
}

static void READBACKSPU_APIENTRY
readbackspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count,
																GLvoid * values)
{
	switch (target)
	{
	case GL_OBJECT_BBOX_CR:
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 6);
		/* make copy of values! */
		readback_spu.bboxValues.xmin = ((GLfloat *) values)[0];
		readback_spu.bboxValues.ymin = ((GLfloat *) values)[1];
		readback_spu.bboxValues.zmin = ((GLfloat *) values)[2];
		readback_spu.bboxValues.xmax = ((GLfloat *) values)[3];
		readback_spu.bboxValues.ymax = ((GLfloat *) values)[4];
		readback_spu.bboxValues.zmax = ((GLfloat *) values)[5];
		readback_spu.bbox = &(readback_spu.bboxValues);
		readback_spu.super.GetFloatv(GL_PROJECTION_MATRIX, readback_spu.proj);
		readback_spu.super.GetFloatv(GL_MODELVIEW_MATRIX, readback_spu.modl);
		break;
	case GL_DEFAULT_BBOX_CR:
		CRASSERT(count == 0);
		readback_spu.bbox = NULL;
		break;
	default:
		readback_spu.child.ChromiumParametervCR(target, type, count, values);
		break;
	}
}

static void READBACKSPU_APIENTRY
readbackspuChromiumParameteriCR(GLenum target, GLint value)
{

	switch (target)
	{
	case GL_READBACK_BARRIER_SIZE_CR:
		readback_spu.barrierSize = value;
		break;
	case GL_GATHER_POST_SWAPBUFFERS_CR:
		if (readback_spu.gather_url)
		{
			const WindowInfo *window;
			const CRMuralInfo *mural;
			const CRrecti *outputwindow;
			GLint draw_parm[7];
			CRMessage *msg;
			static int first_time = 1;
			int w, h;

			/* Get window information */
			window = (const WindowInfo *) crHashtableSearch(readback_spu.windowTable, value);
			CRASSERT(window);

			/* get mural information, if running on a server node */
			/* XXX DMX verify window->renderWindow is correct! */
			mural = getMuralInfo(window->renderWindow);

			if (!mural || mural->numExtents < 0)
					crError("bleh! trying to do GATHER on appfaker.");

			outputwindow = &(mural->extents[0].outputwindow);
			w = outputwindow[0].x2 - outputwindow[0].x1;
			h = outputwindow[0].y2 - outputwindow[0].y1;

			/* only swap 1 tiled-rgb ATM */
			draw_parm[0] = 0;
			draw_parm[1] = 0;
			draw_parm[2] = window->childWidth;
			draw_parm[3] = window->childHeight;
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

			/* send the color image */
			crNetSend(readback_spu.gather_conn, NULL, window->colorBuffer,
								sizeof(CRMessageGather) + window->bytesPerColor * w * h);

			/* inform the child that their is a frame on the way */
			readback_spu.child.ChromiumParametervCR(GL_GATHER_DRAWPIXELS_CR,
																							GL_INT, 7, draw_parm);
			readback_spu.child.Flush();
		}
		break;

	default:
		readback_spu.child.ChromiumParameteriCR(target, value);
		break;
	}
}

SPUNamedFunctionTable _cr_readback_table[] = {
	{"SwapBuffers", (SPUGenericFunction) readbackspuSwapBuffers},
	{"CreateContext", (SPUGenericFunction) readbackspuCreateContext},
	{"DestroyContext", (SPUGenericFunction) readbackspuDestroyContext},
	{"MakeCurrent", (SPUGenericFunction) readbackspuMakeCurrent},
	{"WindowCreate", (SPUGenericFunction) readbackspuWindowCreate},
	{"WindowDestroy", (SPUGenericFunction) readbackspuWindowDestroy},
	{"WindowSize", (SPUGenericFunction) readbackspuWindowSize},
	{"BarrierCreateCR", (SPUGenericFunction) readbackspuBarrierCreateCR},
	{"BarrierDestroyCR", (SPUGenericFunction) readbackspuBarrierDestroyCR},
	{"BarrierExecCR", (SPUGenericFunction) readbackspuBarrierExecCR},
	{"Viewport", (SPUGenericFunction) readbackspuViewport},
	{"Flush", (SPUGenericFunction) readbackspuFlush},
	{"ClearColor", (SPUGenericFunction) readbackspuClearColor},
	{"ChromiumParametervCR", (SPUGenericFunction) readbackspuChromiumParametervCR},
	{"ChromiumParameteriCR", (SPUGenericFunction) readbackspuChromiumParameteriCR},
	{NULL, NULL}
};
