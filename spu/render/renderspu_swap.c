#include "renderspu.h"

#include "cr_spu.h"
#include "cr_error.h"
#include "cr_string.h"

#include <stdio.h>

#include <GL/gl.h>

void SPU_APIENTRY renderspuSwapBuffers( void )
{
#ifdef WINDOWS
	static int first = 1;
	if (first)
	{
		first = 0;
		crDebug( "Swapping HDC=0x%x", render_spu.device_context );
		crDebug( "The current context is 0x%x", render_spu.wglGetCurrentContext() );
		crDebug( "The context *I* created is 0x%x", render_spu.hRC );
	}
	//render_spu.dispatch->DrawBuffer( GL_FRONT );
	if (!render_spu.wglSwapBuffers( render_spu.device_context ))
	{
		static char buf[512], *temp;
		DWORD err = GetLastError();

		sprintf( buf, "err=%d", err );

		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				(LPTSTR) &temp, 0, NULL );
		if ( temp )
		{
			crStrncpy( buf, temp, sizeof(buf)-1 );
		}

		temp = buf + crStrlen(buf) - 1;
		while ( temp > buf && isspace( *temp ) )
		{
			*temp = '\0';
			temp--;
		}

		crError( "wglSwapBuffers failed: %s!", buf );
	}
#else
#endif
}
