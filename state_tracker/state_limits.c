/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_spu.h"


/* This string is the list of OpenGL extensions which Chromium can understand.
 * In practice, this string will get intersected with what's reported by the
 * rendering SPUs to reflect what we can really offer to client apps.
 */
char *__stateExtensionString =
  "GL_CHROMIUM "
#ifdef CR_ARB_imaging
	"GL_ARB_imaging "
#endif
#ifdef CR_ARB_multitexture
	"GL_ARB_multitexture "
#endif
#ifdef CR_ARB_point_parameters
	"GL_ARB_point_parameters "
#endif
#ifdef CR_ARB_texture_border_clamp
	"GL_ARB_texture_border_clamp "
#endif
#ifdef CR_ARB_texture_cube_map
	"GL_ARB_texture_cube_map "
#endif
#ifdef CR_ARB_transpose_matrix
	"GL_ARB_transpose_matrix "
#endif
#ifdef CR_EXT_blend_color
	"GL_EXT_blend_color "
#endif
#ifdef CR_EXT_blend_minmax
	"GL_EXT_blend_minmax "
#endif
#ifdef CR_EXT_blend_subtract
	"GL_EXT_blend_subtract "
#endif
#ifdef CR_EXT_fog_coord
	"GL_EXT_fog_coord "
#endif
#ifdef CR_EXT_secondary_color
	"GL_EXT_secondary_color "
#endif
#ifdef CR_EXT_separate_specular_color
	"GL_EXT_separate_specular_color "
#endif
#ifdef CR_EXT_texture_cube_map
	"GL_EXT_texture_cube_map "
#endif
#ifdef CR_EXT_texture_edge_clamp
	"GL_EXT_texture_edge_clamp "
#endif
#ifdef CR_EXT_texture_filter_anisotropic
	"GL_EXT_texture_filter_anisotropic "
#endif
#ifdef CR_EXT_texture_object
	"GL_EXT_texture_object "
#endif
#ifdef CR_EXT_texture3D
	"GL_EXT_texture3D "
#endif
#ifdef CR_NV_fog_distance
	"GL_NV_fog_distance "
#endif
#ifdef CR_NV_register_combiners
	"GL_NV_register_combiners "
#endif
#ifdef CR_NV_register_combiners2
	"GL_NV_register_combiners2 "
#endif
#ifdef CR_NV_texgen_reflection
	"GL_NV_texgen_reflection "
#endif
#ifdef CR_SGIS_texture_border_clamp
	"GL_SGIS_texture_border_clamp "
#endif
#ifdef CR_SGIS_texture_edge_clamp
	"GL_SGIS_texture_edge_clamp"
#endif
	"";

static char *chromiumExtensions =
	" "
#ifdef GL_CR_state_parameter
	"GL_CR_state_parameter "
#endif
#ifdef GL_CR_cursor_position
	"GL_CR_cursor_position "
#endif
#ifdef GL_CR_bounding_box
	"GL_CR_bounding_box "
#endif
#ifdef GL_CR_print_string
	"GL_CR_print_string "
#endif
#ifdef GL_CR_tilesort_info
	"GL_CR_tilesort_info "
#endif
#ifdef GL_CR_client_clear_control
	"GL_CR_client_clear_control "
#endif
	;


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


/*
 * Initialize the CRLimitsState object to Chromium's defaults.
 */
