/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "api_templates.h"

/* This is necessary because IRIX disagrees with Windows about arg 2. */

void glEdgeFlagPointer( GLsizei stride, const GLboolean *pointer )
{
	glim.EdgeFlagPointer( stride, pointer );
}

/* Since OSF/1 and IRIX share exports_special, add these here */

void glTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexImage3D( target, level, (GLint) internalformat, width, height, depth, border, format, type, pixels );
}

void glProgramParameters4fvNV( GLenum target, GLuint index, GLuint num, const GLfloat *params )
{
  glim.ProgramParameters4fvNV( target, index, num, params );
}

void glProgramParameters4dvNV( GLenum target, GLuint index, GLuint num, const GLdouble *params )
{
  glim.ProgramParameters4dvNV( target, index, num, params );
}

