/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "unpacker.h"
#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_mem.h"
#include "cr_version.h"


void crUnpackExtendProgramParameter4dvNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint index = READ_DATA( 12, GLuint );
	GLdouble params[4];
	params[0] = READ_DOUBLE( 16 );
	params[1] = READ_DOUBLE( 24 );
	params[2] = READ_DOUBLE( 32 );
	params[3] = READ_DOUBLE( 40 );
	cr_unpackDispatch.ProgramParameter4dvNV( target, index, params );
}


void crUnpackExtendProgramParameter4fvNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint index = READ_DATA( 12, GLuint );
	GLfloat params[4];
	params[0] = READ_DATA( 16, GLfloat );
	params[1] = READ_DATA( 20, GLfloat );
	params[2] = READ_DATA( 24, GLfloat );
	params[3] = READ_DATA( 28, GLfloat );
	cr_unpackDispatch.ProgramParameter4fvNV( target, index, params );
}


void crUnpackExtendProgramParameters4dvNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint index = READ_DATA( 12, GLuint );
	GLuint num = READ_DATA( 16, GLuint);
	GLdouble *params = (GLdouble *) crAlloc(num * 4 * sizeof(GLdouble));
	if (params) {
		GLuint i;
		for (i = 0; i < 4 * num; i++) {
			params[i] = READ_DATA( 20 + i * 8, GLdouble );
		}
		cr_unpackDispatch.ProgramParameters4dvNV( target, index, num, params );
		crFree(params);
	}
}


void crUnpackExtendProgramParameters4fvNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint index = READ_DATA( 12, GLuint );
	GLuint num = READ_DATA( 16, GLuint);
	GLfloat *params = (GLfloat *) crAlloc(num * 4 * sizeof(GLfloat));
	if (params) {
		GLuint i;
		for (i = 0; i < 4 * num; i++) {
			params[i] = READ_DATA( 20 + i * 4, GLfloat );
		}
		cr_unpackDispatch.ProgramParameters4fvNV( target, index, num, params );
		crFree(params);
	}
}


void crUnpackExtendAreProgramsResidentNV(void)
{
	GLsizei n = READ_DATA( 8, GLsizei );
	crError( "AreProgramsResidentNV needs to be special cased!" );
	crError( "AreProgramsResidentNV needs to be special cased!" );
	SET_RETURN_PTR( 28 );
	SET_WRITEBACK_PTR( 36 );
	(void) cr_unpackDispatch.AreProgramsResidentNV( n, NULL, NULL );
}


void crUnpackExtendLoadProgramNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint id = READ_DATA( 12, GLuint );
	GLsizei len = READ_DATA( 16, GLsizei );
	crError( "LoadProgramNV needs to be special cased!" );
	cr_unpackDispatch.LoadProgramNV( target, id, len, NULL );
}


void crUnpackExtendDeleteProgramsNV(void)
{
	GLsizei n = READ_DATA( 8, GLsizei );
	crError( "DeleteProgramsNV needs to be special cased!" );
	cr_unpackDispatch.DeleteProgramsNV( n, NULL );
}


void crUnpackExtendExecuteProgramNV(void)
{
	GLenum target = READ_DATA( 8, GLenum );
	GLuint id = READ_DATA( 12, GLuint );
	crError( "ExecuteProgramNV needs to be special cased!" );
	cr_unpackDispatch.ExecuteProgramNV( target, id, NULL );
}

void crUnpackExtendRequestResidentProgramsNV(void)
{
	GLsizei n = READ_DATA( 8, GLsizei );
	crError( "RequestResidentProgramsNV needs to be special cased!" );
	cr_unpackDispatch.RequestResidentProgramsNV( n, NULL );
}
