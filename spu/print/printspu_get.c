#include "cr_glwrapper.h"
#include "printspu.h"

void PRINT_APIENTRY printGetIntegerv( GLenum pname, GLint *params )
{
	fprintf( print_spu.fp, "GetIntegerv( %s, 0x%p )\n", printspuEnumToStr( pname ), params );
	fflush( print_spu.fp );
	print_spu.passthrough.GetIntegerv( pname, params );
}

void PRINT_APIENTRY printGetFloatv( GLenum pname, GLfloat *params )
{
	fprintf( print_spu.fp, "GetFloatv( %s, 0x%p )\n", printspuEnumToStr( pname ), params );
	fflush( print_spu.fp );
	print_spu.passthrough.GetFloatv( pname, params );
}

void PRINT_APIENTRY printGetDoublev( GLenum pname, GLdouble *params )
{
	fprintf( print_spu.fp, "GetDoublev( %s, 0x%p )\n", printspuEnumToStr( pname ), params );
	fflush( print_spu.fp );
	print_spu.passthrough.GetDoublev( pname, params );
}

void PRINT_APIENTRY printGetBooleanv( GLenum pname, GLboolean *params )
{
	fprintf( print_spu.fp, "GetBooleanv( %s, 0x%p )\n", printspuEnumToStr( pname ), params );
	fflush( print_spu.fp );
	print_spu.passthrough.GetBooleanv( pname, params );
}
