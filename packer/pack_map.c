#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_opcodes.h"
#include "cr_error.h"

#include <memory.h>

/* Note -- for these packets, the ustride and vstride are implicit,
 * and are computed into the packet instead of copied.
 */

int __gl_Map2NumComponents( GLenum target )
{
	switch( target )
	{
	case GL_MAP2_VERTEX_3:
	case GL_MAP2_NORMAL:
	case GL_MAP2_TEXTURE_COORD_3:
		return 3;
	case GL_MAP2_VERTEX_4:
	case GL_MAP2_COLOR_4:
	case GL_MAP2_TEXTURE_COORD_4:
		return 4;
	case GL_MAP2_INDEX:
	case GL_MAP2_TEXTURE_COORD_1:
		return 1;
	case GL_MAP2_TEXTURE_COORD_2:
		return 2;
	default:
		crError( "Invalid map target: %d", target );
	}
	return -1;
}

int __gl_Map1NumComponents( GLenum target )
{
	switch( target )
	{
	case GL_MAP1_VERTEX_3:
	case GL_MAP1_NORMAL:
	case GL_MAP1_TEXTURE_COORD_3:
		return 3;
	case GL_MAP1_VERTEX_4:
	case GL_MAP1_COLOR_4:
	case GL_MAP1_TEXTURE_COORD_4:
		return 4;
	case GL_MAP1_INDEX:
	case GL_MAP1_TEXTURE_COORD_1:
		return 1;
	case GL_MAP1_TEXTURE_COORD_2:
		return 2;
	default:
		crError( "Invalid map target: %d", target );
	}
	return -1;
}

void PACK_APIENTRY crPackMap2d(GLenum target, GLdouble u1, 
		GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, 
		GLint vstride, GLint vorder, const GLdouble *points)
{
	unsigned char *data_ptr;
	int u,v;
	GLdouble *dest_data, *src_data;
	int packet_length = 
		sizeof( target ) + 
		sizeof( u1 ) +
		sizeof( u2 ) +
		sizeof( uorder ) +
		sizeof( ustride ) +
		sizeof( v1 ) +
		sizeof( v2 ) + 
		sizeof( vorder ) +
		sizeof( vstride );

	int num_components = __gl_Map2NumComponents( target );
	packet_length += num_components*uorder*vorder*sizeof( *points );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );

	WRITE_DATA( 0, GLenum, target );
	WRITE_DOUBLE( 4, u1 );
	WRITE_DOUBLE( 12, u2 );
	WRITE_DATA( 20, GLint, num_components );
	WRITE_DATA( 24, GLint, uorder );
	WRITE_DOUBLE( 28, v1 );
	WRITE_DOUBLE( 36, v2 );
	WRITE_DATA( 44, GLint, num_components*uorder );
	WRITE_DATA( 48, GLint, vorder );

	dest_data = (GLdouble *) (data_ptr + 52);
	src_data = (GLdouble *) points;
	for (v = 0 ; v < vorder ; v++)
	{
		for (u = 0 ; u < uorder ; u++)
		{
			memcpy( dest_data, src_data, num_components * sizeof( *points ) );
			dest_data += num_components;
			src_data += ustride;
		}
		src_data += vstride - ustride*uorder;
	}

	crHugePacket( CR_MAP2D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackMap2f(GLenum target, GLfloat u1, 
		GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, 
		GLint vstride, GLint vorder, const GLfloat *points)
{
	unsigned char *data_ptr;
	int u,v;
	GLfloat *dest_data, *src_data;
	int packet_length = 
		sizeof( target ) + 
		sizeof( u1 ) +
		sizeof( u2 ) +
		sizeof( uorder ) +
		sizeof( ustride ) +
		sizeof( v1 ) +
		sizeof( v2 ) + 
		sizeof( vorder ) +
		sizeof( vstride );

	int num_components = __gl_Map2NumComponents( target );
	packet_length += num_components*uorder*vorder*sizeof( *points );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );

	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLfloat, u1 );
	WRITE_DATA( 8, GLfloat, u2 );
	WRITE_DATA( 12, GLint, num_components );
	WRITE_DATA( 16, GLint, uorder );
	WRITE_DATA( 20, GLfloat, v1 );
	WRITE_DATA( 24, GLfloat, v2 );
	WRITE_DATA( 28, GLint, num_components*uorder );
	WRITE_DATA( 32, GLint, vorder );

	dest_data = (GLfloat *) (data_ptr + 36);
	src_data = (GLfloat *) points;
	for (v = 0 ; v < vorder ; v++)
	{
		for (u = 0 ; u < uorder ; u++)
		{
			memcpy( dest_data, src_data, num_components * sizeof( *points ) );
			dest_data += num_components;
			src_data += ustride;
		}
		src_data += vstride - ustride*uorder;
	}

	crHugePacket( CR_MAP2F_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackMap1d( GLenum target, GLdouble u1,
		GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
	unsigned char *data_ptr;
	int packet_length = 
		sizeof( target ) + 
		sizeof( u1 ) +
		sizeof( u2 ) + 
		sizeof( stride ) + 
		sizeof( order );

	int num_components = __gl_Map1NumComponents( target );
	GLdouble *src_data, *dest_data;
	int u;

	packet_length += num_components * order * sizeof( *points );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );

	WRITE_DATA( 0, GLenum, target );
	WRITE_DOUBLE( 4, u1 );
	WRITE_DOUBLE( 12, u2 );
	WRITE_DATA( 20, GLint, num_components );
	WRITE_DATA( 24, GLint, order );

	dest_data = (GLdouble *) (data_ptr + 28);
	src_data = (GLdouble *) points;
	for (u = 0 ; u < order ; u++)
	{
		memcpy( dest_data, src_data, num_components * sizeof( *points ) );
		dest_data += num_components;
		src_data += stride;
	}

	crHugePacket( CR_MAP1D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackMap1f( GLenum target, GLfloat u1,
		GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
	unsigned char *data_ptr;
	int packet_length = 
		sizeof( target ) + 
		sizeof( u1 ) +
		sizeof( u2 ) + 
		sizeof( stride ) + 
		sizeof( order );

	int num_components = __gl_Map1NumComponents( target );
	GLfloat *src_data, *dest_data;
	int u;

	packet_length += num_components * order * sizeof( *points );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );

	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLfloat, u1 );
	WRITE_DATA( 8, GLfloat, u2 );
	WRITE_DATA( 12, GLint, num_components );
	WRITE_DATA( 16, GLint, order );

	dest_data = (GLfloat *) (data_ptr + 20);
	src_data = (GLfloat *) points;
	for (u = 0 ; u < order ; u++)
	{
		memcpy( dest_data, src_data, num_components * sizeof( *points ) );
		dest_data += num_components;
		src_data += stride;
	}

	crHugePacket( CR_MAP1F_OPCODE, data_ptr );
	crPackFree( data_ptr );
}
