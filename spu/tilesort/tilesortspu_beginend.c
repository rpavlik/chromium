#include "tilesortspu.h"
#include "cr_error.h"

void TILESORTSPU_APIENTRY tilesortspu_Begin( GLenum mode )
{
	// We have to set this every time because a flush from
	// the state tracker will turn off its flusher.
	
	crStateFlushFunc( tilesortspuFlush );
	crStateBegin( mode );
}

void TILESORTSPU_APIENTRY tilesortspu_End( void )
{
	crStateEnd();
}
