/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/**
 * When functions can be aliased (like glBindTexture and glBindTextureEXT)
 * we can express that here.
 *
 * Also, we can stub-out misc functions until they're implemented in the
 * future.  This is a simple way to coax more OpenGL programs to run.
 *
 * Aliased functions, like glBlendFunc(), should be reimplemented in
 * assembly sometime.
 */


#include "chromium.h"
#include "cr_error.h"
#include "cr_version.h"

void CR_APIENTRY glColorTable( GLenum target, GLenum internalformat,
									 GLsizei width, GLenum format,
									 GLenum type, const GLvoid *table )
{
	(void) target;
	(void) internalformat;
	(void) width;
	(void) format;
	(void) type;
	(void) table;
	crWarning("glColorTable not implemented by Chromium");
}

void CR_APIENTRY glColorSubTable( GLenum target, GLsizei start, GLsizei count,
											GLenum format, GLenum type, const GLvoid *data )
{
	(void) target;
	(void) start;
	(void) count;
	(void) format;
	(void) type;
	(void) data;
	crWarning("glColorSubTable not implemented by Chromium");
}

void CR_APIENTRY glColorTableParameteriv(GLenum target, GLenum pname,
														 const GLint *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameteriv not implemented by Chromium");
}

void CR_APIENTRY glColorTableParameterfv(GLenum target, GLenum pname,
														 const GLfloat *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameterfv not implemented by Chromium");
}

void CR_APIENTRY glCopyColorSubTable( GLenum target, GLsizei start,
													GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) start;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorSubTable not implemented by Chromium");
}

void CR_APIENTRY glCopyColorTable( GLenum target, GLenum internalformat,
											 GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorTable not implemented by Chromium");
}

void CR_APIENTRY glGetColorTable( GLenum target, GLenum format,
											GLenum type, GLvoid *table )
{
	(void) target;
	(void) format;
	(void) type;
	(void) table;
	crWarning("glGetColorTable not implemented by Chromium");
}

void CR_APIENTRY glGetColorTableParameterfv( GLenum target, GLenum pname,
																 GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("id glGetColorTableParameterfv not implemented by Chromium");
}

void CR_APIENTRY glGetColorTableParameteriv( GLenum target, GLenum pname,
																 GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetColorTableParameteriv not implemented by Chromium");
}

void CR_APIENTRY glHistogram( GLenum target, GLsizei width,
									GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) width;
	(void) internalformat;
	(void) sink;
	crWarning("id glHistogram not implemented by Chromium");
}

void CR_APIENTRY glResetHistogram( GLenum target )
{
	(void) target;
	crWarning("glResetHistogram not implemented by Chromium");
}

void CR_APIENTRY glGetHistogram( GLenum target, GLboolean reset,
										 GLenum format, GLenum type, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) type;
	(void) values;
	crWarning("glGetHistogram not implemented by Chromium");
}

void CR_APIENTRY glGetHistogramParameterfv( GLenum target, GLenum pname,
																GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameterfv not implemented by Chromium");
}

void CR_APIENTRY glGetHistogramParameteriv( GLenum target, GLenum pname,
																GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameteriv not implemented by Chromium");
}

void CR_APIENTRY glMinmax( GLenum target, GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) internalformat;
	(void) sink;
	crWarning("glMinmax not implemented by Chromium");
}

void CR_APIENTRY glResetMinmax( GLenum target )
{
	(void) target;
	crWarning("glResetMinmax not implemented by Chromium");
}

void CR_APIENTRY glGetMinmax( GLenum target, GLboolean reset,
									GLenum format, GLenum types, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) types;
	(void) values;
	crWarning("glGetMinmax not implemented by Chromium");
}

void CR_APIENTRY glGetMinmaxParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameterfv not implemented by Chromium");
}

void CR_APIENTRY glGetMinmaxParameteriv( GLenum target, GLenum pname, GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameteriv not implemented by Chromium");
}

void CR_APIENTRY glConvolutionFilter1D( GLenum target, GLenum internalformat,
														GLsizei width, GLenum format, GLenum type,
														const GLvoid *image )
{
	(void) target;
	(void) internalformat;
	(void) width;
	(void) format;
	(void) type;
	(void) image;
	crWarning("glConvolutionFilter1D not implemented by Chromium");
}

void CR_APIENTRY glConvolutionFilter2D( GLenum target, GLenum internalformat,
														GLsizei width, GLsizei height, GLenum format,
														GLenum type, const GLvoid *image )
{
	(void) target;
	(void) internalformat;
	(void) width;
	(void) format;
	(void) height;
	(void) type;
	(void) image;
	crWarning("glConvolutionFilter2D not implemented by Chromium");
}

void CR_APIENTRY glConvolutionParameterf( GLenum target, GLenum pname, GLfloat params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterf not implemented by Chromium");
}

