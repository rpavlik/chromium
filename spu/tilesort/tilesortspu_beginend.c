#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_glstate.h"

void TILESORTSPU_APIENTRY tilesortspu_Begin( GLenum mode )
{
	CRTransformState *t = &(tilesort_spu.ctx->transform);
	// We have to set this every time because a flush from
	// the state tracker will turn off its flusher.
	
	crStateFlushFunc( tilesortspuFlush );
	crPackBegin( mode );
	crStateBegin( mode );
	if (! t->transformValid)
	{
		// Make sure that the state tracker has the very very
		// latest composite modelview + projection matrix
		// computed, since we're going to need it.
		crStateTransformUpdateTransform( t );
	}
}

void TILESORTSPU_APIENTRY tilesortspu_End( void )
{
	crPackEnd();
	crStateEnd();
}
