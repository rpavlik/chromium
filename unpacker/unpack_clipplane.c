#include "cr_unpack.h"
#include "cr_glwrapper.h"

void crUnpackClipPlane( void  )
{
	GLenum plane = READ_DATA( 0, GLenum );
#ifdef WIREGL_UNALIGNED_ACCESS_OKAY
	GLdouble *equation = DATA_POINTER( 4, GLdouble ); 
#else
	GLdouble equation[4];
	memcpy( equation, DATA_POINTER( 4, GLdouble ), sizeof(equation) );
#endif

	cr_unpackDispatch.ClipPlane( plane, equation );
	INCR_DATA_PTR( sizeof( GLenum ) + 4*sizeof( GLdouble ));
}
