#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_Accum( GLenum op, GLfloat value )
{
	tilesortspuFlush( tilesort_spu.ctx );
	crPackAccum( op, value );
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_Clear( GLbitfield mask )
{
	tilesortspuFlush( tilesort_spu.ctx );
	crPackClear( mask );
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_Finish(void)
{
	int writeback = tilesort_spu.num_servers;
	tilesortspuFlush( tilesort_spu.ctx );
	crPackFinish( );
	if (tilesort_spu.syncOnFinish)
	{
		crPackWriteback( &writeback );
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
	crPackFlush();
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}
