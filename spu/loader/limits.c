/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	

/*
All about propogation of OpenGL limits through Chromium:

OpenGL limits (such as maximum texture size, maximum number of lights, etc
and the list of supported extensions) must be correctly reported to Chromium
client programs.  It's especially important that Chromium clients get an
accurate result for glGetString(GL_EXTENSIONS).  Otherwise, we'll have all
kinds of problems.

As an example, suppose we're driving a tiled display with heterogeneous
graphics cards.  The supported extensions and texture size might be different
for each card.  In this case, we need to find the minimum of all card's
max texture size and find the intersection of each card's extension strings.

OpenGL limits must be queried in the render server(s) and propogated up
through all the SPUs to the client.  We also have to consider that
different SPUs may have different OpenGL limits.  For example, the state-
tracker only supports certain OpenGL extensions.  If the renderer supports
GL_ARB_texture_env_combine but the state-tracker SPU doesn't, then we
better not advertise GL_ARB_texture_env_combine to the client!

The code in this file takes care of all these issues.  Here's how it works:

1. OpenGL limits are encapsulated in the CRLimitsState structure which
is defined in include/state/cr_limits.h.  We use this structure to pass
around the OpenGL limits from place to place.

2. We use the mothership to maintain a simple database of OpenGL limits
for all SPUs.  The limits are stored as simple name-value dictionary
entries like {GL_MAX_TEXTURE_SIZE, 1024}.

3.  An SPU can upload its OpenGL limits to the mothership with the
crSPUReportGLLimits() function.

4. An SPU can query the OpenGL limits of any other (downstream) SPU with
the crSPUQueryGLLimits() function.

5. Most SPUs will use the crSPUPropogateGLLimits() function.  If the
calling SPU has a child (as in an SPU chain) then we'll query the child
for its SPU limits and then report these SPU limits as our own.  The
print SPU is an example.

If the calling SPU is at the end of an SPU chain then we'll query the
servers associated with this SPU for their OpenGL limits.  We'll
compute the minimums/intersections of these limits and report them to
the mothership as our own.  For example, the tilesort SPU will query
N servers for their OpenGL limits, compute the minimums/intersection,
and report the results to the mothership.

6. Since SPUs are initialized in bottom-to-top order, the OpenGL
limits are propogated all the way up to the client.



*/


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "state/cr_limits.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_spu.h"
#include "cr_string.h"
#include "cr_version.h"

/* This string is the list of OpenGL extensions which Chromium can understand.
 * In practice, this string will get intersected with what's reported by the
 * rendering SPUs to reflect what we can really offer to client apps.
 */
static char *crExtensionString =
  "GL_CHROMIUM "
#ifdef CR_ARB_imaging
	"GL_ARB_imaging "
#endif
#ifdef CR_ARB_multitexture
	"GL_ARB_multitexture "
#endif
#ifdef CR_ARB_texture_border_clamp
	"GL_ARB_texture_border_clamp "
#endif
#ifdef CR_ARB_texture_cube_map
	"GL_ARB_texture_cube_map "
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



/*
 * Initialize the CRLimitsState object to Chromium's defaults.
 * This function is used below and by the state tracker SPU, for example.
 */
void crSPUInitGLLimits( CRLimitsState *l )
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
	l->extensions = (GLubyte*)crStrdup( crExtensionString );
}


void crSPUCopyGLLimits( CRLimitsState *dest, const CRLimitsState *src )
{
   *dest = *src;
   dest->extensions = (GLubyte*)crStrdup((const char*)src->extensions);
}


/*
 * Query the mothership to learn the OpenGL limits for a particular SPU.
 * Input:  conn - the mothership connection
 *         spu_id - the id of the SPU to query
 * Output:  limits - the OpenGL limits
 */
