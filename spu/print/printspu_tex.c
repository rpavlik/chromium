#include <stdio.h>
#include "cr_error.h"
#include "cr_spu.h"
#include "printspu.h"

void PRINT_APIENTRY printTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	switch(pname) {
	    case GL_TEXTURE_ENV_MODE:
		fprintf( print_spu.fp, "TexEnvf( %s, GL_TEXTURE_ENV_MODE, %s )\n", printspuEnumToStr( target ), printspuEnumToStr( param ));
		break;
	    default:
		fprintf( print_spu.fp, "TexEnvf( %s, %s, %f )\n", printspuEnumToStr( target ), printspuEnumToStr( pname ), param);
		break;
	}
	fflush( print_spu.fp );
	print_spu.passthrough.TexEnvf( target, pname, param );
}

void PRINT_APIENTRY printTexEnvfv( GLenum target, GLenum pname, const GLfloat * params )
{
	switch(pname) {
	    case GL_TEXTURE_ENV_MODE:
		fprintf( print_spu.fp, "TexEnvfv( %s, GL_TEXTURE_ENV_MODE, [%s] )\n", printspuEnumToStr( target ), printspuEnumToStr( params[0] ));
		break;
	    case GL_TEXTURE_ENV_COLOR:
		fprintf( print_spu.fp, "TexEnvfv( %s, GL_TEXTURE_ENV_COLOR, [%f, %f, %f, %f] )\n", printspuEnumToStr( target ), params[0], params[1], params[2], params[3] );
		break;
	    default:
		fprintf( print_spu.fp, "TexEnvfv( %s, %s, %p )\n", printspuEnumToStr( target ), printspuEnumToStr( pname ), (void *)params );
		break;
	}
	fflush( print_spu.fp );
	print_spu.passthrough.TexEnvfv( target, pname, params );
}

void PRINT_APIENTRY printTexEnvi( GLenum target, GLenum pname, GLint param )
{
	fprintf( print_spu.fp, "TexEnvi( %s, %s, %s )\n", printspuEnumToStr( target ), printspuEnumToStr( pname ), printspuEnumToStr(param));
	fflush( print_spu.fp );
	print_spu.passthrough.TexEnvi( target, pname, param );
}

void PRINT_APIENTRY printTexEnviv( GLenum target, GLenum pname, const GLint * params )
{
	switch(pname) {
	    case GL_TEXTURE_ENV_MODE:
		fprintf( print_spu.fp, "TexEnviv( %s, GL_TEXTURE_ENV_MODE, [%s] )\n", printspuEnumToStr( target ), printspuEnumToStr(params[0]) );
		break;
	    case GL_TEXTURE_ENV_COLOR:
		fprintf( print_spu.fp, "TexEnviv( %s, GL_TEXTURE_ENV_COLOR, [%d, %d, %d, %d] )\n", printspuEnumToStr( target ), params[0], params[1], params[2], params[3] );
		break;
	    default:
		fprintf( print_spu.fp, "TexEnviv( %s, %s, %p )\n", printspuEnumToStr( target ), printspuEnumToStr( pname ), (void *)params );
		break;
	}
	fflush( print_spu.fp );
	print_spu.passthrough.TexEnviv( target, pname, params );
}
