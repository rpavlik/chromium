/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_opcodes.h"
#include "cr_glwrapper.h"
#include "cr_mem.h"
#include "cr_string.h"


#define DISPLAY_NAME_LEN 256

#define WRITE_BYTES( offset, data, len ) \
  crMemcpy( data_ptr + (offset), data, len )

void PACK_APIENTRY crPackCreateContext( const char *dpyName, GLint visual, GLint *return_value, int *writeback )
{
	char displayName[DISPLAY_NAME_LEN];
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	if (dpyName) {
		crStrncpy( displayName, dpyName, DISPLAY_NAME_LEN );
		displayName[DISPLAY_NAME_LEN - 1] = 0;
	}
	else {
		displayName[0] = 0;
	}

	GET_BUFFERED_POINTER(pc, DISPLAY_NAME_LEN + 28 );
	WRITE_DATA( 0, GLint, DISPLAY_NAME_LEN + 28 );
	WRITE_DATA( 4, GLenum, CR_CREATECONTEXT_EXTEND_OPCODE );
	WRITE_BYTES( 8, displayName, DISPLAY_NAME_LEN );
	WRITE_DATA( DISPLAY_NAME_LEN + 8, GLint, visual );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 12, (void *) return_value );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 20, (void *) writeback );
	WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}

void PACK_APIENTRY crPackCreateContextSWAP( const char *dpyName, GLint visual, GLint *return_value, int *writeback )
{
	char displayName[DISPLAY_NAME_LEN];
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	if (dpyName) {
		crStrncpy( displayName, dpyName, DISPLAY_NAME_LEN );
		displayName[DISPLAY_NAME_LEN - 1] = 0;
	}
	else {
		displayName[0] = 0;
	}

	GET_BUFFERED_POINTER(pc, DISPLAY_NAME_LEN + 28 );
	WRITE_DATA( 0, GLint, SWAP32(28) );
	WRITE_DATA( 4, GLenum, SWAP32(CR_CREATECONTEXT_EXTEND_OPCODE) );
	WRITE_BYTES( 8, displayName, DISPLAY_NAME_LEN );
	WRITE_DATA( DISPLAY_NAME_LEN + 8, GLenum, SWAP32(visual) );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 12, (void *) return_value );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 20, (void *) writeback );
	WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}


void PACK_APIENTRY crPackcrCreateWindow( const char *dpyName, GLint visBits, GLint *return_value, int *writeback )
{
	char displayName[DISPLAY_NAME_LEN];
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	if (dpyName) {
		crStrncpy( displayName, dpyName, DISPLAY_NAME_LEN );
		displayName[DISPLAY_NAME_LEN - 1] = 0;
	}
	else {
		displayName[0] = 0;
	}

	GET_BUFFERED_POINTER(pc, DISPLAY_NAME_LEN + 28 );
	WRITE_DATA( 0, GLint, 28 );
	WRITE_DATA( 4, GLenum, CR_CRCREATEWINDOW_EXTEND_OPCODE );
	WRITE_BYTES( 8, displayName, DISPLAY_NAME_LEN );
	WRITE_DATA( DISPLAY_NAME_LEN + 8, GLint, visBits );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 12, (void *) return_value );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 20, (void *) writeback );
	WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}

void PACK_APIENTRY crPackcrCreateWindowSWAP( const char *dpyName, GLint visBits, GLint *return_value, int *writeback )
{
	char displayName[DISPLAY_NAME_LEN];
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	if (dpyName) {
		crStrncpy( displayName, dpyName, DISPLAY_NAME_LEN );
		displayName[DISPLAY_NAME_LEN - 1] = 0;
	}
	else {
		displayName[0] = 0;
	}

	GET_BUFFERED_POINTER(pc, DISPLAY_NAME_LEN + 28 );
	WRITE_DATA( 0, GLint, SWAP32(28) );
	WRITE_DATA( 4, GLenum, SWAP32(CR_CRCREATEWINDOW_EXTEND_OPCODE) );
	WRITE_BYTES( 8, displayName, DISPLAY_NAME_LEN );
	WRITE_DATA( DISPLAY_NAME_LEN + 8, GLenum, SWAP32(visBits) );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 12, (void *) return_value );
	WRITE_NETWORK_POINTER( DISPLAY_NAME_LEN + 20, (void *) writeback );
	WRITE_OPCODE( pc, CR_EXTEND_OPCODE );
}
