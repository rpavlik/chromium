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


#define GL_GLEXT_PROTOTYPES
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "api_templates.h"

void APIENTRY glDrawRangeElements( GLenum mode, GLuint start, GLuint end,
													GLsizei count, GLenum type, const GLvoid *indices )
{
	(void) mode;
	(void) start;
	(void) end;
	(void) count;
	(void) type;
	(void) indices;
	crWarning("glDrawRangeElements not implemented by Chromium");
}

void APIENTRY glTexImage3D( GLenum target, GLint level, GLint internalFormat,
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

void APIENTRY glTexSubImage3D( GLenum target, GLint level,
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

void APIENTRY glCopyTexSubImage3D( GLenum target, GLint level,
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

void APIENTRY glColorTable( GLenum target, GLenum internalformat,
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

void APIENTRY glColorSubTable( GLenum target, GLsizei start, GLsizei count,
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

void APIENTRY glColorTableParameteriv(GLenum target, GLenum pname,
														 const GLint *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameteriv not implemented by Chromium");
}

void APIENTRY glColorTableParameterfv(GLenum target, GLenum pname,
														 const GLfloat *params)
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glColorTableParameterfv not implemented by Chromium");
}

void APIENTRY glCopyColorSubTable( GLenum target, GLsizei start,
													GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) start;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorSubTable not implemented by Chromium");
}

void APIENTRY glCopyColorTable( GLenum target, GLenum internalformat,
											 GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyColorTable not implemented by Chromium");
}

void APIENTRY glGetColorTable( GLenum target, GLenum format,
											GLenum type, GLvoid *table )
{
	(void) target;
	(void) format;
	(void) type;
	(void) table;
	crWarning("glGetColorTable not implemented by Chromium");
}

void APIENTRY glGetColorTableParameterfv( GLenum target, GLenum pname,
																 GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("id glGetColorTableParameterfv not implemented by Chromium");
}

void APIENTRY glGetColorTableParameteriv( GLenum target, GLenum pname,
																 GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetColorTableParameteriv not implemented by Chromium");
}

void APIENTRY glBlendEquation( GLenum mode )
{
	glim.BlendEquationEXT(mode);
}

void APIENTRY glBlendColor( GLclampf red, GLclampf green,
									 GLclampf blue, GLclampf alpha )
{
	(void) red;
	(void) green;
	(void) blue;
	(void) alpha;
	crWarning("glBlendColor not implemented by Chromium");
}

void APIENTRY glHistogram( GLenum target, GLsizei width,
									GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) width;
	(void) internalformat;
	(void) sink;
	crWarning("id glHistogram not implemented by Chromium");
}

void APIENTRY glResetHistogram( GLenum target )
{
	(void) target;
	crWarning("glResetHistogram not implemented by Chromium");
}

void APIENTRY glGetHistogram( GLenum target, GLboolean reset,
										 GLenum format, GLenum type, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) type;
	(void) values;
	crWarning("glGetHistogram not implemented by Chromium");
}

void APIENTRY glGetHistogramParameterfv( GLenum target, GLenum pname,
																GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameterfv not implemented by Chromium");
}

void APIENTRY glGetHistogramParameteriv( GLenum target, GLenum pname,
																GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetHistogramParameteriv not implemented by Chromium");
}

void APIENTRY glMinmax( GLenum target, GLenum internalformat, GLboolean sink )
{
	(void) target;
	(void) internalformat;
	(void) sink;
	crWarning("glMinmax not implemented by Chromium");
}

void APIENTRY glResetMinmax( GLenum target )
{
	(void) target;
	crWarning("glResetMinmax not implemented by Chromium");
}

void APIENTRY glGetMinmax( GLenum target, GLboolean reset,
									GLenum format, GLenum types, GLvoid *values )
{
	(void) target;
	(void) reset;
	(void) format;
	(void) types;
	(void) values;
	crWarning("glGetMinmax not implemented by Chromium");
}

void APIENTRY glGetMinmaxParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameterfv not implemented by Chromium");
}

void APIENTRY glGetMinmaxParameteriv( GLenum target, GLenum pname, GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetMinmaxParameteriv not implemented by Chromium");
}

void APIENTRY glConvolutionFilter1D( GLenum target, GLenum internalformat,
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

void APIENTRY glConvolutionFilter2D( GLenum target, GLenum internalformat,
														GLsizei width, GLsizei height, GLenum format,
														GLenum type, const GLvoid *image )
{
	(void) target;
	(void) internalformat;
	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) image;
	crWarning("glConvolutionFilter2D not implemented by Chromium");
}

void APIENTRY glConvolutionParameterf( GLenum target, GLenum pname, GLfloat params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterf not implemented by Chromium");
}

void APIENTRY glConvolutionParameterfv( GLenum target, GLenum pname,
															 const GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameterfv not implemented by Chromium");
}

void APIENTRY glConvolutionParameteri( GLenum target, GLenum pname, GLint params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteri not implemented by Chromium");
}

void APIENTRY glConvolutionParameteriv( GLenum target, GLenum pname,
															 const GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glConvolutionParameteriv not implemented by Chromium");
}

void APIENTRY glCopyConvolutionFilter1D( GLenum target, GLenum internalformat,
																GLint x, GLint y, GLsizei width )
{
	(void) target;
	(void) internalformat;
	(void) x;
	(void) y;
	(void) width;
	crWarning("glCopyConvolutionFilter1D not implemented by Chromium");
}

void APIENTRY glCopyConvolutionFilter2D( GLenum target, GLenum internalformat,
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

void APIENTRY glGetConvolutionFilter( GLenum target, GLenum format,
														 GLenum type, GLvoid *image )
{
	(void) target;
	(void) format;
	(void) type;
	(void) image;
	crWarning("glGetConvolutionFilter not implemented by Chromium");
}

void APIENTRY glGetConvolutionParameterfv( GLenum target, GLenum pname,
																	GLfloat *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameterfv not implemented by Chromium");
}

void APIENTRY glGetConvolutionParameteriv( GLenum target, GLenum pname,
																	GLint *params )
{
	(void) target;
	(void) pname;
	(void) params;
	crWarning("glGetConvolutionParameteriv not implemented by Chromium");
}

void APIENTRY glSeparableFilter2D( GLenum target, GLenum internalformat, GLsizei width,
													GLsizei height, GLenum format, GLenum type,
													const GLvoid *row, const GLvoid *column )
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

void APIENTRY glGetSeparableFilter( GLenum target, GLenum format, GLenum type,
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

void APIENTRY glPointParameterfEXT( GLenum pname, GLfloat param )
{
	(void) pname;
	(void) param;
	crWarning("glPointparameterfEXT not implemented by Chromium");
}

void APIENTRY glPointParameterfvEXT( GLenum pname, const GLfloat *param )
{
	(void) pname;
	(void) param;
	crWarning("glPointparameterfvEXT not implemented by Chromium");
}

