#include "cr_packfunctions.h"
#include "cr_pack.h"
#include <GL/gl.h>
#include "cr_error.h"

static void __handleMaterialData( GLenum face, GLenum pname, const GLfloat *params )
{
	unsigned int packet_length = sizeof( int ) + sizeof( face ) + sizeof( pname );
	unsigned int params_length = 0;
	unsigned char *data_ptr;
	switch( pname )
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_EMISSION:
		case GL_AMBIENT_AND_DIFFUSE:
			params_length = 4*sizeof( *params );
			break;
		case GL_COLOR_INDEXES:
			params_length = 3*sizeof( *params );
			break;
		case GL_SHININESS:
			params_length = sizeof( *params );
			break;
		default:
			crError( "Unknown Parameter: %d", pname );
	}
	packet_length += params_length;

	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, face );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	WRITE_DATA( sizeof( int ) + 8, GLfloat, params[0] );
	if (params_length > sizeof( *params )) 
	{
		WRITE_DATA( sizeof( int ) + 12, GLfloat, params[1] );
		WRITE_DATA( sizeof( int ) + 16, GLfloat, params[2] );
	}
	if (packet_length > 3*sizeof( *params ) ) WRITE_DATA( sizeof( int ) + 20, GLfloat, params[3] );
}

void PACK_APIENTRY crPackMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	__handleMaterialData( face, pname, params );
	WRITE_OPCODE( CR_MATERIALFV_OPCODE );
}

void PACK_APIENTRY crPackMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
	/* floats and ints are the same size, so the packing should be the same */
	__handleMaterialData( face, pname, (const GLfloat *) params );
	WRITE_OPCODE( CR_MATERIALIV_OPCODE );
}
