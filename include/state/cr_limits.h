/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_LIMITS_H
#define CR_LIMITS_H

#include "cr_glwrapper.h"

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
#define CR_MAX_PROJECTION_STACK_DEPTH	4
#define CR_MAX_MODELVIEW_STACK_DEPTH	32
#define CR_MAX_TEXTURE_STACK_DEPTH	4
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
#define CR_ALIASED_POINT_SIZE_MAX	1.0
#define CR_SMOOTH_POINT_SIZE_MIN	1.0
#define CR_SMOOTH_POINT_SIZE_MAX	1.0
#define CR_POINT_SIZE_GRANULARITY	0.5
#define CR_ALIASED_LINE_WIDTH_MIN	1.0
#define CR_ALIASED_LINE_WIDTH_MAX	1.0
#define CR_SMOOTH_LINE_WIDTH_MIN	1.0
#define CR_SMOOTH_LINE_WIDTH_MAX	1.0
#define CR_LINE_WIDTH_GRANULARITY	0.5


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
	GLuint maxViewportDims[2];
	GLuint subpixelBits;
	GLfloat aliasedPointSizeRange[2];
	GLfloat smoothPointSizeRange[2];
	GLfloat pointSizeGranularity;
	GLfloat aliasedLineWidthRange[2];
	GLfloat smoothLineWidthRange[2];
	GLfloat lineWidthGranularity;
	GLubyte *extensions;
} CRLimitsState;

void crStateLimitsInit(CRLimitsState *limits);

void crStateLimitsPrint(const CRLimitsState *limits);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FOG_H */
