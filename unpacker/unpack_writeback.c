#include "unpacker.h"
#include "cr_error.h"
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

void crUnpackExtendWriteback(void)
{
	SET_WRITEBACK_PTR( 8 );
	cr_unpackDispatch.Writeback( NULL );
}

void crUnpackWriteback(void)
{
	crError( "crUnpackWriteback should never be called" );
}
