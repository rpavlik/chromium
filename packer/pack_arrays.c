/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packer.h"
#include "cr_opcodes.h"
#include "cr_error.h"

#define UNUSED(x) ((void)(x))

void PACK_APIENTRY crPackVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackVertexPointer" );
}

void PACK_APIENTRY crPackColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackColorPointer" );
}

void PACK_APIENTRY crPackNormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackNormalPointer" );
}

void PACK_APIENTRY crPackTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackTexCoordPointer" );
}

void PACK_APIENTRY crPackEdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackEdgeFlagPointer" );
}

void PACK_APIENTRY crPackIndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackIndexPointer" );
}

void PACK_APIENTRY crPackSecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackSecondaryColorPointerEXT" );
}

void PACK_APIENTRY crPackInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( format );
	UNUSED( stride );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackInterleavedArrays" );
}


void PACK_APIENTRY crPackVertexArrayRangeNV( GLsizei length, const GLvoid *pointer )
{
	UNUSED( length );
	UNUSED( pointer );
	crWarning( "Unimplemented crPackVertexArrayRangeNV" );
}

