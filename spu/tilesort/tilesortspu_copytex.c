/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include "cr_mem.h"
#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_packfunctions.h"

/*
 * glCopyTex{Sub}Image[12]D()
 *
 * In a tiled configuration, we need to use glReadPixels to get the
 * image out of the framebuffer and glTexImage to redefine the texture.
 */

/**
 * XXX \todo
 * Make sure that pixel pack/unpacking are set correctly for the
 * ReadPixels and glTexImage calls.
 */


/**
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
	GET_THREAD(thread);
	GLubyte *buffer;
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileCopyTexImage1D(target, level, internalFormat, x, y, width, border);
		return;
	}

	buffer = crAlloc(width * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum format = baseFormat( internalFormat );
		const GLenum type = (format == GL_DEPTH_COMPONENT) ? GL_FLOAT : GL_UNSIGNED_BYTE;

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
	GET_THREAD(thread);
	GLubyte *buffer;
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists) crDLMCompileCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
	    else if (tilesort_spu.swap) crPackCopyTexImage2DSWAP(target, level, internalFormat, x, y, width, height, border);
	    else crPackCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
	    return;
	}

	buffer = crAlloc(width * height * sizeof(GLubyte) * 4);
	if (buffer)
	{
		const GLenum format = baseFormat( internalFormat );
		const GLenum type = (format == GL_DEPTH_COMPONENT) ? GL_FLOAT : GL_UNSIGNED_BYTE;

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
	GET_THREAD(thread);
	GLenum format, type;
	GLint intFormat;
	void *buffer;

	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists) crDLMCompileCopyTexSubImage1D(target, level, xoffset, x, y, width);
	    else if (tilesort_spu.swap) crPackCopyTexSubImage1DSWAP(target, level, xoffset, x, y, width);
	    else crPackCopyTexSubImage1D(target, level, xoffset, x, y, width);
	    return;
	}

	crStateGetTexLevelParameteriv( target, level, GL_TEXTURE_INTERNAL_FORMAT,
																 &intFormat );

	format = baseFormat( intFormat );
	if (format == GL_DEPTH_COMPONENT)
	{
		type = GL_FLOAT;
		buffer = crAlloc(width * sizeof(GLfloat));
	}
	else
	{
		type = GL_UNSIGNED_BYTE;
		buffer = crAlloc(width * 4 * sizeof(GLubyte));
	}

	if (buffer)
	{
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
	GET_THREAD(thread);
	GLenum format, type;
	GLint intFormat;
	void *buffer;

	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just compiling or creating display lists */
	    if (tilesort_spu.lazySendDLists) crDLMCompileCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	    else if (tilesort_spu.swap) crPackCopyTexSubImage2DSWAP(target, level, xoffset, yoffset, x, y, width, height);
	    else crPackCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	    return;
	}

	crStateGetTexLevelParameteriv( target, level, GL_TEXTURE_INTERNAL_FORMAT,
																 &intFormat );

	format = baseFormat( intFormat );
	if (format == GL_DEPTH_COMPONENT)
	{
		type = GL_FLOAT;
		buffer = crAlloc(width * height * sizeof(GLfloat));
	}
	else
	{
		type = GL_UNSIGNED_BYTE;
		buffer = crAlloc(width * height * 4 * sizeof(GLubyte));
	}

	if (buffer)
	{
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

#if defined(CR_OPENGL_VERSION_1_2)
void TILESORTSPU_APIENTRY tilesortspu_CopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	GET_THREAD(thread);
	GLenum format, type;
	GLint intFormat;
	void *buffer;
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE) {
	    /* just creating or compiling display lists */
	    if (tilesort_spu.lazySendDLists) crDLMCompileCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	    else if (tilesort_spu.swap) crPackCopyTexSubImage3DSWAP(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	    else crPackCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	    return;
	}

	crStateGetTexLevelParameteriv( target, level, GL_TEXTURE_INTERNAL_FORMAT,
																 &intFormat );

	format = baseFormat( intFormat );
	if (format == GL_DEPTH_COMPONENT)
	{
		type = GL_FLOAT;
		buffer = crAlloc(width * height * sizeof(GLfloat));
	}
	else
	{
		type = GL_UNSIGNED_BYTE;
		buffer = crAlloc(width * height * 4 * sizeof(GLubyte));
	}

	if (buffer)
	{
		tilesortspu_ReadPixels( x, y, width, height, format, type, buffer );

		crStateTexSubImage3D( target, level, xoffset, yoffset, zoffset,
													width, height, 1 /* depth */, format, type, buffer );

		crFree(buffer);
	}
	else
	{
		crStateError( __LINE__, __FILE__, GL_OUT_OF_MEMORY, "glCopyTexSubImage3D" );
	}
}
#endif /* CR_OPENGL_VERSION_1_2 */
