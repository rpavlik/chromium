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

void TILESORTSPU_APIENTRY tilesortspu_DrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	CRCurrentState *c = &(tilesort_spu.ctx->current);
	CRViewportState *v = &(tilesort_spu.ctx->viewport);

	GLenum hint;

	if (c->inBeginEnd)
	{
		crError("DrawPixels called in begin/end");
		return;
	}

	if (width < 0 || height < 0)
	{
		crError("DrawPixels called with neg dims: %dx%d", width, height);
		return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	tilesortspuFlush( tilesort_spu.ctx );

	cr_packer_globals.bounds_min.x = (c->rasterPos.x)/v->viewportW;
	cr_packer_globals.bounds_min.y = (c->rasterPos.y)/v->viewportH;
	cr_packer_globals.bounds_max.x = (c->rasterPos.x + width)/v->viewportW;
	cr_packer_globals.bounds_max.y = (c->rasterPos.y + height)/v->viewportH;

	cr_packer_globals.bounds_min.x *= 2.0f;
	cr_packer_globals.bounds_min.y *= 2.0f;
	cr_packer_globals.bounds_max.x *= 2.0f;
	cr_packer_globals.bounds_max.y *= 2.0f;

	cr_packer_globals.bounds_min.x -= 1.0f;
	cr_packer_globals.bounds_min.y -= 1.0f;
	cr_packer_globals.bounds_max.x -= 1.0f;
	cr_packer_globals.bounds_max.y -= 1.0f;

	hint = tilesort_spu.providedBBOX;
	tilesort_spu.providedBBOX = CR_SCREEN_BBOX_HINT;

	/* don't do a flush, DrawPixels understand that it needs to flush,
		 and will handle all that for us.  our HugeFunc routine will
		 specially handle the DrawPixels call*/

	tilesort_spu.inDrawPixels = 1;
	crPackDrawPixels (width, height, format, type, pixels, &(tilesort_spu.ctx->pixel.unpack));

	if (cr_packer_globals.buffer.data_current != cr_packer_globals.buffer.data_start)
	{
		tilesortspuFlush( tilesort_spu.ctx );
	}
	tilesort_spu.inDrawPixels = 0;

	tilesort_spu.providedBBOX = hint;
}

void TILESORTSPU_APIENTRY tilesortspu_Bitmap( 
		GLsizei width, GLsizei height,
		GLfloat xorig, GLfloat yorig,
		GLfloat xmove, GLfloat ymove,
		const GLubyte * bitmap) 
{
	CRCurrentState *c = &(tilesort_spu.ctx->current);
	CRViewportState *v = &(tilesort_spu.ctx->viewport);
	GLfloat screen_bbox[8];

	if (!c->rasterValid)
	{
		return;
	}

	tilesortspuFlush( tilesort_spu.ctx );

	screen_bbox[0] = (c->rasterPos.x - xorig)/v->viewportW;
	screen_bbox[1] = (c->rasterPos.y - yorig)/v->viewportH;
	screen_bbox[4] = (c->rasterPos.x - xorig + width)/v->viewportW;
	screen_bbox[5] = (c->rasterPos.y - yorig + height)/v->viewportH;

	screen_bbox[0] *= 2.0f;
	screen_bbox[1] *= 2.0f;
	screen_bbox[4] *= 2.0f;
	screen_bbox[5] *= 2.0f;

	screen_bbox[0] -= 1.0f;
	screen_bbox[1] -= 1.0f;
	screen_bbox[4] -= 1.0f;
	screen_bbox[5] -= 1.0f;

	crStateBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
	if (tilesort_spu.swap)
	{
		crPackBitmapSWAP ( width, height, xorig, yorig, xmove, ymove, bitmap, &(tilesort_spu.ctx->pixel.unpack) );
	}
	else
	{
		crPackBitmap ( width, height, xorig, yorig, xmove, ymove, bitmap, &(tilesort_spu.ctx->pixel.unpack) );
	}

	tilesortspu_Hint( CR_SCREEN_BBOX_HINT, (GLenum) screen_bbox );
	c->rasterPosPre.x -= xmove;
	c->rasterPosPre.y -= ymove;
	tilesortspuFlush( tilesort_spu.ctx );
	c->rasterPosPre.x += xmove;
	c->rasterPosPre.y += ymove;
	tilesortspu_Hint( CR_SCREEN_BBOX_HINT, (GLenum) NULL );
}
