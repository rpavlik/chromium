/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

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
		default:
			break;
	}
	return 0;
}

int crPackFogParamsLength( GLenum param )
{
	static int one_param = sizeof( GLfloat );
	switch( param )
	{
#ifdef GL_NV_fog_distance
		case GL_FOG_DISTANCE_MODE_NV:
			return one_param;
#endif
		default:
			break;
	}
	return 0;
}
