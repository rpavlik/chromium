#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_Accum( GLenum op, GLfloat value )
{
	crDebug( "Accum!\n");
	tilesortspuFlush( tilesort_spu.ctx );
	crPackAccum( op, value );
	tilesortspuBroadcastGeom();
}

void TILESORTSPU_APIENTRY tilesortspu_Clear( GLbitfield mask )
{
	//crDebug( "Clear!\n");
	tilesortspuFlush( tilesort_spu.ctx );
	crPackClear( mask );
	tilesortspuBroadcastGeom();
}
