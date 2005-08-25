/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_pixeldata.h"
#include "cr_mem.h"
#include "cr_dlm.h"


/**
 * Given the position (x, y) and size (width, height) of a glDrawPixels
 * command, the UNPACK_SKIP_PIXELS and UNPACK_SKIP_ROWS values, X/Y zoom
 * factors and a scissor box, return modified x, y, width, height, skipPixels
 * and skipRows values so that drawing the image will result in only the
 * pixels inside the scissor box being drawn.
 * Note: When xZoom != 1 or yZoom != 1, there can be some off-by-one errors.
 * We haven't exhaustively examined that problem.
 * \return GL_TRUE if there's something to draw, otherwise GL_FALSE if
 * there's nothing left to draw.
 */
static GLboolean
ComputeSubImage(GLint *x, GLint *y, GLsizei *width, GLsizei *height,
								GLint *skipPixels, GLint *skipRows,
								GLfloat xZoom, GLfloat yZoom,
								const CRrecti *scissor)
{
	GLint clip; /* number of clipped pixels */
	GLint w, h;

	if (xZoom != 1.0F || yZoom != 1.0F) {
		/* scale up the image to zoomed size, for sake of clipping */
		w = (GLint) (*width  * xZoom + 0.5F);
		h = (GLint) (*height * yZoom + 0.5F);
		*skipPixels = (GLint) (*skipPixels * xZoom); /* don't round up */
		*skipRows   = (GLint) (*skipRows   * yZoom);
	}
	else {
		w = *width;
		h = *height;
	}

	if (*x + w <= scissor->x1 ||
			*x >= scissor->x2 ||
			*y + h <= scissor->y1 ||
			*y >= scissor->y2) {
		/* totally clipped */
		return GL_FALSE;
	}

	if (*x < scissor->x1) {
		/* image crosses left edge of scissor box */
		CRASSERT(*x + w > scissor->x1);
		clip = scissor->x1 - *x;
		CRASSERT(clip > 0);
		*x = scissor->x1;
		*skipPixels += clip;
		w -= clip;
	}
	if (*y < scissor->y1) {
		/* image crosses bottom edge of scissor box */
		CRASSERT(*y + h > scissor->y1);
		clip = scissor->y1 - *y;
		CRASSERT(clip > 0);
		*y = scissor->y1;
		*skipRows += clip;
		h -= clip;
	}
	if (*x + w > scissor->x2) {
		/* image crossed right edge of scissor box */
		CRASSERT(*x <= scissor->x2);
		clip = *x + w - scissor->x2;
		CRASSERT(clip > 0);
		w -= clip;
	}
	if (*y + h > scissor->y2) {
		/* image crossed top edge of scissor box */
		CRASSERT(*y <= scissor->y2);
		clip = *y + h - scissor->y2;
		CRASSERT(clip > 0);
		h -= clip;
	}

	/* now undo the zoom factor if needed */
	if (xZoom != 1.0F || yZoom != 1.0F) {
		 *width  = (GLint) (w / xZoom + 0.5F);  /* XXX round up more? */
		 *height = (GLint) (h / yZoom + 0.5F);
		 *skipPixels = (GLint) (*skipPixels / xZoom); /* don't round up */
		 *skipRows   = (GLint) (*skipRows   / yZoom);
	}
	else {
		 *width = w;
		 *height = h;
	}

	return GL_TRUE;
}


