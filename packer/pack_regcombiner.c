/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_opcodes.h"

static GLboolean __handleCombinerParameterData( GLenum pname, const GLfloat *params, GLenum extended_opcode )
{
	GET_PACKER_CONTEXT(pc);
	int params_length = 0;
	int packet_length = sizeof( int ) + sizeof( extended_opcode ) + sizeof( pname );
	unsigned char *data_ptr;

	switch( pname )
	{
		case GL_CONSTANT_COLOR0_NV:
		case GL_CONSTANT_COLOR1_NV:
			params_length = 4*sizeof( *params );
			break;
		case GL_NUM_GENERAL_COMBINERS_NV:
		case GL_COLOR_SUM_CLAMP_NV:
			params_length = sizeof( *params );
			break;
		default:
			__PackError( __LINE__, __FILE__, GL_INVALID_ENUM,
									 "crPackCombinerParameter(bad pname)" );
			return GL_FALSE;
	}
	packet_length += params_length;
	GET_BUFFERED_POINTER(pc, packet_length );
	WRITE_DATA( 0, int, packet_length );
	WRITE_DATA( sizeof( int ) + 0, GLenum, extended_opcode );
	WRITE_DATA( sizeof( int ) + 4, GLenum, pname );
	WRITE_DATA( sizeof( int ) + 8, GLfloat, params[0] );
	if (params_length > sizeof( *params ))
	{
		WRITE_DATA( sizeof( int ) + 12, GLfloat, params[1] );
		WRITE_DATA( sizeof( int ) + 16, GLfloat, params[2] );
		WRITE_DATA( sizeof( int ) + 20, GLfloat, params[3] );
	}
	return GL_TRUE;
}

void PACK_APIENTRY crPackCombinerParameterfvNV( GLenum pname, const GLfloat *params )
{
	GET_PACKER_CONTEXT(pc);
	if (__handleCombinerParameterData( pname, params, CR_COMBINERPARAMETERFVNV_EXTEND_OPCODE ))
		WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}

void PACK_APIENTRY crPackCombinerParameterivNV( GLenum pname, const GLint *params )
{
	/* floats and ints are the same size, so the packing should be the same */
	GET_PACKER_CONTEXT(pc);
	if (__handleCombinerParameterData( pname, (const GLfloat *) params, CR_COMBINERPARAMETERIVNV_EXTEND_OPCODE ))
		WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}

void PACK_APIENTRY crPackCombinerStageParameterfvNV( GLenum stage, GLenum pname, const GLfloat *params )
{
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	GET_BUFFERED_POINTER(pc, 28 );
	WRITE_DATA( 0, int, 28 );
	WRITE_DATA( sizeof( int ) + 0, GLenum, CR_COMBINERSTAGEPARAMETERFVNV_EXTEND_OPCODE );
	WRITE_DATA( sizeof( int ) + 4, GLenum, stage );
	WRITE_DATA( sizeof( int ) + 8, GLenum, pname );
	WRITE_DATA( sizeof( int ) + 12, GLfloat, params[0] );
	WRITE_DATA( sizeof( int ) + 16, GLfloat, params[1] );
	WRITE_DATA( sizeof( int ) + 20, GLfloat, params[2] );
	WRITE_DATA( sizeof( int ) + 24, GLfloat, params[3] );
	WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}
