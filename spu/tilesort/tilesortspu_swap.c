#include "cr_packfunctions.h"
#include "cr_error.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_SwapBuffers(void)
{
	//crDebug( "SWAP!!!\n");
	tilesortspuFlush( tilesort_spu.ctx );
	crPackSwapBuffers( );
	tilesortspuBroadcastGeom();
	tilesortspuShipBuffers();
}
