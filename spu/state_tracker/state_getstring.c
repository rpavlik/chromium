/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_version.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#include <stdio.h>


const GLubyte * STATE_APIENTRY crStateGetString( GLenum name )
{
	static const GLubyte *vendor = (const GLubyte *) "Humper";
	static const GLubyte *renderer = (const GLubyte *) "Chromium";
#if defined(CR_OPENGL_VERSION_1_3)
	static const GLubyte *version = (const GLubyte *) "1.3";
#elif defined(CR_OPENGL_VERSION_1_2)
	static const GLubyte *version = (const GLubyte *) "1.2";
#elif defined(CR_OPENGL_VERSION_1_1)
	static const GLubyte *version = (const GLubyte *) "1.1";
#elif defined(CR_OPENGL_VERSION_1_0)
	static const GLubyte *version = (const GLubyte *) "1.0";
#else
#error "cr_version.h wasn't included!"
#endif

	static const GLubyte *extensions = (const GLubyte *)
#ifdef CR_ARB_imaging
	  "GL_ARB_imaging "
#endif
#ifdef CR_ARB_multitexture
	  "GL_ARB_multitexture "
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
#ifdef CR_NV_texgen_reflection
	  "GL_NV_texgen_reflection"
#endif
           ;

/*
GL_NV_texture_shader
GL_NV_texture_shader2
*/

	switch( name )
	{
		case GL_VENDOR:
			return vendor;
		case GL_RENDERER:
			return renderer;
		case GL_VERSION:
			return version;
		case GL_EXTENSIONS:
			return extensions;
		default:
			crError( "Unknown parameter in crStateGetString: %d", name );
			return NULL;
	}
}
