#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_pixeldata.h"
#include "cr_error.h"

#include "state/cr_pixel.h"

void PACK_APIENTRY crPackTexImage1D(GLenum target, GLint level, 
		GLint internalformat, GLsizei width, GLint border, GLenum format, 
		GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	unsigned char *data_ptr;
	int packet_length;

	packet_length = 
		sizeof( target ) +
		sizeof( level ) +
		sizeof( internalformat ) +
		sizeof( width ) + 
		sizeof( border ) +
		sizeof( format ) +
		sizeof( type ) +
		sizeof( int );

	if (pixels)
	{
		packet_length += crPixelSize( format, type, width, 1 );
	}

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLint, level );
	WRITE_DATA( 8, GLint, internalformat );
	WRITE_DATA( 12, GLsizei, width );
	WRITE_DATA( 16, GLint, border );
	WRITE_DATA( 20, GLenum, format );
	WRITE_DATA( 24, GLenum, type );
	WRITE_DATA( 28, int, (pixels == NULL) );

	if (pixels) {
		crPixelCopy1D( (void *)(data_ptr + 32), pixels, format, type, width, packstate );
	}

	crHugePacket( CR_TEXIMAGE1D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackTexImage2D(GLenum target, GLint level, 
		GLint internalformat, GLsizei width, GLsizei height, GLint border, 
		GLenum format, GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	unsigned char *data_ptr;
	int packet_length;

	packet_length = 
		sizeof( target ) +
		sizeof( level ) +
		sizeof( internalformat ) +
		sizeof( width ) +
		sizeof( height ) + 
		sizeof( border ) +
		sizeof( format ) +
		sizeof( type ) +
		sizeof( int );

	if (pixels)
	{
		packet_length += crPixelSize( format, type, width, height );
	}

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLint, level );
	WRITE_DATA( 8, GLint, internalformat );
	WRITE_DATA( 12, GLsizei, width );
	WRITE_DATA( 16, GLsizei, height );
	WRITE_DATA( 20, GLint, border );
	WRITE_DATA( 24, GLenum, format );
	WRITE_DATA( 28, GLenum, type );
	WRITE_DATA( 32, int, (pixels == NULL) );

	if (pixels)
	{
		crPixelCopy2D( 
			(void *)(data_ptr + 36), 
			pixels, format, type, width, height, packstate );
	}

	crHugePacket( CR_TEXIMAGE2D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackDeleteTextures( GLsizei n, const GLuint *textures )
{
	unsigned char *data_ptr;
	int packet_length = 
		sizeof( int ) + 
		sizeof( n ) + 
		n*sizeof( *textures );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLsizei, n );
	memcpy( data_ptr + sizeof( int ) + 4, textures, n*sizeof(*textures) );
	crHugePacket( CR_DELETETEXTURES_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

static void __handleTexEnvData( GLenum target, GLenum pname, const GLfloat *params )
{
	unsigned char *data_ptr;
	int params_length;

	int packet_length = 
		sizeof( int ) + 
		sizeof( target ) + 
		sizeof( pname );

	if ( pname == GL_TEXTURE_ENV_COLOR )
	{
		params_length = 4*sizeof( *params );
	}
	else
	{
		params_length = sizeof( *params );
	}

	packet_length += params_length;

	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, target );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	memcpy( data_ptr + sizeof( int ) + 8, params, params_length );
}


void PACK_APIENTRY crPackTexEnvfv( GLenum target, GLenum pname,
		const GLfloat *params )
{
	__handleTexEnvData( target, pname, params );
	WRITE_OPCODE( CR_TEXENVFV_OPCODE );
}

void PACK_APIENTRY crPackTexEnviv( GLenum target, GLenum pname,
		const GLint *params )
{
	/* floats and ints are the same size, so the packing should be the same */
	__handleTexEnvData( target, pname, (const GLfloat *) params );
	WRITE_OPCODE( CR_TEXENVIV_OPCODE );
}

void PACK_APIENTRY crPackPrioritizeTextures( GLsizei n,
		const GLuint *textures, const GLclampf *priorities )
{
	unsigned char *data_ptr;
	int packet_length = 
		sizeof(int) +
		sizeof( n ) + 
		n*sizeof( *textures ) + 
		n*sizeof( *priorities );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );

	WRITE_DATA( 0, GLsizei, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLsizei, n );
	memcpy( data_ptr + sizeof( int ) + 4, textures, n*sizeof( *textures ) );
	memcpy( data_ptr + sizeof( int ) + 4 + n*sizeof( *textures ),
			priorities, n*sizeof( *priorities ) );

	crHugePacket( CR_PRIORITIZETEXTURES_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

static void __handleTexGenData( GLenum coord, GLenum pname, 
		int sizeof_param, const GLvoid *params )
{
	unsigned char *data_ptr;
	int packet_length = sizeof( int ) + sizeof( coord ) + sizeof( pname ) + sizeof_param;
	int params_length = sizeof_param;
	if (pname == GL_OBJECT_PLANE || pname == GL_EYE_PLANE)
	{
		packet_length += 3*sizeof_param;
		params_length += 3*sizeof_param;
	}
	
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, coord );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	memcpy( data_ptr + sizeof( int ) + 8, params, params_length );
}

void PACK_APIENTRY crPackTexGendv( GLenum coord, GLenum pname,
		const GLdouble *params )
{
	__handleTexGenData( coord, pname, sizeof( *params ), params );
	WRITE_OPCODE( CR_TEXGENDV_OPCODE );
}

void PACK_APIENTRY crPackTexGenfv( GLenum coord, GLenum pname,
		const GLfloat *params )
{
	__handleTexGenData( coord, pname, sizeof( *params ), params );
	WRITE_OPCODE( CR_TEXGENFV_OPCODE );
}

void PACK_APIENTRY crPackTexGeniv( GLenum coord, GLenum pname,
		const GLint *params )
{
	__handleTexGenData( coord, pname, sizeof( *params ), params );
	WRITE_OPCODE( CR_TEXGENIV_OPCODE );
}

void __handleTexParameterData( GLenum target, GLenum pname, const GLfloat *params )
{
	unsigned char *data_ptr;
	int packet_length = sizeof( int ) + sizeof( target ) + sizeof( pname );
	int params_length = 0;

	switch( pname )
	{
		case GL_TEXTURE_MIN_FILTER:
		case GL_TEXTURE_MAG_FILTER:
		case GL_TEXTURE_WRAP_S:
		case GL_TEXTURE_WRAP_T:
		case GL_TEXTURE_PRIORITY:
			params_length = sizeof( *params );
			break;
		case GL_TEXTURE_BORDER_COLOR:
			params_length = 4* sizeof( *params );
			break;
		default:
			if (!crPackTexParameterParamsLength( pname ))
			{
				crError( "Unnown pname: %d", pname );
			}
			break;
	}
	packet_length += params_length;

	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, target );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	memcpy( data_ptr + sizeof( int ) + 8, params, params_length );
}

void PACK_APIENTRY crPackTexParameterfv( GLenum target, GLenum pname, 
		const GLfloat *params )
{
	__handleTexParameterData( target, pname, params );
	WRITE_OPCODE( CR_TEXPARAMETERFV_OPCODE );
}

void PACK_APIENTRY crPackTexParameteriv( GLenum target, GLenum pname, 
		const GLint *params )
{
	__handleTexParameterData( target, pname, (GLfloat *) params );
	WRITE_OPCODE( CR_TEXPARAMETERIV_OPCODE );
}

void PACK_APIENTRY crPackTexSubImage2D (GLenum target, GLint level, 
		GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, 
		GLenum format, GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	unsigned char *data_ptr;
	int packet_length;

	packet_length = 
		sizeof( target ) +
		sizeof( level ) +
		sizeof( xoffset ) +
		sizeof( yoffset ) +
		sizeof( width ) +
		sizeof( height ) +
		sizeof( format ) +
		sizeof( type ) +
		crPixelSize( format, type, width, height );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLint, level );
	WRITE_DATA( 8, GLint, xoffset );
	WRITE_DATA( 12, GLint, yoffset );
	WRITE_DATA( 16, GLsizei, width );
	WRITE_DATA( 20, GLsizei, height );
	WRITE_DATA( 24, GLenum, format );
	WRITE_DATA( 28, GLenum, type );

	crPixelCopy2D((GLvoid *) (data_ptr + 32), pixels, format, type, width, height, packstate );

	crHugePacket( CR_TEXSUBIMAGE2D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackTexSubImage1D (GLenum target, GLint level, 
		GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	unsigned char *data_ptr;
	int packet_length;

	packet_length = 
		sizeof( target ) +
		sizeof( level ) +
		sizeof( xoffset ) +
		sizeof( width ) +
		sizeof( format ) +
		sizeof( type ) +
		crPixelSize( format, type, width, 1 );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLenum, target );
	WRITE_DATA( 4, GLint, level );
	WRITE_DATA( 8, GLint, xoffset );
	WRITE_DATA( 12, GLsizei, width );
	WRITE_DATA( 16, GLenum, format );
	WRITE_DATA( 20, GLenum, type );

	crPixelCopy1D((GLvoid *) (data_ptr + 24), pixels, format, type, width, packstate );

	crHugePacket( CR_TEXSUBIMAGE1D_OPCODE, data_ptr );
	crPackFree( data_ptr );
}

void PACK_APIENTRY crPackAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences, GLboolean *return_val, int *writeback )
{
	unsigned char *data_ptr;
	int packet_length;

	packet_length = 
		sizeof(int) +            // packet length 
		sizeof( GLenum ) +       // extend-o opcode
		sizeof( n ) +            // num_textures
		n*sizeof( *textures ) +  // textures
		8 + 8 + 8;               // return pointers

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, CR_ARETEXTURESRESIDENT_EXTEND_OPCODE );
	WRITE_DATA( sizeof( int ) + 4, GLsizei, n );
	memcpy( data_ptr + sizeof( int ) + 8, textures, n*sizeof( *textures ) );
	WRITE_NETWORK_POINTER( sizeof( int ) + 8 + n*sizeof( *textures ), (void *) residences );
	WRITE_NETWORK_POINTER( sizeof( int ) + 16 + n*sizeof( *textures ), (void *) return_val );
	WRITE_NETWORK_POINTER( sizeof( int ) + 24 + n*sizeof( *textures ), (void *) writeback );
	WRITE_OPCODE( CR_EXTEND_OPCODE );
}