void crStateLimitsInit (CRLimitsState *l)
{
	l->maxTextureUnits = CR_MAX_TEXTURE_UNITS;
	l->maxTextureSize = CR_MAX_TEXTURE_SIZE;
	l->max3DTextureSize = CR_MAX_3D_TEXTURE_SIZE;
	l->maxCubeMapTextureSize = CR_MAX_CUBE_TEXTURE_SIZE;
	l->maxTextureAnisotropy = CR_MAX_TEXTURE_ANISOTROPY;
	l->maxGeneralCombiners = CR_MAX_GENERAL_COMBINERS;
	l->maxLights = CR_MAX_LIGHTS;
	l->maxClipPlanes = CR_MAX_CLIP_PLANES;
	l->maxClientAttribStackDepth = CR_MAX_ATTRIB_STACK_DEPTH;
	l->maxProjectionStackDepth = CR_MAX_PROJECTION_STACK_DEPTH;
	l->maxModelviewStackDepth = CR_MAX_MODELVIEW_STACK_DEPTH;
	l->maxTextureStackDepth = CR_MAX_TEXTURE_STACK_DEPTH;
	l->maxColorStackDepth = CR_MAX_COLOR_STACK_DEPTH;
	l->maxAttribStackDepth = CR_MAX_ATTRIB_STACK_DEPTH;
	l->maxClientAttribStackDepth = CR_MAX_ATTRIB_STACK_DEPTH;
	l->maxNameStackDepth = CR_MAX_NAME_STACK_DEPTH;
	l->maxElementsIndices = CR_MAX_ELEMENTS_INDICES;
	l->maxElementsVertices = CR_MAX_ELEMENTS_VERTICES;
	l->maxEvalOrder = CR_MAX_EVAL_ORDER;
	l->maxListNesting = CR_MAX_LIST_NESTING;
	l->maxPixelMapTable = CR_MAX_PIXEL_MAP_TABLE;
	l->maxViewportDims[0] = l->maxViewportDims[1] = CR_MAX_VIEWPORT_DIM;
	l->subpixelBits = CR_SUBPIXEL_BITS;
	l->aliasedPointSizeRange[0] = CR_ALIASED_POINT_SIZE_MIN;
	l->aliasedPointSizeRange[1] = CR_ALIASED_POINT_SIZE_MAX;
	l->smoothPointSizeRange[0] = CR_SMOOTH_POINT_SIZE_MIN;
	l->smoothPointSizeRange[1] = CR_SMOOTH_POINT_SIZE_MAX;
	l->pointSizeGranularity = CR_POINT_SIZE_GRANULARITY;
	l->aliasedLineWidthRange[0] = CR_ALIASED_LINE_WIDTH_MIN;
	l->aliasedLineWidthRange[1] = CR_ALIASED_LINE_WIDTH_MAX;
	l->smoothLineWidthRange[0] = CR_SMOOTH_LINE_WIDTH_MIN;
	l->smoothLineWidthRange[1] = CR_SMOOTH_LINE_WIDTH_MAX;
	l->lineWidthGranularity = CR_LINE_WIDTH_GRANULARITY;
	l->extensions = (GLubyte*)crStrdup( __stateExtensionString );

	/* init frame buffer limits to typical maximums */
	l->redBits = 8;
	l->greenBits = 8;
	l->blueBits = 8;
	l->alphaBits = 8;
	l->depthBits = 24;
	l->stencilBits = 8;
	l->accumRedBits = 16;
	l->accumGreenBits = 16;
	l->accumBlueBits = 16;
	l->accumAlphaBits = 16;

	/* XXX these values are hard-coded for now */
	l->auxBuffers = 0;
	l->rgbaMode = GL_TRUE;
	l->doubleBuffer = GL_TRUE;
	l->stereo = GL_FALSE;
}



static char *merge_ext_strings(const char *s1, const char *s2)
{
	const int len1 = crStrlen(s1);
	const int len2 = crStrlen(s2);
	char *result;
	char **exten1, **exten2;
	int i, j;

	/* allocate storage for result */
	result = (char*)crAlloc(((len1 > len2) ? len1 : len2) + 2);
	if (!result)
	{
		return NULL;
	}
	result[0] = 0;

	/* split s1 and s2 at space chars */
	exten1 = crStrSplit(s1, " ");
	exten2 = crStrSplit(s2, " ");

	for (i = 0; exten1[i]; i++)
	{
		for (j = 0; exten2[j]; j++)
		{
			if (crStrcmp(exten1[i], exten2[j]) == 0)
			{
				/* found an intersection, append to result */
				crStrcat(result, exten1[i]);
				crStrcat(result, " ");
				break;
			}
		}
	}

	/* free split strings */
	crFreeStrings( exten1 );
	crFreeStrings( exten2 );

	/* all done! */
	return result;
}



/*
 * <extenions> is an array [n] of GLubyte pointers which contain lists of
 * OpenGL extensions.
 * Compute the intersection of those strings, then append the Chromium
 * extension strings.
 */
GLubyte * crStateMergeExtensions(GLuint n, const GLubyte **extensions)
{
	char *merged, *result;
	GLuint i;

	/* find intersection of all extension strings */
	merged = crStrdup(__stateExtensionString);
	for (i = 0; i < n; i++)
	{
		char *m = merge_ext_strings(merged, extensions[i]);
		if (merged)
			crFree(merged);
		merged = m;
	}

	/* append Cr extensions */
	result = crStrjoin(merged, chromiumExtensions);
	crFree(merged);
	return (GLubyte *) result;
}