/**
 * Execute glDrawPixels().  Immediate mode only.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DrawPixels(GLsizei width, GLsizei height, GLenum format,
		       GLenum type, const GLvoid *pixels)
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	CRPixelState *p = &(ctx->pixel);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	GLfloat screen_bbox[8];
	GLint zoomedWidth, zoomedHeight;
	GLfloat zoomX, zoomY;
	GLfloat oldZoomX, oldZoomY;
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE) {
		/* just creating or compiling display lists */
		if (tilesort_spu.lazySendDLists)
			crDLMCompileDrawPixels(width, height, format, type, pixels, clientState);
		else if (tilesort_spu.swap)
			crPackDrawPixelsSWAP(width, height, format, type, pixels, &(ctx->client.unpack));
		else
			crPackDrawPixels(width, height, format, type, pixels, &(ctx->client.unpack));
		return;
	}

	CRASSERT(ctx->lists.mode == 0);

	(void) v;

	if (c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glDrawPixels" );
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "glDrawPixels" );
		return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	if (crImageSize(format, type, width, height)
		+ sizeof(format) + sizeof(type) + sizeof(width) + sizeof(height)
		> tilesort_spu.MTU ) {
		crWarning("DrawPixels called with insufficient MTU size\n"
			"Needed %d, but currently MTU is set at %d\n", 
			crImageSize(format, type, width, height), 
			tilesort_spu.MTU );
		return;
	}

	tilesortspuFlush( thread );
	
	oldZoomX = p->xZoom;
	oldZoomY = p->yZoom;
	if (tilesort_spu.scaleImages) {
		 /* Zoom the pixels up to match the output.  If an app draws to
			* the full size of it's window, for the output to look right on
			* a tiled display, we should zoom here as well.
			*/
		 zoomX = oldZoomX * winInfo->widthScale;
		 zoomY = oldZoomY * winInfo->heightScale;
		 crStatePixelZoom(zoomX, zoomY);
	}
	else {
		 zoomX = oldZoomX;
		 zoomY = oldZoomY;
	}

	/* The "+ 0.5" causes a round up when we cast to int */
	zoomedWidth  = (GLint) (zoomX * width  + 0.5);
	zoomedHeight = (GLint) (zoomY * height + 0.5);

	/* min x, y, z, w */
	screen_bbox[0] = c->rasterAttrib[VERT_ATTRIB_POS][0] / v->viewportW;
	screen_bbox[1] = c->rasterAttrib[VERT_ATTRIB_POS][1] / v->viewportH;
	screen_bbox[2] = 0.0;
	screen_bbox[3] = 1.0;
	/* max x, y, z, w */
	screen_bbox[4] = (c->rasterAttrib[VERT_ATTRIB_POS][0] + zoomedWidth) / v->viewportW;
	screen_bbox[5] = (c->rasterAttrib[VERT_ATTRIB_POS][1] + zoomedHeight) / v->viewportH;
	screen_bbox[6] = 0.0;
	screen_bbox[7] = 1.0;

	/* compute NDC coords */
	screen_bbox[0] = screen_bbox[0] * 2.0f - 1.0f;
	screen_bbox[1] = screen_bbox[1] * 2.0f - 1.0f;
	screen_bbox[4] = screen_bbox[4] * 2.0f - 1.0f;
	screen_bbox[5] = screen_bbox[5] * 2.0f - 1.0f;

	thread->currentContext->inDrawPixels = GL_TRUE;

	if (winInfo->bucketMode != BROADCAST && oldZoomX > 0.0 && oldZoomY > 0.0) {
		/* Test glDrawPixels image bounds against all tile extents and only
		 * send sub-images instead of full-size images.
		 */
		int i;

		/* release current geometry buffer */
		crPackReleaseBuffer(thread->packer);

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			int j;

			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			/* Determine if the DrawPixel destination region overlaps any
			 * tile extents of this server
			 */
			for (j = 0; j < winInfo->server[i].num_extents; j++) {
				CRPixelPackState unpacking = ctx->client.unpack;
				GLint newX = (GLint) c->rasterAttrib[VERT_ATTRIB_POS][0];
				GLint newY = (GLint) c->rasterAttrib[VERT_ATTRIB_POS][1];
				GLsizei newWidth = width;
				GLsizei newHeight = height;
				if (!unpacking.rowLength)
					unpacking.rowLength = width;

				if (ComputeSubImage(&newX, &newY, &newWidth, &newHeight,
														&unpacking.skipPixels, &unpacking.skipRows,
														oldZoomX, oldZoomY,
														&winInfo->server[i].extents[j])) {
					if (tilesort_spu.swap) {
						crPackWindowPos2iARBSWAP(newX, newY);
						crPackDrawPixelsSWAP(newWidth, newHeight, format, type, pixels,
																 &unpacking);
					}
					else {
						crPackWindowPos2iARB(newX, newY);
						crPackDrawPixels(newWidth, newHeight, format, type, pixels,
														 &unpacking);
					}
				}
			}

			/* release server buffer */
			crPackReleaseBuffer(thread->packer);

			/* Flush buffer (send to server) */
			tilesortspuSendServerBuffer(i);
		}

		/* Restore the default geometry pack buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
	}
	else {
		/*
		 * Send the whole image to all servers chosen by bucketing
		 */
		GLenum hint = thread->currentContext->providedBBOX;
		tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

		/* 
		 * don't do a flush, DrawPixels understand that it needs to flush,
		 * and will handle all that for us.  our HugeFunc routine will
		 * specially handle the DrawPixels call
		 */

		if (tilesort_spu.swap)
			crPackDrawPixelsSWAP(width, height, format, type, 
													 pixels, &(ctx->client.unpack));
		else
			crPackDrawPixels(width, height, format, type, 
											 pixels, &(ctx->client.unpack));

		if (thread->packer->buffer.data_current != thread->packer->buffer.data_start)
		{
			tilesortspuFlush( thread );
		}

		thread->currentContext->providedBBOX = hint;
	}

	thread->currentContext->inDrawPixels = GL_FALSE;
	crStatePixelZoom(oldZoomX, oldZoomY);
}


