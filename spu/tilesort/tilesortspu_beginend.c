/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_glstate.h"

void TILESORTSPU_APIENTRY tilesortspu_Begin( GLenum mode )
{
	CRTransformState *t = &(tilesort_spu.ctx->transform);
	CRCurrentState *c = &(tilesort_spu.ctx->current);
	// We have to set this every time because a flush from
	// the state tracker will turn off its flusher.
	
	tilesort_spu.pinchState.beginOp = cr_packer_globals.buffer.opcode_current;
	tilesort_spu.pinchState.beginData = cr_packer_globals.buffer.data_current;
	tilesort_spu.pinchState.wind = 0;
	tilesort_spu.pinchState.isLoop = 0;

	crStateFlushFunc( tilesortspuFlush );
	crPackBegin( mode );
	crStateBegin( mode );
	if (! t->transformValid)
	{
		// Make sure that the state tracker has the very very
		// latest composite modelview + projection matrix
		// computed, since we're going to need it.
		crStateTransformUpdateTransform( t );
	}
	c->current->vtx_count_begin = c->current->vtx_count;
}

void TILESORTSPU_APIENTRY tilesortspu_End( void )
{
	if (tilesort_spu.pinchState.isLoop)
	{
		int i;

		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
		{
			crPackMultiTexCoord4fvARB ( i + GL_TEXTURE0_ARB, (GLfloat *) &(tilesort_spu.pinchState.vtx->texCoord[i].s));
		}
		crPackNormal3fv((GLfloat *) &(tilesort_spu.pinchState.vtx->normal.x));
		crPackEdgeFlag(tilesort_spu.pinchState.vtx->edgeFlag);
		crPackColor4fv((GLfloat *) &(tilesort_spu.pinchState.vtx->color.r));
		crPackVertex4fvBBOX_COUNT((GLfloat *) &(tilesort_spu.pinchState.vtx->pos.x));

		tilesort_spu.pinchState.isLoop = 0;
	}
	crPackEnd();
	crStateEnd();
}
