/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef PRINTSPU_H
#define PRINTSPU_H

#include "spu_dispatch_table.h"
#include "cr_spu.h"

#if defined(WINDOWS)
#define PRINT_APIENTRY __stdcall
#else
#define PRINT_APIENTRY
#endif

#include <stdio.h>

typedef struct {
	int id;
	SPUDispatchTable passthrough;
	FILE *fp;
} PrintSpu;

extern PrintSpu print_spu;

extern void printspuGatherConfiguration( const SPU *child_spu );
extern const char *printspuEnumToStr( GLenum e );

extern void PRINT_APIENTRY printGetIntegerv( GLenum pname, GLint *params );
extern void PRINT_APIENTRY printGetFloatv( GLenum pname, GLfloat *params );
extern void PRINT_APIENTRY printGetDoublev( GLenum pname, GLdouble *params );
extern void PRINT_APIENTRY printGetBooleanv( GLenum pname, GLboolean *params );

extern void PRINT_APIENTRY printMaterialfv( GLenum face, GLenum mode, const GLfloat *params );
extern void PRINT_APIENTRY printMaterialiv( GLenum face, GLenum mode, const GLint *params );
extern void PRINT_APIENTRY printLightfv( GLenum light, GLenum pname, const GLfloat *params );
extern void PRINT_APIENTRY printLightiv( GLenum light, GLenum pname, const GLint *params );

extern void PRINT_APIENTRY printLoadMatrixf( const GLfloat *m );
extern void PRINT_APIENTRY printLoadMatrixd( const GLdouble *m );
extern void PRINT_APIENTRY printMultMatrixf( const GLfloat *m );
extern void PRINT_APIENTRY printMultMatrixd( const GLdouble *m );

extern void PRINT_APIENTRY printChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values);
extern void PRINT_APIENTRY printChromiumParameteriCR(GLenum target, GLint value);
extern void PRINT_APIENTRY printGenTextures( GLsizei n, GLuint * textures );

extern void PRINT_APIENTRY printTexEnvf( GLenum target, GLenum pname, GLfloat param );
extern void PRINT_APIENTRY printTexEnvfv( GLenum target, GLenum pname, const GLfloat * params );
extern void PRINT_APIENTRY printTexEnvi( GLenum target, GLenum pname, GLint param );
extern void PRINT_APIENTRY printTexEnviv( GLenum target, GLenum pname, const GLint * params );



#endif /* PRINTSPU_H */
