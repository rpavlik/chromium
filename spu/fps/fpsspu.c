/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "fpsspu.h"

static void FPSSPU_APIENTRY fpsSwapBuffers( GLint window, GLint flags )
{
	static int frame_counter = 0;
	float elapsed = (float) crTimerTime( fps_spu.timer );
	static float elapsed_base = 0;

	(void) window;

	frame_counter++;
	if (fps_spu.total_frames == 0) {
	    /* Get the first timer time */
	    fps_spu.first_swap_time = elapsed;
	}
	fps_spu.total_frames++;

	if (
	    (fps_spu.report_frames > 0 && frame_counter == fps_spu.report_frames) ||
	    (fps_spu.report_seconds > 0.0 && (elapsed - elapsed_base > fps_spu.report_seconds))
	) {
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
