/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "cr_glstate.h"
#include "cr_string.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"


static GLboolean hasExtension(const char *haystack, const char *needle)
{
	const int needleLen = crStrlen(needle);
	const char *s;

	while (1) {
		s = crStrstr(haystack, needle);
		if (!s)
			return GL_FALSE;
		if (s && (s[needleLen] == ' ' || s[needleLen] == 0))
			return GL_TRUE;
		haystack += needleLen;
	}
}

/*
 * Examine the context's extension string and set the boolean extension
 * flags accordingly.  This is to be called during context initialization.
 */
void crStateExtensionsInit( CRContext *g )
{
	/*
	fprintf(stderr, "##### crStateExtensionsInit ext = %s#\n", g->limits.extensions);
	*/

	/* init all booleans to false */
	memset(&(g->extensions), 0, sizeof(g->extensions));

	if (hasExtension((const char*)g->limits.extensions, "GL_ARB_imaging"))
		g->extensions.ARB_imaging = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_ARB_texture_border_clamp") ||
		hasExtension((const char*)g->limits.extensions, "GL_SGIS_texture_border_clamp"))
		g->extensions.ARB_texture_border_clamp = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_ARB_multitexture"))
		g->extensions.ARB_multitexture = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_ARB_texture_cube_map") ||
		hasExtension((const char*)g->limits.extensions, "GL_EXT_texture_cube_map"))
		g->extensions.ARB_texture_cube_map = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_blend_color"))
		g->extensions.EXT_blend_color= GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_blend_minmax"))
		g->extensions.EXT_blend_minmax = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_blend_subtract"))
		g->extensions.EXT_blend_subtract = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_secondary_color"))
		g->extensions.EXT_secondary_color = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_separate_specular_color"))
		g->extensions.EXT_separate_specular_color = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_texture_edge_clamp") ||
		hasExtension((const char*)g->limits.extensions, "GL_SGIS_texture_edge_clamp"))
		g->extensions.EXT_texture_edge_clamp = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_EXT_texture_filter_anisotropic"))
		g->extensions.EXT_texture_filter_anisotropic = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_NV_fog_distance"))
		g->extensions.NV_fog_distance = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_NV_texgen_reflection"))
		g->extensions.NV_texgen_reflection = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_NV_register_combiners2"))
		g->extensions.NV_register_combiners2 = GL_TRUE;

	if (hasExtension((const char*)g->limits.extensions, "GL_NV_texgen_reflection"))
		g->extensions.NV_texgen_reflection = GL_TRUE;
}
