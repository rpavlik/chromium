/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * Packer functions for GL_NV_vertex_program extension.
 * XXX: Quite a few of these functions are unfinished.
 */


#include "packer.h"
#include "cr_error.h"


void PACK_APIENTRY crPackProgramParameters4dvNV (GLenum target, GLuint index, GLuint num, const GLdouble * params)
{
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;
	int packet_length = sizeof(int) + sizeof(target) + sizeof(index) + sizeof(num) + num * 4 * sizeof(GLdouble);

	GET_BUFFERED_POINTER(pc, packet_length);
	WRITE_DATA(0, int, packet_length);
	WRITE_DATA(sizeof(int) + 0, GLenum, target);
	WRITE_DATA(sizeof(int) + 4, GLuint, index);
	WRITE_DATA(sizeof(int) + 8, GLuint, num);
	crMemcpy(data_ptr + sizeof(int) + 12, params, num * 4 * sizeof(GLdouble));

	WRITE_OPCODE(pc, CR_PROGRAMPARAMETERS4DVNV_EXTEND_OPCODE);
}


void PACK_APIENTRY crPackProgramParameters4fvNV (GLenum target, GLuint index, GLuint num, const GLfloat * params)
{
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;
	int packet_length = sizeof(int) + sizeof(target) + sizeof(index) + sizeof(num) + num * 4 * sizeof(GLfloat);

	GET_BUFFERED_POINTER(pc, packet_length);
	WRITE_DATA(0, int, packet_length);
	WRITE_DATA(sizeof(int) + 0, GLenum, target);
	WRITE_DATA(sizeof(int) + 4, GLuint, index);
	WRITE_DATA(sizeof(int) + 8, GLuint, num);
	crMemcpy(data_ptr + sizeof(int) + 12, params, num * 4 * sizeof(GLfloat));

	WRITE_OPCODE(pc, CR_PROGRAMPARAMETERS4FVNV_EXTEND_OPCODE);
}


void PACK_APIENTRY crPackVertexAttribPointerNV( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	GET_PACKER_CONTEXT(pc);
	crError ( "VertexAttribPointerNV needs to be special cased!");
	(void) pc;
	(void) index;
	(void) size;
	(void) type;
	(void) stride;
	(void) pointer;
}


void PACK_APIENTRY crPackVertexAttribs1dvNV( GLuint index, GLsizei n, const GLdouble *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib1dvNV(index + i, v + i);
}


void PACK_APIENTRY crPackVertexAttribs1fvNV( GLuint index, GLsizei n, const GLfloat *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib1fvNV(index + i, v + i);
}


void PACK_APIENTRY crPackVertexAttribs1svNV( GLuint index, GLsizei n, const GLshort *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib1svNV(index + i, v + i);
}


void PACK_APIENTRY crPackVertexAttribs2dvNV( GLuint index, GLsizei n, const GLdouble *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib2dvNV(index + i, v + 2 * i);
}

void PACK_APIENTRY crPackVertexAttribs2fvNV( GLuint index, GLsizei n, const GLfloat *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib2fvNV(index + i, v + 2 * i);
}

void PACK_APIENTRY crPackVertexAttribs2svNV( GLuint index, GLsizei n, const GLshort *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib2svNV(index + i, v + 2 * i);
}

void PACK_APIENTRY crPackVertexAttribs3dvNV( GLuint index, GLsizei n, const GLdouble *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib3dvNV(index + i, v + 3 * i);
}

void PACK_APIENTRY crPackVertexAttribs3fvNV( GLuint index, GLsizei n, const GLfloat *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib3fvNV(index + i, v + 3 * i);
}

void PACK_APIENTRY crPackVertexAttribs3svNV( GLuint index, GLsizei n, const GLshort *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib3svNV(index + i, v + 3 * i);
}

void PACK_APIENTRY crPackVertexAttribs4dvNV( GLuint index, GLsizei n, const GLdouble *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib4dvNV(index + i, v + 4 * i);
}

void PACK_APIENTRY crPackVertexAttribs4fvNV( GLuint index, GLsizei n, const GLfloat *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib4fvNV(index + i, v + 4 * i);
}

void PACK_APIENTRY crPackVertexAttribs4svNV( GLuint index, GLsizei n, const GLshort *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib4svNV(index + i, v + 4 * i);
}

void PACK_APIENTRY crPackVertexAttribs4ubvNV( GLuint index, GLsizei n, const GLubyte *v )
{
	GLint i;
	/* reverse order so we hit index 0 last (provoking glVertex) */
	for (i = n - 1; i >= 0; i--)
		crPackVertexAttrib4ubvNV(index + i, v + 4 * i);
}

void PACK_APIENTRY crPackDeleteProgramsNV( GLsizei n, const GLuint *ids )
{
	GET_PACKER_CONTEXT(pc);
	crError ( "DeleteProgramsNV needs to be special cased!");
	(void) pc;
	(void) n;
	(void) ids;
}

void PACK_APIENTRY crPackExecuteProgramNV( GLenum target, GLuint id, const GLfloat *params )
{
	GET_PACKER_CONTEXT(pc);
	crError ( "ExecuteProgramNV needs to be special cased!");
	(void) pc;
	(void) target;
	(void) id;
	(void) params;
}

void PACK_APIENTRY crPackLoadProgramNV( GLenum target, GLuint id, GLsizei len, const GLubyte *program )
{
	GET_PACKER_CONTEXT(pc);
	crError ( "LoadProgramNV needs to be special cased!");
	(void) pc;
	(void) target;
	(void) id;
	(void) len;
	(void) program;
}

void PACK_APIENTRY crPackRequestResidentProgramsNV( GLsizei n, const GLuint *ids )
{
	GET_PACKER_CONTEXT(pc);
	crError ( "RequestResidentProgramsNV needs to be special cased!");
	(void) pc;
	(void) n;
	(void) ids;
}
