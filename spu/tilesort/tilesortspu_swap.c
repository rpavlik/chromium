#include "tilesortspu.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_SwapBuffers(void)
{
	int bytesleft;

	bytesleft = tilesort_spu.geometry_pack.data_end - tilesort_spu.geometry_pack.data_current;

	crDebug( "Tile/sort SwapBuffers unimplemented (%d bytes left)", bytesleft);
}