void TILESORTSPU_APIENTRY
tilesortspu_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
											 GLenum format, GLenum type, GLvoid *pixels)
{
	GET_CONTEXT(ctx);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	unsigned char *data_ptr;
	int i, stride;
	int bytes_per_pixel = 0;
	int len = 48 + sizeof(CRNetworkPointer);
	int zoomedWidth, zoomedHeight;
	GLenum hint;

	/* Start with basic error checking */
	if (c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
		return;
	}

	if (format == GL_STENCIL_INDEX && ctx->limits.stencilBits == 0) {
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
		return;
	}
	if (format ==  GL_DEPTH_COMPONENT && ctx->limits.depthBits == 0) {
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
		return;
	}

	bytes_per_pixel = crPixelSize(format, type);
	if (bytes_per_pixel < 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "ReadPixels(format=0x%x, type=0x%x)", format, type);
		return;
	}

	/* Tweak the pixel zoom */
	if (tilesort_spu.scaleImages) {
		/* compute adjusted x, y, width, height */
		zoomedWidth = (int) (width * winInfo->widthScale + 0.5);
		zoomedHeight = (int) (height * winInfo->heightScale + 0.5);
		x = (int) (x * winInfo->widthScale + 0.5);
		y = (int) (y * winInfo->heightScale + 0.5);
	}
	else {
		/* no image rescaling */
		zoomedWidth = width;
		zoomedHeight = height;
	}

	/* compute/set bounding box */
	{
		const GLfloat *rastPos = c->rasterAttrib[VERT_ATTRIB_POS];
		GLfloat screen_bbox[8];

		/* min x, y, z, w */
		screen_bbox[0] = (rastPos[0]) / v->viewportW;
		screen_bbox[1] = (rastPos[1]) / v->viewportH;
		screen_bbox[2] = 0.0;
		screen_bbox[3] = 1.0;
		/* max x, y, z, w */
		screen_bbox[4] = (rastPos[0] + width) / v->viewportW;
		screen_bbox[5] = (rastPos[1] + height) / v->viewportH;
		screen_bbox[6] = 0.0;
		screen_bbox[7] = 1.0;

		/* map x,y from [0, 1] to [-1, 1] */
		screen_bbox[0] = screen_bbox[0] * 2.0f - 1.0f;
		screen_bbox[1] = screen_bbox[1] * 2.0f - 1.0f;
		screen_bbox[4] = screen_bbox[4] * 2.0f - 1.0f;
		screen_bbox[5] = screen_bbox[5] * 2.0f - 1.0f;

		hint = thread->currentContext->providedBBOX;
		tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT,
																		 8, screen_bbox);
		tilesortspuFlush( thread );
	}

	/*
	 * Loop over servers.  Compute intersection of ReadPixels region with
	 * the servers' extents.  Adjust ReadPixels and pixel packing parameters
	 * to read just the sub-region from each intersecting server.
	 */
	thread->currentContext->readPixelsCount = 0;
	for ( i = 0; i < tilesort_spu.num_servers; i++ )
	{
		CRPixelPackState packing;
		const CRrecti *extent;
		int new_width, new_height, new_x, new_y, bytes_per_row;

		/* Server[i]'s tile region (XXX we should loop over extents) */
		extent = winInfo->server[i].extents + 0;  /* x1,y1,x2,y2 */

		/* init vars */
		packing = ctx->client.pack; /* copy struct */
		packing.rowLength = width;
		new_x = x;
		new_y = y;
		new_width = width;
		new_height = height;

		/* compute intersection of tile region and readpixels region */
		if (ComputeSubImage(&new_x, &new_y, &new_width, &new_height,
												&packing.skipPixels, &packing.skipRows,
												1.0F, 1.0F, extent))
		{
			CRPackBuffer *buffer = &(thread->buffer[i]);

			/* adjust position by this tile's origin (relative to window) */
			new_x -= extent->x1;
			new_y -= extent->y1;

			/* We've got one in the pot ! */
			thread->currentContext->readPixelsCount++;

			bytes_per_row = new_width * bytes_per_pixel;

			/* Build the network message */
			/* XXX try to use the crPackReadPixels function here instead someday! */
			data_ptr = buffer->data_current; 
			if (!crPackCanHoldOpcode( 1, len ) )
			{ 
				tilesortspuFlush( thread );
				data_ptr = buffer->data_current; 
				CRASSERT( crPackCanHoldOpcode( 1, len ) );
			}
			buffer->data_current += len;
			WRITE_DATA( 0,  GLint,  new_x );
			WRITE_DATA( 4,  GLint,  new_y );
			WRITE_DATA( 8,  GLsizei,  new_width );
			WRITE_DATA( 12, GLsizei,  new_height );
			WRITE_DATA( 16, GLenum, format );
			WRITE_DATA( 20, GLenum, type );
			WRITE_DATA( 24, GLint, stride ); /* not really used! */
			WRITE_DATA( 28, GLint, packing.alignment );
			WRITE_DATA( 32, GLint, packing.skipRows );
			WRITE_DATA( 36, GLint, packing.skipPixels );
			WRITE_DATA( 40, GLint, bytes_per_row );
			WRITE_DATA( 44, GLint, packing.rowLength);
			WRITE_NETWORK_POINTER( 48, pixels );
			*(buffer->opcode_current--) = (unsigned char) CR_READPIXELS_OPCODE;

			tilesortspuSendServerBuffer( i );
		}
	}

	/* Ensure all readpixel buffers are on the way !! */
	tilesortspuFlush( thread );

	/* We need to receive them all back before continuing */
	while ( thread->currentContext->readPixelsCount > 0 )
	{
		crNetRecv( );
	}

	if (tilesort_spu.scaleImages) {
		/* rescale image from zoomedWidth X zoomedHeight to width X height */

	}

	thread->currentContext->providedBBOX = hint;
}


