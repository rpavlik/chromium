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


/*
 * Define some OpenGL 1.2 tokens in case we're using an old gl.h header.
 */
#ifndef GL_SMOOTH_POINT_SIZE_RANGE
#define GL_SMOOTH_POINT_SIZE_RANGE		0x0B12
#endif

#ifndef GL_SMOOTH_POINT_SIZE_GRANULARITY
#define GL_SMOOTH_POINT_SIZE_GRANULARITY	0x0B13
#endif

#ifndef GL_SMOOTH_LINE_WIDTH_RANGE
#define GL_SMOOTH_LINE_WIDTH_RANGE		0x0B22
#endif

#ifndef GL_SMOOTH_LINE_WIDTH_GRANULARITY
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY	0x0B23
#endif

#ifndef GL_ALIASED_POINT_SIZE_RANGE
#define GL_ALIASED_POINT_SIZE_RANGE		0x846D
#endif

#ifndef GL_ALIASED_LINE_WIDTH_RANGE
#define GL_ALIASED_LINE_WIDTH_RANGE		0x846E
#endif

#ifndef GL_COLOR_MATRIX_STACK_DEPTH
#define GL_COLOR_MATRIX_STACK_DEPTH		0x80B2
#endif

#ifndef GL_COLOR_MATRIX
#define GL_COLOR_MATRIX				0x80B1
#endif

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D				0x806F
#endif

#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE			0x8073
#endif

#ifndef GL_PACK_SKIP_IMAGES
#define GL_PACK_SKIP_IMAGES			0x806B
#endif

#ifndef GL_PACK_IMAGE_HEIGHT
#define GL_PACK_IMAGE_HEIGHT			0x806C
#endif

#ifndef GL_UNPACK_SKIP_IMAGES
#define GL_UNPACK_SKIP_IMAGES			0x806D
#endif

#ifndef GL_UNPACK_IMAGE_HEIGHT
#define GL_UNPACK_IMAGE_HEIGHT			0x806E
#endif

#ifndef GL_PROXY_TEXTURE_3D
#define GL_PROXY_TEXTURE_3D			0x8070
#endif

#ifndef GL_TEXTURE_DEPTH
#define GL_TEXTURE_DEPTH			0x8071
#endif

#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R			0x8072
#endif

#ifndef GL_TEXTURE_BINDING_3D
#define GL_TEXTURE_BINDING_3D			0x806A
#endif

#ifndef GL_MAX_ELEMENTS_VERTICES
#define GL_MAX_ELEMENTS_VERTICES		0x80E8
#endif

#ifndef GL_MAX_ELEMENTS_INDICES
#define GL_MAX_ELEMENTS_INDICES			0x80E9
#endif


/*
 * Tokens for OpenGL 1.2's ARB_imaging subset
 */

#ifndef GL_BLEND_EQUATION
#define GL_BLEND_EQUATION			0x8009
#endif

#ifndef GL_MIN
#define GL_MIN					0x8007
#endif

#ifndef GL_MAX
#define GL_MAX					0x8008
#endif

#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD				0x8006
#endif

#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT			0x800A
#endif

#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT		0x800B
#endif

#ifndef GL_BLEND_COLOR
#define GL_BLEND_COLOR				0x8005
#endif



#endif /* CR_GLWRAPPER_H */
