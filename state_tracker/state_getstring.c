/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_error.h"
#include "cr_version.h"
#include "state.h"
#include "state/cr_statetypes.h"


const GLubyte * STATE_APIENTRY crStateGetString( GLenum name )
{
   /*	CRContext   *g = GetCurrentContext( ); */
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

	CRContext *g = GetCurrentContext();
	if (!g)
		return NULL;

	switch( name )
	{
		case GL_VENDOR:
			return vendor;
		case GL_RENDERER:
			return renderer;
		case GL_VERSION:
			return version;
		case GL_EXTENSIONS:
			return g->limits.extensions;
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,
									"calling glGetString() with invalid name" );
			return NULL;
	}
}