void crSPUQueryGLLimits( CRConnection *conn, int spu_id, CRLimitsState *limits )
{

#define GET_INT_LIMIT(LIMIT, TOKEN, DEFAULT)					\
	{									\
		char response[1000];						\
		if (crMothershipGetNamedSPUParam( conn, spu_id, #TOKEN, response))	\
		{								\
			limits->LIMIT = crStrToInt(response);			\
			if (limits->LIMIT == 0)					\
				limits->LIMIT = DEFAULT;			\
		}								\
		else								\
		{								\
			limits->LIMIT = DEFAULT;				\
		}								\
	}

#define GET_INT_LIMIT2(LIMIT, TOKEN, DEFAULT0, DEFAULT1)			\
	{									\
		char response[1000];						\
		if (crMothershipGetNamedSPUParam( conn, spu_id, #TOKEN, response))	\
		{								\
			char *response2 = crStrchr(response, ' ');		\
			limits->LIMIT[0] = crStrToInt(response);		\
			if (limits->LIMIT[0] == 0)				\
				limits->LIMIT[0] = DEFAULT0;			\
			if (response2)						\
			{							\
				limits->LIMIT[1] = crStrToInt(response2+1);	\
				if (!limits->LIMIT[1])				\
					limits->LIMIT[1] = DEFAULT1;		\
			}							\
			else							\
			{							\
				limits->LIMIT[1] = DEFAULT1;			\
			}							\
		}								\
		else								\
		{								\
			limits->LIMIT[0] = DEFAULT0;				\
			limits->LIMIT[1] = DEFAULT1;				\
		}								\
	}

#define GET_FLOAT_LIMIT(LIMIT, TOKEN, DEFAULT)					\
	{									\
		char response[1000];						\
		if (crMothershipGetNamedSPUParam( conn, spu_id, #TOKEN, response))	\
		{								\
			limits->LIMIT = crStrToFloat(response);			\
			if (limits->LIMIT == 0.0)				\
				limits->LIMIT = DEFAULT;			\
		}								\
		else								\
		{								\
			limits->LIMIT = DEFAULT;				\
		}								\
	}

#define GET_FLOAT_LIMIT2(LIMIT, TOKEN, DEFAULT0, DEFAULT1)			\
	{									\
		char response[1000];						\
		if (crMothershipGetNamedSPUParam( conn, spu_id, #TOKEN, response))	\
		{								\
			char *response2 = crStrchr(response, ' ');		\
			limits->LIMIT[0] = crStrToFloat(response);		\
			if (limits->LIMIT[0] == 0.0)				\
				limits->LIMIT[0] = DEFAULT0;			\
			if (response2)						\
			{							\
				limits->LIMIT[1] = crStrToFloat(response2+1);	\
				if (!limits->LIMIT[1])				\
					limits->LIMIT[1] = DEFAULT1;		\
			}							\
			else							\
			{							\
				limits->LIMIT[1] = DEFAULT1;			\
			}							\
		}								\
		else								\
		{								\
			limits->LIMIT[0] = DEFAULT0;				\
			limits->LIMIT[1] = DEFAULT1;				\
		}								\
	}

#define GET_STRING_LIMIT(LIMIT, TOKEN, DEFAULT)					\
	{									\
		char response[10000];						\
		if (crMothershipGetNamedSPUParam( conn, spu_id, #TOKEN, response))	\
		{								\
			assert(crStrlen(response) < 10000); /* sanity check */	\
			if (response[0])					\
				limits->LIMIT = (GLubyte*)crStrdup(response);		\
			else							\
				limits->LIMIT = (GLubyte*)crStrdup(DEFAULT);		\
		}								\
		else								\
		{								\
			limits->LIMIT = (GLubyte*)crStrdup(DEFAULT);			\
		}								\
	}

	GET_INT_LIMIT(maxTextureUnits, GL_MAX_TEXTURE_UNITS_ARB, 1);
	GET_INT_LIMIT(maxTextureSize, GL_MAX_TEXTURE_SIZE, 256);
	GET_INT_LIMIT(max3DTextureSize, GL_MAX_3D_TEXTURE_SIZE, 256);
	GET_INT_LIMIT(maxCubeMapTextureSize, GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, 256);
	GET_FLOAT_LIMIT(maxTextureAnisotropy, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 0.0);
	GET_INT_LIMIT(maxGeneralCombiners, GL_MAX_GENERAL_COMBINERS_NV, 0);
	GET_INT_LIMIT(maxLights, GL_MAX_LIGHTS, 8);
	GET_INT_LIMIT(maxClipPlanes, GL_MAX_CLIP_PLANES, 8);
	GET_INT_LIMIT(maxProjectionStackDepth, GL_MAX_PROJECTION_STACK_DEPTH, 2);
	GET_INT_LIMIT(maxModelviewStackDepth, GL_MAX_MODELVIEW_STACK_DEPTH, 32);
	GET_INT_LIMIT(maxTextureStackDepth, GL_MAX_TEXTURE_STACK_DEPTH, 2);
	GET_INT_LIMIT(maxColorStackDepth, GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI, 2);
	GET_INT_LIMIT(maxAttribStackDepth, GL_MAX_ATTRIB_STACK_DEPTH, 16);
	GET_INT_LIMIT(maxClientAttribStackDepth, GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, 16);
	GET_INT_LIMIT(maxNameStackDepth, GL_MAX_NAME_STACK_DEPTH, 64);
	GET_INT_LIMIT(maxElementsIndices, GL_MAX_ELEMENTS_INDICES, 16384);
	GET_INT_LIMIT(maxElementsVertices, GL_MAX_ELEMENTS_VERTICES, 16384);
	GET_INT_LIMIT(maxEvalOrder, GL_MAX_EVAL_ORDER, 8);
	GET_INT_LIMIT(maxListNesting, GL_MAX_LIST_NESTING, 64);
	GET_INT_LIMIT(maxPixelMapTable, GL_MAX_PIXEL_MAP_TABLE, 256);
	GET_INT_LIMIT2(maxViewportDims, GL_MAX_VIEWPORT_DIMS, 1024, 1024);
	GET_INT_LIMIT(subpixelBits, GL_SUBPIXEL_BITS, 4);
	GET_FLOAT_LIMIT2(aliasedPointSizeRange, GL_ALIASED_POINT_SIZE_RANGE, 1.0, 1.0);
	GET_FLOAT_LIMIT2(smoothPointSizeRange, GL_SMOOTH_POINT_SIZE_RANGE, 1.0, 1.0);
	GET_FLOAT_LIMIT(pointSizeGranularity, GL_SMOOTH_POINT_SIZE_GRANULARITY, 0.5);
	GET_FLOAT_LIMIT2(aliasedLineWidthRange, GL_ALIASED_LINE_WIDTH_RANGE, 1.0, 1.0);
	GET_FLOAT_LIMIT2(smoothLineWidthRange, GL_SMOOTH_LINE_WIDTH_RANGE, 1.0, 1.0);
	GET_FLOAT_LIMIT(lineWidthGranularity, GL_SMOOTH_LINE_WIDTH_GRANULARITY, 0.5);
	GET_STRING_LIMIT(extensions, GL_EXTENSIONS, "***error***");

#undef GET_INT_LIMIT
#undef GET_STRING_LIMIT
}


/*
 * This function reports OpenGL limits to the mothership.
 * Other SPU's may then query this info.  For example, the
 * tilesort SPU can query this info for each tile renderer in
 * order to compute the effective max texture size.
 */
void crSPUReportGLLimits( const CRLimitsState *limits, int spu_id )
{
	CRConnection *conn;

	conn = crMothershipConnect();
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}
	crMothershipIdentifySPU( conn, spu_id );


#define REPORT_INT(TOKEN, VALUE)				\
	{							\
		char str[100];					\
		sprintf(str, "%d", VALUE);			\
		crMothershipSetSPUParam(conn, #TOKEN, str);	\
	}

#define REPORT_INT2(TOKEN, VALUE0, VALUE1)			\
	{							\
		char str[100];					\
		sprintf(str, "%d %d", VALUE0, VALUE1);		\
		crMothershipSetSPUParam(conn, #TOKEN, str);	\
	}

#define REPORT_FLOAT(TOKEN, VALUE)				\
	{							\
		char str[100];					\
		sprintf(str, "%f", VALUE);			\
		crMothershipSetSPUParam(conn, #TOKEN, str);	\
	}

#define REPORT_FLOAT2(TOKEN, VALUE0, VALUE1)			\
	{							\
		char str[100];					\
		sprintf(str, "%f %f", VALUE0, VALUE1);		\
		crMothershipSetSPUParam(conn, #TOKEN, str);	\
	}

#define REPORT_STRING(TOKEN, VALUE)				\
	{							\
		crMothershipSetSPUParam(conn, #TOKEN, VALUE);	\
	}

	REPORT_INT(GL_MAX_TEXTURE_UNITS_ARB, limits->maxTextureUnits);
	REPORT_INT(GL_MAX_TEXTURE_SIZE, limits->maxTextureSize);
	REPORT_INT(GL_MAX_3D_TEXTURE_SIZE, limits->max3DTextureSize);
	REPORT_INT(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, limits->maxCubeMapTextureSize);

	if (crStrstr((const char*)limits->extensions, "GL_EXT_texture_filter_anisotropic"))
	{
		REPORT_FLOAT(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, limits->maxTextureAnisotropy);
	}
	if (crStrstr((const char*)limits->extensions, "GL_NV_register_combiners"))
	{
		REPORT_INT(GL_MAX_GENERAL_COMBINERS_NV, limits->maxGeneralCombiners);
	}

	REPORT_INT(GL_MAX_LIGHTS, limits->maxLights);
	REPORT_INT(GL_MAX_CLIP_PLANES, limits->maxClipPlanes);
	REPORT_INT(GL_MAX_PROJECTION_STACK_DEPTH, limits->maxProjectionStackDepth);
	REPORT_INT(GL_MAX_MODELVIEW_STACK_DEPTH, limits->maxModelviewStackDepth);
	REPORT_INT(GL_MAX_TEXTURE_STACK_DEPTH, limits->maxTextureStackDepth);
	REPORT_INT(GL_MAX_COLOR_MATRIX_STACK_DEPTH, limits->maxColorStackDepth);
	REPORT_INT(GL_MAX_ATTRIB_STACK_DEPTH, limits->maxAttribStackDepth);
	REPORT_INT(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, limits->maxClientAttribStackDepth);
	REPORT_INT(GL_MAX_NAME_STACK_DEPTH, limits->maxNameStackDepth);
	REPORT_INT(GL_MAX_ELEMENTS_INDICES, limits->maxElementsIndices);
	REPORT_INT(GL_MAX_ELEMENTS_VERTICES, limits->maxElementsVertices);
	REPORT_INT(GL_MAX_EVAL_ORDER, limits->maxEvalOrder);
	REPORT_INT(GL_MAX_LIST_NESTING, limits->maxListNesting);
	REPORT_INT(GL_MAX_PIXEL_MAP_TABLE, limits->maxPixelMapTable);
	REPORT_INT2(GL_MAX_VIEWPORT_DIMS, limits->maxViewportDims[0], limits->maxViewportDims[1]);
	REPORT_INT(GL_SUBPIXEL_BITS, limits->subpixelBits);
	REPORT_FLOAT2(GL_ALIASED_POINT_SIZE_RANGE, limits->aliasedPointSizeRange[0], limits->aliasedPointSizeRange[1]);
	REPORT_FLOAT2(GL_SMOOTH_POINT_SIZE_RANGE, limits->smoothPointSizeRange[0], limits->smoothPointSizeRange[1]);
	REPORT_FLOAT(GL_SMOOTH_POINT_SIZE_GRANULARITY, limits->pointSizeGranularity);
	REPORT_FLOAT2(GL_ALIASED_LINE_WIDTH_RANGE, limits->aliasedLineWidthRange[0], limits->aliasedLineWidthRange[1]);
	REPORT_FLOAT2(GL_SMOOTH_LINE_WIDTH_RANGE, limits->smoothLineWidthRange[0], limits->smoothLineWidthRange[1]);
	REPORT_FLOAT(GL_SMOOTH_LINE_WIDTH_GRANULARITY, limits->lineWidthGranularity);
	if (limits->extensions[0] == 0) {
		crWarning( "Chromium extension string is empty." );
	}
	else {
		REPORT_STRING(GL_EXTENSIONS, (const char*)limits->extensions);
	}

#undef REPORT_INT
#undef REPORT_INT2
#undef REPORT_FLOAT
#undef REPORT_FLOAT2
#undef REPORT_STRING

	crMothershipDisconnect( conn );
}


/*
 * This function uses glGetInteger and glGetString to query the renderer's
 * OpenGL limits (max texture size, etc) and stores the values in the
 * given CRLimitsState struct.
 */
void crSPUGetGLLimits( const SPUNamedFunctionTable *table, CRLimitsState *limits )
{
	typedef void (SPU_APIENTRY *GetIntegervFunc)(GLenum, GLint *);
	typedef void (SPU_APIENTRY *GetFloatvFunc)(GLenum, GLfloat *);
	typedef const GLubyte * (SPU_APIENTRY *GetStringFunc)(GLenum);
	typedef GLenum (SPU_APIENTRY *GetErrorFunc)(void);
	GetIntegervFunc getIntegerv;
	GetFloatvFunc getFloatv;
	GetStringFunc getString;
	GetErrorFunc getError;
	const char *str;

	/* For memory safety, zero out the limits structure */
	memset(limits, 0, sizeof(CRLimitsState));

	/* Get function pointers */
	getIntegerv = (GetIntegervFunc) crSPUFindFunction(table, "GetIntegerv");
	if (!getIntegerv)
	{
		return;
	}
	getFloatv = (GetFloatvFunc) crSPUFindFunction(table, "GetFloatv");
	if (!getFloatv)
	{
		return;
	}
	getString = (GetStringFunc) crSPUFindFunction(table, "GetString");
	if (!getString)
	{
		return;
	}
	getError = (GetErrorFunc) crSPUFindFunction(table, "GetError");
	if (!getError)
	{
		return;
	}

	/* Get extensions first */
	str = (const char *) (*getString)(GL_EXTENSIONS);
	limits->extensions = (GLubyte*)crStrdup(str);

	if (crStrstr((const char*)limits->extensions, "GL_ARB_multitexture"))
	{
		(*getIntegerv)(GL_MAX_TEXTURE_UNITS_ARB, (GLint*)&limits->maxTextureUnits);
	}
	else
	{
		limits->maxTextureUnits = 1;
	}

	(*getIntegerv)(GL_MAX_TEXTURE_SIZE, (GLint*)&limits->maxTextureSize);
	(*getIntegerv)(GL_MAX_3D_TEXTURE_SIZE, (GLint*)&limits->max3DTextureSize);

	if (crStrstr((const char*)limits->extensions, "GL_EXT_texture_cube_map") ||
		crStrstr((const char*)limits->extensions, "GL_ARB_texture_cube_map"))
	{
		(*getIntegerv)(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, (GLint*)&limits->maxCubeMapTextureSize);
	}
	else
	{
		limits->maxCubeMapTextureSize = 0; 
	}
	if (crStrstr((const char*)limits->extensions, "GL_EXT_texture_filter_anisotropic"))
	{
		(*getFloatv)(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &limits->maxTextureAnisotropy);
	}
	else
	{
		limits->maxTextureAnisotropy = 0.0;
	}
	if (crStrstr((const char*)limits->extensions, "GL_NV_register_combiners"))
	{
		(*getIntegerv)(GL_MAX_GENERAL_COMBINERS_NV, &limits->maxGeneralCombiners);
	}
	else
	{
		limits->maxGeneralCombiners = 0;
	}

	(*getIntegerv)(GL_MAX_LIGHTS, (GLint*)&limits->maxLights);
	(*getIntegerv)(GL_MAX_CLIP_PLANES, (GLint*)&limits->maxClipPlanes);
	(*getIntegerv)(GL_MAX_PROJECTION_STACK_DEPTH, (GLint*)&limits->maxProjectionStackDepth);
	(*getIntegerv)(GL_MAX_MODELVIEW_STACK_DEPTH, (GLint*)&limits->maxModelviewStackDepth);
	(*getIntegerv)(GL_MAX_TEXTURE_STACK_DEPTH, (GLint*)&limits->maxTextureStackDepth);
	(*getIntegerv)(GL_MAX_COLOR_MATRIX_STACK_DEPTH, (GLint*)&limits->maxColorStackDepth);
	(*getIntegerv)(GL_MAX_ATTRIB_STACK_DEPTH, (GLint*)&limits->maxAttribStackDepth);
	(*getIntegerv)(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, (GLint*)&limits->maxClientAttribStackDepth);
	(*getIntegerv)(GL_MAX_NAME_STACK_DEPTH, (GLint*)&limits->maxNameStackDepth);
#ifndef WINDOWS
	(*getIntegerv)(GL_MAX_ELEMENTS_INDICES, (GLint*)&limits->maxElementsIndices);
	(*getIntegerv)(GL_MAX_ELEMENTS_VERTICES, (GLint*)&limits->maxElementsVertices);
#endif
	(*getIntegerv)(GL_MAX_EVAL_ORDER, (GLint*)&limits->maxEvalOrder);
	(*getIntegerv)(GL_MAX_LIST_NESTING, (GLint*)&limits->maxListNesting);
	(*getIntegerv)(GL_MAX_PIXEL_MAP_TABLE, (GLint*)&limits->maxPixelMapTable);
	(*getIntegerv)(GL_MAX_VIEWPORT_DIMS, (GLint*)limits->maxViewportDims);
	(*getIntegerv)(GL_SUBPIXEL_BITS, (GLint*)&limits->subpixelBits);
#ifndef WINDOWS
	(*getFloatv)(GL_ALIASED_POINT_SIZE_RANGE, limits->aliasedPointSizeRange);
	(*getFloatv)(GL_SMOOTH_POINT_SIZE_RANGE, limits->smoothPointSizeRange);
	(*getFloatv)(GL_SMOOTH_POINT_SIZE_GRANULARITY, &limits->pointSizeGranularity);
	(*getFloatv)(GL_ALIASED_LINE_WIDTH_RANGE, limits->aliasedLineWidthRange);
	(*getFloatv)(GL_SMOOTH_LINE_WIDTH_RANGE, limits->smoothLineWidthRange);
	(*getFloatv)(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &limits->lineWidthGranularity);
#endif
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
 * Find the minimums of a set of CRLimitsState structures and return
 * the results in <merged>
 */
void crSPUMergeGLLimits( int n, const CRLimitsState *limits, CRLimitsState *merged )
{
	int i;

	assert(n > 0);

	*merged = limits[0];
	merged->extensions = (GLubyte*)crStrdup((const char*)limits[0].extensions);

#define CHECK_LIMIT(LIMIT)				\
	if (limits->LIMIT < merged->LIMIT)		\
	{ \
			merged->LIMIT = limits->LIMIT; \
	}

	for (i = 1; i < n; i++)
	{
		char *s;

		CHECK_LIMIT(maxTextureUnits);
		CHECK_LIMIT(maxTextureSize);
		CHECK_LIMIT(max3DTextureSize);
		CHECK_LIMIT(maxCubeMapTextureSize);
		CHECK_LIMIT(maxTextureAnisotropy);
		CHECK_LIMIT(maxLights);
		CHECK_LIMIT(maxClipPlanes);
		CHECK_LIMIT(maxProjectionStackDepth);
		CHECK_LIMIT(maxModelviewStackDepth);
		CHECK_LIMIT(maxTextureStackDepth);
		CHECK_LIMIT(maxColorStackDepth);
		CHECK_LIMIT(maxAttribStackDepth);
		CHECK_LIMIT(maxClientAttribStackDepth);
		CHECK_LIMIT(maxNameStackDepth);
		CHECK_LIMIT(maxElementsIndices);
		CHECK_LIMIT(maxElementsVertices);
		CHECK_LIMIT(maxEvalOrder);
		CHECK_LIMIT(maxListNesting);
		CHECK_LIMIT(maxPixelMapTable);
		CHECK_LIMIT(maxViewportDims[0]);
		CHECK_LIMIT(maxViewportDims[1]);
		CHECK_LIMIT(subpixelBits);
		CHECK_LIMIT(smoothPointSizeRange[0]);
		CHECK_LIMIT(smoothPointSizeRange[1]);
		CHECK_LIMIT(aliasedPointSizeRange[0]);
		CHECK_LIMIT(aliasedPointSizeRange[1]);
		CHECK_LIMIT(pointSizeGranularity);
		CHECK_LIMIT(smoothLineWidthRange[0]);
		CHECK_LIMIT(smoothLineWidthRange[1]);
		CHECK_LIMIT(aliasedLineWidthRange[0]);
		CHECK_LIMIT(aliasedLineWidthRange[1]);
		CHECK_LIMIT(lineWidthGranularity);
		CHECK_LIMIT(maxGeneralCombiners);

        /* find the intersection of limits[i].extensions and
         * merged.extensions */

        s = merge_ext_strings((const char*)merged->extensions,
                              (const char*)limits[i].extensions);

		if (merged->extensions)
		{
			crFree(merged->extensions);
		}
		merged->extensions = (GLubyte*)s;
	}


#undef CHECK_LIMIT
}


/*
 * This is a helper function that can be used by most non-render SPUs.
 * It queries the OpenGL limits of its children (servers), merges them,
 *  and uploads them to the mothership as its own.
 * Input:  conn - mothership connection
 *         spu_id - the ID of the calling SPU
 *         child_spu - pointer to the child SPU, or NULL if end of chain.
 * Output:  limitsResult - the results we report to the mothership
 */
void crSPUPropogateGLLimits( CRConnection *conn, int spu_id, const SPU *child_spu, CRLimitsState *limitsResult )
{
	CRLimitsState tmpLimits, *myLimits;

	if (limitsResult)
	{
		myLimits = limitsResult;
	}
	else
	{
		myLimits = &tmpLimits;
	}

	if (child_spu) {
		/* query motherhip for child's OpenGL limits */
		crSPUQueryGLLimits( conn, child_spu->id, myLimits );
		crSPUReportGLLimits( myLimits, spu_id );
	}
	else {
		/* no children; end of chain - query server for "children" */
		char response[8096];
		char **serverids;
		int i, num_servers;
		CRLimitsState *serverLimits;

		/* Get count and IDs of the servers (children).
		 * Response will be "<n> <id0> <id1> ..."
		 * For example: 3 8 9 10
		 */
		if (!crMothershipSendString( conn, response, "serverids" )) {
			/* something went wrong, report default Chromium limits */
			crSPUInitGLLimits( myLimits );
			crSPUReportGLLimits( myLimits, spu_id );
			return;
		}

		serverids = crStrSplit( response, " " );
		num_servers = crStrToInt( serverids[0] );

		if (num_servers < 1) {
			/* no children, report default Chromium GL limits */
			crSPUInitGLLimits( myLimits );
			crSPUReportGLLimits( myLimits, spu_id );
			return;
		}

		serverLimits = (CRLimitsState *) crAlloc( num_servers * sizeof(CRLimitsState) );

		for (i = 0 ; i < num_servers ; i++)
		{
			int server_id = crStrToInt( serverids[i+1] );
			crSPUQueryGLLimits( conn, server_id, &serverLimits[i] );
		}

		/* Merge the OpenGL limits of the servers to find common minimums */
		crSPUMergeGLLimits( num_servers, serverLimits, myLimits );
		/* crStateLimitsPrint( myLimits ); */
		crSPUReportGLLimits( myLimits, spu_id );

		crFree( serverLimits );
		crFreeStrings( serverids );
	}
}