void TILESORTSPU_APIENTRY
tilesortspu_CopyPixels( GLint x, GLint y, GLsizei width, GLsizei height,
												GLenum type )
{
	GET_CONTEXT(ctx);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	CRCurrentState *c = &(ctx->current);
	void *buffer;
	CRPixelPackState pack, unpack;
	GLfloat origZoomX, origZoomY;
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE) {
		/* just creating or compiling display lists */
		if (tilesort_spu.lazySendDLists)
			crDLMCompileCopyPixels(x, y, width, height, type);
		else if (tilesort_spu.swap)
			crPackCopyPixelsSWAP(x, y, width, height, type);
		else
			crPackCopyPixels(x, y, width, height, type);
		return;
	}

	if (!c->rasterValid)
		return;

	/* save current pixel packing state */
	pack = ctx->client.pack;
	unpack = ctx->client.unpack;
	/* set pixel packing state to defaults */
	ctx->client.pack.rowLength = 0;
	ctx->client.pack.skipRows = 0;
	ctx->client.pack.alignment = 1;
	ctx->client.pack.swapBytes = 0;
	ctx->client.unpack.rowLength = 0;
	ctx->client.unpack.skipRows = 0;
	ctx->client.unpack.alignment = 1;
	ctx->client.unpack.swapBytes = 0;

	/* save current pixel zoom values */
	origZoomX = ctx->pixel.xZoom;
	origZoomY = ctx->pixel.yZoom;

	if (tilesort_spu.scaleImages) {
		/* compute adjusted x, y, width, height */
		width = (int) (width * winInfo->widthScale + 0.5);
		height = (int) (height * winInfo->heightScale + 0.5);
		x = (int) (x * winInfo->widthScale + 0.5);
		y = (int) (y * winInfo->heightScale + 0.5);
		/* need this scaling adjustment here since tilesortspu_DrawPixels will
		 * also muck with it.
		 */
		ctx->pixel.xZoom /= winInfo->widthScale;
		ctx->pixel.yZoom /= winInfo->heightScale;
	}

	/* allocate image buffer */
	buffer = crAlloc(width * height * 4);

	/**
	 * XXX \todo this isn't quite good enough.  If any pixel transfer operations
	 * are enabled, they'll be applied twice: during readback and drawing.
	 */
	switch (type) {
	case GL_COLOR:
		tilesortspu_ReadPixels(x, y, width, height,
													 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		tilesortspu_DrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;
	case GL_DEPTH:
		tilesortspu_ReadPixels(x, y, width, height,
													 GL_DEPTH_COMPONENT, GL_FLOAT, buffer);
		tilesortspu_DrawPixels(width, height,
													 GL_DEPTH_COMPONENT, GL_FLOAT, buffer);
		return;
	case GL_STENCIL:
		tilesortspu_ReadPixels(x, y, width, height,
													 GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buffer);
		tilesortspu_DrawPixels(width, height,
													 GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buffer);
		return;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glCopyPixels(type)");
		return;
	}

	/* restore zoom */
	ctx->pixel.xZoom = origZoomX;
	ctx->pixel.yZoom = origZoomY;
	/* restore pixel packing */
	ctx->client.pack = pack;
	ctx->client.unpack = unpack;

	crFree(buffer);
}


/**
 * Execute glBitmap.  Immediate mode only.
 */
