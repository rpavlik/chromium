/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* Chromium sources include this file instead of including
 * the GL/gl.h and GL/glext.h headers directly.
 */

#ifndef CR_GLWRAPPER_H
#define CR_GLWRAPPER_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#ifndef WINDOWS

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CR_GLXFuncPtr)();
CR_GLXFuncPtr glXGetProcAddressARB( const GLubyte *name );

#ifdef __cplusplus
}
#endif

#endif


#endif /* CR_GLWRAPPER_H */
