#include "renderspu.h"

#include "cr_spu.h"
#include "cr_error.h"

void SPU_APIENTRY renderspuSwapBuffers( void )
{
#ifdef WINDOWS
	crDebug( "Swapping buffers!  Device_context = %d", render_spu.device_context );
	render_spu.wglSwapBuffers( render_spu.device_context );
#else
#endif
}
