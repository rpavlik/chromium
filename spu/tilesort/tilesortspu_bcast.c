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
	tilesortspuFlush( tilesort_spu.ctx );
	if (tilesort_spu.swap)
	{
		crPackAccumSWAP( op, value );
	}
	else
	{
		crPackAccum( op, value );
	}
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_Clear( GLbitfield mask )
{
	tilesortspuFlush( tilesort_spu.ctx );
	if (tilesort_spu.swap)
	{
		crPackClearSWAP( mask );
	}
	else
	{
		crPackClear( mask );
	}
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_Finish(void)
{
	int writeback = tilesort_spu.num_servers;
	tilesortspuFlush( tilesort_spu.ctx );
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
	tilesortspuBroadcastGeom();
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
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackFlushSWAP();
	}
	else
	{
		crPackFlush();
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierCreate(GLuint name, GLuint count)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackBarrierCreateSWAP(name,count);
	}
	else
	{
		crPackBarrierCreate(name,count);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierDestroy(GLuint name)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackBarrierDestroySWAP(name);
	}
	else
	{
		crPackBarrierDestroy(name);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_BarrierExec(GLuint name)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackBarrierExecSWAP(name);
	}
	else
	{
		crPackBarrierExec(name);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreCreate(GLuint name, GLuint count)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackSemaphoreCreateSWAP(name,count);
	}
	else
	{
		crPackSemaphoreCreate(name,count);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreDestroy(GLuint name)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackSemaphoreDestroySWAP(name);
	}
	else
	{
		crPackSemaphoreDestroy(name);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreP(GLuint name)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackSemaphorePSWAP(name);
	}
	else
	{
		crPackSemaphoreP(name);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_SemaphoreV(GLuint name)
{
	tilesortspuFlush(tilesort_spu.ctx);
	if (tilesort_spu.swap)
	{
		crPackSemaphoreVSWAP(name);
	}
	else
	{
		crPackSemaphoreV(name);
	}
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}

void TILESORTSPU_APIENTRY tilesortspu_CallList( GLuint list )
{
	tilesortspuFlush( tilesort_spu.ctx );
	if (tilesort_spu.swap)
	{
		crPackCallListSWAP( list );
	}
	else
	{
		crPackCallList( list );
	}
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_CallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	tilesortspuFlush( tilesort_spu.ctx );
	if (tilesort_spu.swap)
	{
		crPackListBaseSWAP( tilesort_spu.ctx->lists.base );
	}
	else
	{
		crPackListBase( tilesort_spu.ctx->lists.base );
	}
	if (tilesort_spu.swap)
	{
		crPackCallListsSWAP( n, type, lists );
	}
	else
	{
		crPackCallLists( n, type, lists );
	}
	tilesortspuBroadcastGeom();
}
