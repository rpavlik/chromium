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
	GLfloat rastpos[2];
	
	if (first_time)
	{
		GLint zBits;

		first_time = 0;

		if ((readback_spu.server) && (readback_spu.server->numExtents))
		{
			/* no sence in reading the whole window if the tile 
			 * only convers part of it.. */
			geometry[2] = readback_spu.server->x2[0] - readback_spu.server->x1[0];
			geometry[3] = readback_spu.server->y2[0] - readback_spu.server->y1[0];
		}
		else
		{
			/* if the server is null, we are running on the 
			 * app node, not a network node, so just readout
			 * the whole shebang. if we dont have tiles, we're 
			 * likely not doing sort-first., so do it all 
			 *
			 * also, as i dont have the super nvidia drivers,
			 * im not sure if this works correctly with readback
			 * spus that are on the application node.....
			 * */
			readback_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
		}										 

		/* Determine best type for the depth buffer image */
		readback_spu.super.GetIntegerv( GL_DEPTH_BITS, &zBits );
		if (zBits <= 16)
			readback_spu.depthType = GL_UNSIGNED_SHORT;
		else
			readback_spu.depthType = GL_FLOAT;

		color_buffer = (GLubyte *) crAlloc( geometry[2] * geometry[3]
																				* 3 * sizeof( *color_buffer ) );
		readback_spu.child.BarrierCreate( READBACK_BARRIER, 0 );
		if (readback_spu.extract_depth)
		{
			depth_buffer = (GLfloat *) crAlloc( geometry[2] * geometry[3]
																					* 3 * sizeof( *depth_buffer ) );
		}
	}

	readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3],
																 GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	if (readback_spu.extract_depth)
	{
		readback_spu.super.ReadPixels( 0, 0, geometry[2], geometry[3],
																	 GL_DEPTH_COMPONENT, readback_spu.depthType,
																	 depth_buffer );
	}


	if (readback_spu.extract_depth)
	{
		readback_spu.child.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
															| GL_SINGLE_CLIENT_BIT_CR );
	}
	else {
		readback_spu.child.Clear( GL_COLOR_BUFFER_BIT | GL_SINGLE_CLIENT_BIT_CR );
	}

	readback_spu.child.BarrierExec( READBACK_BARRIER );

	/* Move raster pos to (readback_spu.drawX, readback_spu.drawY) */
	/*
	readback_spu.child.Bitmap( 0, 0, 0.0, 0.0,
														 (GLfloat) readback_spu.drawX, (GLfloat) readback_spu.drawY, NULL );
*/
	/* presumable if we're not a tile, we can start drawing back
	 * at (0, 0), or wherever the raster happens to be */
	if ((readback_spu.server) && (readback_spu.server->numExtents) &&
				(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
	{		
		rastpos[0] = (float)2.0*((float)readback_spu.server->x1[0] /
				(float)readback_spu.server->muralWidth)-(float)1.0;
		rastpos[1] = (float)2.0*((float)readback_spu.server->y1[0] / 
				(float)readback_spu.server->muralHeight)-(float)1.0;
		readback_spu.child.RasterPos2f(rastpos[0], rastpos[1]);
	}
	
	if (readback_spu.extract_depth)
	{
		/* Draw the depth image into the depth buffer, setting the stencil
		 * to one wherever we pass the Z test.
		 */
		readback_spu.child.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		readback_spu.child.StencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
		readback_spu.child.StencilFunc( GL_ALWAYS, 1, ~0 );
		readback_spu.child.Enable( GL_STENCIL_TEST );
		readback_spu.child.Enable( GL_DEPTH_TEST );
		readback_spu.child.DepthFunc( GL_LESS );
		readback_spu.child.Clear( GL_STENCIL_BUFFER_BIT );
		readback_spu.child.DrawPixels( geometry[2], geometry[3],
																	 GL_DEPTH_COMPONENT, readback_spu.depthType,
																	 depth_buffer );
		if ((readback_spu.server) && (readback_spu.server->numExtents) &&
					(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
		{		
			rastpos[0] = (float)2.0*((float)(readback_spu.server->x1[0]) /
					(float)readback_spu.server->muralWidth)-(float)1.0;
			rastpos[1] = (float)2.0*((float)(readback_spu.server->y1[0]) / 
					(float)readback_spu.server->muralHeight)-(float)1.0;
			readback_spu.child.RasterPos2f(rastpos[0], rastpos[1]);
		}

		/* Now draw the RGBA image, only where the stencil is one */
		readback_spu.child.Disable( GL_DEPTH_TEST );
		readback_spu.child.StencilFunc( GL_EQUAL, 1, ~0 );
		readback_spu.child.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		if (readback_spu.visualize_depth) {

			readback_spu.child.PixelTransferf(GL_RED_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_GREEN_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_BLUE_BIAS, 1.0);
			readback_spu.child.PixelTransferf(GL_RED_SCALE, -1.0);
			readback_spu.child.PixelTransferf(GL_GREEN_SCALE, -1.0);
			readback_spu.child.PixelTransferf(GL_BLUE_SCALE, -1.0);
			readback_spu.child.DrawPixels( geometry[2], geometry[3],
																		 GL_LUMINANCE, readback_spu.depthType,
																		 depth_buffer );
		}
		else {
			/* the usual case */
			readback_spu.child.DrawPixels( geometry[2], geometry[3],
																		 GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
		}
		readback_spu.child.Disable(GL_STENCIL_TEST);
	}

	else
	{
		readback_spu.child.DrawPixels( geometry[2], geometry[3],
																	 GL_RGB, GL_UNSIGNED_BYTE, color_buffer );
	}

	/* Move raster pos back to (0, 0) */
	/*
	readback_spu.child.Bitmap( 0, 0, 0.0, 0.0,
														 (GLfloat) -readback_spu.drawX, (GLfloat) -readback_spu.drawY, NULL );
	*/
		
	if ((readback_spu.server) && (readback_spu.server->numExtents) &&
				(readback_spu.server->x1[0]+readback_spu.server->y1[0]))
	{		
		readback_spu.child.RasterPos2f(-1., -1.);
	}


	/*
	readback_spu.child.Finish();
	*/
	readback_spu.child.BarrierExec( READBACK_BARRIER );

	readback_spu.child.SwapBuffers();

	if (readback_spu.local_visualization)
	{
		readback_spu.super.SwapBuffers();
	}
}


static GLint READBACKSPU_APIENTRY readbackspuCreateContext( void *dpy, GLint visual)
{
	GLint retVal;
	readback_spu.child.BarrierCreate( CREATE_CONTEXT_BARRIER, 0 );
	retVal = readback_spu.super.CreateContext(dpy, visual);
	if (retVal) {
		/* If doing z-compositing, need stencil buffer */
		if (readback_spu.extract_depth)
			visual |= CR_STENCIL_BIT;
		retVal = readback_spu.child.CreateContext(dpy, visual);
	}
	readback_spu.child.BarrierExec( CREATE_CONTEXT_BARRIER );
	return retVal;
}


static void READBACKSPU_APIENTRY readbackspuDestroyContext( void *dpy, GLint ctx )
{
	readback_spu.child.BarrierCreate( DESTROY_CONTEXT_BARRIER, 0 );
	readback_spu.super.DestroyContext(dpy, ctx);
	readback_spu.child.DestroyContext(dpy, ctx);
	readback_spu.child.BarrierExec( DESTROY_CONTEXT_BARRIER );
}


static void READBACKSPU_APIENTRY readbackspuMakeCurrent(void *dpy, GLint drawable, GLint ctx)
{
	readback_spu.child.BarrierCreate( MAKE_CURRENT_BARRIER, 0 );
	readback_spu.super.MakeCurrent(dpy, drawable, ctx);
	readback_spu.child.MakeCurrent(dpy, drawable, ctx);
	readback_spu.child.BarrierExec( MAKE_CURRENT_BARRIER );
}


SPUNamedFunctionTable readback_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) rbSwapBuffers },
	{ "CreateContext", (SPUGenericFunction) readbackspuCreateContext },
	{ "DestroyContext", (SPUGenericFunction) readbackspuDestroyContext },
	{ "MakeCurrent", (SPUGenericFunction) readbackspuMakeCurrent },
	{ NULL, NULL }
};