void TILESORTSPU_APIENTRY
tilesortspu_Bitmap(GLsizei width, GLsizei height,
		   GLfloat xorig, GLfloat yorig,
		   GLfloat xmove, GLfloat ymove,
		   const GLubyte * bitmap)
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	GLfloat screen_bbox[8];
	GLenum hint;
	GLfloat xmove2, ymove2;
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompileBitmap(width, height, xorig, yorig, xmove, ymove, bitmap, clientState);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackBitmapSWAP(width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack));
	    else crPackBitmap(width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack));
	    return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	tilesortspuFlush( thread );

	/* For glBitmap-based text strings, we usually don't want to scale the
	 * bitmap moves by the mural size.  Otherwise we might want scaling.
	 * Use a config option to control that.
	 */
	if (tilesort_spu.scaleImages) {
		xmove *= winInfo->widthScale;
		ymove *= winInfo->heightScale;
	}

	/*
	 * Compute screen-space bounding box for this bitmap
	 */
	if (xmove || ymove) {
		/**
		 ** If the raster position is going to be moved by drawing this
		 ** bitmap, we have to broadcast the bitmap to all servers (by using
		 ** a really big bounding box).  If we don't, the current raster
		 ** position on some servers will be incorrect.
		 **/
		/* min x, y, z, w */
		screen_bbox[0] = -1000000;
		screen_bbox[1] = -1000000;
		screen_bbox[2] = 0.0;
		screen_bbox[3] = 1.0;
		/* max x, y, z, w */
		screen_bbox[4] = 1000000;
		screen_bbox[5] = 1000000;
		screen_bbox[6] = 0.0;
		screen_bbox[7] = 1.0;
	}
	else {
		/**
		 ** No xmove or ymove so we can just send this bitmap to the
		 ** relevant servers.
		 **/
		/* min x, y, z, w */
		screen_bbox[0] = (c->rasterAttrib[VERT_ATTRIB_POS][0] - xorig) / v->viewportW;
		screen_bbox[1] = (c->rasterAttrib[VERT_ATTRIB_POS][1] - yorig) / v->viewportH;
		screen_bbox[2] = 0.0;
		screen_bbox[3] = 1.0;
		/* max x, y, z, w */
		screen_bbox[4] = (c->rasterAttrib[VERT_ATTRIB_POS][0] - xorig + width) / v->viewportW;
		screen_bbox[5] = (c->rasterAttrib[VERT_ATTRIB_POS][1] - yorig + height) / v->viewportH;
		screen_bbox[6] = 0.0;
		screen_bbox[7] = 1.0;
	}

	screen_bbox[0] *= 2.0f;
	screen_bbox[1] *= 2.0f;
	screen_bbox[4] *= 2.0f;
	screen_bbox[5] *= 2.0f;

	screen_bbox[0] -= 1.0f;
	screen_bbox[1] -= 1.0f;
	screen_bbox[4] -= 1.0f;
	screen_bbox[5] -= 1.0f;

	hint = thread->currentContext->providedBBOX;
	tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

	/* The state tracker will take care of relative position updates. */
	xmove2 = 0;
	ymove2 = 0;

	if (tilesort_spu.scaleImages) {
		/* use glDrawPixels to draw a scaled bitmap */
		static GLfloat red[2] = { 0, 1 };
		static GLfloat green[2] = { 0, 1 };
		static GLfloat blue[2] = { 0, 1 };
		static GLfloat alpha[2] = { 0, 1 };
		red[1] = c->rasterAttrib[VERT_ATTRIB_COLOR0][0];
		green[1] = c->rasterAttrib[VERT_ATTRIB_COLOR0][0];
		blue[1] = c->rasterAttrib[VERT_ATTRIB_COLOR0][0];
		if (tilesort_spu.swap) {
			/* save current state */
			crPackPushAttribSWAP(GL_PIXEL_MODE_BIT);
			/* zooming */
			crPackPixelZoomSWAP(winInfo->widthScale, winInfo->heightScale);
			/* map 0/1 color indexes to rgba */
			crPackPixelMapfvSWAP(GL_PIXEL_MAP_I_TO_R, 2, red);
			crPackPixelMapfvSWAP(GL_PIXEL_MAP_I_TO_G, 2, green);
			crPackPixelMapfvSWAP(GL_PIXEL_MAP_I_TO_B, 2, blue);
			crPackPixelMapfvSWAP(GL_PIXEL_MAP_I_TO_A, 2, alpha);
			crPackPixelTransferiSWAP(GL_MAP_COLOR, GL_TRUE);
			/* alpha test */
			crPackEnableSWAP(GL_ALPHA_TEST);
			crPackAlphaFuncSWAP(GL_NOTEQUAL, 0.0);
			if (xorig || yorig) {
				/* adjust raster pos */
				crPackBitmapSWAP(0, 0, 0, 0,
												 -xorig * winInfo->widthScale,
												 -yorig * winInfo->heightScale,
												 NULL, &(ctx->client.unpack));
			}
			/* draw bitmap */
			crPackDrawPixelsSWAP(width, height, GL_COLOR_INDEX, GL_BITMAP,
													 bitmap, &(ctx->client.unpack));
			if (xorig || yorig) {
				/* undo raster pos adjustment */
				crPackBitmapSWAP(0, 0, 0, 0,
												 xorig * winInfo->widthScale,
												 yorig * winInfo->heightScale,
												 NULL, &(ctx->client.unpack));
			}
			/* restore state */
			crPackPopAttribSWAP();
			if (!ctx->buffer.alphaTest)
				 crPackDisableSWAP(GL_ALPHA_TEST);
		}
		else {
			/* save current state */
			crPackPushAttrib(GL_PIXEL_MODE_BIT);
			/* zooming */
			crPackPixelZoom(winInfo->widthScale, winInfo->heightScale);
			/* map 0/1 color indexes to rgba */
			crPackPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, red);
			crPackPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, green);
			crPackPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, blue);
			crPackPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, alpha);
			crPackPixelTransferi(GL_MAP_COLOR, GL_TRUE);
			/* alpha test */
			crPackEnable(GL_ALPHA_TEST);
			crPackAlphaFunc(GL_NOTEQUAL, 0.0);
			if (xorig || yorig) {
				/* adjust raster pos */
				crPackBitmap(0, 0, 0, 0,
										 -xorig * winInfo->widthScale,
										 -yorig * winInfo->heightScale,
										 NULL, &(ctx->client.unpack));
			}
			/* draw bitmap */
			crPackDrawPixels(width, height, GL_COLOR_INDEX, GL_BITMAP,
											 bitmap, &(ctx->client.unpack));
			if (xorig || yorig) {
				/* undo raster pos adjustment */
				crPackBitmap(0, 0, 0, 0,
										 xorig * winInfo->widthScale,
										 yorig * winInfo->heightScale,
										 NULL, &(ctx->client.unpack));
			}
			/* restore state */
			crPackPopAttrib();
			if (!ctx->buffer.alphaTest)
				 crPackDisable(GL_ALPHA_TEST);
		}
	}
	else {
		/* draw bitmap as-is, unscaled */
		if (tilesort_spu.swap)
			crPackBitmapSWAP( width, height, xorig, yorig, xmove2, ymove2, bitmap, &(ctx->client.unpack) );
		else
			crPackBitmap( width, height, xorig, yorig, xmove2, ymove2, bitmap, &(ctx->client.unpack) );
	}

	tilesortspuFlush( thread );

	/* update state and set dirty flag */
	crStateBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );

	thread->currentContext->providedBBOX = hint;
}

