/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"

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

void TILESORTSPU_APIENTRY tilesortspu_BarrierCreate(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierCreateSWAP(name,count);
	}
	else
	{
		crPackBarrierCreate(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierDestroy(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierDestroySWAP(name);
	}
	else
	{
		crPackBarrierDestroy(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierExec(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackBarrierExecSWAP(name);
	}
	else
	{
		crPackBarrierExec(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreCreate(GLuint name, GLuint count)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreCreateSWAP(name,count);
	}
	else
	{
		crPackSemaphoreCreate(name,count);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreDestroy(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreDestroySWAP(name);
	}
	else
	{
		crPackSemaphoreDestroy(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreP(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphorePSWAP(name);
	}
	else
	{
		crPackSemaphoreP(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreV(GLuint name)
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackSemaphoreVSWAP(name);
	}
	else
	{
		crPackSemaphoreV(name);
	}
	tilesortspuBroadcastGeom(0);
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_CallList( GLuint list )
{
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackCallListSWAP( list );
	}
	else
	{
		crPackCallList( list );
	}
	tilesortspuBroadcastGeom(1);
}

void TILESORTSPU_APIENTRY tilesortspu_CallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	GET_CONTEXT(ctx);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
	{
		crPackListBaseSWAP( ctx->lists.base );
	}
	else
	{
		crPackListBase( ctx->lists.base );
	}
	if (tilesort_spu.swap)
	{
		crPackCallListsSWAP( n, type, lists );
	}
	else
	{
		crPackCallLists( n, type, lists );
	}
	tilesortspuBroadcastGeom(1);
}
