/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"

#include <GL/glext.h>

GLboolean crEnableSetExtensions( CRContext *g, CRStateBits *sb, GLbitvalue neg_bitid, GLenum cap, GLboolean val )
{
	switch( cap )
	{
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			g->texture.unit[g->texture.curTextureUnit].enabledCubeMap = val;
			sb->texture.enable[g->texture.curTextureUnit] = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
#ifdef GL_NV_register_combiners
		case GL_REGISTER_COMBINERS_NV:
			g->texture.extensions.regCombiners = val;
			sb->texture.extensions.regCombiners = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#ifdef GL_NV_register_combiners2
		case GL_PER_STAGE_CONSTANTS_NV:
			g->texture.extensions.regPerStageConstants = val;
			sb->texture.extensions.regPerStageConstants = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#endif /* 0 */
		default:
			return 1;
	}
	return 0;
}

void crStateTextureInitExtensions( CRTextureState *t )
{
#ifdef GL_EXT_texture_filter_anisotropic
	t->extensions.maxTextureMaxAnisotropy = 8.0f;
#endif
#if 0
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map)
	t->extensions.cubeMap = GL_FALSE;
#endif
#ifdef GL_NV_register_combiners
	t->extensions.regCombiners = GL_FALSE;
#endif
#ifdef GL_NV_register_combiners2
	t->extensions.regPerStageConstants = GL_FALSE;
#endif
#endif /* 0 */
}

void crStateTextureInitTextureObjExtensions( CRTextureState *t, CRTextureObj *tobj )
{
	(void) t;
#ifdef GL_EXT_texture_filter_anisotropic
	tobj->extensions.maxAnisotropy = 1.0f;
#endif
}

int crStateTexImage2DTargetExtensions( GLenum target )
{
	switch( target )
	{
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map)
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
			break;
#endif
		default:
			return 0;
	}
	return 1;
}

int crStateTexSubImage2DTargetExtensions( GLenum target )
{
	switch( target )
	{
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map)
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
			break;
#endif
		default:
			return 0;
	}
	return 1;
}

int crStateTexParameterfvExtensions( CRTextureState *t, CRTextureObj *tobj, GLenum pname, const GLfloat *param )
{
	switch( pname)
	{
#ifdef GL_EXT_texture_edge_clamp
		case GL_TEXTURE_WRAP_S:
		case GL_TEXTURE_WRAP_T:
		case GL_TEXTURE_WRAP_R:
			if (param[0] != GL_CLAMP_TO_EDGE_EXT)
			{
				return 0;
			}
			return 1;
#endif
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
		default:
			break;
	}
	return 0;
}

int crStateTexParameterivExtensions( GLenum target, GLenum pname, const GLint *param )
{
	GLfloat f_param = 0;
	switch( pname )
	{
#ifdef GL_EXT_texture_edge_clamp
		case GL_TEXTURE_WRAP_S:
		case GL_TEXTURE_WRAP_T:
		case GL_TEXTURE_WRAP_R:
#endif
#ifdef GL_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
			f_param = (GLfloat) (*param);
			crStateTexParameterfv( target, pname, &(f_param) );
			return 1;
		default:
			break;
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
		default:
			break;
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
		default:
			break;
	}
	return 0;
}

int crStateTexGenTextureGenModeExtensions( GLenum coord, GLenum param )
{
	switch( param )
	{
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map) || defined(GL_NV_texgen_reflection)
		case GL_REFLECTION_MAP_ARB:
		case GL_NORMAL_MAP_ARB:
			if( coord == GL_S || coord == GL_T || coord == GL_R )
			{
				return 0;
			}
			// specifically GL_Q is not able to be generated.
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"TexGen: ARB_texture_cube_map called with bogus coord: 0x%x", (GLenum) param);
			return 1;
#endif
		default:
			return 1;
	}
}

void crStateTextureDiffParameterExtensions( CRTextureObj *tobj )
{
	(void) tobj;
#ifdef GL_EXT_texture_filter_anisotropic
	diff_api.TexParameterf(tobj->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tobj->extensions.maxAnisotropy);
#endif
}

void crStateFogInitExtensions( CRFogState *f )
{
#ifdef GL_NV_fog_distance
	f->extensions.fogDistanceMode = GL_EYE_PLANE_ABSOLUTE_NV;
#endif
}

int crStateFogivExtensions( GLenum pname, const GLint *params )
{
	GLfloat f_param;
	switch( pname )
	{
#ifdef GL_NV_fog_distance
		case GL_FOG_DISTANCE_MODE_NV:
			f_param = (GLfloat) (*params);
			crStateFogfv( pname, &f_param );
			return 1;
#endif
		default:
			break;
	}
	return 0;
}

