/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "state/cr_stateerror.h"

void TILESORTSPU_APIENTRY tilesortspu_Accum( GLenum op, GLfloat value )
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackAccumSWAP( op, value );
	}
	else
	{
		crPackAccum( op, value );
	}
	tilesortspuBroadcastGeom(1);
}

void TILESORTSPU_APIENTRY tilesortspu_Clear( GLbitfield mask )
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackClearSWAP( mask );
	}
	else
	{
		crPackClear( mask );
	}
	tilesortspuBroadcastGeom(1);
}

void TILESORTSPU_APIENTRY tilesortspu_Finish(void)
{
	GET_THREAD(thread);
	int writeback = tilesort_spu.num_servers;
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackFinishSWAP( );
	}
	else
	{
		crPackFinish( );
	}
	if (tilesort_spu.syncOnFinish)
	{
		if (tilesort_spu.swap)
		{
			crPackWritebackSWAP( &writeback );
		}
		else
		{
			crPackWriteback( &writeback );
		}
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
	if (tilesort_spu.syncOnFinish)
	{
		while(writeback)
		{
			crNetRecv();
		}
	}
}

void TILESORTSPU_APIENTRY tilesortspu_Flush(void)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackFlushSWAP();
	}
	else
	{
		crPackFlush();
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierCreateCR(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierCreateCRSWAP(name,count);
	}
	else
	{
		crPackBarrierCreateCR(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierDestroyCR(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierDestroyCRSWAP(name);
	}
	else
	{
		crPackBarrierDestroyCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierExecCR(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierExecCRSWAP(name);
	}
	else
	{
		crPackBarrierExecCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreCreateCR(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreCreateCRSWAP(name,count);
	}
	else
	{
		crPackSemaphoreCreateCR(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreDestroyCR(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreDestroyCRSWAP(name);
	}
	else
	{
		crPackSemaphoreDestroyCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphorePCR(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphorePCRSWAP(name);
	}
	else
	{
		crPackSemaphorePCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreVCR(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreVCRSWAP(name);
	}
	else
	{
		crPackSemaphoreVCR(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

static GLuint
translate_id(const GLvoid *lists, GLenum type, GLuint i)
{
	GLuint list;
	switch (type)
	{
		case GL_BYTE:
			{
				const GLbyte *p = (const GLbyte *) lists;
				list = p[i];
			}
			break;
		case GL_UNSIGNED_BYTE:
			{
				const GLubyte *p = (const GLubyte *) lists;
				list = p[i];
			}
			break;
		case GL_SHORT:
			{
				const GLshort *p = (const GLshort *) lists;
				list = p[i];
			}
			break;
		case GL_UNSIGNED_SHORT:
			{
				const GLushort *p = (const GLushort *) lists;
				list = p[i];
			}
			break;
		case GL_INT:
			{
				const GLint *p = (const GLint *) lists;
				list = p[i];
			}
			break;
		case GL_UNSIGNED_INT:
			{
				const GLuint *p = (const GLuint *) lists;
				list = p[i];
			}
			break;
		case GL_FLOAT:
			{
				const GLfloat *p = (const GLfloat *) lists;
				list = (GLuint) p[i];
			}
			break;
		case GL_2_BYTES:
			{
				const GLubyte *p = (const GLubyte *) lists;
				list = ((GLuint) p[i*2]) * 256 + (GLuint) p[i*2+1];
			}
			break;
		case GL_3_BYTES:
			{
				const GLubyte *p = (const GLubyte *) lists;
				list = ((GLuint) p[i*3+0]) * (256 * 256)
				     + ((GLuint) p[i*3+1]) * 256
						 + ((GLuint) p[i*3+2]);
			}
			break;
		case GL_4_BYTES:
			{
				const GLubyte *p = (const GLubyte *) lists;
				list = ((GLuint) p[i*4+0]) * (256 * 256 * 256)
				     + ((GLuint) p[i*4+1]) * (256 * 256)
				     + ((GLuint) p[i*4+2]) * 256
						 + ((GLuint) p[i*4+3]);
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glCallLists bad type");
			return (GLuint) -1;  /* kind of a hack */
	}

	return list;
}


/*
 * Execute the state-change side-effect associated with executing
 * the given display list.
 * We only do glBitmap raster pos changes, for now.
 * Also, nested display lists are a problem (but they're rare).
 */
static void execute_side_effects(GLuint list)
{
	GET_CONTEXT(ctx);
	const CRListEffect *effect = (const CRListEffect *) crHashtableSearch(ctx->lists.hash, list);
	if (effect) {
		int j;
		/* loop over back-end server contexts */
		for (j = 0; j < tilesort_spu.num_servers; j++) {
			CRContext *c;
			c = tilesort_spu.servers[j].context[thread->currentContextIndex];
			CRASSERT(c);
			/* update context's raster pos */
			c->current.rasterPos.x += effect->rasterPosDx;
			c->current.rasterPos.y += effect->rasterPosDy;
			/*
			printf("Post CallLists %d  ctx=%p dx/dy = %f, %f  new=%f, %f\n", list,
						 (void *) (&c->current),
						 effect->rasterPosDx, effect->rasterPosDy,
						 c->current.rasterPos.x, c->current.rasterPos.y);
			*/
		}
	}
}


void TILESORTSPU_APIENTRY tilesortspu_CallList( GLuint list )
{
	GET_THREAD(thread);

	if (tilesort_spu.providedBBOX == GL_DEFAULT_BBOX_CR) {
		tilesortspuFlush( thread );
	}

	if (tilesort_spu.swap)
	   crPackCallListSWAP( list );
	else
	   crPackCallList( list );

	if (tilesort_spu.providedBBOX == GL_DEFAULT_BBOX_CR) {
		/* we don't have a bounding box for the geometry in this
		 * display list, so need to broadcast.
		 */
		/*printf("%s broadcast\n", __FUNCTION__);*/
		tilesortspuBroadcastGeom(1);
	}
	else {
		/* we relied upon the bucketing system to send the list to the
		 * appropriate servers.
		 */
		/*printf("%s bucket\n", __FUNCTION__);*/
		tilesortspuFlush( thread );
	}

	execute_side_effects(list);
}

void TILESORTSPU_APIENTRY tilesortspu_CallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	GET_CONTEXT(ctx);
	GLint i;

	if (tilesort_spu.providedBBOX == GL_DEFAULT_BBOX_CR) {
		tilesortspuFlush( thread );
	}

	if (tilesort_spu.swap) {
		crPackListBaseSWAP( ctx->lists.base );
		crPackCallListsSWAP( n, type, lists );
	}
	else {
		crPackListBase( ctx->lists.base );
		crPackCallLists( n, type, lists );
	}

	if (tilesort_spu.providedBBOX == GL_DEFAULT_BBOX_CR) {
		/* we don't have a bounding box for the geometry in this
		 * display list, so need to broadcast.
		 */
		tilesortspuBroadcastGeom(1);
	}
	else {
		/* we relied upon the bucketing system to send the list to the
		 * appropriate servers.
		 */
		tilesortspuFlush( thread );
	}

	for (i = 0; i < n; i++) {
		const GLuint list = translate_id(lists, type, i) + ctx->lists.base;
		execute_side_effects(list);
	}
}
