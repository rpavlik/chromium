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

	crDebug( "IN THE READBACK SWAP 1" );
	if (first_time)
	{
	crDebug( "IN THE READBACK SWAP 2" );
		first_time = 0;
	crDebug( "IN THE READBACK SWAP 3" );
		readback_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
	crDebug( "IN THE READBACK SWAP 4" );

		color_buffer = (GLubyte *) crAlloc( geometry[2] * geometry[3] * 3 * sizeof( *color_buffer ) );
	crDebug( "IN THE READBACK SWAP 5" );
		if (readback_spu.extract_depth)
		{
	crDebug( "IN THE READBACK SWAP 6" );
			depth_buffer = (GLfloat *) crAlloc( geometry[2] * geometry[3] * 3 * sizeof( *depth_buffer ) );
		}
	crDebug( "IN THE READBACK SWAP 7" );
		readback_spu.child.BarrierCreate( READBACK_BARRIER, 0 );
	crDebug( "IN THE READBACK SWAP 8" );
	}
	crDebug( "IN THE READBACK SWAP 9" );

	readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3], GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	if (readback_spu.extract_depth)
	{
		readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3], GL_DEPTH_COMPONENT, GL_FLOAT, depth_buffer );
	}

	crDebug( "IN THE READBACK SWAP" );
	readback_spu.child.Clear( GL_COLOR_BUFFER_BIT );
	if (readback_spu.extract_depth)
	{
		readback_spu.child.Clear( GL_DEPTH_BUFFER_BIT );
	}
	crDebug( "IN THE READBACK SWAP" );

	readback_spu.child.BarrierExec( READBACK_BARRIER );
	if (readback_spu.extract_depth)
	{
		readback_spu.child.Clear( GL_STENCIL_BUFFER_BIT );
		readback_spu.child.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		readback_spu.child.StencilFunc( GL_ALWAYS, 1, 1 );
		readback_spu.child.Enable( GL_DEPTH_TEST );
		readback_spu.child.DepthFunc( GL_LESS );
		readback_spu.child.DrawPixels( geometry[2], geometry[3], GL_DEPTH_COMPONENT, GL_FLOAT, depth_buffer );
		readback_spu.child.StencilFunc( GL_EQUAL, 1, 1 );
		readback_spu.child.Disable( GL_DEPTH_TEST );
	}
	crDebug( "IN THE READBACK SWAP" );
	readback_spu.child.DrawPixels( geometry[2], geometry[3], GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	readback_spu.child.BarrierExec( READBACK_BARRIER );

	readback_spu.child.SwapBuffers();
	if (readback_spu.local_visualization)
	{
		readback_spu.super.SwapBuffers();
	}
	crDebug( "EXITING THE READBACK SWAP" );
}

SPUNamedFunctionTable readback_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) rbSwapBuffers },
	{ NULL, NULL }
};