int crStateFogfvExtensions( CRFogState *f, GLenum pname, const GLfloat *params )
{
	switch( pname )
	{
#ifdef GL_NV_fog_distance
		case GL_FOG_DISTANCE_MODE_NV:
			if (params[0] != GL_EYE_RADIAL_NV && params[0] != GL_EYE_PLANE && params[0] != GL_EYE_PLANE_ABSOLUTE_NV )
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"Fogfv: GL_FOG_DISTANCE_MODE_NV called with illegal parameter: 0x%x", (GLenum) params[0]);
				return 0;
			}
			f->extensions.fogDistanceMode = (GLenum) params[0];
			return 1;
#endif
		default:
			break;
	}
	return 0;
}

void crStateFogDiffExtensions( CRFogState *from, CRFogState *to )
{
#ifdef GL_NV_fog_distance
	if (from->extensions.fogDistanceMode != to->extensions.fogDistanceMode)
	{
		diff_api.Fogi( GL_FOG_DISTANCE_MODE_NV, to->extensions.fogDistanceMode );
		from->extensions.fogDistanceMode = to->extensions.fogDistanceMode;
	}
#endif
}

void crStateFogSwitchExtensions( CRFogState *from, CRFogState *to )
{
#ifdef GL_NV_fog_distance
	if (from->extensions.fogDistanceMode != to->extensions.fogDistanceMode)
	{
		diff_api.Fogi( GL_FOG_DISTANCE_MODE_NV, to->extensions.fogDistanceMode );
	}
#endif
}

int crStateBlendFuncExtensionsCheckFactor( GLenum factor )
{
	switch( factor )
	{
#ifdef GL_EXT_blend_color
		case GL_CONSTANT_COLOR_EXT:
		case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		case GL_CONSTANT_ALPHA_EXT:
		case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
			return 1;
#endif
		default:
			return 0;
	}
}

void crStateBufferInitExtensions( CRBufferState *b )
{
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	b->extensions.blendColor = zero_color;
#if defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract)
	b->extensions.blendEquation = GL_FUNC_ADD_EXT;
#endif
}

void STATE_APIENTRY crStateBlendColorEXT( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "BlendColorEXT called inside a Begin/End" );
		return;
	}

	b->extensions.blendColor.r = red;
	b->extensions.blendColor.g = green;
	b->extensions.blendColor.b = blue;
	b->extensions.blendColor.a = alpha;
	bb->extensions = g->neg_bitid;
	bb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateBlendEquationEXT( GLenum mode )
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if( g->current.inBeginEnd )
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "BlendEquationEXT called inside a Begin/End" );
		return;
	}
	switch( mode )
	{
#if defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract)
		case GL_FUNC_ADD_EXT:
#ifdef GL_EXT_blend_subtract
		case GL_FUNC_SUBTRACT_EXT:
#endif /* GL_EXT_blend_subtract */
#ifdef GL_EXT_blend_minmax
		case GL_MIN_EXT:
		case GL_MAX_EXT:
#endif /* GL_EXT_blend_minmax */
			b->extensions.blendEquation = mode;
			break;
#endif /* defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract) */
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,
				"BlendEquationEXT: mode called with illegal parameter: 0x%x", (GLenum) mode );
			return;
	}
	bb->extensions = g->neg_bitid;
	bb->dirty = g->neg_bitid;
}

void crStateBufferDiffExtensions( CRBufferState *from, CRBufferState *to )
{
#ifdef GL_EXT_blend_color
	if (from->extensions.blendColor.r != to->extensions.blendColor.r ||
	    from->extensions.blendColor.g != to->extensions.blendColor.g ||
	    from->extensions.blendColor.b != to->extensions.blendColor.b ||
	    from->extensions.blendColor.a != to->extensions.blendColor.a   )
	{
		diff_api.BlendColorEXT( to->extensions.blendColor.r,
					to->extensions.blendColor.g,
					to->extensions.blendColor.b,
					to->extensions.blendColor.a );
		from->extensions.blendColor = to->extensions.blendColor;
	}
#endif
#if defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract)
	if( from->extensions.blendEquation != to->extensions.blendEquation )
	{
		diff_api.BlendEquationEXT( to->extensions.blendEquation );
		from->extensions.blendEquation = to->extensions.blendEquation;
	}
#endif
}

void crStateBufferSwitchExtensions( CRBufferState *from, CRBufferState *to )
{
#ifdef GL_EXT_blend_color
	if (from->extensions.blendColor.r != to->extensions.blendColor.r ||
	    from->extensions.blendColor.g != to->extensions.blendColor.g ||
	    from->extensions.blendColor.b != to->extensions.blendColor.b ||
	    from->extensions.blendColor.a != to->extensions.blendColor.a   )
	{
		diff_api.BlendColorEXT( to->extensions.blendColor.r,
					to->extensions.blendColor.g,
					to->extensions.blendColor.b,
					to->extensions.blendColor.a );
	}
#endif
#if defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract)
	if( from->extensions.blendEquation != to->extensions.blendEquation )
	{
		diff_api.BlendEquationEXT( to->extensions.blendEquation );
	}
#endif
}

