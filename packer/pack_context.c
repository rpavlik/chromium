/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_opcodes.h"
#include "cr_glwrapper.h"
#include <stdio.h>

void PACK_APIENTRY crPackCreateContext( void *arg1, GLint visual, GLint *return_value, int *writeback )
{
	unsigned char *data_ptr;
	(void) arg1;

	GET_BUFFERED_POINTER( 28 );
	WRITE_DATA( 0, GLint, 28 );
	WRITE_DATA( 4, GLenum, CR_CREATECONTEXT_EXTEND_OPCODE );
	WRITE_DATA( 8, GLint, visual );
	WRITE_NETWORK_POINTER( 12, (void *) return_value );
	WRITE_NETWORK_POINTER( 20, (void *) writeback );
	WRITE_OPCODE( CR_EXTEND_OPCODE );
}

void PACK_APIENTRY crPackCreateContextSWAP( void *arg1, GLint visual, GLint *return_value, int *writeback )
{
	unsigned char *data_ptr;
	(void) arg1;

	GET_BUFFERED_POINTER( 28 );
	WRITE_DATA( 0, GLint, SWAP32(28) );
	WRITE_DATA( 4, GLenum, SWAP32(CR_CREATECONTEXT_EXTEND_OPCODE) );
	WRITE_DATA( 8, GLenum, SWAP32(visual) );
	WRITE_NETWORK_POINTER( 12, (void *) return_value );
	WRITE_NETWORK_POINTER( 20, (void *) writeback );
	WRITE_OPCODE( CR_EXTEND_OPCODE );
}
