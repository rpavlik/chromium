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
#include "cr_applications.h"
#include "cr_pixeldata.h"
#include "cr_mem.h"

/*
 * Execute glDrawPixels().
 */
static void tilesortspu_exec_DrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	GLfloat screen_bbox[8];
	GLenum hint;

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

	if (crImageSize(format, type, width, height) > thread->net[0].conn->mtu ) {
		crError("DrawPixels called with insufficient MTU size\n"
			"Needed %d, but currently MTU is set at %d\n", 
			crImageSize(format, type, width, height), 
			thread->net[0].conn->mtu );
		return;
	}

	tilesortspuFlush( thread );

	screen_bbox[0] = (c->rasterPos.x)/v->viewportW;
	screen_bbox[1] = (c->rasterPos.y)/v->viewportH;
	screen_bbox[4] = (c->rasterPos.x + width)/v->viewportW;
	screen_bbox[5] = (c->rasterPos.y + height)/v->viewportH;

	screen_bbox[0] *= 2.0f;
	screen_bbox[1] *= 2.0f;
	screen_bbox[4] *= 2.0f;
	screen_bbox[5] *= 2.0f;

	screen_bbox[0] -= 1.0f;
	screen_bbox[1] -= 1.0f;
	screen_bbox[4] -= 1.0f;
	screen_bbox[5] -= 1.0f;

	hint = tilesort_spu.providedBBOX;
	tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

	/* 
	 * don't do a flush, DrawPixels understand that it needs to flush,
	 * and will handle all that for us.  our HugeFunc routine will
	 * specially handle the DrawPixels call
	 */

	tilesort_spu.inDrawPixels = 1;
	crPackDrawPixels (width, height, format, type, pixels, &(ctx->client.unpack));

	if (thread->packer->buffer.data_current != thread->packer->buffer.data_start)
	{
		tilesortspuFlush( thread );
	}
	tilesort_spu.inDrawPixels = 0;

	tilesort_spu.providedBBOX = hint;
}


/*
 * glDrawPixels entry point.  We'll either execute DrawPixels, compile it
 * into the display list, or both.
 * We can't just call the crPackDrawPixels() function from the dispatch table
 * since it requires a pixel-unpacking argument.
 */
void TILESORTSPU_APIENTRY tilesortspu_DrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);

	if (ctx->lists.mode == 0 || ctx->lists.mode == GL_COMPILE_AND_EXECUTE)
	{
		/* execute */
		tilesortspu_exec_DrawPixels(width, height, format, type, pixels);
	}
	if (ctx->lists.mode != 0)
	{
		/* compile into display list */
		if (tilesort_spu.swap)
		{
			crPackDrawPixelsSWAP(width, height, format, type, pixels, &(ctx->client.unpack));
		}
		else
		{
			crPackDrawPixels(width, height, format, type, pixels, &(ctx->client.unpack));
		}
	}
}

void TILESORTSPU_APIENTRY tilesortspu_ReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRPixelPackState *p = &(ctx->client.pack);
	CRViewportState *v = &(ctx->viewport);
	unsigned char *data_ptr;
	GLfloat screen_bbox[8];
	int i, stride;
	int bytes_per_pixel = 0;
	GLrecti rect, isect;
	int len = 44 + sizeof(CRNetworkPointer);
	int offset;
	int new_width, new_height, new_x, new_y, bytes_per_row;
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

	tilesortspuFlush( thread );

	screen_bbox[0] = (c->rasterPos.x)/v->viewportW;
	screen_bbox[1] = (c->rasterPos.y)/v->viewportH;
	screen_bbox[4] = (c->rasterPos.x + width)/v->viewportW;
	screen_bbox[5] = (c->rasterPos.y + height)/v->viewportH;

	screen_bbox[0] *= 2.0f;
	screen_bbox[1] *= 2.0f;
	screen_bbox[4] *= 2.0f;
	screen_bbox[5] *= 2.0f;

	screen_bbox[0] -= 1.0f;
	screen_bbox[1] -= 1.0f;
	screen_bbox[4] -= 1.0f;
	screen_bbox[5] -= 1.0f;

	hint = tilesort_spu.providedBBOX;
	tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

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
		crError( "ReadPixels: type=0x%x", type );
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
		crError( "ReadPixels: format=0x%x", format );
	}

	/* Build the rectangle here to save the addition each time */
	rect.x1 = x;
	rect.y1 = y;
	rect.x2 = x + width;
	rect.y2 = y + height;
	stride  = width * bytes_per_pixel;

	tilesort_spu.ReadPixels = 0;
	for ( i = 0; i < tilesort_spu.num_servers; i++ )
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		CRPackBuffer *pack = &(thread->pack[i]);

		/* Grab current server's boundaries */
		isect = server->extents[0];  /* x1,y1,x2,y2 */

		/* Reset rectangle to current boundary for server */
		if ( isect.x1 < rect.x1 ) isect.x1 = rect.x1;
		if ( isect.y1 < rect.y1 ) isect.y1 = rect.y1;
		if ( isect.x2 > rect.x2 ) isect.x2 = rect.x2;
		if ( isect.y2 > rect.y2 ) isect.y2 = rect.y2;

		/* Don't bother with this server if we're out of bounds */
		if (!( isect.x2 > isect.x1 && isect.y2 > isect.y1 ))
			continue;

		/* We've got one in the pot ! */
		tilesort_spu.ReadPixels++;

		/* Build new ReadPixels parameters */
		new_width  = isect.x2 - isect.x1;
		new_height = isect.y2 - isect.y1;
		new_x      = isect.x1 - server->extents[0].x1;
		new_y      = isect.y1 - server->extents[0].y1;

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
		data_ptr = pack->data_current; 
		if (data_ptr + len > pack->data_end ) 
 		{ 
			tilesortspuFlush( thread );
			data_ptr = pack->data_current; 
			CRASSERT( data_ptr + len <= pack->data_end ); 
		}
		pack->data_current += len;
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
		*(pack->opcode_current--) = (unsigned char) CR_READPIXELS_OPCODE;

		tilesortspuSendServerBuffer( i );
	}

	/* Ensure all readpixel buffers are on the way !! */
	tilesortspuFlush( thread );

	/* We need to receive them all back before continuing */
	while ( tilesort_spu.ReadPixels > 0 )
	{
		crNetRecv( );
	}

	tilesort_spu.providedBBOX = hint;
}

