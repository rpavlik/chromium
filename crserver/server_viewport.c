#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

static const GLmatrix identity_matrix = { 
	(GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0
};

void crServerClampViewport( int x, int y, unsigned int width, unsigned int height,
		int *server_x, int *server_y, unsigned int *server_width, unsigned int *server_height, int extent )
{
	*server_x = x - cr_server.x1[extent];
	*server_y = y - cr_server.y1[extent];
	*server_width = width;
	*server_height = height;
	if (*server_x < 0)
	{
		*server_x = 0;
	}
	if (*server_y < 0)
	{
		*server_y = 0;
	}
	if (*server_x + width > (unsigned int) (cr_server.x2[extent] - cr_server.x1[extent]))
	{
		*server_width = cr_server.x2[extent] - cr_server.x1[extent] - *server_x;
	}
	if (*server_y + height > (unsigned int) (cr_server.y2[extent] - cr_server.y1[extent]))
	{
		*server_height = cr_server.y2[extent] - cr_server.y1[extent] - *server_y;
	}
}

void crServerApplyBaseProjection(void)
{
	cr_server.head_spu->dispatch_table.PushAttrib( GL_TRANSFORM_BIT );
	cr_server.head_spu->dispatch_table.MatrixMode( GL_PROJECTION );
	cr_server.head_spu->dispatch_table.LoadMatrixf( (GLfloat *) &(cr_server.current_client->baseProjection) );
	cr_server.head_spu->dispatch_table.MultMatrixf( (GLfloat *) (cr_server.current_client->ctx->transform.projection + cr_server.current_client->ctx->transform.projectionDepth) );
	cr_server.head_spu->dispatch_table.PopAttrib( );
}

void crServerRecomputeBaseProjection(GLmatrix *base) 
{
	GLfloat xscale, yscale;
	GLfloat xtrans, ytrans;

	GLrectf p;

	p.x1 = ((GLfloat) (cr_server.x1[cr_server.current_extent])) / cr_server.muralWidth;
	p.x2 = ((GLfloat) (cr_server.x2[cr_server.current_extent])) / cr_server.muralWidth;
	p.y1 = ((GLfloat) (cr_server.y1[cr_server.current_extent])) / cr_server.muralHeight;
	p.y2 = ((GLfloat) (cr_server.y2[cr_server.current_extent])) / cr_server.muralHeight;
	
	/* Rescale [0,1] -> [-1,1] */
	p.x1 = p.x1*2.0f - 1.0f;
	p.x2 = p.x2*2.0f - 1.0f;
	p.y1 = p.y1*2.0f - 1.0f;
	p.y2 = p.y2*2.0f - 1.0f;

	xscale = 2.0f / (p.x2 - p.x1);
	yscale = 2.0f / (p.y2 - p.y1);
	xtrans = -(p.x2 + p.x1) / 2.0f;
	ytrans = -(p.y2 + p.y1) / 2.0f;

	*base = identity_matrix;
	base->m00 = xscale;
	base->m11 = yscale;
	base->m30 = xtrans*xscale;
	base->m31 = ytrans*yscale;
}

void SERVER_DISPATCH_APIENTRY crServerDispatchViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	if (cr_server.num_extents == 0)
	{
		cr_server.head_spu->dispatch_table.Viewport( x, y, width, height );
	}
	else if (cr_server.num_extents == 1)
	{
		int server_x, server_y;
		unsigned int server_width, server_height;

		crServerClampViewport( x, y, width, height, &server_x, &server_y, &server_width, &server_height, 0 );
		crStateViewport( server_x, server_y, server_width, server_height );
		cr_server.head_spu->dispatch_table.Viewport( server_x, server_y, server_width, server_height );
		crServerRecomputeBaseProjection( &(cr_server.current_client->baseProjection) );
		crServerApplyBaseProjection();
	}
	else
	{
		crError( "This just isn't supported yet!" );
	}
}
