/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "state/cr_statefuncs.h"
#include "tilesortspu.h"
#include "tilesortspu_gen.h"


void TILESORTSPU_APIENTRY tilesortspu_Materiali(GLenum face, GLenum mode, GLint param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
		/* just creating and/or compiling display lists */
		if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		    crDLMCompileMateriali(face, mode, param);
		    if (tilesort_spu.lazySendDLists) return;
		}
		if (tilesort_spu.swap) crPackMaterialiSWAP( face, mode, param );
		else crPackMateriali( face, mode, param );
		return;
	}

	crStateMateriali( face, mode, param );
	if (ctx->current.inBeginEnd)
	{
		if (tilesort_spu.swap)
		{
			crPackMaterialiSWAP( face, mode, param );
		}
		else
		{
			crPackMateriali( face, mode, param );
		}
	}
}

void TILESORTSPU_APIENTRY tilesortspu_Materialf(GLenum face, GLenum mode, GLfloat param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompileMaterialf(face, mode, param);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackMaterialfSWAP( face, mode, param );
	    else crPackMaterialf( face, mode, param );
	    return;
	}

	crStateMaterialf( face, mode, param );
	if (ctx->current.inBeginEnd)
	{
		if (tilesort_spu.swap)
		{
			crPackMaterialfSWAP( face, mode, param );
		}
		else
		{
			crPackMaterialf( face, mode, param );
		}	
	}
}

void TILESORTSPU_APIENTRY tilesortspu_Materialiv(GLenum face, GLenum mode, const GLint *param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompileMaterialiv(face, mode, param);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackMaterialivSWAP( face, mode, param );
	    else crPackMaterialiv( face, mode, param );
	    return;
	}

	crStateMaterialiv( face, mode, param );
	if (ctx->current.inBeginEnd || dlMode != GL_FALSE)
	{
		if (tilesort_spu.swap)
		{
			crPackMaterialivSWAP( face, mode, param );
		}
		else
		{
			crPackMaterialiv( face, mode, param );
		}
	}
}

void TILESORTSPU_APIENTRY tilesortspu_Materialfv(GLenum face, GLenum mode, const GLfloat *param)
{
	GET_CONTEXT(ctx);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		crDLMCompileMaterialfv(face, mode, param);
		if (tilesort_spu.lazySendDLists) return;
	    }
	    if (tilesort_spu.swap) crPackMaterialfvSWAP( face, mode, param );
	    else crPackMaterialfv( face, mode, param );
	    return;
	}

	crStateMaterialfv( face, mode, param );
	if (ctx->current.inBeginEnd)
	{
		if (tilesort_spu.swap)
		{
			crPackMaterialfvSWAP( face, mode, param );
		}
		else
		{
			crPackMaterialfv( face, mode, param );
		}
	}
}
