#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"

static GLenum __sizeQuery( GLenum map )
{
	switch( map )
	{
		case GL_PIXEL_MAP_I_TO_I: return GL_PIXEL_MAP_I_TO_I_SIZE;
		case GL_PIXEL_MAP_S_TO_S: return GL_PIXEL_MAP_S_TO_S_SIZE;
		case GL_PIXEL_MAP_I_TO_R: return GL_PIXEL_MAP_I_TO_R_SIZE;
		case GL_PIXEL_MAP_I_TO_G: return GL_PIXEL_MAP_I_TO_G_SIZE;
		case GL_PIXEL_MAP_I_TO_B: return GL_PIXEL_MAP_I_TO_B_SIZE;
		case GL_PIXEL_MAP_I_TO_A: return GL_PIXEL_MAP_I_TO_A_SIZE;
		case GL_PIXEL_MAP_R_TO_R: return GL_PIXEL_MAP_R_TO_R_SIZE;
		case GL_PIXEL_MAP_G_TO_G: return GL_PIXEL_MAP_G_TO_G_SIZE;
		case GL_PIXEL_MAP_B_TO_B: return GL_PIXEL_MAP_B_TO_B_SIZE;
		case GL_PIXEL_MAP_A_TO_A: return GL_PIXEL_MAP_A_TO_A_SIZE;
		default: crError( "Bad map in crServerDispatchGetPixelMap: %d", map );
	}
	return 0;
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapfv( GLenum map, GLfloat *values )
{
	int size = sizeof( GLfloat );
	int tabsize = __sizeQuery( map );
	GLfloat *local_values;

	(void) values;

	size *= tabsize;
	local_values = crAlloc( size );

	cr_server.head_spu->dispatch_table.GetPixelMapfv( map, local_values );
	crServerReturnValue( local_values, size );
	crFree( local_values );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapuiv( GLenum map, GLuint *values )
{
	int size = sizeof( GLuint );
	int tabsize = __sizeQuery( map );
	GLuint *local_values;
	(void) values;

	size *= tabsize;
	local_values = crAlloc( size );

	cr_server.head_spu->dispatch_table.GetPixelMapuiv( map, local_values );
	crServerReturnValue( local_values, size );
	crFree( local_values );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetPixelMapusv( GLenum map, GLushort *values )
{
	int size = sizeof( GLushort );
	int tabsize = __sizeQuery( map );
	GLushort *local_values;
	(void) values;

	size *= tabsize;
	local_values = crAlloc( size );

	cr_server.head_spu->dispatch_table.GetPixelMapusv( map, local_values );
	crServerReturnValue( local_values, size );
	crFree( local_values );
}