void TILESORTSPU_APIENTRY tilesortspu_CopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	char *buffer;
	(void)v;
	(void)c;

	buffer = (char*) crAlloc(width * height * 4);

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
			crError("CopyPixels - unknown type\n");
			return;
	}

	crFree(buffer);
}


/*
 * Execute glBitmap.
 */
static void tilesortspu_exec_Bitmap( 
		GLsizei width, GLsizei height,
		GLfloat xorig, GLfloat yorig,
		GLfloat xmove, GLfloat ymove,
		const GLubyte * bitmap) 
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	CRViewportState *v = &(ctx->viewport);
	GLfloat screen_bbox[8];
	GLenum hint;

	if (!c->rasterValid)
	{
		return;
	}

	tilesortspuFlush( thread );

	crStateBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
	if (tilesort_spu.swap)
	{
		crPackBitmapSWAP ( width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack) );
	}
	else
	{
		crPackBitmap ( width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack) );
	}

	screen_bbox[0] = (c->rasterPos.x - xorig)/v->viewportW;
	screen_bbox[1] = (c->rasterPos.y - yorig)/v->viewportH;
	screen_bbox[4] = (c->rasterPos.x - xorig + width) / v->viewportW;
	screen_bbox[5] = (c->rasterPos.y - yorig + height) / v->viewportH;

	screen_bbox[0] *= 2.0f;
	screen_bbox[1] *= 2.0f;
	screen_bbox[4] *= 2.0f;
	screen_bbox[5] *= 2.0f;

	screen_bbox[0] -= 1.0f;
	screen_bbox[1] -= 1.0f;
	screen_bbox[4] -= 1.0f;
	screen_bbox[5] -= 1.0f;

	hint = tilesort_spu.providedBBOX;
	tilesortspu_ChromiumParametervCR(GL_SCREEN_BBOX_CR, GL_FLOAT, 8, screen_bbox);

	c->rasterPosPre.x -= xmove;
	c->rasterPosPre.y -= ymove;

	tilesortspuFlush( thread );

	c->rasterPosPre.x += xmove;
	c->rasterPosPre.y += ymove;

	tilesort_spu.providedBBOX = hint;
}


/*
 * glBitmap entry point.  We'll either execute Bitmap, compile it
 * into the display list, or both.
 * We can't just call the crPackBitmap() function from the dispatch table
 * since it requires a pixel-unpacking argument.
 */
void TILESORTSPU_APIENTRY tilesortspu_Bitmap( 
		GLsizei width, GLsizei height,
		GLfloat xorig, GLfloat yorig,
		GLfloat xmove, GLfloat ymove,
		const GLubyte * bitmap)
{
	GET_CONTEXT(ctx);

	if (ctx->lists.mode == 0 || ctx->lists.mode == GL_COMPILE_AND_EXECUTE)
	{
		/* execute */
		tilesortspu_exec_Bitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
	}
	if (ctx->lists.mode != 0)
	{
		/* compile into display list */
		if (tilesort_spu.swap)
		{
			crPackBitmapSWAP(width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack));
		}
		else
		{
			crPackBitmap(width, height, xorig, yorig, xmove, ymove, bitmap, &(ctx->client.unpack));
		}
	}
}



void TILESORTSPU_APIENTRY tilesortspu_PixelMapfv (GLenum map, GLint mapsize, const GLfloat * values)
{
	crStatePixelMapfv ( map, mapsize, values );
	if (tilesort_spu.swap) {
		crPackPixelMapfvSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapfv ( map, mapsize, values );
	}
}

void TILESORTSPU_APIENTRY tilesortspu_PixelMapuiv (GLenum map, GLint mapsize, const GLuint * values)
{
	crStatePixelMapuiv ( map, mapsize, values );
	if (tilesort_spu.swap) {
		crPackPixelMapuivSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapuiv ( map, mapsize, values );
	}
}
 
void TILESORTSPU_APIENTRY tilesortspu_PixelMapusv (GLenum map, GLint mapsize, const GLushort * values)
{
	crStatePixelMapusv ( map, mapsize, values );
	if (tilesort_spu.swap) {
		crPackPixelMapusvSWAP ( map, mapsize, values );
	} else {
		crPackPixelMapusv ( map, mapsize, values );
	}
}
