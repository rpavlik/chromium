/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
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

/* Parallel API Extensions */

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY *glBarrierCreateProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glBarrierDestroyProc) (GLuint name);
typedef void (APIENTRY *glBarrierExecProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreCreateProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glSemaphoreDestroyProc) (GLuint name);
typedef void (APIENTRY *glSemaphorePProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreVProc) (GLuint name);

typedef GLint (APIENTRY *crCreateContextProc)(void *display, GLint visBits);
typedef void (APIENTRY *crDestroyContextProc)(void *display, GLint context);
typedef void (APIENTRY *crMakeCurrentProc)(void *display, GLint drawable, GLint context);
typedef void (APIENTRY *crSwapBuffersProc)(void *display, GLint context);

typedef int (CR_APIENTRY *CR_PROC)();
CR_PROC APIENTRY crGetProcAddress( const char *name );

#ifdef __cplusplus
}
#endif

#endif /* CR_APPLICATIONS_H */
