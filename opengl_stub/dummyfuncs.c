/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * Eventually these functions should be properly implemented in Chromium.
 * For now this is a simple way to coax more OpenGL programs to run.
 *
 * Aliased functions, like glBlendFunc(), should be reimplemented in
 * assembly sometime.
 */


/*#define GL_GLEXT_PROTOTYPES*/
#include "cr_glwrapper.h"
#include "cr_error.h"

void glTexImage3D( GLenum target, GLint level,
#if defined(IRIX) || defined(IRIX64) || defined(AIX) || defined (SunOS) || defined(OSF1)
									 GLenum internalFormat,
#else
									 GLint internalFormat,
#endif
									 GLsizei width, GLsizei height, GLsizei depth,
									 GLint border, GLenum format, GLenum type,
									 const GLvoid *pixels )
{
	(void) target;
	(void) level;
	(void) internalFormat;
	(void) width;
	(void) height;
	(void) depth;
	(void) border;
	(void) format;
	(void) type;
	(void) pixels;
	crWarning("glTexImage3D not implemented by Chromium");
}

void glTexSubImage3D( GLenum target, GLint level,
											GLint xoffset, GLint yoffset,	GLint zoffset,
											GLsizei width, GLsizei height, GLsizei depth,
											GLenum format, GLenum type, const GLvoid *pixels)
{
	(void) target;
	(void) level;
	(void) xoffset;
	(void) yoffset;
	(void) zoffset;
	(void) width;
	(void) height;
	(void) depth;
	(void) format;
	(void) type;
	(void) pixels;
	crWarning("glTexSubImage3D not implemented by Chromium");
}

void glCopyTexSubImage3D( GLenum target, GLint level,
                          GLint xoffset, GLint yoffset,
                          GLint zoffset, GLint x, GLint y, GLsizei width,
                          GLsizei height )
{
	(void) target;
	(void) level;
	(void) xoffset;
	(void) yoffset;
	(void) zoffset;
	(void) x;
	(void) y;
	(void) width;
	(void) height;
	crWarning("glCopyTexSubImage3D not implemented by Chromium");
}

void glColorTable( GLenum target, GLenum internalformat,
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

void glColorSubTable( GLenum target, GLsizei start, GLsizei count,
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

void glColorTableParameteriv(GLenum target, GLenum pname,
														 const GLint *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameteriv not implemented by Chromium");
}

void glColorTableParameterfv(GLenum target, GLenum pname,
														 const GLfloat *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameterfv not implemented by Chromium");
}

void glCopyColorSubTable( GLenum target, GLsizei start,
													GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) start;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorSubTable not implemented by Chromium");
}

void glCopyColorTable( GLenum target, GLenum internalformat,
											 GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorTable not implemented by Chromium");
}

void glGetColorTable( GLenum target, GLenum format,
											GLenum type, GLvoid *table )
{
	(void) target;
	(void) format;
	(void) type;
	(void) table;
	crWarning("glGetColorTable not implemented by Chromium");
}

void glGetColorTableParameterfv( GLenum target, GLenum pname,
																 GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("id glGetColorTableParameterfv not implemented by Chromium");
}

void glGetColorTableParameteriv( GLenum target, GLenum pname,
																 GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetColorTableParameteriv not implemented by Chromium");
}

#ifndef WINDOWS
extern void glBlendEquationEXT(GLenum);

void glBlendEquation( GLenum mode )
{
	glBlendEquationEXT(mode);
}
#endif

void glBlendColor( GLclampf red, GLclampf green,
									 GLclampf blue, GLclampf alpha )
{
	(void) red;
	(void) green;
	(void) blue;
	(void) alpha;
	crWarning("glBlendColor not implemented by Chromium");
}

void glHistogram( GLenum target, GLsizei width,
									GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) width;
	(void) internalformat;
	(void) sink;
	crWarning("id glHistogram not implemented by Chromium");
}

void glResetHistogram( GLenum target )
{
	(void) target;
	crWarning("glResetHistogram not implemented by Chromium");
}

void glGetHistogram( GLenum target, GLboolean reset,
										 GLenum format, GLenum type, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) type;
	(void) values;
	crWarning("glGetHistogram not implemented by Chromium");
}

void glGetHistogramParameterfv( GLenum target, GLenum pname,
																GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameterfv not implemented by Chromium");
}

void glGetHistogramParameteriv( GLenum target, GLenum pname,
																GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameteriv not implemented by Chromium");
}

void glMinmax( GLenum target, GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) internalformat;
	(void) sink;
	crWarning("glMinmax not implemented by Chromium");
}

void glResetMinmax( GLenum target )
{
	(void) target;
	crWarning("glResetMinmax not implemented by Chromium");
}

void glGetMinmax( GLenum target, GLboolean reset,
									GLenum format, GLenum types, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) types;
	(void) values;
	crWarning("glGetMinmax not implemented by Chromium");
}

void glGetMinmaxParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameterfv not implemented by Chromium");
}

void glGetMinmaxParameteriv( GLenum target, GLenum pname, GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameteriv not implemented by Chromium");
}

void glConvolutionFilter1D( GLenum target, GLenum internalformat,
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

void glConvolutionFilter2D( GLenum target, GLenum internalformat,
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

void glConvolutionParameterf( GLenum target, GLenum pname, GLfloat params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterf not implemented by Chromium");
}

void glConvolutionParameterfv( GLenum target, GLenum pname,
															 const GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterfv not implemented by Chromium");
}

void glConvolutionParameteri( GLenum target, GLenum pname, GLint params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteri not implemented by Chromium");
}

void glConvolutionParameteriv( GLenum target, GLenum pname,
															 const GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteriv not implemented by Chromium");
}

void glCopyConvolutionFilter1D( GLenum target, GLenum internalformat,
																GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyConvolutionFilter1D not implemented by Chromium");
}

void glCopyConvolutionFilter2D( GLenum target, GLenum internalformat,
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

void glGetConvolutionFilter( GLenum target, GLenum format,
														 GLenum type, GLvoid *image )
{
	(void) target;
	(void) format;
	(void) type;
	(void) image;
	crWarning("glGetConvolutionFilter not implemented by Chromium");
}

void glGetConvolutionParameterfv( GLenum target, GLenum pname,
																	GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameterfv not implemented by Chromium");
}

void glGetConvolutionParameteriv( GLenum target, GLenum pname,
																	GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameteriv not implemented by Chromium");
}

#ifdef SunOS
void glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *row, GLvoid *column)
#else
void glSeparableFilter2D( GLenum target, GLenum internalformat, GLsizei width,
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

void glGetSeparableFilter( GLenum target, GLenum format, GLenum type,
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

void glPointParameterfEXT( GLenum pname, GLfloat param )
{
	(void) pname;
	(void) param;
	crWarning("glPointparameterfEXT not implemented by Chromium");
}

void glPointParameterfvEXT( GLenum pname, const GLfloat *param )
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
