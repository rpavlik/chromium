#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


void SERVER_DISPATCH_APIENTRY crServerDispatchGenTextures( GLsizei n, GLuint *textures )
{
	GLuint *local_textures = (GLuint *) crAlloc( n*sizeof( *local_textures) );
	cr_server.head_spu->dispatch_table.GenTextures( n, local_textures );
	crDebug( "Server got a texture!  It was %d", local_textures[0] );
	crServerReturnValue( local_textures, n*sizeof( *local_textures ) );
	crFree( local_textures );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetClipPlane( GLenum plane, GLdouble *equation )
{
	crError( "crServerDispatchGetClipPlane unimplemented!" );
	cr_server.head_spu->dispatch_table.GetClipPlane( plane, equation );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetLightfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetLightfv( light, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetLightiv( GLenum light, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetLightiv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetLightiv( light, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
	crError( "crServerDispatchGetMapdv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetMapdv( target, query, v );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
	crError( "crServerDispatchGetMapfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetMapfv( target, query, v );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapiv( GLenum target, GLenum query, GLint *v )
{
	crError( "crServerDispatchGetMapiv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetMapiv( target, query, v );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetMaterialfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetMaterialfv( face, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetMaterialiv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetMaterialiv( face, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapfv( GLenum map, GLfloat *values )
{
	crError( "crServerDispatchGetPixelMapfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetPixelMapfv( map, values );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapuiv( GLenum map, GLuint *values )
{
	crError( "crServerDispatchGetPixelMapuiv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetPixelMapuiv( map, values );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapusv( GLenum map, GLushort *values )
{
	crError( "crServerDispatchGetPixelMapusv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetPixelMapusv( map, values );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetPointerv( GLenum pname, GLvoid* *params )
{
	crError( "crServerDispatchGetPointerv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetPointerv( pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetPolygonStipple( GLubyte *mask )
{
	crError( "crServerDispatchGetPolygonStipple unimplemented!" );
	cr_server.head_spu->dispatch_table.GetPolygonStipple( mask );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetTexEnvfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexEnvfv( target, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetTexEnviv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexEnviv( target, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	crError( "crServerDispatchGetTexGendv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexGendv( coord, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetTexGenfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexGenfv( coord, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetTexGeniv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexGeniv( coord, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels )
{
	crError( "crServerDispatchGetTexImage unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexImage( target, level, format, type, pixels );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetTexLevelParameterfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexLevelParameterfv( target, level, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetTexLevelParameteriv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexLevelParameteriv( target, level, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	crError( "crServerDispatchGetTexParameterfv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexParameterfv( target, pname, params );
}
void SERVER_DISPATCH_APIENTRY crServerDispatchGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
	crError( "crServerDispatchGetTexParameteriv unimplemented!" );
	cr_server.head_spu->dispatch_table.GetTexParameteriv( target, pname, params );
}
