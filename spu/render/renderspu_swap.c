/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "renderspu.h"

#include "cr_spu.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_glwrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

float *data;

void ComputeCube( int dice )
{
    int i,j;
    float step = 2.0f/dice;
    float *temp;

    data = (float *) malloc( dice*dice*8*sizeof(float) );
    temp = data;
    for (i = 0 ; i < dice ; i++)
    {
        float x = ((float)i)/((float) dice)* 2 - 1;
        for (j = 0 ; j < dice ; j++)
        {
            float y = ((float)j)/((float) dice)* 2 - 1;

            *(temp++) = x;        *(temp++) = y;
            *(temp++) = x + step; *(temp++) = y;
            *(temp++) = x;        *(temp++) = y + step;
            *(temp++) = x + step; *(temp++) = y + step;
        }
    }
}

void DrawCube( )
{
    float *temp = data;
		float color[4];

    render_spu.self.Color3f( 1.0, 1.0, 0.0 );
		render_spu.self.GetFloatv( GL_CURRENT_COLOR, color );
		if (color[0] != 1.0 && color[1] != 1.0 && color[2] != 0.0)
		{
			crError( "crap" );
		}
    render_spu.self.Begin( GL_TRIANGLE_STRIP );
		render_spu.self.Vertex3f( temp[0], temp[1], 1 );
		render_spu.self.Vertex3f( temp[2], temp[3], 1 );
		render_spu.self.Vertex3f( temp[4], temp[5], 1 );
		render_spu.self.Vertex3f( temp[6], temp[7], 1 );
		render_spu.self.Vertex3f( temp[0], temp[1], -1 );
		render_spu.self.Vertex3f( temp[2], temp[3], -1 );
		render_spu.self.Vertex3f( temp[4], temp[5], -1 );
		render_spu.self.Vertex3f( temp[6], temp[7], -1 );
		render_spu.self.Vertex3f( temp[0], 1, temp[1] );
		render_spu.self.Vertex3f( temp[2], 1, temp[3] );
		render_spu.self.Vertex3f( temp[4], 1, temp[5] );
		render_spu.self.Vertex3f( temp[6], 1, temp[7] );
		render_spu.self.Vertex3f( temp[0], -1, temp[1] );
		render_spu.self.Vertex3f( temp[2], -1, temp[3] );
		render_spu.self.Vertex3f( temp[4], -1, temp[5] );
		render_spu.self.Vertex3f( temp[6], -1, temp[7] );
		render_spu.self.Vertex3f( 1, temp[0], temp[1] );
		render_spu.self.Vertex3f( 1, temp[2], temp[3] );
		render_spu.self.Vertex3f( 1, temp[4], temp[5] );
		render_spu.self.Vertex3f( 1, temp[6], temp[7] );
		render_spu.self.Vertex3f( -1, temp[0], temp[1] );
		render_spu.self.Vertex3f( -1, temp[2], temp[3] );
		render_spu.self.Vertex3f( -1, temp[4], temp[5] );
		render_spu.self.Vertex3f( -1, temp[6], temp[7] );
    render_spu.self.End();
}

int angle = 0;

void CubeDisplay( void )
{
		float color[4];

    render_spu.self.ClearColor( 0.5, 0, 0, 0 );
		render_spu.self.GetFloatv( GL_CURRENT_COLOR, color );
		crWarning ("Color: %f %f %f", color[0], color[1], color[2] );
		render_spu.self.Color3f( 0, 1, 0 );
		render_spu.self.GetFloatv( GL_CURRENT_COLOR, color );
		crWarning ("Color: %f %f %f", color[0], color[1], color[2] );
		render_spu.self.GetFloatv( GL_CURRENT_COLOR, color );
		render_spu.self.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		render_spu.self.Enable( GL_DEPTH_TEST );
		if (!render_spu.self.IsEnabled( GL_DEPTH_TEST ) )
		{
			crError( "huh" );
		}

		render_spu.self.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		render_spu.self.Enable( GL_POLYGON_OFFSET_FILL );
		render_spu.self.PolygonOffset( 1.5f, 0.00001f );
		render_spu.self.PolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    render_spu.self.MatrixMode( GL_PROJECTION );
    render_spu.self.LoadIdentity();
    render_spu.self.Ortho( -10, 10, -10, 10, -10, 10 );

    render_spu.self.MatrixMode( GL_MODELVIEW );
    render_spu.self.LoadIdentity();
    render_spu.self.Rotatef( (float) angle, 0, 0, 1 );
    render_spu.self.Rotatef( (float) angle, 1, 1, 0 );
    render_spu.self.Scalef( 3.0f,3.0f,3.0f );
    DrawCube( );

		render_spu.self.PolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		render_spu.self.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    render_spu.self.MatrixMode( GL_PROJECTION );
    render_spu.self.LoadIdentity();
    render_spu.self.Ortho( -10, 10, -10, 10, -10, 10 );

    render_spu.self.MatrixMode( GL_MODELVIEW );
    render_spu.self.LoadIdentity();
    render_spu.self.Rotatef( (float) angle, 0, 0, 1 );
    render_spu.self.Rotatef( (float) angle, 1, 1, 0 );
    render_spu.self.Scalef( 3.0f,3.0f,3.0f );
    DrawCube( );

		angle++;
}

void SPU_APIENTRY renderspuSwapBuffers( void )
{
#ifdef WINDOWS
	static int first = 1;
	if (first)
	{
		first = 0;
		ComputeCube( 1 );
		crDebug( "Swapping HDC=0x%x", render_spu.device_context );
		crDebug( "The current context is 0x%x", render_spu.wglGetCurrentContext() );
		crDebug( "The context *I* created is 0x%x", render_spu.hRC );
	}
	/* CubeDisplay(); */
	if (!render_spu.wglSwapBuffers( render_spu.device_context ))
	{
		/* GOD DAMN IT.  The latest versions of the NVIDIA drivers
		 * return failure from wglSwapBuffers, but it works just fine.
		 * WHAT THE HELL?! */

		/* crError( "wglSwapBuffers failed!"); */
	}
#else
	render_spu.glXSwapBuffers( render_spu.dpy, render_spu.window );
#endif
}
