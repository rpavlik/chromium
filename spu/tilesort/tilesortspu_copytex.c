/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include "tilesortspu.h"
#include "cr_mem.h"


extern void TILESORTSPU_APIENTRY tilesortspu_ReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );


/*
 * glCopyTex{Sub}Image[12]D()
 *
 * In a tiled configuration, we need to use glReadPixels to get the
 * image out of the framebuffer and glTexImage to redefine the texture.
 */

/*
 * XXX to do
 * Make sure that pixel pack/unpacking are set correctly for the
 * ReadPixels and glTexImage calls.
 */


/*
 * Return the basic format of a texture internal format.
 */
static GLenum baseFormat(GLint intFormat)
{
	switch (intFormat) {
	case GL_ALPHA:
	case GL_ALPHA4:
	case GL_ALPHA8:
	case GL_ALPHA12:
	case GL_ALPHA16:
		return GL_ALPHA;
	case 1:
	case GL_LUMINANCE:
	case GL_LUMINANCE4:
	case GL_LUMINANCE8:
	case GL_LUMINANCE12:
	case GL_LUMINANCE16:
		return GL_LUMINANCE;
	case 2:
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE4_ALPHA4:
	case GL_LUMINANCE6_ALPHA2:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE12_ALPHA4:
	case GL_LUMINANCE12_ALPHA12:
	case GL_LUMINANCE16_ALPHA16:
		return GL_LUMINANCE_ALPHA;
	case GL_INTENSITY:
	case GL_INTENSITY4:
	case GL_INTENSITY8:
	case GL_INTENSITY12:
	case GL_INTENSITY16:
		return GL_INTENSITY;
	case 3:
	case GL_RGB:
	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16:
		return GL_RGB;
	case 4:
	case GL_RGBA:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGB10_A2:
	case GL_RGBA12:
	case GL_RGBA16:
		return GL_RGBA;
	case GL_COLOR_INDEX:
	case GL_COLOR_INDEX1_EXT:
	case GL_COLOR_INDEX2_EXT:
	case GL_COLOR_INDEX4_EXT:
	case GL_COLOR_INDEX8_EXT:
	case GL_COLOR_INDEX12_EXT:
	case GL_COLOR_INDEX16_EXT:
		return GL_COLOR_INDEX;
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16_SGIX:
	case GL_DEPTH_COMPONENT24_SGIX:
	case GL_DEPTH_COMPONENT32_SGIX:
		return GL_DEPTH_COMPONENT;
	default:
		crError("Bad internal texture format in baseFormat() in tilesortspu_copytex.c");
		return 0;
	}
}



void TILESORTSPU_APIENTRY tilesortspu_CopyTexImage1D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border )
{
	GLubyte *buffer = crAlloc(width * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum type = GL_UNSIGNED_BYTE;
		const GLenum format = baseFormat( internalFormat );

		tilesortspu_ReadPixels( x, y, width, 1, format, type, buffer );

		crStateTexImage1D( target, level, internalFormat, width, border,
											 format, type, buffer );

		crFree(buffer);
	}
	else
	{
		crStateError( __LINE__, __FILE__, GL_OUT_OF_MEMORY, "glCopyTexImage1D" );
	}
}


void TILESORTSPU_APIENTRY tilesortspu_CopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
	GLubyte *buffer = crAlloc(width * height * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum type = GL_UNSIGNED_BYTE;
		const GLenum format = baseFormat( internalFormat );

		tilesortspu_ReadPixels( x, y, width, height, format, type, buffer );

		crStateTexImage2D( target, level, internalFormat, width, height,
											 border, format, type, buffer );

		crFree(buffer);
	}
	else
	{
		crStateError( __LINE__, __FILE__, GL_OUT_OF_MEMORY, "glCopyTexImage2D" );
	}
}


void TILESORTSPU_APIENTRY tilesortspu_CopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width )
{
	GLubyte *buffer = crAlloc(width * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum type = GL_UNSIGNED_BYTE;
		GLenum format;
		GLint intFormat;

		crStateGetTexLevelParameteriv( target, level, GL_TEXTURE_INTERNAL_FORMAT,
																	 &intFormat );

		format = baseFormat( intFormat );

		tilesortspu_ReadPixels( x, y, width, 1, format, type, buffer );

		crStateTexSubImage1D( target, level, xoffset, width,
													format, type, buffer );

		crFree(buffer);
	}
	else
	{
		crStateError( __LINE__, __FILE__, GL_OUT_OF_MEMORY, "glCopyTexSubImage1D" );
	}
}


void TILESORTSPU_APIENTRY tilesortspu_CopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	GLubyte *buffer = crAlloc(width * height * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum type = GL_UNSIGNED_BYTE;
		GLenum format;
		GLint intFormat;

		crStateGetTexLevelParameteriv( target, level, GL_TEXTURE_INTERNAL_FORMAT,
																	 &intFormat );

		format = baseFormat( intFormat );

		tilesortspu_ReadPixels( x, y, width, height, format, type, buffer );

		crStateTexSubImage2D( target, level, xoffset, yoffset, width, height,
													format, type, buffer );

		crFree(buffer);
	}
	else
	{
		crStateError( __LINE__, __FILE__, GL_OUT_OF_MEMORY, "glCopyTexSubImage2D" );
	}
}
