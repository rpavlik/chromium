#include "cr_unpack.h"
#include "cr_glwrapper.h"

void crUnpackPolygonStipple( void  )
{
	GLubyte *mask = DATA_POINTER( 0, GLubyte );

	cr_unpackDispatch.PolygonStipple( mask );
	INCR_DATA_PTR( 32*32/8 );
}