/** 
 * Here we want to flush texture state before we call PixelTransfer*().
 * This will ensure that situations such as:
 *
 *    glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
 *    glTexImage2D(....);
 *    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
 *
 * are handled correctly. If the state is not flushed, the transfer map 
 * will not effect the texture, as it will have been disabled by the time
 * the state differences are evaluated
 */
static void pixeltransfer_flush(void)
{
	GET_CONTEXT(ctx);  /* this gets thread too */
	GLboolean tex_state[5];
	GLint i, orig_active_unit;
	int already_flushed[5];
	unsigned int unit;
	
	tex_state[0] = 
	tex_state[1] = 
	tex_state[2] = 
	tex_state[3] = 
	tex_state[4] = 0;
	
	crStateGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &orig_active_unit);
	/* 
 	* First, we need to enable texture state, so that the state tracker
 	* will actually do a diff. So, save the current enabled texture modes
 	*/
	for (unit=0; unit<ctx->limits.maxTextureUnits; unit++)
	{
		crStateActiveTextureARB(GL_TEXTURE0_ARB+unit);

		crStateGetBooleanv(GL_TEXTURE_1D, &tex_state[0]);
		if (tex_state[0])
			crStateDisable(GL_TEXTURE_1D);

		crStateGetBooleanv(GL_TEXTURE_2D, &tex_state[1]);
		if (tex_state[1])
			crStateDisable(GL_TEXTURE_2D);
 
#ifdef CR_OPENGL_VERSION_1_2
		crStateGetBooleanv(GL_TEXTURE_3D, &tex_state[2]);
		if (tex_state[2])
			crStateDisable(GL_TEXTURE_3D);
#endif
#ifdef CR_ARB_texture_cube_map
		if (ctx->extensions.ARB_texture_cube_map)
			crStateGetBooleanv(GL_TEXTURE_CUBE_MAP_ARB, &tex_state[3]);
		if (tex_state[3])
			crStateDisable(GL_TEXTURE_CUBE_MAP_ARB);
#endif
#ifdef CR_NV_texture_rectangle
		if (ctx->extensions.NV_texture_rectangle)
			crStateGetBooleanv(GL_TEXTURE_RECTANGLE_NV, &tex_state[4]);
		if (tex_state[4])
			crStateDisable(GL_TEXTURE_RECTANGLE_NV);
#endif
 	
		/* 
		 * Texture precidence rules in the state tracker as well. We know
		 *	that we've munged with _some_ texture type, but we're not really
		 * sure which. So, flush each of them individually 
		 */
		crMemset(already_flushed, 0, 5 * sizeof(int));
		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			CRContext *serverState = thread->currentContext->server[i].State;

			/* First flush, 1D */
			if ((crStateTextureCheckDirtyImages(serverState, ctx, GL_TEXTURE_1D, unit)) && 
								 (!already_flushed[0]))
			{
				crStateEnable(GL_TEXTURE_1D);
				tilesortspuBroadcastGeom(1);
				crStateDisable(GL_TEXTURE_1D);
				already_flushed[0] = 1;
			}

			/* Now, flush 2D */
			if ((crStateTextureCheckDirtyImages(serverState, ctx, GL_TEXTURE_2D, unit)) &&
								 (!already_flushed[1]))
			{
				crStateEnable(GL_TEXTURE_2D);
				tilesortspuBroadcastGeom(1);
				crStateDisable(GL_TEXTURE_2D);
				already_flushed[1] = 1;
 			}
		
			/* yada yada yada */
#ifdef CR_OPENGL_VERSION_1_2
			if ((crStateTextureCheckDirtyImages(serverState, ctx, GL_TEXTURE_3D, unit)) && 
								 (!already_flushed[2]))
			{
				crStateEnable(GL_TEXTURE_3D);
				tilesortspuBroadcastGeom(1);
				crStateDisable(GL_TEXTURE_3D);
				already_flushed[2] = 1;
			}
#endif
 	
#ifdef CR_ARB_texture_cube_map
			if ((crStateTextureCheckDirtyImages(serverState, ctx, GL_TEXTURE_CUBE_MAP_ARB, unit)) &&
								(!already_flushed[3]))
			{
				if (ctx->extensions.ARB_texture_cube_map)
				{
					crStateEnable(GL_TEXTURE_CUBE_MAP_ARB);
					tilesortspuBroadcastGeom(1);
					crStateDisable(GL_TEXTURE_CUBE_MAP_ARB);
					already_flushed[3] = 1;
				}
			}
#endif

#ifdef CR_NV_texture_rectangle
			if ((crStateTextureCheckDirtyImages(serverState, ctx, GL_TEXTURE_RECTANGLE_NV, unit)) &&
								(!already_flushed[4]))
			{
				if (ctx->extensions.NV_texture_rectangle)
				{
					crStateEnable(GL_TEXTURE_RECTANGLE_NV);
					tilesortspuBroadcastGeom(1);
					crStateDisable(GL_TEXTURE_RECTANGLE_NV);
					already_flushed[4] = 1;
				}
			}
#endif
		}

		/* Finally, restore the state to the way it originally was */
		if (tex_state[0])
			crStateEnable(GL_TEXTURE_1D);
		if (tex_state[1])
			crStateEnable(GL_TEXTURE_2D);
