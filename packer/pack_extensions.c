#include "packer.h"

#include <GL/glext.h>

int crPackTexParameterParamsLength( GLenum param )
{
	static int one_param = sizeof( GLfloat );
	switch( param )
	{
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			return one_param;
#endif
	}
	return 0;
}
