
#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"

unsigned int LookupComponents( GLenum pname )
{
	switch( pname )
	{

			case GL_AMBIENT: return 4;
			case GL_COLOR_INDEXES: return 3;
			case GL_CONSTANT_ATTENUATION: return 1;
			case GL_DIFFUSE: return 4;
			case GL_EMISSION: return 4;
			case GL_EYE_PLANE: return 4;
			case GL_LINEAR_ATTENUATION: return 1;
			case GL_OBJECT_PLANE: return 4;
			case GL_POSITION: return 4;
			case GL_QUADRATIC_ATTENUATION: return 1;
			case GL_SHININESS: return 1;
			case GL_SPECULAR: return 4;
			case GL_SPOT_CUTOFF: return 1;
			case GL_SPOT_DIRECTION: return 3;
			case GL_SPOT_EXPONENT: return 1;
			case GL_TEXTURE_BORDER_COLOR: return 4;
			case GL_TEXTURE_ENV_COLOR: return 4;
			case GL_TEXTURE_ENV_MODE: return 1;
			case GL_TEXTURE_GEN_MODE: return 1;
			case GL_TEXTURE_MAG_FILTER: return 1;
			case GL_TEXTURE_MIN_FILTER: return 1;
			case GL_TEXTURE_WRAP_S: return 1;
			case GL_TEXTURE_WRAP_T: return 1;

		default:
			crError( "Unknown paramater name in LookupComponents: %d", pname );
			break;
	}
	// NOTREACHED
	return 0;
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetClipPlane( GLenum plane, GLdouble *equation )
{
	GLdouble  local_equation[4];
	(void) equation;
	cr_server.head_spu->dispatch_table.GetClipPlane( plane, local_equation );
	crServerReturnValue( &(local_equation[0]), 4*sizeof(GLdouble ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetLightfv( light, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetLightiv( GLenum light, GLenum pname, GLint *params )
{
	GLint  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetLightiv( light, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLint ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetMaterialfv( face, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	GLint  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetMaterialiv( face, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLint ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetPolygonStipple( GLubyte *mask )
{
	GLubyte  local_mask[128];
	(void) mask;
	cr_server.head_spu->dispatch_table.GetPolygonStipple( local_mask );
	crServerReturnValue( &(local_mask[0]), 128*sizeof(GLubyte ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexEnvfv( target, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	GLint  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexEnviv( target, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLint ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	GLdouble  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexGendv( coord, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLdouble ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexGenfv( coord, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	GLint  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexGeniv( coord, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLint ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[1];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexLevelParameterfv( target, level, pname, local_params );
	crServerReturnValue( &(local_params[0]), 1*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params )
{
	GLint  local_params[1];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexLevelParameteriv( target, level, pname, local_params );
	crServerReturnValue( &(local_params[0]), 1*sizeof(GLint ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	GLfloat  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexParameterfv( target, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLfloat ) );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
	GLint  local_params[4];
	(void) params;
	cr_server.head_spu->dispatch_table.GetTexParameteriv( target, pname, local_params );
	crServerReturnValue( &(local_params[0]), LookupComponents(pname)*sizeof(GLint ) );
}

