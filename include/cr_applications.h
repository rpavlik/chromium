/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/*
 * This is the header file which Chromium-customized applications will include.
 * Parallel API Extensions.
 */

#ifndef CR_APPLICATIONS_H
#define CR_APPLICATIONS_H

#include "cr_glwrapper.h"

#ifdef WINDOWS
#define CR_APIENTRY __stdcall
#else
#define CR_APIENTRY
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
#define APIENTRY
#endif


#define CR_SUPPRESS_SWAP_BIT 0X1


/* 
 * For the performance SPU. Allows application to obtain it's own
 * statistics, and reset etc. through glChromiumParameter*CR().
 */
typedef struct {
	int 	count;

	int 	v2d, v2f, v2i, v2s;
	int 	v2dv, v2fv, v2iv, v2sv;
	int 	v3d, v3f, v3i, v3s;
	int 	v3dv, v3fv, v3iv, v3sv;
	int 	v4d, v4f, v4i, v4s;
	int 	v4dv, v4fv, v4iv, v4sv;

	int	ipoints;	/* Interpreted points */
	int	ilines; 	/* Interpreted lines */
	int	itris;		/* Interpreted tris */
	int	iquads; 	/* Interpreted quads */
	int	ipolygons; 	/* Interpreted polygons */
} PerfVertex;

typedef struct {
	PerfVertex points;
	PerfVertex lines;
	PerfVertex lineloop;
	PerfVertex linestrip;
	PerfVertex triangles;
	PerfVertex tristrip;
	PerfVertex trifan;
	PerfVertex quads;
	PerfVertex quadstrip;
	PerfVertex polygon;
} PerfPrim;


typedef void (APIENTRY *glBarrierCreateCRProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glBarrierDestroyCRProc) (GLuint name);
typedef void (APIENTRY *glBarrierExecCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreCreateCRProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glSemaphoreDestroyCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphorePCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreVCRProc) (GLuint name);

typedef void (APIENTRY *glChromiumParameteriCRProc) (GLenum target, GLint value);
typedef void (APIENTRY *glChromiumParameterfCRProc) (GLenum target, GLfloat value);
typedef void (APIENTRY *glChromiumParametervCRProc) (GLenum target, GLenum type, GLsizei count, const GLvoid *values);
typedef void (APIENTRY *glGetChromiumParametervCRProc) (GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values);

typedef GLint (APIENTRY *crCreateContextProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crDestroyContextProc)(GLint context);
typedef void (APIENTRY *crMakeCurrentProc)(GLint window, GLint context);
typedef void (APIENTRY *crSwapBuffersProc)(GLint window, GLint flags);

typedef GLint (APIENTRY *crCreateWindowProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crDestroyWindowProc)(GLint window);
typedef void (APIENTRY *crWindowSizeProc)(GLint window, GLint w, GLint h);
typedef void (APIENTRY *crWindowPositionProc)(GLint window, GLint x, GLint y);

typedef int (CR_APIENTRY *CR_PROC)();
CR_PROC APIENTRY crGetProcAddress( const char *name );

#ifdef __cplusplus
}
#endif

#endif /* CR_APPLICATIONS_H */
