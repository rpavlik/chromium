#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"

#include <GL/glext.h>

void crStateTextureInitExtensions( CRTextureState *t )
{
#ifdef GL_EXT_texture_filter_anisotropic
	t->extensions.maxTextureMaxAnisotropy = 8.0f;
#endif
}

void crStateTextureInitTextureObjExtensions( CRTextureState *t, CRTextureObj *tobj )
{
	(void) t;
#ifdef GL_EXT_texture_filter_anisotropic
	tobj->extensions.maxAnisotropy = 1.0f;
#endif
}

int crStateTexParameterfvExtensions( CRTextureState *t, CRTextureObj *tobj, GLenum pname, const GLfloat *param )
{
	switch( pname)
	{
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			if (param[0] < 1.0f)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
						"TexParameterfv: GL_TEXTURE_MAX_ANISOTROPY_EXT called with parameter less than 1: %f", param[0]);
				return 0;
			}
			tobj->extensions.maxAnisotropy = param[0];
			if (tobj->extensions.maxAnisotropy > t->extensions.maxTextureMaxAnisotropy)
			{
				tobj->extensions.maxAnisotropy = t->extensions.maxTextureMaxAnisotropy;
			}
			return 1;
#endif
	}
	return 0;
}

int crStateTexParameterivExtensions( GLenum target, GLenum pname, const GLint *param )
{
	GLfloat f_param = 0;
	switch( pname )
	{
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
			f_param = (GLfloat) (*param);
			crStateTexParameterfv( target, pname, &(f_param) );
			return 1;
	}
	return 0;
}

int crStateGetTexParameterfvExtensions( CRTextureObj *tobj, GLenum pname, GLfloat *params )
{
	switch( pname )
	{
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			*params = (GLfloat) tobj->extensions.maxAnisotropy;
			return 1;
#endif
	}
	return 0;
}

int crStateGetTexParameterivExtensions( CRTextureObj *tobj, GLenum pname, GLint *params )
{
	switch( pname )
	{
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			*params = (GLint) tobj->extensions.maxAnisotropy;
			return 1;
#endif
	}
	return 0;
}

void crStateTextureDiffParameterExtensions( CRTextureObj *tobj )
{
#ifdef GL_EXT_texture_filter_anisotropic
	diff_api.TexParameterf(tobj->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tobj->extensions.maxAnisotropy);
#endif
}
