/* cpg - 5/22/02 */

#include "chromium.h"
#include "api_templates.h"

/* This is necessary because OSF1 disagrees with Windows about arg 2. */

void glEdgeFlagPointer( GLsizei stride, const GLboolean *pointer )
{
	glim.EdgeFlagPointer( stride, pointer );
}

void glTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexImage3D( target, level, (GLint)internalformat, width, height, depth, border, format, type, pixels );
}

void glProgramParameters4fvNV( GLenum target, GLuint index, GLsizei num, const GLfloat *params )
{
  glim.ProgramParameters4fvNV( target, index, (GLuint)num, params );
}

void glProgramParameters4dvNV( GLenum target, GLuint index, GLsizei num, const GLdouble *params )
{
  glim.ProgramParameters4dvNV( target, index, (GLuint)num, params );
}

