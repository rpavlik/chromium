#include "cr_error.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#include <stdio.h>

const GLubyte * STATE_APIENTRY crStateGetString( GLenum name )
{
	static const GLubyte *vendor = (const GLubyte *) "Humper";
	static const GLubyte *renderer = (const GLubyte *) "Chromium";
	static const GLubyte *versions = (const GLubyte *) "0.0001";
	static const GLubyte *extensions = (const GLubyte *) "GL_EXT_texture_filter_anisotropic";
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
