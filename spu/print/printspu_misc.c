/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "printspu.h"

void PRINT_APIENTRY printspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	switch (target) {
	case GL_PRINT_STRING_CR:
		fprintf( print_spu.fp, "%s\n", (char *)values );
		fflush( print_spu.fp );
		break;
	}
}

void PRINT_APIENTRY printspuChromiumParameteriCR(GLenum target, GLint value)
{
	switch (target) {
	default:
		fprintf( print_spu.fp, "printspu ( %d, %d )\n", target, value );
		fflush( print_spu.fp );
		break;
	}
}
