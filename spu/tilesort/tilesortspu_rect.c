/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_packfunctions.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_Rectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) 
{
	GET_CONTEXT(ctx);
	if (ctx->current.inBeginEnd)
	{
			crError( "tilesortspu_Rect?? called in Begin/End");
	}

	crStateFlushFunc( tilesortspuFlush_callback );

#if 1
	/* the usual implementation */
	if (tilesort_spu.swap)
	{
		crPackRectfSWAP (x1, y1, x2, y2);
	}
	else
	{
		crPackRectf (x1, y1, x2, y2);
	}

	if (thread->packer->bounds_min.x > x1) thread->packer->bounds_min.x = x1;
	if (thread->packer->bounds_min.y > y1) thread->packer->bounds_min.y = y1;
	if (thread->packer->bounds_max.x < x1) thread->packer->bounds_max.x = x1;
	if (thread->packer->bounds_max.y < y1) thread->packer->bounds_max.y = y1;

	if (thread->packer->bounds_min.x > x2) thread->packer->bounds_min.x = x2;
	if (thread->packer->bounds_min.y > y2) thread->packer->bounds_min.y = y2;
	if (thread->packer->bounds_max.x < x2) thread->packer->bounds_max.x = x2;
	if (thread->packer->bounds_max.y < y2) thread->packer->bounds_max.y = y2;

	if (thread->packer->bounds_min.z > 0.0f) thread->packer->bounds_min.z = 0.0f;
	if (thread->packer->bounds_max.z < 0.0f) thread->packer->bounds_max.z = 0.0f;
#else
	/* render as polygon */
	tilesortspu_Begin(GL_POLYGON);
	crPackVertex2fBBOX_COUNT(x1, y1);
	crPackVertex2fBBOX_COUNT(x2, y1);
	crPackVertex2fBBOX_COUNT(x2, y2);
	crPackVertex2fBBOX_COUNT(x1, y2);
	tilesortspu_End();
#endif
}

void TILESORTSPU_APIENTRY tilesortspu_Rectfv (const GLfloat *v1, const GLfloat *v2) 
{
	tilesortspu_Rectf( (GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1] );
}

void TILESORTSPU_APIENTRY tilesortspu_Rectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
	tilesortspu_Rectf( (GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2 );
}

void TILESORTSPU_APIENTRY tilesortspu_Rectdv( const GLdouble *v1, const GLdouble *v2 )
{
	tilesortspu_Rectf( (GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1] );
}

void TILESORTSPU_APIENTRY tilesortspu_Recti (GLint x1, GLint y1, GLint x2, GLint y2) 
{
	tilesortspu_Rectf( (GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2 );
}

void TILESORTSPU_APIENTRY tilesortspu_Rectiv (const GLint *v1, const GLint *v2) 
{
	tilesortspu_Rectf( (GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1] );
}

void TILESORTSPU_APIENTRY tilesortspu_Rects (GLshort x1, GLshort y1, GLshort x2, GLshort y2) 
{
	tilesortspu_Rectf( (GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2 );
}

void TILESORTSPU_APIENTRY tilesortspu_Rectsv (const GLshort *v1, const GLshort *v2) 
{
	tilesortspu_Rectf( (GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1] );
}
