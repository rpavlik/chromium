#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_ArrayElement( GLint index )
{
	crPackArrayElement( index, &(tilesort_spu.ctx->client) );
}

void TILESORTSPU_APIENTRY tilesortspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	if (tilesort_spu.ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawElements called in a Begin/End" );
	}
	crPackDrawElements( mode, count, type, indices, &(tilesort_spu.ctx->client) );
}

void TILESORTSPU_APIENTRY tilesortspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	if (tilesort_spu.ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawArrays called in a Begin/End" );
	}
	crPackDrawArrays( mode, first, count, &(tilesort_spu.ctx->client) );
}
