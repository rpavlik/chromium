/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "fpsspu.h"
#include "cr_error.h"

static void FPSSPU_APIENTRY fpsSwapBuffers( GLint window, GLint flags )
{
	static int frame_counter = 0;
	float elapsed = (float) crTimerTime( fps_spu.timer );
	static float elapsed_base = 0;

	(void) window;

	frame_counter++;
	if ((frame_counter == 10) || (elapsed - elapsed_base > 1))
	{
		float fps = frame_counter / (elapsed - elapsed_base);
		elapsed_base = elapsed;
		frame_counter = 0;
        if (fps<1)
            crDebug( "SPF: %f", 1.0/fps );
        else
            crDebug( "FPS: %f", fps );
	}

	fps_spu.super.SwapBuffers( window, flags );
}

SPUNamedFunctionTable _cr_fps_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) fpsSwapBuffers },
	{ NULL, NULL }
};
