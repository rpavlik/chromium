#include "cr_unpack.h"
#include "cr_glwrapper.h"

void crUnpackCreateContext( void )
{
	cr_unpackDispatch.CreateContext( NULL, NULL );
	INCR_DATA_PTR_NO_ARGS( );
}
