#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_SwapBuffers(void)
{
	int bytesleft;

	bytesleft = cr_packer_globals.buffer.data_end - cr_packer_globals.buffer.data_current;

	crDebug( "Tile/sort SwapBuffers unimplemented (%d bytes left)", bytesleft);
}
