/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_TYPES_H
#define CR_STATE_TYPES_H

#include "cr_glwrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef GLfloat GLdefault;
#define GL_DEFAULTTYPE_FLOAT

#define GL_MAXBYTE		(GLbyte)	0x7F
#define GL_MAXUBYTE		(GLubyte)	0xFF
#define GL_MAXSHORT		(GLshort)	0x7FFF
#define GL_MAXUSHORT	(GLushort)	0xFFFF
#define GL_MAXINT		(GLint)		0x7FFFFFFF
#define GL_MAXUINT		(GLuint)	0xFFFFFFFF
#define GL_MAXFLOAT		1.0f
#define GL_MAXDOUBLE	1.0

#define GLBITS_LENGTH 32
#define GLBITS_ONES 0xFFFFFFFF
typedef unsigned long GLbitvalue;

typedef struct {
	GLfloat x1, x2, y1, y2;
} GLrectf;

typedef struct {
	GLint x1, x2, y1, y2;
} GLrecti;

typedef struct {
	GLdefault m00, m01, m02, m03;
	GLdefault m10, m11, m12, m13;
	GLdefault m20, m21, m22, m23;
	GLdefault m30, m31, m32, m33;
} GLmatrix;

typedef struct {
	GLfloat m00, m01, m02, m03;
	GLfloat m10, m11, m12, m13;
	GLfloat m20, m21, m22, m23;
	GLfloat m30, m31, m32, m33;
} GLmatrixf;

typedef struct {
	GLdouble m00, m01, m02, m03;
	GLdouble m10, m11, m12, m13;
	GLdouble m20, m21, m22, m23;
	GLdouble m30, m31, m32, m33;
} GLmatrixd;

#define VECTOR(type, name) typedef struct { type x,y,z,w; } name
#define COLOR(type, name) typedef struct { type r,g,b,a; } name
#define TEXCOORD(type, name) typedef struct { type s,t,r,q; } name

VECTOR(GLdefault,GLvector);
VECTOR(GLenum,GLvectore);
VECTOR(GLubyte,GLvectorub);
VECTOR(GLbyte,GLvectorb);
VECTOR(GLushort,GLvectorus);
VECTOR(GLshort,GLvectors);
VECTOR(GLint,GLvectori);
VECTOR(GLuint,GLvectorui);
VECTOR(GLfloat,GLvectorf);
VECTOR(GLdouble,GLvectord);
COLOR(GLdefault,GLcolor);
COLOR(GLenum,GLcolore);
COLOR(GLubyte,GLcolorub);
COLOR(GLbyte,GLcolorb);
COLOR(GLushort,GLcolorus);
COLOR(GLshort,GLcolors);
COLOR(GLint,GLcolori);
COLOR(GLuint,GLcolorui);
COLOR(GLfloat,GLcolorf);
COLOR(GLdouble,GLcolord);
TEXCOORD(GLdefault,GLtexcoord);
TEXCOORD(GLenum,GLtexcoorde);
TEXCOORD(GLubyte,GLtexcoordub);
TEXCOORD(GLbyte,GLtexcoordb);
TEXCOORD(GLushort,GLtexcoordus);
TEXCOORD(GLshort,GLtexcoords);
TEXCOORD(GLint,GLtexcoordi);
TEXCOORD(GLuint,GLtexcoordui);
TEXCOORD(GLfloat,GLtexcoordf);
TEXCOORD(GLdouble,GLtexcoordd);

#undef VECTOR
#undef COLOR
#undef TEXCOORD

#define COMPARE_VECTOR(a,b)		((a).x != (b).x || (a).y != (b).y || (a).z != (b).z || (a).w != (b).w)
#define COMPARE_TEXCOORD(a,b)	((a).s != (b).s || (a).t != (b).t || (a).r != (b).r || (a).q != (b).q)
#define COMPARE_COLOR(x,y)		((x).r != (y).r || (x).g != (y).g || (x).b != (y).b || (x).a != (y).a)


#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TYPES_H */
