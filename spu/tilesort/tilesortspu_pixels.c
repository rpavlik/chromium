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
 * command, the UNPACK_SKIP_PIXELS and UNPACK_SKIP_ROWS values and a scissor
 * box, return modified x, y, width, height, skipPixels and skipRows
 * values so that drawing the image will result in only the pixels inside
 * the scissor box being drawn.
 * \return GL_TRUE if there's something to draw, otherwise GL_FALSE if
 * there's nothing left to draw.
 */
static GLboolean
ComputeSubImage(GLint *x, GLint *y, GLsizei *width, GLsizei *height,
								GLint *skipPixels, GLint *skipRows,
								const CRrecti *scissor)
{
	GLint clip; /* number of clipped pixels */

	if (*x + *width <= scissor->x1 ||
			*x >= scissor->x2 ||
			*y + *height <= scissor->y1 ||
			*y >= scissor->y2) {
		/* totally clipped */
		return GL_FALSE;
	}

	if (*x < scissor->x1) {
		/* image crosses left edge of scissor box */
		CRASSERT(*x + *width > scissor->x1);
		clip = scissor->x1 - *x;
		CRASSERT(clip > 0);
		*x = scissor->x1;
		*skipPixels += clip;
		*width -= clip;
	}
	if (*y < scissor->y1) {
		/* image crosses bottom edge of scissor box */
		CRASSERT(*y + *height > scissor->y1);
		clip = scissor->y1 - *y;
		CRASSERT(clip > 0);
		*y = scissor->y1;
		*skipRows += clip;
		*height -= clip;
	}
	if (*x + *width > scissor->x2) {
		/* image crossed right edge of scissor box */
		CRASSERT(*x <= scissor->x2);
		clip = *x + *width - scissor->x2;
		CRASSERT(clip > 0);
		*width -= clip;
	}
	if (*y + *height > scissor->y2) {
		/* image crossed top edge of scissor box */
		CRASSERT(*y <= scissor->y2);
		clip = *y + *height - scissor->y2;
		CRASSERT(clip > 0);
		*height -= clip;
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

	if (winInfo->bucketMode != BROADCAST && oldZoomX == 1.0 && oldZoomY == 1.0) {
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
				GLint newX = (GLint)c->rasterAttrib[VERT_ATTRIB_POS][0];
				GLint newY = (GLint)c->rasterAttrib[VERT_ATTRIB_POS][1];
				GLsizei newWidth = zoomedWidth;
				GLsizei newHeight = zoomedHeight;
				if (!unpacking.rowLength)
					unpacking.rowLength = width;
				if (ComputeSubImage(&newX, &newY, &newWidth, &newHeight,
														&unpacking.skipPixels, &unpacking.skipRows,
														&winInfo->server[i].extents[j])) {
					/*
					printf("clipped: %d, %d  %d x %d  skip %d, %d\n",
								 newX, newY, newWidth, newHeight,
								 unpacking.skipPixels, unpacking.skipRows);
					*/
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
	CRPixelPackState *p = &(ctx->client.pack);
	CRViewportState *v = &(ctx->viewport);
	unsigned char *data_ptr;
	GLfloat screen_bbox[8];
	int i, stride;
	int bytes_per_pixel = 0;
	CRrecti rect, isect;
	int len = 44 + sizeof(CRNetworkPointer);
	int offset;
	int zoomedWidth, zoomedHeight;
	GLenum hint;

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

	if (!c->rasterValid)
	{
		return;
	}

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

	/* min x, y, z, w */
	screen_bbox[0] = (c->rasterAttrib[VERT_ATTRIB_POS][0])/v->viewportW;
	screen_bbox[1] = (c->rasterAttrib[VERT_ATTRIB_POS][1])/v->viewportH;
	screen_bbox[2] = 0.0;
	screen_bbox[3] = 1.0;
	/* max x, y, z, w */
	screen_bbox[4] = (c->rasterAttrib[VERT_ATTRIB_POS][0] + width)/v->viewportW;
	screen_bbox[5] = (c->rasterAttrib[VERT_ATTRIB_POS][1] + height)/v->viewportH;
	screen_bbox[6] = 0.0;
	screen_bbox[7] = 1.0;

	/* map from [0, 1] to [-1, 1] */
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
	tilesortspuFlush( thread );

	switch ( type )
	{
#ifdef CR_OPENGL_VERSION_1_2
	  case GL_UNSIGNED_BYTE_3_3_2:
	  case GL_UNSIGNED_BYTE_2_3_3_REV:
#endif
	  case GL_UNSIGNED_BYTE:
	  case GL_BYTE:
			bytes_per_pixel = 1;
			break;

#ifdef CR_OPENGL_VERSION_1_2
	  case GL_UNSIGNED_SHORT_5_6_5:
	  case GL_UNSIGNED_SHORT_5_6_5_REV:
	  case GL_UNSIGNED_SHORT_5_5_5_1:
	  case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	  case GL_UNSIGNED_SHORT_4_4_4_4:
	  case GL_UNSIGNED_SHORT_4_4_4_4_REV:
#endif
	  case GL_UNSIGNED_SHORT:
	  case GL_SHORT:
			bytes_per_pixel = 2;
			break;

#ifdef CR_OPENGL_VERSION_1_2
	  case GL_UNSIGNED_INT_8_8_8_8:
	  case GL_UNSIGNED_INT_8_8_8_8_REV:
	  case GL_UNSIGNED_INT_10_10_10_2:
	  case GL_UNSIGNED_INT_2_10_10_10_REV:
#endif
	  case GL_UNSIGNED_INT:
	  case GL_INT:
	  case GL_FLOAT:
			bytes_per_pixel = 4;
			break;

	  default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "ReadPixels(type=0x%x)", type );
			return;
	}

	switch ( format )
	{
	  case GL_COLOR_INDEX:
			break;
	  case GL_STENCIL_INDEX:
			if (ctx->limits.stencilBits == 0)
			{
				crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
				return;
			}
			break;
	  case GL_DEPTH_COMPONENT:
			if (ctx->limits.depthBits == 0)
			{
				crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glReadPixels" );
				return;
			}
			break;
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_ALPHA:
	  case GL_INTENSITY:
	  case GL_LUMINANCE:
			break;

	  case GL_LUMINANCE_ALPHA:
			bytes_per_pixel *= 2;
			break;

	  case GL_RGB:
			bytes_per_pixel *= 3;
			break;

	  case GL_RGBA:
			bytes_per_pixel *= 4;
			break;

	  default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,
										"ReadPixels(format=0x%x)", format );
	}

	/* Build the rectangle here to save the addition each time */
	rect.x1 = x;
	rect.y1 = y;
	rect.x2 = x + width;  /** XXX \todo zoomedWidth, someday */
	rect.y2 = y + height;
	stride  = width * bytes_per_pixel;

	thread->currentContext->readPixelsCount = 0;
	for ( i = 0; i < tilesort_spu.num_servers; i++ )
	{
		int new_width, new_height, new_x, new_y, bytes_per_row;
		CRPackBuffer *buffer = &(thread->buffer[i]);

		/* Grab current server's boundaries */
		isect = winInfo->server[i].extents[0];  /* x1,y1,x2,y2 */

		/* Reset rectangle to current boundary for server */
		if ( isect.x1 < rect.x1 ) isect.x1 = rect.x1;
		if ( isect.y1 < rect.y1 ) isect.y1 = rect.y1;
		if ( isect.x2 > rect.x2 ) isect.x2 = rect.x2;
		if ( isect.y2 > rect.y2 ) isect.y2 = rect.y2;

		/* Don't bother with this server if we're out of bounds */
		if (!( isect.x2 > isect.x1 && isect.y2 > isect.y1 ))
			continue;

		/* We've got one in the pot ! */
		thread->currentContext->readPixelsCount++;

		/* Build new ReadPixels parameters */
		new_width  = isect.x2 - isect.x1;
		new_height = isect.y2 - isect.y1;
		new_x      = isect.x1 - winInfo->server[i].extents[0].x1;
		new_y      = isect.y1 - winInfo->server[i].extents[0].y1;

		bytes_per_row = new_width * bytes_per_pixel;

		/* 
		 * FIXME - this is brought from the pack SPU
		 * needs tweaking for the tilesort SPU.
		 */
#if 0
		if (p->alignment != 1) {
			GLint remainder = bytes_per_row % p->alignment;
		 	if (remainder)
				stride = bytes_per_row + (p->alignment - remainder);
		}
#endif
		
		/* Calculate X offset */
		offset = (isect.x1 - x) * bytes_per_pixel;
		/* Calculate Y offset */
		offset += ((isect.y1 - y) * stride);

		/* Munge the per server packing structures */
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
		WRITE_DATA( 24, GLint,  stride );
		WRITE_DATA( 28, GLint, p->alignment );
		WRITE_DATA( 32, GLint, p->skipRows );
		WRITE_DATA( 36, GLint, p->skipPixels );
		WRITE_DATA( 40, GLint,  bytes_per_row );
		WRITE_NETWORK_POINTER( 44, (char *) pixels + offset );
		*(buffer->opcode_current--) = (unsigned char) CR_READPIXELS_OPCODE;

		tilesortspuSendServerBuffer( i );
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


void TILESORTSPU_APIENTRY tilesortspu_CopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
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
	    if (tilesort_spu.lazySendDLists) crDLMCompileCopyPixels(x, y, width, height, type);
	    else if (tilesort_spu.swap) crPackCopyPixelsSWAP(x, y, width, height, type);
	    else crPackCopyPixels(x, y, width, height, type);
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
			tilesortspu_ReadPixels(x,y,width,height,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
			tilesortspu_DrawPixels(width,height,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
			break;
		case GL_DEPTH:
			tilesortspu_ReadPixels(x,y,width,height,GL_DEPTH_COMPONENT,GL_FLOAT,buffer);
			tilesortspu_DrawPixels(width,height,GL_DEPTH_COMPONENT,GL_FLOAT,buffer);
			return;
		case GL_STENCIL:
			tilesortspu_ReadPixels(x,y,width,height,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,buffer);
			tilesortspu_DrawPixels(width,height,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,buffer);
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