#ifdef CR_OPENGL_VERSION_1_2
		if (tex_state[2])
			crStateEnable(GL_TEXTURE_3D);
#endif
#ifdef CR_ARB_texture_cube_map
		if (tex_state[3])
			crStateEnable(GL_TEXTURE_CUBE_MAP_ARB);
#endif
#ifdef CR_NV_texture_rectangle
		if (tex_state[4])
			crStateEnable(GL_TEXTURE_RECTANGLE_NV);
#endif
	}

	crStateActiveTextureARB(orig_active_unit);

}
void TILESORTSPU_APIENTRY tilesortspu_PixelTransferi (GLenum pname, GLint param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompilePixelTransferi(pname, param);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackPixelTransferiSWAP(pname, param);
	    else crPackPixelTransferi(pname, param);
	    return;
	}

	/* Don't flush if we're not really changing the on/off state.
	 * This is a special case for OpenRM, but safe for everyone.
	 */
	if (pname == GL_MAP_COLOR && ctx->pixel.mapColor == param)
		return;

	pixeltransfer_flush();

	crStatePixelTransferi( pname, param );

	if (tilesort_spu.swap) {
		crPackPixelTransferiSWAP(pname, param);
	} else {
		crPackPixelTransferi(pname, param);
	}
}
 
void TILESORTSPU_APIENTRY tilesortspu_PixelTransferf (GLenum pname, GLfloat param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompilePixelTransferf(pname, param);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackPixelTransferfSWAP(pname, param);
	    else crPackPixelTransferf(pname, param);
	    return;
	}

	/* Don't flush if we're not really changing the on/off state.
	 * This is a special case for OpenRM, but safe for everyone.
	 */
	if (pname == GL_MAP_COLOR && ctx->pixel.mapColor == (GLint) param)
		return;
	pixeltransfer_flush();

	crStatePixelTransferf( pname, param );

	if (tilesort_spu.swap) {
		crPackPixelTransferfSWAP(pname, param);
	} else {
		crPackPixelTransferf(pname, param);
	}
}


void TILESORTSPU_APIENTRY tilesortspu_PixelMapfv (GLenum map, GLint mapsize, const GLfloat * values)
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompilePixelMapfv(map, mapsize, values);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackPixelMapfvSWAP(map, mapsize, values);
	    else crPackPixelMapfv(map, mapsize, values);
	    return;
	}

	/* This flush ensures that any pixel operations that should be touched by
	 * the old map are pushed out the door.
	 */
	pixeltransfer_flush();

	crStatePixelMapfv ( map, mapsize, values );

	if (tilesort_spu.swap) {
		crPackPixelMapfvSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapfv ( map, mapsize, values );
	}
	/* This broadcast makes sure that the new map goes out the door before anyone
	 * that is supposed to use it.
	 */
	tilesortspuBroadcastGeom(1);
}

void TILESORTSPU_APIENTRY tilesortspu_PixelMapuiv (GLenum map, GLint mapsize, const GLuint * values)
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompilePixelMapuiv(map, mapsize, values);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackPixelMapuivSWAP(map, mapsize, values);
	    else crPackPixelMapuiv(map, mapsize, values);
	    return;
	}

	/* see longer comment in PixelMapfv */
	pixeltransfer_flush();

	crStatePixelMapuiv ( map, mapsize, values );
	if (tilesort_spu.swap) {
		crPackPixelMapuivSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapuiv ( map, mapsize, values );
	}

	/* see longer comment in PixelMapfv */
	tilesortspuBroadcastGeom(1);
}
 
