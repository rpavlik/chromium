/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_opcodes.h"

void PACK_APIENTRY crPackRectdv( const GLdouble *v1, const GLdouble *v2 )
{
	unsigned char *data_ptr;
	GET_PACKER_CONTEXT(pc);
	GET_BUFFERED_POINTER(pc, 32 );
	WRITE_DOUBLE( 0, v1[0] );
	WRITE_DOUBLE( 8, v1[1] );
	WRITE_DOUBLE( 16, v2[0] );
	WRITE_DOUBLE( 24, v2[1] );
	WRITE_OPCODE( pc, CR_RECTD_OPCODE );
}

void PACK_APIENTRY crPackRectfv( const GLfloat *v1, const GLfloat *v2 )
{
	unsigned char *data_ptr;
	GET_PACKER_CONTEXT(pc);
	GET_BUFFERED_POINTER(pc, 16 );
	WRITE_DATA( 0, GLfloat, v1[0] );
	WRITE_DATA( 4, GLfloat, v1[1] );
	WRITE_DATA( 8, GLfloat, v2[0] );
	WRITE_DATA( 12, GLfloat, v2[1] );
	WRITE_OPCODE( pc, CR_RECTF_OPCODE );
}

void PACK_APIENTRY crPackRectiv( const GLint *v1, const GLint *v2 )
{
	unsigned char *data_ptr;
	GET_PACKER_CONTEXT(pc);
	GET_BUFFERED_POINTER(pc, 16 );
	WRITE_DATA( 0, GLint, v1[0] );
	WRITE_DATA( 4, GLint, v1[1] );
	WRITE_DATA( 8, GLint, v2[0] );
	WRITE_DATA( 12, GLint, v2[1] );
	WRITE_OPCODE( pc, CR_RECTI_OPCODE );
}

void PACK_APIENTRY crPackRectsv( const GLshort *v1, const GLshort *v2 )
{
	unsigned char *data_ptr;
	GET_PACKER_CONTEXT(pc);
	GET_BUFFERED_POINTER(pc, 8 );
	WRITE_DATA( 0, GLshort, v1[0] );
	WRITE_DATA( 2, GLshort, v1[1]);
	WRITE_DATA( 4, GLshort, v2[0] );
	WRITE_DATA( 6, GLshort, v2[1] );
	WRITE_OPCODE( pc, CR_RECTS_OPCODE );
}	
