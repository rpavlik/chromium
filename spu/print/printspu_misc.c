/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "printspu.h"

void PRINT_APIENTRY printChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	int i ;
	switch (target) {
	case GL_PRINT_STRING_CR:
		fprintf( print_spu.fp, "%s\n", (char *)values );
		fflush( print_spu.fp );
		break;
	default:
		fprintf( print_spu.fp, "ChromiumParametervCR( " ) ;
		fprintf( print_spu.fp, "%s, ", printspuEnumToStr( target ) ) ;
		fprintf( print_spu.fp, "%s, ", printspuEnumToStr( type ) ) ;
		fprintf( print_spu.fp, "%d, %p=[ ", (int) count, values ) ;
		switch( type ) {
			case GL_INT:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%d ", ((int*)values)[i] ) ;
				break ;
			case GL_UNSIGNED_INT:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%u ", ((unsigned int*)values)[i] ) ;
				break ;
			case GL_SHORT:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%hd ", ((short*)values)[i] ) ;
				break ;
			case GL_UNSIGNED_SHORT:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%hu ", ((unsigned short*)values)[i] ) ;
				break ;
			case GL_BYTE:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%d ", (int)((char*)values)[i] ) ;
				break ;
			case GL_UNSIGNED_BYTE:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%u ", (unsigned int)((unsigned char*)values)[i] ) ;
				break ;
			case GL_FLOAT:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%g ", (double)((float*)values)[i] ) ;
				break ;
			case GL_DOUBLE:
				for (i=0; i<count; i++)
					fprintf( print_spu.fp, "%g ", ((double*)values)[i] ) ;
				break ;
			default:
				fprintf( print_spu.fp, "unhandled fmt " ) ;
				break ;
		}
		fprintf( print_spu.fp, "] )\n" ) ;
		fflush( print_spu.fp ) ;
	}
	print_spu.passthrough.ChromiumParametervCR( target, type, count, values ) ;
}

void PRINT_APIENTRY printChromiumParameteriCR(GLenum target, GLint value)
{
	switch (target) {
	default:
		fprintf( print_spu.fp, "ChromiumParameteri( %s, ", printspuEnumToStr(target) ) ;
		fprintf( print_spu.fp, "%d )\n", (int) value );
		fflush( print_spu.fp );
		break;
	}
	print_spu.passthrough.ChromiumParameteriCR( target, value ) ;
}
