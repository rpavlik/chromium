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

	/* XXX check if we're compiling a display list, if so, error! */

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
	/* XXX check if we're compiling a display list, if so, error! */
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
	/* XXX check if we're compiling a display list, if so, error! */
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
	/* XXX check if we're compiling a display list, if so, error! */
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
	/* XXX check if we're compiling a display list, if so, error! */
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
	/* XXX check if we're compiling a display list, if so, error! */
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
	/* XXX check if we're compiling a display list, if so, error! */
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


