#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_error.h"

static void __handleLightData( GLenum light, GLenum pname, const GLfloat *params )
{
	unsigned int packet_length = sizeof( int ) + sizeof( light ) + sizeof( pname );
	unsigned int params_length = 0;
	unsigned char *data_ptr;
	switch( pname )
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION:
			params_length = 4*sizeof( *params );
			break;
		case GL_SPOT_DIRECTION:
			params_length = 3*sizeof( *params );
			break;
		case GL_SPOT_EXPONENT:
		case GL_SPOT_CUTOFF:
		case GL_CONSTANT_ATTENUATION:
		case GL_LINEAR_ATTENUATION:
		case GL_QUADRATIC_ATTENUATION:
			params_length = sizeof( *params );
			break;
		default:
			crError( "Unkown parameter: %d", pname );
	}
	packet_length += params_length;
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, light );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	WRITE_DATA( sizeof( int ) + 8, GLfloat, params[0] );
	if (params_length > sizeof( *params )) 
	{
		WRITE_DATA( sizeof( int ) + 12, GLfloat, params[1] );
		WRITE_DATA( sizeof( int ) + 16, GLfloat, params[2] );
	}
	if (packet_length > 3*sizeof( *params )) WRITE_DATA( sizeof( int ) + 20, GLfloat, params[3] );
}

void PACK_APIENTRY crPackLightfv (GLenum light, GLenum pname, const GLfloat *params)
{
	__handleLightData( light, pname, params );
	WRITE_OPCODE( CR_LIGHTFV_OPCODE );
}

void PACK_APIENTRY crPackLightiv (GLenum light, GLenum pname, const GLint *params)
{
	/* floats and ints are the same size, so the packing should be the same */
	__handleLightData( light, pname, (const GLfloat *) params );
	WRITE_OPCODE( CR_LIGHTIV_OPCODE );
}

void __handleLightModelData( GLenum pname, const GLfloat *params )
{
	unsigned int packet_length = sizeof( int ) + sizeof( pname );
	unsigned int params_length = 0;
	unsigned char *data_ptr;
	switch( pname )
	{
		case GL_LIGHT_MODEL_AMBIENT:
			params_length = 4*sizeof( *params );
			break;
		case GL_LIGHT_MODEL_TWO_SIDE:
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
			params_length = sizeof( *params );
			break;
		default:
			crError( "Unkown parameter: %d", pname );
	}
	packet_length += params_length;
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, pname );
	WRITE_DATA( sizeof( int ) + 4, GLfloat, params[0] );
	if (params_length > sizeof( *params ))
	{
		WRITE_DATA( sizeof( int ) + 8, GLfloat, params[1] );
		WRITE_DATA( sizeof( int ) + 12, GLfloat, params[2] );
		WRITE_DATA( sizeof( int ) + 16, GLfloat, params[3] );
	}
}

void PACK_APIENTRY crPackLightModelfv (GLenum pname, const GLfloat *params)
{
	__handleLightModelData( pname, params );
	WRITE_OPCODE( CR_LIGHTMODELFV_OPCODE );
}

void PACK_APIENTRY crPackLightModeliv (GLenum pname, const GLint *params)
{
	/* floats and ints are the same size, so the packing should be the same */
	__handleLightModelData( pname, (const GLfloat *) params );
	WRITE_OPCODE( CR_LIGHTMODELIV_OPCODE );
}
