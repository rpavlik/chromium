#include "cr_pack.h"
#include "cr_opengl_types.h"
#include "cr_opcodes.h"
#include "cr_error.h"

#define UNUSED(x) ((void)(x))

void PACK_APIENTRY crPackVertexPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackColorPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackNormalPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackTexCoordPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackEdgeFlagPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackIndexPointer( GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer )
{
	UNUSED( size );
	UNUSED( type );
	UNUSED( stride );
	UNUSED( count );
	UNUSED( pointer );
	crError( "Unimplemented Vertex Array Pack Function called!" );
}

void PACK_APIENTRY crPackDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	UNUSED( mode );
	UNUSED( count );
	UNUSED( type );
	UNUSED( indices );
	crError( "Unimplemented DrawElements called!" );
}
