#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
	GLdouble *coeffs = NULL;
	GLdouble order[2];
	GLint temporder[2];
	GLdouble domain[4];
	GLdouble *retptr = NULL;
	int dimension = 0;
	unsigned int size = sizeof(GLdouble);

	switch( target )
	{
		case GL_MAP1_COLOR_4:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
			dimension = 1; 
			break;
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4: 
			dimension = 2;
			break;
		default:
			crError( "Bad target in crServerDispatchGetMapdv: %d", target );
			break;
	}

	switch(query)
	{
		case GL_ORDER:
			cr_server.head_spu->dispatch_table.GetMapdv( target, query, order );
			retptr = &(order[0]);
			size *= dimension;
			break;
		case GL_DOMAIN:
			cr_server.head_spu->dispatch_table.GetMapdv( target, query, domain );
			retptr = &(domain[0]);
			size *= dimension*2;
			break;
		case GL_COEFF:
			cr_server.head_spu->dispatch_table.GetMapiv( target, GL_ORDER, temporder );
			size *= temporder[0];
			if (dimension)
				size *= temporder[1];
			coeffs = crAlloc( size );
			cr_server.head_spu->dispatch_table.GetMapdv( target, query, coeffs );
			retptr = coeffs;
			break;
	}
			
	crServerReturnValue( retptr, size );
	if (query == GL_COEFF)
	{
		crFree( coeffs );
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
	GLfloat *coeffs = NULL;
	GLfloat order[2];
	GLint temporder[2];
	GLfloat domain[4];
	GLfloat *retptr = NULL;
	int dimension = 0;
	unsigned int size = sizeof(GLfloat);

	switch( target )
	{
		case GL_MAP1_COLOR_4:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
			dimension = 1; 
			break;
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4: 
			dimension = 2;
			break;
		default:
			crError( "Bad target in crServerDispatchGetMapfv: %d", target );
			break;
	}

	switch(query)
	{
		case GL_ORDER:
			cr_server.head_spu->dispatch_table.GetMapfv( target, query, order );
			retptr = &(order[0]);
			size *= dimension;
			break;
		case GL_DOMAIN:
			cr_server.head_spu->dispatch_table.GetMapfv( target, query, domain );
			retptr = &(domain[0]);
			size *= dimension*2;
			break;
		case GL_COEFF:
			cr_server.head_spu->dispatch_table.GetMapiv( target, GL_ORDER, temporder );
			size *= temporder[0];
			if (dimension)
				size *= temporder[1];
			coeffs = crAlloc( size );
			cr_server.head_spu->dispatch_table.GetMapfv( target, query, coeffs );
			retptr = coeffs;
			break;
	}
			
	crServerReturnValue( retptr, size );
	if (query == GL_COEFF)
	{
		crFree( coeffs );
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchGetMapiv( GLenum target, GLenum query, GLint *v )
{
	GLint *coeffs = NULL;
	GLint order[2];
	GLint temporder[2];
	GLint domain[4];
	GLint *retptr = NULL;
	int dimension = 0;
	unsigned int size = sizeof(GLint);

	switch( target )
	{
		case GL_MAP1_COLOR_4:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
			dimension = 1; 
			break;
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4: 
			dimension = 2;
			break;
		default:
			crError( "Bad target in crServerDispatchGetMapiv: %d", target );
			break;
	}

	switch(query)
	{
		case GL_ORDER:
			cr_server.head_spu->dispatch_table.GetMapiv( target, query, order );
			retptr = &(order[0]);
			size *= dimension;
			break;
		case GL_DOMAIN:
			cr_server.head_spu->dispatch_table.GetMapiv( target, query, domain );
			retptr = &(domain[0]);
			size *= dimension*2;
			break;
		case GL_COEFF:
			cr_server.head_spu->dispatch_table.GetMapiv( target, GL_ORDER, temporder );
			size *= temporder[0];
			if (dimension)
				size *= temporder[1];
			coeffs = crAlloc( size );
			cr_server.head_spu->dispatch_table.GetMapiv( target, query, coeffs );
			retptr = coeffs;
			break;
	}
			
	crServerReturnValue( retptr, size );
	if (query == GL_COEFF)
	{
		crFree( coeffs );
	}
}

