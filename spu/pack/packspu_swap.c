#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_glstate.h"

void PACKSPU_APIENTRY packspu_SwapBuffers( )
{
	crPackSwapBuffers( );
	packspuFlush( NULL );
}

void PACKSPU_APIENTRY packspu_Flush( )
{
	crPackFlush( );
	packspuFlush( NULL );
}

void PACKSPU_APIENTRY packspu_Finish( )
{
	crPackFinish( );
	packspuFlush( NULL );
}
