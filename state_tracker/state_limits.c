/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "cr_string.h"
#include "cr_spu.h"


void crStateLimitsInit (CRLimitsState *l)
{
	/* Use the SPU utility function to do this */
	crSPUInitGLLimits( l );
}


/* This is a debug helper function. */
void crStateLimitsPrint (const CRLimitsState *l)
{
	fprintf(stderr, "----------- OpenGL limits ----------------\n");
	fprintf(stderr, "GL_MAX_TEXTURE_UNITS = %d\n", l->maxTextureUnits);
	fprintf(stderr, "GL_MAX_TEXTURE_SIZE = %d\n", l->maxTextureSize);
	fprintf(stderr, "GL_MAX_3D_TEXTURE_SIZE = %d\n", l->max3DTextureSize);
	fprintf(stderr, "GL_MAX_CUBE_MAP_TEXTURE_SIZE = %d\n", l->maxCubeMapTextureSize);
	fprintf(stderr, "GL_MAX_TEXTURE_ANISOTROPY = %f\n", l->maxTextureAnisotropy);
	fprintf(stderr, "GL_MAX_LIGHTS = %d\n", l->maxLights);
	fprintf(stderr, "GL_MAX_CLIP_PLANES = %d\n", l->maxClipPlanes);
	fprintf(stderr, "GL_MAX_ATTRIB_STACK_DEPTH = %d\n", l->maxClientAttribStackDepth);
	fprintf(stderr, "GL_MAX_PROJECTION_STACK_DEPTH = %d\n", l->maxProjectionStackDepth);
	fprintf(stderr, "GL_MAX_MODELVIEW_STACK_DEPTH = %d\n", l->maxModelviewStackDepth);
	fprintf(stderr, "GL_MAX_TEXTURE_STACK_DEPTH = %d\n", l->maxTextureStackDepth);
	fprintf(stderr, "GL_MAX_COLOR_STACK_DEPTH = %d\n", l->maxColorStackDepth);
	fprintf(stderr, "GL_MAX_ATTRIB_STACK_DEPTH = %d\n", l->maxAttribStackDepth);
	fprintf(stderr, "GL_MAX_ATTRIB_STACK_DEPTH = %d\n", l->maxClientAttribStackDepth);
	fprintf(stderr, "GL_MAX_NAME_STACK_DEPTH = %d\n", l->maxNameStackDepth);
	fprintf(stderr, "GL_MAX_ELEMENTS_INDICES = %d\n", l->maxElementsIndices);
	fprintf(stderr, "GL_MAX_ELEMENTS_VERTICES = %d\n", l->maxElementsVertices);
	fprintf(stderr, "GL_MAX_EVAL_ORDER = %d\n", l->maxEvalOrder);
	fprintf(stderr, "GL_MAX_LIST_NESTING = %d\n", l->maxListNesting);
	fprintf(stderr, "GL_MAX_PIXEL_MAP_TABLE = %d\n", l->maxPixelMapTable);
	fprintf(stderr, "GL_MAX_VIEWPORT_DIMS = %d %d\n",
		l->maxViewportDims[0], l->maxViewportDims[1]);
	fprintf(stderr, "GL_SUBPIXEL_BITS = %d\n", l->subpixelBits);
	fprintf(stderr, "GL_ALIASED_POINT_SIZE_RANGE = %f .. %f\n",
		l->aliasedPointSizeRange[0], l->aliasedPointSizeRange[1]);
	fprintf(stderr, "GL_SMOOTH_POINT_SIZE_RANGE = %f .. %f\n",
		l->aliasedPointSizeRange[0], l->aliasedPointSizeRange[1]);
	fprintf(stderr, "GL_POINT_SIZE_GRANULARITY = %f\n", l->pointSizeGranularity);
	fprintf(stderr, "GL_ALIASED_LINE_WIDTH_RANGE = %f .. %f\n",
		l->aliasedLineWidthRange[0], l->aliasedLineWidthRange[1]);
	fprintf(stderr, "GL_SMOOTH_LINE_WIDTH_RANGE = %f .. %f\n",
		l->smoothLineWidthRange[0], l->smoothLineWidthRange[1]);
	fprintf(stderr, "GL_LINE_WIDTH_GRANULARITY = %f\n", l->lineWidthGranularity);
	fprintf(stderr, "GL_MAX_GENERAL_COMBINERS_NV = %d\n", l->maxGeneralCombiners);
	fprintf(stderr, "GL_EXTENSIONS = %s\n", (const char *) l->extensions);
	fprintf(stderr, "------------------------------------------\n");
}