void CR_APIENTRY glConvolutionParameterfv( GLenum target, GLenum pname,
															 const GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterfv not implemented by Chromium");
}

void CR_APIENTRY glConvolutionParameteri( GLenum target, GLenum pname, GLint params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteri not implemented by Chromium");
}

void CR_APIENTRY glConvolutionParameteriv( GLenum target, GLenum pname,
															 const GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteriv not implemented by Chromium");
}

void CR_APIENTRY glCopyConvolutionFilter1D( GLenum target, GLenum internalformat,
																GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyConvolutionFilter1D not implemented by Chromium");
}

void CR_APIENTRY glCopyConvolutionFilter2D( GLenum target, GLenum internalformat,
																GLint x, GLint y, GLsizei width,
																GLsizei height)
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	(void) height;
	crWarning("glCopyConvolutionFilter2D not implemented by Chromium");
}

void CR_APIENTRY glGetConvolutionFilter( GLenum target, GLenum format,
														 GLenum type, GLvoid *image )
{
	(void) target;
	(void) format;
	(void) type;
	(void) image;
	crWarning("glGetConvolutionFilter not implemented by Chromium");
}

void CR_APIENTRY glGetConvolutionParameterfv( GLenum target, GLenum pname,
																	GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameterfv not implemented by Chromium");
}

void CR_APIENTRY glGetConvolutionParameteriv( GLenum target, GLenum pname,
																	GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameteriv not implemented by Chromium");
}

#ifdef SunOS
void CR_APIENTRY glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *row, GLvoid *column)
#else
void CR_APIENTRY glSeparableFilter2D( GLenum target, GLenum internalformat, GLsizei width,
													GLsizei height, GLenum format, GLenum type,
													const GLvoid *row, const GLvoid *column )
#endif
{
	(void) target;
	(void) internalformat;
	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) row;
	(void) column;
	crWarning("glSeparableFilter2D not implemented by Chromium");
}

void CR_APIENTRY glGetSeparableFilter( GLenum target, GLenum format, GLenum type,
													 GLvoid *row, GLvoid *column, GLvoid *span )
{
	(void) target;
	(void) format;
	(void) type;
	(void) row;
	(void) column;
	(void) span;
	crWarning("glGetSeparableFilter not implemented by Chromium");
}

void CR_APIENTRY glPointParameterfEXT( GLenum pname, GLfloat param )
{
	(void) pname;
	(void) param;
	crWarning("glPointparameterfEXT not implemented by Chromium");
}

void CR_APIENTRY glPointParameterfvEXT( GLenum pname, const GLfloat *param )
{
	(void) pname;
	(void) param;
	crWarning("glPointparameterfvEXT not implemented by Chromium");
}

#ifndef WINDOWS
/*
 * GL_EXT_vertex_array, just call the standard functions
 */

void glVertexPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr )
{
	(void) count;
	glVertexPointer( size, type, stride, ptr );
}

void glNormalPointerEXT( GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr )
{
	(void) count;
	glNormalPointer( type, stride, ptr );
}

void glColorPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr )
{
	(void) count;
	glColorPointer( size, type, stride, ptr );
}

void glIndexPointerEXT( GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr )
{
	(void) count;
	glIndexPointer( type, stride, ptr );
}

void glTexCoordPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr )
{
	(void) count;
	glTexCoordPointer( size, type, stride, ptr );
}

void glEdgeFlagPointerEXT( GLsizei stride, GLsizei count, const GLboolean *ptr )
{
	(void) count;
	glEdgeFlagPointer( stride, ptr );
}

void glGetPointervEXT( GLenum pname, void **params )
{
	glGetPointerv( pname, params );
}

void glArrayElementEXT( GLint i )
{
	glArrayElement( i );
}

void glDrawArraysEXT( GLenum mode, GLint first, GLsizei count )
{
	glDrawArrays( mode, first, count );
}

/*
 * GL_EXT_texture_object, just call the standard functions
 */

#if 0
void glGenTexturesEXT(GLsizei n, GLuint *textures)
{
	glGenTextures(n, textures);
}

void glDeleteTexturesEXT(GLsizei n, const GLuint *textures)
{
	glDeleteTextures(n, textures);
}

void glBindTextureEXT(GLenum target, GLuint texture)
{
	glBindTexture(target, texture);
}

void glPrioritizeTexturesEXT(GLsizei n, const GLuint *textures,
														 const GLclampf *priorities)
{
	glPrioritizeTextures(n, textures, priorities);
}

GLboolean glAreTexturesResidentEXT(GLsizei n, const GLuint *textures,
                                   GLboolean *residences)
{
	return glAreTexturesResident(n, textures, residences);
}

GLboolean glIsTextureEXT(GLuint texture)
{
	return glIsTexture(texture);
}
#endif

#endif
