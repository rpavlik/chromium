#include "cr_error.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#include <stdio.h>

const GLubyte * STATE_APIENTRY crStateGetString( GLenum name )
{
	static const GLubyte *vendor = (const GLubyte *) "Humper";
	static const GLubyte *renderer = (const GLubyte *) "Chromium";
	static const GLubyte *versions = (const GLubyte *) "1.1";
	static const GLubyte *extensions = (const GLubyte *) "GL_ARB_multitexture GL_EXT_blend_color GL_EXT_blend_minmax GL_EXT_blend_subtract GL_EXT_texture_edge_clamp GL_EXT_texture_filter_anisotropic GL_NV_fog_distance GL_NV_texgen_reflection";
	switch( name )
	{
		case GL_VENDOR:
			return vendor;
		case GL_RENDERER:
			return renderer;
		case GL_VERSION:
			return versions;
		case GL_EXTENSIONS:
			return extensions;
		default:
			crError( "Unknown parameter in crStateGetString: %d", name );
			return NULL;
	}
}
