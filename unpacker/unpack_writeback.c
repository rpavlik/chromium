#include "cr_unpack.h"
#include <stdio.h>

CRNetworkPointer *return_ptr = NULL, *writeback_ptr = NULL;

void crUnpackSetReturnPointer( CRNetworkPointer *ret )
{
	return_ptr = ret;
}

void crUnpackSetWritebackPointer( CRNetworkPointer *wri )
{
	writeback_ptr = wri;
}
