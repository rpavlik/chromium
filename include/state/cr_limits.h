/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_LIMITS_H
#define CR_LIMITS_H

#include "chromium.h"
#include "cr_version.h"

#ifdef __cplusplus
extern "C" {
#endif


#define CR_MAX_TEXTURE_UNITS		8
#define CR_MAX_GENERAL_COMBINERS	8
#define CR_MAX_TEXTURE_SIZE		2048
#define CR_MAX_3D_TEXTURE_SIZE		512
#define CR_MAX_CUBE_TEXTURE_SIZE	2048
#define CR_MAX_TEXTURE_ANISOTROPY	8.0
#define CR_MAX_LIGHTS			8
#define CR_MAX_CLIP_PLANES		8
#define CR_MAX_PROJECTION_STACK_DEPTH	32
#define CR_MAX_MODELVIEW_STACK_DEPTH	32
#define CR_MAX_TEXTURE_STACK_DEPTH	10
#define CR_MAX_COLOR_STACK_DEPTH	4
#define CR_MAX_ATTRIB_STACK_DEPTH	16
#define CR_MAX_NAME_STACK_DEPTH		64
#define CR_MAX_ELEMENTS_INDICES		16384
#define CR_MAX_ELEMENTS_VERTICES	16384
#define CR_MAX_EVAL_ORDER		8
#define CR_MAX_LIST_NESTING		64
#define CR_MAX_PIXEL_MAP_TABLE		256
#define CR_MAX_VIEWPORT_DIM		16384
#define CR_SUBPIXEL_BITS		4
#define CR_ALIASED_POINT_SIZE_MIN	1.0
#define CR_ALIASED_POINT_SIZE_MAX	64.0
#define CR_SMOOTH_POINT_SIZE_MIN	1.0
#define CR_SMOOTH_POINT_SIZE_MAX	64.0
#define CR_POINT_SIZE_GRANULARITY	0.5
#define CR_ALIASED_LINE_WIDTH_MIN	1.0
#define CR_ALIASED_LINE_WIDTH_MAX	64.0
#define CR_SMOOTH_LINE_WIDTH_MIN	1.0
#define CR_SMOOTH_LINE_WIDTH_MAX	64.0
#define CR_LINE_WIDTH_GRANULARITY	0.5
#define CR_MAX_VERTEX_ATTRIBS           16


/* glGetString strings */
#define CR_RENDERER "Chromium 1.0.1"
#define CR_VENDOR "Humper"
#if defined(CR_OPENGL_VERSION_1_4)
#define CR_VERSION "1.4"
#elif defined(CR_OPENGL_VERSION_1_3)
#define CR_VERSION "1.3"
#elif defined(CR_OPENGL_VERSION_1_2)
#define CR_VERSION "1.2"
#elif defined(CR_OPENGL_VERSION_1_1)
#define CR_VERSION "1.1"
#elif defined(CR_OPENGL_VERSION_1_0)
#define CR_VERSION "1.0"
#endif


/*
 * OpenGL's implementation-dependent values (not part of any attribute group).
 */
typedef struct {
	GLuint maxTextureUnits;
	GLuint maxTextureSize;
	GLuint max3DTextureSize;	/* OpenGL 1.2 */
	GLuint maxCubeMapTextureSize;	/* GL_ARB_texture_cube_map */
	GLfloat maxTextureAnisotropy;	/* GL_EXT_texture_filter_anisotropic */
	GLuint maxGeneralCombiners;		/* GL_NV_register_combiners */
	GLuint maxLights;
	GLuint maxClipPlanes;
	GLuint maxProjectionStackDepth;
	GLuint maxModelviewStackDepth;
	GLuint maxTextureStackDepth;
	GLuint maxColorStackDepth;	/* OpenGL 1.2 */
	GLuint maxAttribStackDepth;
	GLuint maxClientAttribStackDepth;
	GLuint maxNameStackDepth;
	GLuint maxElementsIndices;
	GLuint maxElementsVertices;
	GLuint maxEvalOrder;
	GLuint maxListNesting;
	GLuint maxPixelMapTable;
	GLint maxViewportDims[2];
	GLuint subpixelBits;
	GLfloat aliasedPointSizeRange[2];
	GLfloat smoothPointSizeRange[2];
	GLfloat pointSizeGranularity;
	GLfloat aliasedLineWidthRange[2];
	GLfloat smoothLineWidthRange[2];
	GLfloat lineWidthGranularity;
	GLubyte *extensions;

	/* Framebuffer/visual attributes */
	GLuint redBits, greenBits, blueBits, alphaBits;
	GLuint depthBits, stencilBits, indexBits;
	GLuint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
	GLuint auxBuffers;
	GLboolean rgbaMode;
	GLboolean doubleBuffer;
	GLboolean stereo;

} CRLimitsState;

extern void crStateLimitsInit(CRLimitsState *limits);

extern void crStateLimitsPrint(const CRLimitsState *limits);

extern GLubyte * crStateMergeExtensions(GLuint n, const GLubyte **extensions);


#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FOG_H */
