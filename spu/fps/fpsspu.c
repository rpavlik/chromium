/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "fpsspu.h"
#include "cr_error.h"

void FPSSPU_APIENTRY fpsSwapBuffers( void )
{
	static int frame_counter = 0;
	float elapsed = (float) crTimerTime( fps_spu.timer );
	static float elapsed_base = 0;

	frame_counter++;
	if (frame_counter == 10)
	{
		float fps = frame_counter / (elapsed - elapsed_base);
		elapsed_base = elapsed;
		frame_counter = 0;
		crDebug( "FPS: %f", fps );
	}

	fps_spu.super.SwapBuffers();
}

SPUNamedFunctionTable fps_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) fpsSwapBuffers },
	{ NULL, NULL }
};