void TILESORTSPU_APIENTRY tilesortspu_PixelMapusv (GLenum map, GLint mapsize, const GLushort * values)
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompilePixelMapusv(map, mapsize, values);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackPixelMapusvSWAP(map, mapsize, values);
	    else crPackPixelMapusv(map, mapsize, values);
	    return;
	}
	/* see longer comment in PixelMapfv */
	pixeltransfer_flush();

	crStatePixelMapusv ( map, mapsize, values );
	if (tilesort_spu.swap) {
		crPackPixelMapusvSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapusv ( map, mapsize, values );
	}

	/* see longer comment in PixelMapfv */
	tilesortspuBroadcastGeom(1);
}

/**
 * Execute crZPixCR().  Immediate mode only.
 */
void TILESORTSPU_APIENTRY
tilesortspu_ZPixCR(GLsizei width, GLsizei height, GLenum format,
    GLenum type, GLenum ztype, GLint zparm, GLint length, const GLvoid *pixels)
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	CRPixelState *p = &(ctx->pixel);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	GLfloat screen_bbox[8];
	GLenum hint;
	GLint zoomedWidth, zoomedHeight;
	GLfloat zoomX, zoomY;
	GLfloat oldZoomX, oldZoomY;
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists) crDLMCompileZPixCR(width, height, format, type, ztype, zparm, length, pixels, clientState);
	    else if (tilesort_spu.swap) crPackZPixCRSWAP(width, height, format, type, ztype, zparm, length, pixels, &(ctx->client.unpack));
	    else crPackZPixCR(width, height, format, type, ztype, zparm, length, pixels, &(ctx->client.unpack));
	    return;
	}

	(void) v;

	if (c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "crZPixCR" );
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "crZPixCR" );
		return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	if (length
		+ sizeof(format) + sizeof(type) + sizeof(width) + sizeof(height)
		> tilesort_spu.MTU ) {
		crWarning("ZPixCR called with insufficient MTU size\n"
			"Needed %d, but currently MTU is set at %d\n", 
			length, 
			tilesort_spu.MTU );
		return;
	}

	tilesortspuFlush( thread );
	
	oldZoomX = p->xZoom;
	oldZoomY = p->yZoom;
	if (tilesort_spu.scaleImages) {
		 /* Zoom the pixels up to match the output.  If an app draws to
			* the full size of it's window, for the output to look right on
			* a tiled display, we should zoom here as well.
			*/
		 zoomX = oldZoomX * winInfo->widthScale;
		 zoomY = oldZoomY * winInfo->heightScale;
		 crStatePixelZoom(zoomX, zoomY);
		 /* The "+ 0.5" causes a round up when we cast to int */
		 zoomedWidth  = (GLint) (zoomX * width  + 0.5);
		 zoomedHeight = (GLint) (zoomY * height + 0.5);
	}
	else {
		 zoomedWidth = width;
		 zoomedHeight = height;
	}

	/* min x, y, z, w */
	screen_bbox[0] = c->rasterAttrib[VERT_ATTRIB_POS][0] / v->viewportW;
	screen_bbox[1] = c->rasterAttrib[VERT_ATTRIB_POS][1] / v->viewportH;
	screen_bbox[2] = 0.0;
	screen_bbox[3] = 1.0;
	/* max x, y, z, w */
	screen_bbox[4] = (c->rasterAttrib[VERT_ATTRIB_POS][0] + zoomedWidth) / v->viewportW;
	screen_bbox[5] = (c->rasterAttrib[VERT_ATTRIB_POS][1] + zoomedHeight) / v->viewportH;
	screen_bbox[6] = 0.0;
	screen_bbox[7] = 1.0;

	/* compute NDC coords */
	screen_bbox[0] = screen_bbox[0] * 2.0f - 1.0f;
	screen_bbox[1] = screen_bbox[1] * 2.0f - 1.0f;
	screen_bbox[4] = screen_bbox[4] * 2.0f - 1.0f;
	screen_bbox[5] = screen_bbox[5] * 2.0f - 1.0f;

	hint = thread->currentContext->providedBBOX;
	tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

	/* 
	 * don't do a flush, ZPixCR understand that it needs to flush,
	 * and will handle all that for us.  our HugeFunc routine will
	 * specially handle the ZPixCR call
	 */

	thread->currentContext->inZPix = GL_TRUE;
	if (tilesort_spu.swap)
	     crPackZPixCRSWAP(width, height, format, type, ztype, zparm, length, 
				  pixels, &(ctx->client.unpack));
	else
	     crPackZPixCR(width, height, format, type, ztype, zparm, length,
			      pixels, &(ctx->client.unpack));

	if (thread->packer->buffer.data_current != thread->packer->buffer.data_start)
	{
		tilesortspuFlush( thread );
	}
	thread->currentContext->inZPix = GL_FALSE;

	thread->currentContext->providedBBOX = hint;
	crStatePixelZoom(oldZoomX, oldZoomY);
}
