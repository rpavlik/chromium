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

#include <stdio.h> 
#include <memory.h>

void PACK_APIENTRY crPackDrawPixels( GLsizei width, GLsizei height, 
		GLenum format, GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	unsigned char *data_ptr;
	int packet_length;

	if (pixels == NULL)
	{
		return;
	}

	if (type == GL_BITMAP)
	{
		crPackBitmap( width, height, 0, 0, 0, 0, (const GLubyte *) pixels, packstate );
	}

	packet_length = 
		sizeof( width ) +
		sizeof( height ) +
		sizeof( format ) +
		sizeof( type );

	packet_length += crPixelSize( format, type, width, height );

	data_ptr = (unsigned char *) crPackAlloc( packet_length );
	WRITE_DATA( 0, GLenum, width );
	WRITE_DATA( 4, GLint, height );
	WRITE_DATA( 8, GLint, format );
	WRITE_DATA( 12, GLsizei, type );

	crPixelCopy2D( 
		(void *)(data_ptr + 16), 
		pixels, format, type, width, height, packstate );

	crHugePacket( CR_DRAWPIXELS_OPCODE, data_ptr );
}

void PACK_APIENTRY crPackReadPixels( GLint x, GLint y, GLsizei width, 
		GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	crError( "Unimplemented crPackReadPixels" );

	(void) x;
	(void) y;
	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) pixels;
}

void PACK_APIENTRY crPackBitmap( GLsizei width, GLsizei height, 
		GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
		const GLubyte *bitmap, CRPackState *unpack )
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
		if ( unpack->rowLength != 0 ||
			   unpack->alignment != 1 ||
			   unpack->skipRows != 0 ||
			   unpack->skipPixels != 0 )
		{
			crError( "crPackBitmap: I don't know how to unpack the data!" );
		}

		row_length = ( ( width + 7 ) & ~7 ) >> 3;
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
