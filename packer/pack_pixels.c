/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_pixeldata.h"
#include "cr_error.h"

#include "state/cr_pixel.h"

#include <memory.h>

void PACK_APIENTRY crPackDrawPixels( GLsizei width, GLsizei height, 
																		 GLenum format, GLenum type,
																		 const GLvoid *pixels,
																		 const CRPixelPackState *unpackstate )
{
	unsigned char *data_ptr;
	int packet_length;

	if (pixels == NULL)
	{
		return;
	}

	if (type == GL_BITMAP)
	{
		crPackBitmap( width, height, 0, 0, 0, 0,
									(const GLubyte *) pixels, unpackstate );
	}

	packet_length = 
		sizeof( width ) +
		sizeof( height ) +
		sizeof( format ) +
		sizeof( type );

	packet_length += crImageSize( format, type, width, height );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLsizei, width );
	WRITE_DATA( 4, GLsizei, height );
	WRITE_DATA( 8, GLenum, format );
	WRITE_DATA( 12, GLenum, type );

	crPixelCopy2D( width, height,
								 (void *) (data_ptr + 16), format, type, NULL, /* dst */
								 pixels, format, type, unpackstate );  /* src */

	crHugePacket( CR_DRAWPIXELS_OPCODE, data_ptr );
}

void PACK_APIENTRY crPackReadPixels( GLint x, GLint y, GLsizei width, 
																		 GLsizei height, GLenum format,
																		 GLenum type, GLvoid *pixels,
																		 const CRPixelPackState *packstate )
{
	unsigned char *data_ptr;
	GLint stride = 0;
	GLint bytes_per_row;
	int bytes_per_pixel;

	switch ( type )
	{
	  case GL_UNSIGNED_BYTE:
	  case GL_BYTE:
			 bytes_per_pixel = 1;
			 break;

	  case GL_UNSIGNED_SHORT:
	  case GL_SHORT:
			bytes_per_pixel = 2;
			break;

	  case GL_UNSIGNED_INT:
	  case GL_INT:
	  case GL_FLOAT:
			 bytes_per_pixel = 4;
			 break;

	  default:
			__PackError( __LINE__, __FILE__, GL_INVALID_ENUM,
									 "crPackReadPixels(bad type)" );
			return;
	}

	switch ( format )
	{
	  case GL_COLOR_INDEX:
	  case GL_STENCIL_INDEX:
	  case GL_DEPTH_COMPONENT:
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_ALPHA:
	  case GL_LUMINANCE:
			 break;

	  case GL_LUMINANCE_ALPHA:
			 bytes_per_pixel *= 2;
			 break;

	  case GL_RGB:
			 bytes_per_pixel *= 3;
			 break;

	  case GL_RGBA:
			 bytes_per_pixel *= 4;
			 break;

	  default:
			__PackError( __LINE__, __FILE__, GL_INVALID_ENUM,
									 "crPackReadPixels(bad format)" );
			return;
	}

	/* default bytes_per_row so crserver can allocate memory */
	bytes_per_row = width * bytes_per_pixel;

	stride = bytes_per_row;
	if (packstate->alignment != 1) {
		 GLint remainder = bytes_per_row % packstate->alignment;
		 if (remainder)
				stride = bytes_per_row + (packstate->alignment - remainder);
	}

	GET_BUFFERED_POINTER( 44 + sizeof(CRNetworkPointer) );
	WRITE_DATA( 0,  GLint,  x );
	WRITE_DATA( 4,  GLint,  y );
	WRITE_DATA( 8,  GLsizei,  width );
	WRITE_DATA( 12, GLsizei,  height );
	WRITE_DATA( 16, GLenum, format );
	WRITE_DATA( 20, GLenum, type );
	WRITE_DATA( 24, GLint,  stride );
	WRITE_DATA( 28, GLint, packstate->alignment );
	WRITE_DATA( 32, GLint, packstate->skipRows );
	WRITE_DATA( 36, GLint, packstate->skipPixels );
	WRITE_DATA( 40, GLint,  bytes_per_row );
	WRITE_NETWORK_POINTER( 44, (char *) pixels );
	WRITE_OPCODE( CR_READPIXELS_OPCODE );
}

void PACK_APIENTRY crPackBitmap( GLsizei width, GLsizei height, 
		GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
		const GLubyte *bitmap, const CRPixelPackState *unpack )
{
	unsigned char *data_ptr;
	int row_length, data_length=0;
	int isnull = (bitmap == NULL);
	int packet_length = 
		sizeof( width ) + 
		sizeof( height ) +
		sizeof( xorig ) + 
		sizeof( yorig ) + 
		sizeof( xmove ) + 
		sizeof( ymove ) +
		sizeof( GLuint );

	if ( bitmap )
	{
		if ( unpack->skipRows != 0 ||
			   unpack->skipPixels != 0 )
		{
			crError( "crPackBitmap: I don't know how to unpack the data!" );
		}

		if (unpack->rowLength > 0)
			row_length = unpack->rowLength;
		else 
			row_length = width;

		switch (unpack->alignment) {
			case 1:
				row_length = ( ( row_length + 7 ) & ~7 ) >> 3;
				break;
			case 2:
				row_length = ( ( row_length + 15 ) & ~15 ) >> 4;
				break;
			case 4:
				row_length = ( ( row_length + 31 ) & ~31 ) >> 5;
				break;
			case 8:
				row_length = ( ( row_length + 63 ) & ~63 ) >> 6;
				break;
			default:
				crError( "crPackBitmap: I don't know how to unpack the data!" );
				return;
		}

		if (unpack->imageHeight > 0)
			data_length = row_length * unpack->imageHeight;
		else
			data_length = row_length * height;

		packet_length += data_length;
	}

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLsizei, width );
	WRITE_DATA( 4, GLsizei, height );
	WRITE_DATA( 8, GLfloat, xorig );
	WRITE_DATA( 12, GLfloat, yorig );
	WRITE_DATA( 16, GLfloat, xmove );
	WRITE_DATA( 20, GLfloat, ymove );
	WRITE_DATA( 24, GLuint, isnull );
	if ( bitmap )
	{
		memcpy( data_ptr + 28, bitmap, data_length );
	}

	crHugePacket( CR_BITMAP_OPCODE, data_ptr );
}
