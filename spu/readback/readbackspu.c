/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "readbackspu.h"

ReadbackSPU readback_spu;

void READBACKSPU_APIENTRY rbSwapBuffers( void )
{
	static int first_time = 1;
	static int geometry[4];
	static GLubyte *color_buffer;
	static GLfloat *depth_buffer;

	if (first_time)
	{
		first_time = 0;
		readback_spu.super.GetIntegerv( GL_VIEWPORT, geometry );

		color_buffer = (GLubyte *) crAlloc( geometry[2] * geometry[3] * 3 * sizeof( *color_buffer ) );
		if (readback_spu.extract_depth)
		{
			depth_buffer = (GLfloat *) crAlloc( geometry[2] * geometry[3] * 3 * sizeof( *depth_buffer ) );
		}
		readback_spu.child.BarrierCreate( READBACK_BARRIER, 0 );
	}

	readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3], GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	if (readback_spu.extract_depth)
	{
		readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3], GL_DEPTH_COMPONENT, GL_FLOAT, depth_buffer );
	}

	readback_spu.child.Clear( GL_COLOR_BUFFER_BIT );
	if (readback_spu.extract_depth)
	{
		readback_spu.child.Clear( GL_DEPTH_BUFFER_BIT );
	}

	readback_spu.child.BarrierExec( READBACK_BARRIER );
	if (readback_spu.extract_depth)
	{
		crError( "Depth compositing not yet supported" );
	}
	else
	{
		readback_spu.child.DrawPixels( geometry[2], geometry[3], GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	}
	readback_spu.child.BarrierExec( READBACK_BARRIER );

	readback_spu.child.SwapBuffers();
	if (readback_spu.local_visualization)
	{
		readback_spu.super.SwapBuffers();
	}
}

SPUNamedFunctionTable readback_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) rbSwapBuffers },
	{ NULL, NULL }
};
