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

void RENDER_APIENTRY renderspuCursorPositionCR( GLint x, GLint y )
{
	render_spu.cursorX = x;
	render_spu.cursorY = y;
}


/*
 * Set the current raster position to the given window coordinate.
 */
static void SetRasterPos( GLint winX, GLint winY )
{
	GLfloat fx, fy;

	/* Push current matrix mode and viewport attributes */
	render_spu.self.PushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

	/* Setup projection parameters */
	render_spu.self.MatrixMode( GL_PROJECTION );
	render_spu.self.PushMatrix();
	render_spu.self.LoadIdentity();
	render_spu.self.MatrixMode( GL_MODELVIEW );
	render_spu.self.PushMatrix();
	render_spu.self.LoadIdentity();

	render_spu.self.Viewport( winX - 1, winY - 1, 2, 2 );

	/* set the raster (window) position */
	/* huh ? */
	fx = (GLfloat) (winX - (int) winX);
	fy = (GLfloat) (winY - (int) winY);
	render_spu.self.RasterPos4f( fx, fy, 0.0, 1.0 );

	/* restore matrices, viewport and matrix mode */
	render_spu.self.PopMatrix();
	render_spu.self.MatrixMode( GL_PROJECTION );
	render_spu.self.PopMatrix();

	render_spu.self.PopAttrib();
}


/*
 * Draw the mouse pointer bitmap at (x,y) in window coords.
 */
static void DrawCursor( GLint x, GLint y )
{
#define POINTER_WIDTH   32
#define POINTER_HEIGHT  32
	/* Somebody artistic could probably do better here */
	static const char *pointerImage[POINTER_HEIGHT] =
	{
		"XX..............................",
		"XXXX............................",
		".XXXXX..........................",
		".XXXXXXX........................",
		"..XXXXXXXX......................",
		"..XXXXXXXXXX....................",
		"...XXXXXXXXXXX..................",
		"...XXXXXXXXXXXXX................",
		"....XXXXXXXXXXXXXX..............",
		"....XXXXXXXXXXXXXXXX............",
		".....XXXXXXXXXXXXXXXXX..........",
		".....XXXXXXXXXXXXXXXXXXX........",
		"......XXXXXXXXXXXXXXXXXXXX......",
		"......XXXXXXXXXXXXXXXXXXXXXX....",
		".......XXXXXXXXXXXXXXXXXXXXXXX..",
		".......XXXXXXXXXXXXXXXXXXXXXXXX.",
		"........XXXXXXXXXXXXX...........",
		"........XXXXXXXX.XXXXX..........",
		".........XXXXXX...XXXXX.........",
		".........XXXXX.....XXXXX........",
		"..........XXX.......XXXXX.......",
		"..........XX.........XXXXX......",
		"......................XXXXX.....",
		".......................XXXXX....",
		"........................XXX.....",
		".........................X......",
		"................................",
		"................................",
		"................................",
		"................................",
		"................................",
		"................................"

	};
	static GLubyte pointerBitmap[POINTER_HEIGHT][POINTER_WIDTH / 8];
	static GLboolean firstCall = GL_TRUE;
	GLboolean lighting, depthTest, scissorTest;

	if (firstCall) {
		/* Convert pointerImage into pointerBitmap */
		GLint i, j;
		for (i = 0; i < POINTER_HEIGHT; i++) {
			for (j = 0; j < POINTER_WIDTH; j++) {
				if (pointerImage[POINTER_HEIGHT - i - 1][j] == 'X') {
					GLubyte bit = 128 >> (j & 0x7);
					pointerBitmap[i][j / 8] |= bit;
				}
			}
		}
		firstCall = GL_FALSE;
	}

	render_spu.self.GetBooleanv(GL_LIGHTING, &lighting);
	render_spu.self.GetBooleanv(GL_DEPTH_TEST, &depthTest);
	render_spu.self.GetBooleanv(GL_SCISSOR_TEST, &scissorTest);
	render_spu.self.Disable(GL_LIGHTING);
	render_spu.self.Disable(GL_DEPTH_TEST);
	render_spu.self.Disable(GL_SCISSOR_TEST);
	render_spu.self.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	render_spu.self.Color3f(1, 1, 1);
	SetRasterPos(x, y);
	render_spu.self.Bitmap(POINTER_WIDTH, POINTER_HEIGHT, 1.0, 31.0, 0, 0,
								(const GLubyte *) pointerBitmap);

	if (lighting)
	   render_spu.self.Enable(GL_LIGHTING);
	if (depthTest)
	   render_spu.self.Enable(GL_DEPTH_TEST);
	if (scissorTest)
		render_spu.self.Enable(GL_SCISSOR_TEST);
}

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

void DrawCube( void )
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

void RENDER_APIENTRY renderspuSwapBuffers( void )
{
#ifdef WINDOWS
	static int first = 1;
	int return_value;
#endif

	if (render_spu.drawCursor)
		DrawCursor( render_spu.cursorX, render_spu.cursorY );

#ifdef WINDOWS
	return_value=render_spu.ws.wglSwapBuffers( render_spu.device_context );
	if (!return_value)
	{
		/* GOD DAMN IT.  The latest versions of the NVIDIA drivers
		 * return failure from wglSwapBuffers, but it works just fine.
		 * WHAT THE HELL?! */

		crWarning( "wglSwapBuffers failed: return value of %d!", return_value);
	}
	first = 0;
#else
	render_spu.ws.glXSwapBuffers( render_spu.dpy, render_spu.window );
#endif
}
