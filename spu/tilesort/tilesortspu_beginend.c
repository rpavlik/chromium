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
	GET_CONTEXT(ctx);
	CRTransformState *t = &(ctx->transform);
	CRCurrentState *c = &(ctx->current);
	/* We have to set this every time because a flush from 
	 * the state tracker will turn off its flusher. */

	thread->pinchState.beginOp = thread->packer->buffer.opcode_current;
	thread->pinchState.beginData = thread->packer->buffer.data_current;
	thread->pinchState.wind = 0;
	thread->pinchState.isLoop = 0;

	crStateFlushFunc( tilesortspuFlush_callback );
	if (tilesort_spu.swap)
	{
		crPackBeginSWAP( mode );
	}
	else
	{
		crPackBegin( mode );
	}
	crStateBegin( mode );
	if (! t->transformValid)
	{
		/* Make sure that the state tracker has the very very 
		 * latest composite modelview + projection matrix 
		 * computed, since we're going to need it. */
		crStateTransformUpdateTransform( t );
	}
	c->current->vtx_count_begin = c->current->vtx_count;
}

void TILESORTSPU_APIENTRY tilesortspu_End( void )
{
	GET_CONTEXT(ctx);
	CRLimitsState *limits = &(ctx->limits);
	if (thread->pinchState.isLoop)
	{
		unsigned int i;

		for (i = 0 ; i < limits->maxTextureUnits; i++)
		{
			if (i == 0)
			{
				if (tilesort_spu.swap)
				{
					crPackTexCoord4fvSWAP ( (GLfloat *) &(thread->pinchState.vtx->texCoord[i].s));
				}
				else
				{
					crPackTexCoord4fv ( (GLfloat *) &(thread->pinchState.vtx->texCoord[i].s));
				}
			}
			else
			{
				if (tilesort_spu.swap)
				{
					crPackMultiTexCoord4fvARBSWAP ( i + GL_TEXTURE0_ARB, (GLfloat *) &(thread->pinchState.vtx->texCoord[i].s));
				}
				else
				{
					crPackMultiTexCoord4fvARB ( i + GL_TEXTURE0_ARB, (GLfloat *) &(thread->pinchState.vtx->texCoord[i].s));
				}
			}
		}
		if (tilesort_spu.swap)
		{
			crPackNormal3fvSWAP((GLfloat *) &(thread->pinchState.vtx->normal.x));
		}
		else
		{
			crPackNormal3fv((GLfloat *) &(thread->pinchState.vtx->normal.x));
		}
		if (tilesort_spu.swap)
		{
			crPackEdgeFlagSWAP(thread->pinchState.vtx->edgeFlag);
		}
		else
		{
			crPackEdgeFlag(thread->pinchState.vtx->edgeFlag);
		}
		if (tilesort_spu.swap)
		{
			crPackColor4fvSWAP((GLfloat *) &(thread->pinchState.vtx->color.r));
		}
		else
		{
			crPackColor4fv((GLfloat *) &(thread->pinchState.vtx->color.r));
		}
		if (tilesort_spu.swap)
		{
			crPackVertex4fvBBOX_COUNTSWAP((GLfloat *) &(thread->pinchState.vtx->pos.x));
		}
		else
		{
			crPackVertex4fvBBOX_COUNT((GLfloat *) &(thread->pinchState.vtx->pos.x));
		}

		thread->pinchState.isLoop = 0;
	}
	if (tilesort_spu.swap)
	{
		crPackEndSWAP();
	}
	else
	{
		crPackEnd();
	}
	crStateEnd();
}
