#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_opcodes.h"
#include "cr_error.h"

#define UNUSED(x) ((void)(x))

void PACK_APIENTRY crPackVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackVertexPointer!" );
}

void PACK_APIENTRY crPackColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackColorPointer!" );
}

void PACK_APIENTRY crPackNormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackNormalPointer!" );
}

void PACK_APIENTRY crPackTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackTexCoordPointer!" );
}

void PACK_APIENTRY crPackEdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackEdgeFlagPointer!" );
}

void PACK_APIENTRY crPackIndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	UNUSED( type );
	UNUSED( stride );
	UNUSED( pointer );
	crError( "Unimplemented crPackIndexPointer!" );
}

void PACK_APIENTRY crPackDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	UNUSED( mode );
	UNUSED( count );
	UNUSED( type );
	UNUSED( indices );
	crError( "Unimplemented crPackDrawElements!" );
}
