#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_ArrayElement( GLint index )
{
	crPackArrayElement( index, tilesort_spu.ctx );
}

void TILESORTSPU_APIENTRY tilesortspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	crPackDrawElements( mode, count, type, indices, tilesort_spu.ctx );
}

void TILESORTSPU_APIENTRY tilesortspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	crPackDrawArrays( mode, first, count, tilesort_spu.ctx );
}
